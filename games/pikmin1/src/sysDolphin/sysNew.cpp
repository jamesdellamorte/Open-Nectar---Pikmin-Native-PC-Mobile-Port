#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX // libstdc++ on MinGW already defines it
#define NOMINMAX
#endif
#include <windows.h>
#undef ERROR
#undef near
#undef far
#undef small
#endif
#include "sysNew.h"

#include "DebugLog.h"
#include "MemStat.h"
#include <cstdint>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <mutex>
#include <new>
#ifndef _WIN32
#include <sys/mman.h>
#endif

/**
 * @todo: Documentation
 * @note UNUSED Size: 00009C
 */
DEFINE_ERROR(4)

/**
 * @todo: Documentation
 * @note UNUSED Size: 0000F0
 */
DEFINE_PRINT("sysNew");

#if defined(PIKI_PC_PORT)
// The global operator new/delete are interposed process-wide: dylibs that
// import them (AudioToolbox's caulk, for one) pair our operator new with the
// system's free() and vice versa. That only stays consistent when the pointer
// handed out is the C allocator's own pointer: an intrusive header (returning
// malloc()+16) made those cross-allocator frees corrupt the zone freelist.
// So the override is calloc/free, which is interchangeable with malloc/free/
// operator new/operator delete in every pairing, while still honouring the
// console contract that fresh memory is zeroed. macOS calloc of a big block
// is an untouched mmap: pages stay non-resident until written.
//
// Why everything goes to the C heap: the game's AyuHeap arenas are reset by
// moving a cursor, which requires every allocation between resets to die at
// once. Native secondary threads (DVD worker, SDL audio, card worker) would
// allocate into whatever heap the main thread has active, and a concurrent
// reset would corrupt them. Until allocation carries per-thread heap
// context, routing global new into arenas is unsafe; game systems that need
// arena lifetimes go through System::alloc explicitly.
#if defined(__APPLE__)
#include <malloc/malloc.h>
static size_t block_size(const void* p) { return malloc_size(p); }
#elif defined(__GLIBC__)
#include <malloc.h>
static size_t block_size(const void* p) { return malloc_usable_size(const_cast<void*>(p)); }
#else
static size_t block_size(const void*) { return 0; }
#endif

// Constructed on first use and never destroyed. A static std::mutex is torn
// down at exit, but static destructors in other translation units still free
// through piki_pc_free after that (the DVD map in dvd_stubs.cpp) and lock it.
// The first allocation can come from another file's static initialiser, so a
// namespace-scope object is not safe either.
static std::mutex& allocMutex()
{
	alignas(std::mutex) static unsigned char storage[sizeof(std::mutex)];
	static std::mutex& mutex = *new (storage) std::mutex;
	return mutex;
}

static size_t sLiveAllocations = 0;
static size_t sLiveBytes = 0;
static size_t sPeakAllocations = 0;
static size_t sPeakBytes = 0;
static size_t sTotalAllocations = 0;
static size_t sTotalFrees = 0;
static bool sDumpRegistered = false;
// Where the bytes are, by block size. 810 MB across twelve thousand blocks is
// not a game object leak; it is a small number of very large ones, and the
// totals alone cannot say which.
static const size_t SIZE_CLASS_COUNT = 8;
static size_t sClassBytes[SIZE_CLASS_COUNT] = {};
static size_t sClassCount[SIZE_CLASS_COUNT] = {};
static size_t sLargestBlock = 0;

static size_t sizeClassOf(size_t bytes)
{
    if (bytes < 1024) return 0;
    if (bytes < 16 * 1024) return 1;
    if (bytes < 64 * 1024) return 2;
    if (bytes < 256 * 1024) return 3;
    if (bytes < 1024 * 1024) return 4;
    if (bytes < 8 * 1024 * 1024) return 5;
    if (bytes < 64 * 1024 * 1024) return 6;
    return 7;
}

void piki_pc_dump_alloc_stats(void)
{
	std::lock_guard<std::mutex> lock(allocMutex());
	static const char* kClassNames[SIZE_CLASS_COUNT] = {
		"<1K", "1K-16K", "16K-64K", "64K-256K", "256K-1M", "1M-8M", "8M-64M", ">64M"
	};
	fprintf(stderr, "[PC Alloc] largest single block %zu bytes\n", sLargestBlock);
	for (size_t i = 0; i < SIZE_CLASS_COUNT; ++i) {
		if (sClassCount[i] == 0) continue;
		fprintf(stderr, "[PC Alloc]   %-8s %zu live, %zu MB\n",
		        kClassNames[i], sClassCount[i], sClassBytes[i] >> 20);
	}
	fprintf(stderr,
	        "[PC Alloc] live=%zu bytes=%zu peak=%zu/%zu total=%zu frees=%zu\n",
	        sLiveAllocations, sLiveBytes, sPeakAllocations, sPeakBytes,
	        sTotalAllocations, sTotalFrees);
}

void* piki_pc_alloc(size_t size)
{
	if (size == 0) {
		size = 1;
	}
	if (size & 0x3) {
		if (size > std::numeric_limits<size_t>::max() - 3) {
			throw std::bad_alloc();
		}
		size = (size + 3) & ~static_cast<size_t>(0x3);
	}

	// Zeroed memory is part of the contract (the console heaps zeroed every
	// block); calloc keeps it without touching big blocks' pages.
	void* result = std::calloc(1, size);
	if (!result) {
		ERROR("allocation of %zu bytes failed", size);
		throw std::bad_alloc();
	}
	{
		std::lock_guard<std::mutex> lock(allocMutex());
		sLiveAllocations++;
		sLiveBytes += size;
		{
			const size_t cls = sizeClassOf(size);
			sClassBytes[cls] += size;
			sClassCount[cls]++;
			if (size > sLargestBlock) sLargestBlock = size;
			// Fase 7: name the big ones as they happen so the log around them
			// says what was loading. Reported once per size class step.
			// getenv here, not once: the first big blocks predate env.txt.
			if (size >= (4u << 20) && getenv("PIKMIN_ALLOC_TRACE")) {
				fprintf(stderr, "[PC Alloc] big block %zu MB (live now %zu MB in %zu blocks)\n",
				        size >> 20, (sLiveBytes) >> 20, sLiveAllocations);
			}
		}
		sTotalAllocations++;
		if (sLiveAllocations > sPeakAllocations) sPeakAllocations = sLiveAllocations;
		if (sLiveBytes > sPeakBytes) sPeakBytes = sLiveBytes;
		if (!sDumpRegistered) {
			std::atexit(piki_pc_dump_alloc_stats);
			sDumpRegistered = true;
		}
	}
	return result;
}

void piki_pc_free(void* ptr)
{
	if (!ptr) {
		return;
	}

	const size_t size = block_size(ptr);
	{
		std::lock_guard<std::mutex> lock(allocMutex());
		sLiveAllocations--;
		sLiveBytes -= size;
		const size_t cls = sizeClassOf(size);
		if (sClassCount[cls] > 0) {
			sClassCount[cls]--;
			sClassBytes[cls] = sClassBytes[cls] > size ? sClassBytes[cls] - size : 0;
		}
		sTotalFrees++;
	}
	std::free(ptr);
}

void* operator new(size_t size)
{
	return piki_pc_alloc(size);
}

void* operator new[](size_t size)
{
	return piki_pc_alloc(size);
}

void operator delete(void* ptr) noexcept
{
	piki_pc_free(ptr);
}

void operator delete[](void* ptr) noexcept
{
	piki_pc_free(ptr);
}

void operator delete(void* ptr, size_t) noexcept
{
	piki_pc_free(ptr);
}

void operator delete[](void* ptr, size_t) noexcept
{
	piki_pc_free(ptr);
}
#endif

/**
 * @todo: Documentation
 */
void* System::alloc(size_t size)
{
	void* result = nullptr;
	System* system = gsys;
	if (!system) {
		fprintf(stderr, "[PC Port Fatal] System::alloc(%zu) called without an active System instance\n", size);
		abort();
	}
	if (size & 0x3) {
		size = (size + 3) & ~0x3;
	}

	if (system->mActiveHeapIdx >= 0) {
		AyuHeap* heap = &system->mHeaps[system->mActiveHeapIdx];
		if (size == 0) {
			PRINT("trying to allocate %d bytes on heap\n", 0);
		}
		result = heap->push(size);
		if (!result) {
			ERROR("new[] %d failed in heap '%s'", size, heap->mName);
		}

		if (size == 0 || system->mForcePrint) {
			bool print             = system->mTogglePrint;
			system->mTogglePrint = TRUE;
			system->mTogglePrint = print;
		}

		MemInfo* info = system->mCurrMemInfo;
		while (info) {
			info->mMemorySize += size;
			info = static_cast<MemInfo*>(info->mParent);
		}

		if ((u32)(uintptr_t)result & 0x3) {
			ERROR("acquired memory not long aligned %08x!!\n", (u32)(uintptr_t)result);
		}

		u32* resPtr = (u32*)result;
		int length  = size / 4;
		for (int i = 0; i < length; i++) {
			resPtr[i] = 0;
		}
	} else {
#if defined(WIN32)
		// The DLL uses `GlobalAlloc` here and has an ERROR if that fails.  This branch of code is probably DLL exclusive,
		// since the GCN can't just ask WinAPI for unlimited memory.  We'll see once JPN Demo version matching begins.
		if (!result) {
			ERROR("new[] %d failed", size);
		}
#else
		ERROR("no heap specified\n");
#endif
	}

	return result;
}

/**
 * @todo: Documentation
 * @note UNUSED Size: 000044 (Matching by size)
 */
#if defined(PIKI_PC_PORT)
void* operator new(size_t size, PikiAlignment requestedAlignment)
#else
void* operator new(size_t size, int requestedAlignment)
#endif
{
	int alignment =
#if defined(PIKI_PC_PORT)
	    requestedAlignment.value;
#else
	    requestedAlignment;
#endif
	uintptr_t alloc  = reinterpret_cast<uintptr_t>(System::alloc(size + alignment));
	uintptr_t result = (alloc + (alignment - 1)) & ~static_cast<uintptr_t>(alignment - 1);
	return (void*)result;
}

/**
 * @todo: Documentation
 */
#if defined(PIKI_PC_PORT)
void* operator new[](size_t size, PikiAlignment requestedAlignment)
#else
void* operator new[](size_t size, int requestedAlignment)
#endif
{
	int alignment =
#if defined(PIKI_PC_PORT)
	    requestedAlignment.value;
#else
	    requestedAlignment;
#endif
	uintptr_t alloc  = reinterpret_cast<uintptr_t>(System::alloc(size + alignment));
	uintptr_t result = (alloc + (alignment - 1)) & ~static_cast<uintptr_t>(alignment - 1);
	return (void*)result;
}

/**
 * @todo: Documentation
 */
void dummy_delete(void*)
{
}

/**
 * @todo: Documentation
 */
void dummy_delete_arr(void*)
{
}
