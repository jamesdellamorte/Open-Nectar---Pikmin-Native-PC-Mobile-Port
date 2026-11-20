#pragma once

struct SDL_Window;
struct AuroraEvent;

// Initialize Aurora with a managed SDL3 window. Returns true on success.
// On Mac this uses AURORA_SURFACE_SDL_MANAGED + BACKEND_METAL.
bool pc_aurora_init(int argc, char* argv[], const char* title, int width, int height);

// Return the SDL_Window* owned by Aurora (borrowed, do not free).
SDL_Window* pc_aurora_get_window();

void pc_aurora_shutdown();
void pc_aurora_resize();
bool pc_aurora_present();
const AuroraEvent* pc_aurora_update();
void pc_aurora_enable_vsync(bool enabled);
void pc_aurora_get_window_size(unsigned* width, unsigned* height);
