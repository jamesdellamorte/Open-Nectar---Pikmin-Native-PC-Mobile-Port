/**
 * @file gx_stubs.cpp
 * @brief Aurora is the GX implementation: every GX* function the game calls
 * (GXInit, GXBegin, GXSetTevOp, GXSetArray, GXEnd, ...) is exported with the
 * same extern "C" name by <dolphin/gx.h> and defined in aurora_gx. No adapter
 * layer is needed here; redefining them would only shadow Aurora's versions.
 *
 * What remains are the no-op replacements for the removed OpenGL renderer
 * (pc_port/gl/pc_gfx.cpp) and texture pack (pc_port/gl/pc_texpack.cpp), which
 * the settings menu, keycaps and touch overlay still reference.
 */
#include "aurora/pc_aurora.h"
#include "gl/pc_gfx.h"
#include "gl/pc_texpack.h"
#include <cstdio>
#include <string>
#include <vector>

extern "C" {

/* ── Settings the Aurora renderer does not expose yet ── */
void pc_gfx_set_render_scale(float scale) { (void)scale; }
void pc_gfx_set_aspect_ratio_mode(int mode) { (void)mode; }
void pc_gfx_set_per_pixel_lighting(int enabled) { (void)enabled; }
void pc_gfx_set_hud_virtual_size(int w, int h) { (void)w; (void)h; }
void pc_gfx_set_ui_43_no_bars(int enabled) { (void)enabled; }
void pc_gfx_blur_gx_rect(int gxX, int gxY, int gxW, int gxH, int passes) {
    (void)gxX; (void)gxY; (void)gxW; (void)gxH; (void)passes;
}
void pc_gfx_set_hud_wide(int enabled) { (void)enabled; }
int  pc_gfx_menu_wide(void) { return 0; }
int  pc_gfx_menu_virt_width(void) { return 640; }
void pc_gfx_set_menu_clip_43(int enabled) { (void)enabled; }
void pc_gfx_set_dump_texture_names(int enabled) { (void)enabled; }
void pc_gfx_set_fog_allowed(int allowed) { (void)allowed; }
void pc_gfx_set_anisotropy(int samples) { (void)samples; }
void pc_gfx_note_menu_tap_space(int graphWidth, int graphHeight) {
    (void)graphWidth; (void)graphHeight;
}
bool pc_gfx_menu_graph_point(bool click, float* x, float* y) {
    (void)click; (void)x; (void)y;
    return false;
}
void pc_gfx_title_debug_probe(const char* tag) { (void)tag; }

} /* extern "C" */

/* C++ linkage: declared outside the extern "C" block in pc_gfx.h. */
void pc_gfx_set_post_effects(const PcPostEffects& effects) { (void)effects; }

unsigned pc_gfx_overlay_texture_create(int width, int height, const unsigned char* rgba) {
    (void)width; (void)height; (void)rgba;
    return 0;
}
void pc_gfx_overlay_texture_destroy(unsigned texture) { (void)texture; }
void pc_gfx_overlay_begin(void) { }
void pc_gfx_overlay_sprite(unsigned texture, float x, float y, float w, float h,
                           float u, float v, float uSize, float vSize) {
    (void)texture; (void)x; (void)y; (void)w; (void)h;
    (void)u; (void)v; (void)uSize; (void)vSize;
}
void pc_gfx_overlay_end(void) { }
void pc_gfx_overlay_sprite(unsigned texture, float x, float y, float w, float h,
                           float r, float g, float b, float a, float angleRadians) {
    (void)texture; (void)x; (void)y; (void)w; (void)h;
    (void)r; (void)g; (void)b; (void)a; (void)angleRadians;
}
void pc_gfx_get_drawable_size(int* width, int* height) {
    unsigned int w = 640, h = 480;
    pc_aurora_get_window_size(&w, &h);
    if (width) *width = (int)w;
    if (height) *height = (int)h;
}
bool pc_gfx_project_current(float x, float y, float z, float* winX, float* winY) {
    (void)x; (void)y; (void)z;
    if (winX) *winX = 0.0f;
    if (winY) *winY = 0.0f;
    return false;
}
bool pc_gfx_project_with_mtx(const float (*mtx)[4], float x, float y, float z,
                             float* winX, float* winY) {
    (void)mtx; (void)x; (void)y; (void)z;
    if (winX) *winX = 0.0f;
    if (winY) *winY = 0.0f;
    return false;
}

/* ── Texture pack: disabled until Aurora implements pc_texpack uploads ── */
static std::string s_texpack_selected;

void pc_texpack_request_enable(void) { }
void pc_texpack_request_disable(void) { }
bool pc_texpack_enabled(void) { return false; }
void pc_texpack_select_pack(const char* folder) {
    s_texpack_selected = folder ? folder : "";
}
const std::string& pc_texpack_selected_pack(void) { return s_texpack_selected; }
std::vector<std::string> pc_texpack_list_packs(void) { return {}; }
void pc_texpack_init(void) { }
const char* pc_texpack_status(void) { return ""; }
void pc_texpack_stats(size_t* replaced, size_t* missing, size_t* failed) {
    if (replaced) *replaced = 0;
    if (missing) *missing = 0;
    if (failed) *failed = 0;
}

/* ── DGX legacy vertex feed (drawModel/drawParticle in pikmin_legacy) ──
 * The old OpenGL backend decoded these into GL calls; Aurora's GX pipeline
 * already models the hardware FIFO, so forward straight into it. */
void pc_gfx_begin(GXPrimitive type, GXVtxFmt vtxfmt, u16 nverts) {
    GXBegin(type, vtxfmt, nverts);
}
void pc_gfx_position(f32 x, f32 y, f32 z) { GXPosition3f32(x, y, z); }
void pc_gfx_normal(f32 x, f32 y, f32 z)   { GXNormal3f32(x, y, z); }
void pc_gfx_color(u8 r, u8 g, u8 b, u8 a) { GXColor4u8(r, g, b, a); }
void pc_gfx_texcoord(f32 u, f32 v)        { GXTexCoord2f32(u, v); }
void pc_gfx_end(void)                     { GXEnd(); }
void pc_gfx_set_scissor(u32 xOrig, u32 yOrig, u32 wd, u32 ht) {
    GXSetScissor(xOrig, yOrig, wd, ht);
}
void pc_gfx_init_tex_obj_rgba(GXTexObj* obj, void* rgba, u16 width, u16 height,
                              GXTexWrapMode wrapS, GXTexWrapMode wrapT) {
    GXInitTexObj(obj, rgba, width, height, GX_TF_RGBA8, wrapS, wrapT, GX_FALSE);
}
void pc_gfx_release_texture(void* gxTexObj) { (void)gxTexObj; }
void pc_gfx_invalidate_cpu_range(const void* addr, size_t bytes) { (void)addr; (void)bytes; }
void pc_gfx_invalidate_resident_meshes(void) { }

/* ── GX runtime the Dolphin SDK headers declare but Aurora leaves out ── */
struct OSThread* GXSetCurrentGXThread(void) { return nullptr; }

/* ── DGX state the Aurora renderer does not model: no-ops / defaults ── */
float pc_gfx_get_current_aspect_ratio(void) { return 4.0f / 3.0f; }
float pc_gfx_get_window_aspect_ratio(void) { return 4.0f / 3.0f; }
int   pc_gfx_get_hud_virtual_width(void) { return 640; }
int   pc_gfx_get_hud_virtual_height(void) { return 528; }
int   pc_gfx_get_hud_wide(void) { return 0; }
int   pc_gfx_get_ui_43(void) { return 1; }
int   pc_gfx_menu_shift_center(void) { return 0; }
int   pc_gfx_menu_shift_right(void) { return 0; }
bool  pc_gfx_get_shader_specialisation(void) { return false; }
size_t pc_gfx_get_specialised_program_count(void) { return 0; }
bool  pc_gfx_replay_captured_frame(void) { return false; }
void  pc_gfx_apply_menu_clip_43(void) { }
void  pc_gfx_begin_capture(uint64_t serial) { (void)serial; }
void  pc_gfx_begin_menu_2d(void) { }
void  pc_gfx_clear_amb_boost(void) { }
void  pc_gfx_clear_mat_color_tint(void) { }
void  pc_gfx_clear_out_tint(void) { }
void  pc_gfx_clear_view_subrect(void) { }
void  pc_gfx_dim_full_target(unsigned char alpha) { (void)alpha; }
void  pc_gfx_enable_capture(bool enabled) { (void)enabled; }
void  pc_gfx_end_capture() { }
void  pc_gfx_filesel_debug_note_aspect(float aspect, int screenW, int screenH) {
    (void)aspect; (void)screenW; (void)screenH;
}
void  pc_gfx_filesel_debug_note_ptcl(unsigned blendFactor, unsigned zMode, unsigned tevMode, float scaleSize) {
    (void)blendFactor; (void)zMode; (void)tevMode; (void)scaleSize;
}
void  pc_gfx_filesel_debug_probe(const char* tag) { (void)tag; }
void  pc_gfx_filesel_debug_set_fx(int active) { (void)active; }
void  pc_gfx_flush_batch(void) { GXFlush(); }
void  pc_gfx_flush_submit_stats(void) { }
void  pc_gfx_get_alpha_compare(GXCompare* comp0, u8* ref0, GXAlphaOp* op, GXCompare* comp1, u8* ref1) {
    if (comp0) *comp0 = GX_ALWAYS;
    if (ref0) *ref0 = 0;
    if (op) *op = GX_AOP_AND;
    if (comp1) *comp1 = GX_ALWAYS;
    if (ref1) *ref1 = 0;
}
void  pc_gfx_get_proj_offset(float* ndcX, float* ndcY) {
    if (ndcX) *ndcX = 0.0f;
    if (ndcY) *ndcY = 0.0f;
}
void  pc_gfx_get_texture_stats(size_t* live, size_t* liveBytes, size_t* peakBytes,
                               size_t* created, size_t* released) {
    if (live) *live = 0;
    if (liveBytes) *liveBytes = 0;
    if (peakBytes) *peakBytes = 0;
    if (created) *created = 0;
    if (released) *released = 0;
}
bool  pc_gfx_menu_pointer_to_graph(float nx, float ny, float* x, float* y) {
    (void)nx; (void)ny; (void)x; (void)y;
    return false;
}
void  pc_gfx_perf_scope_begin(const char* name) { (void)name; }
void  pc_gfx_perf_scope_end(void) { }
void  pc_gfx_reset_ui_state(void) { }
void  pc_gfx_set_amb_boost(GXColor boost) { (void)boost; }
void  pc_gfx_set_dof_focus(float viewDistance) { (void)viewDistance; }
void  pc_gfx_set_mat_color_tint(GXColor tint) { (void)tint; }
void  pc_gfx_set_out_tint(float r, float g, float b) { (void)r; (void)g; (void)b; }
void  pc_gfx_set_pipeline_state(const PcGfxPipelineState& state) { (void)state; }
PcGfxPipelineState pc_gfx_get_pipeline_state(void) { return {}; }
void  pc_gfx_set_proj_offset(float ndcX, float ndcY) { (void)ndcX; (void)ndcY; }
void  pc_gfx_set_view_aspect_override(float aspect) { (void)aspect; }
void  pc_gfx_set_view_subrect(float x0, float y0, float x1, float y1) {
    (void)x0; (void)y0; (void)x1; (void)y1;
}
void  pc_gfx_shadow_exclude(int on) { (void)on; }
