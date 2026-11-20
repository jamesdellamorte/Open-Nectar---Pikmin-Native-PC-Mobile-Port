#include "pc_icon.h"

#include <SDL3/SDL.h>

#include "pc_icon_data.h"

void pc_icon_apply(SDL_Window* window)
{
	if (!window) return;
	SDL_Surface* icon = SDL_CreateSurfaceFrom(kPcIconW, kPcIconH, SDL_PIXELFORMAT_RGBA32, (void*)kPcIconRgba, kPcIconW * 4);
	if (!icon) return;
	SDL_SetWindowIcon(window, icon);
	SDL_DestroySurface(icon);
}
