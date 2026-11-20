/**
 * @file vi_stubs.cpp
 * @brief Stub implementations for Dolphin VI (Video Interface) functions.
 *
 * In Stage 3, VIWaitForRetrace will be replaced with SDL2 vsync/frame limiting.
 */
#include "Dolphin/vi.h"
#include "Dolphin/gx.h"
#include "pc_window.h"
#include "pc_keycaps.h"
#include <cstdio>
#include <cstring>

#if defined(PIKI_PC_PORT) && defined(PIKI_PC_SETTINGS_MENU)
#include "settings/pc_settings.h"
#include "settings/pc_glass_menu.h"
#include "pc_speedrun.h"
#include "pc_record.h"
#if PIKI_PC_TOUCH
#include "touch/pc_touch.h"
#endif
#endif

#if defined(PIKI_PC_PORT)
// Netplay session HUD: strong-defined by pc_netplay_hud.cpp in netplay builds
// only, null otherwise.
void pc_netplay_hud_draw(void);
// Title local/online choice: pc_online_menu.cpp, online netplay builds only.
void pc_online_menu_draw(void);
#endif

static u32 sRetraceCount = 0;

static VIRetraceCallback sPostRetraceCallback = nullptr;

extern "C" {

void __VIInit(VITVMode mode)   { (void)mode; }
/* VIInit, VIFlush, VIConfigure, VIConfigurePan and VIGetTvFormat are provided
 * by Aurora's aurora_vi, which models the timing the GX pipeline expects. */
void pc_vi_present_frame(void* displayBuffer, bool clear) {
    sRetraceCount++;
    if (sPostRetraceCallback) {
        sPostRetraceCallback(sRetraceCount);
    }
#if defined(PIKI_PC_PORT) && defined(PIKI_PC_SETTINGS_MENU)
    // Draw the settings overlay through the game GX stack before the native
    // framebuffer is blitted to the window, so it appears on top.
    // Ctrl+1 (atajo de grabación) también quita los contadores del port.
    if (!pc_record_hud_hidden()) {
        pc_settings_draw_idle_counter();
        pc_settings_draw_vs_hud();
        pc_settings_draw_mission_hud();
    }
    pc_speedrun_draw();
    pc_netplay_hud_draw();
    pc_settings_draw_achievement_toast();
    pc_settings_draw_quick_load(); // Quick Save: the load screen, or the last result
    pc_settings_draw_repair_note();
    pc_settings_draw();
    pc_glass_menu_draw();
    pc_online_menu_draw();
#endif
    pc_keycaps_draw(); // teclas en lugar de los botones de GameCube (teclado y ratón)
#if PIKI_PC_TOUCH
    pc_touch_draw();
#endif
    if (displayBuffer) {
        // Aurora presents the XFB at end_frame. Copy this frame after its overlays.
        GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
        GXCopyDisp(displayBuffer, clear);
    }
    pc_window_swap_buffers();
    pc_window_poll_events(nullptr);
}

void VIWaitForRetrace(void) {
    pc_vi_present_frame(nullptr, false);
}

VIRetraceCallback VISetPreRetraceCallback(VIRetraceCallback callback) {
    (void)callback; return nullptr;
}
VIRetraceCallback VISetPostRetraceCallback(VIRetraceCallback callback) {
    VIRetraceCallback old = sPostRetraceCallback;
    sPostRetraceCallback = callback;
    return old;
}

void  VISetNextFrameBuffer(void* fb) { (void)fb; }
void* VIGetCurrentFrameBuffer()      { return nullptr; }
void  __VIGetCurrentPosition(s16* x, s16* y) { *x = 0; *y = 0; }
void  VISetBlack(BOOL isBlack)       { (void)isBlack; }
u32   VIGetRetraceCount(void)        { return sRetraceCount; }
u32   VIGetNextField(void)           { return 0; }
u32   VIGetCurrentLine(void)         { return 0; }
u32   VIGetDTVStatus(void)           { return 0; }

void* VIGetNextFrameBuffer()         { return nullptr; }
void  VISetNextRightFrameBuffer(void* fb) { (void)fb; }
void  VISet3D()                      { }

} // extern "C"
