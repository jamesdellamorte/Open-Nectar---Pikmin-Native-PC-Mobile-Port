#include "pc_aurora.h"
#include <aurora/aurora.h>
#include <aurora/gfx.h>
#include <SDL3/SDL.h>
#include <cstdio>

namespace {
struct pc_aurora_state {
    bool initialized = false;
    bool frameOpen = false;
    AuroraInfo info{};
};
pc_aurora_state g;

void logAurora(AuroraLogLevel level, const char* module, const char* message, unsigned length) {
    std::fprintf(stderr, "[Aurora/%s] %.*s\n", module, static_cast<int>(length), message);
}
}

bool pc_aurora_init(int argc, char* argv[], const char* title, int width, int height) {
    if (g.initialized) return true;

    AuroraConfig config{};
    config.appName = title ? title : "Open Nectar";
    config.desiredBackend = BACKEND_METAL;
    config.surface.type = AURORA_SURFACE_SDL_MANAGED;
    config.windowWidth = width;
    config.windowHeight = height;
    config.windowPosX = -1;
    config.windowPosY = -1;
    config.msaa = 1;
    config.vsync = true;
    config.emulateXfb = true;
    config.pauseOnFocusLost = false;
    config.gxProcessingMode = GX_PROCESSING_INLINE;
    config.mem1Size = MEM1_DEFAULT_SIZE;
    config.logCallback = logAurora;
    config.logLevel = LOG_INFO;

    g.info = aurora_initialize(argc, argv, &config);
    g.initialized = true;

    if (g.info.window == nullptr) {
        std::fprintf(stderr, "[Aurora] Could not create a window\n");
        pc_aurora_shutdown();
        return false;
    }

    const char* backendName = "?";
    switch (g.info.backend) {
    case BACKEND_METAL: backendName = "Metal"; break;
    case BACKEND_D3D12: backendName = "D3D12"; break;
    case BACKEND_VULKAN: backendName = "Vulkan"; break;
    case BACKEND_OPENGL: backendName = "OpenGL"; break;
    case BACKEND_OPENGLES: backendName = "OpenGL ES"; break;
    case BACKEND_WEBGPU: backendName = "WebGPU"; break;
    case BACKEND_NULL: backendName = "Null"; break;
    default: break;
    }
    std::fprintf(stderr, "[Aurora] Initialized backend=%s (requested METAL), window=%p\n",
                 backendName, (void*)g.info.window);

    g.frameOpen = aurora_begin_frame();
    if (!g.frameOpen) {
        std::fprintf(stderr, "[Aurora] Could not begin first frame\n");
        pc_aurora_shutdown();
        return false;
    }
    return true;
}

SDL_Window* pc_aurora_get_window() {
    return g.info.window;
}

void pc_aurora_shutdown() {
    if (g.initialized) {
        if (g.frameOpen) aurora_end_frame();
        g.frameOpen = false;
        aurora_shutdown();
        g.initialized = false;
        g.info = {};
    }
}

void pc_aurora_resize() {
    if (!g.initialized) return;
    const AuroraWindowSize size = aurora_get_window_size();
    if (size.native_fb_width && size.native_fb_height)
        aurora_set_surface_size(size.native_fb_width, size.native_fb_height, size.scale);
}

bool pc_aurora_present() {
    if (!g.frameOpen) return false;
    aurora_end_frame();
    g.frameOpen = aurora_begin_frame();
    return g.frameOpen;
}

const AuroraEvent* pc_aurora_update() {
    return g.initialized ? aurora_update() : nullptr;
}

void pc_aurora_enable_vsync(bool enabled) {
    if (g.initialized) aurora_enable_vsync(enabled);
}

void pc_aurora_get_window_size(unsigned* width, unsigned* height) {
    const AuroraWindowSize size = aurora_get_window_size();
    if (width) *width = size.native_fb_width;
    if (height) *height = size.native_fb_height;
}
