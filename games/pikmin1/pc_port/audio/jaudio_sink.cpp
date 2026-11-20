// SDL3 stream adapter for the NextOS JAudio host renderer. Native S16 stereo PCM.
#include "port/audio_sink.h"
#include <SDL3/SDL.h>
#include <limits>
#include <cstdio>
#include <vector>

extern "C" bool pc_audio_output_muted(void); // pc_audio.cpp (pc_audio.h)

namespace {
SDL_AudioStream* stream = nullptr;
SDL_AudioSpec spec {};
uint64_t submitted = 0;
}
extern "C" {
void PikiAudioSinkClose()
{
    if (stream) { SDL_DestroyAudioStream(stream); stream = nullptr; }
    submitted = 0;
}
void PikiAudioSinkShutdown()
{
    PikiAudioSinkClose();
    if (SDL_WasInit(SDL_INIT_AUDIO)) SDL_QuitSubSystem(SDL_INIT_AUDIO);
}
int PikiAudioSinkTryOpen(int rate)
{
    PikiAudioSinkClose();
    if (rate <= 0) return 0;
    if (!SDL_WasInit(SDL_INIT_AUDIO)) {
        if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) return 0;
    }
    spec.freq = rate;
    spec.format = SDL_AUDIO_S16;
    spec.channels = 2;
    stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, nullptr, nullptr);
    if (!stream) { PikiAudioSinkClose(); return 0; }
    std::fprintf(stderr, "[jaudio] NextOS DSP, SDL3 S16 stereo %d Hz\n", rate);
    return 1;
}
int PikiAudioSinkOpen(int rate) { return PikiAudioSinkTryOpen(rate); }
int PikiAudioSinkQueue(const int16_t* pcm, size_t frames)
{
    if (!stream || !pcm || !frames || frames > std::numeric_limits<Uint32>::max() / 4) return 0;
    if (pc_audio_output_muted()) {
        static std::vector<int16_t> silence;
        silence.assign(frames * 2, 0);
        pcm = silence.data();
    }
    if (!SDL_PutAudioStreamData(stream, pcm, static_cast<Uint32>(frames * 4))) return 0;
    submitted += frames;
    return 1;
}
int PikiAudioSinkResume() { if (!stream) return 0; SDL_ResumeAudioStreamDevice(stream); return 1; }
void PikiAudioSinkPause() { if (stream) SDL_PauseAudioStreamDevice(stream); }
int PikiAudioSinkFlush() { return stream != 0; }
uint64_t PikiAudioSinkSubmittedFrames() { return submitted; }
uint64_t PikiAudioSinkQueuedFrames() { return stream ? SDL_GetAudioStreamQueued(stream) / 4 : 0; }
uint64_t PikiAudioSinkPlayedFrames() { const auto q = PikiAudioSinkQueuedFrames(); return q < submitted ? submitted - q : 0; }
int PikiAudioSinkDrained() { return PikiAudioSinkQueuedFrames() == 0; }
int PikiAudioSinkGetProgress(uint64_t* total, uint64_t* queued)
{
    if (!stream) return 0;
    if (total) *total = submitted;
    if (queued) *queued = PikiAudioSinkQueuedFrames();
    return 1;
}
}
