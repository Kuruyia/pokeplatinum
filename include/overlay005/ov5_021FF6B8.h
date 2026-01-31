#ifndef POKEPLATINUM_OV5_021FF6B8_H
#define POKEPLATINUM_OV5_021FF6B8_H

#include "constants/field/field_effect_renderer.h"

#include "overlay005/struct_ov5_021DF47C_decl.h"

typedef void *(*FieldEffectRenderer_NewFunc)(FieldEffectManager *);
typedef void (*FieldEffectRenderer_FreeFunc)(void *);

typedef struct FieldEffectRendererFuncs {
    u32 id;
    FieldEffectRenderer_NewFunc newFunc;
    FieldEffectRenderer_FreeFunc freeFunc;
} FieldEffectRendererFuncs;

extern const u32 sDistWorldFieldEffectRenderers[FIELD_EFFECT_RENDERER_COUNT + 1];
extern const u32 sDefaultFieldEffectRenderers[FIELD_EFFECT_RENDERER_COUNT + 1];
extern const u32 sUndergroundFieldEffectRenderers[FIELD_EFFECT_RENDERER_COUNT + 1];
extern const FieldEffectRendererFuncs sFieldEffectRendererHandlers[FIELD_EFFECT_RENDERER_COUNT + 1];

#endif // POKEPLATINUM_OV5_021FF6B8_H
