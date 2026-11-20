#ifndef _DOLPHIN_GXGEOMETRY_H
#define _DOLPHIN_GXGEOMETRY_H

#include "types.h"

#include "Dolphin/GX/GXEnum.h"
#include "Dolphin/GX/GXTypes.h"

BEGIN_SCOPE_EXTERN_C

//////////// GEOMETRY FUNCTIONS ////////////
// Basic GX functions.
extern void __GXSetDirtyState();
extern void GXBegin(GXPrimitive type, GXVtxFmt format, u16 numVertices);
extern void __GXSendFlushPrim();

// Attr functions.
extern void GXSetVtxDesc(GXAttr attr, GXAttrType type);
extern void GXClearVtxDesc();

extern void GXSetVtxAttrFmt(GXVtxFmt format, GXAttr attr, GXCompCnt count, GXCompType type, u8 frac);
extern void GXSetVtxAttrFmtv(GXVtxFmt format, GXVtxAttrFmtList* list);

#if PIKI_USE_AURORA
void GXSetArray(GXAttr attr, const void* data, u32 size, u8 stride, bool littleEndian);
#define GX_SET_HOST_ARRAY(attr, ptr, count, stride) GXSetArray(attr, ptr, (count) * (stride), stride, true)
#else
extern void GXSetArray(GXAttr attr, void* basePtr, u8 stride);
#define GX_SET_HOST_ARRAY(attr, ptr, count, stride) GXSetArray(attr, ptr, stride)
#endif
extern void GXInvalidateVtxCache();
extern void GXSetTexCoordGen2(GXTexCoordID coord, GXTexGenType genType, GXTexGenSrc srcParam, u32 mtx, GXBool doNormalise, u32 postMtx);
extern void GXSetNumTexGens(u8 count);

// Geometry functions.
extern void GXSetLineWidth(u8 width, GXTexOffset offset);
extern void GXSetPointSize(u8 pointSize, GXTexOffset offset);
extern void GXEnableTexOffsets(GXTexCoordID coord, GXBool enableLine, GXBool enablePoint);
extern void __GXSetGenMode();

// Cull and manip functions.
extern void GXSetCullMode(GXCullMode mode);
extern void GXSetCoPlanar(GXBool doEnable);

// Unused/inlined in P2.
extern void GXGetLineWidth(u8* width, GXTexOffset* offset);
extern void GXGetPointSize(u8* pointSize, GXTexOffset* offset);
extern void GXGetCullMode(GXCullMode* mode);

////////////////////////////////////////////

END_SCOPE_EXTERN_C

#endif
