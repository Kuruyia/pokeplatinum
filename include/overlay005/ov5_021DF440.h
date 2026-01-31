#ifndef POKEPLATINUM_OV5_021DF440_H
#define POKEPLATINUM_OV5_021DF440_H

#include <nitro/fx/fx.h>

#include "struct_decls/struct_020216E0_decl.h"
#include "struct_decls/struct_02061AB4_decl.h"
#include "struct_defs/struct_020217F4.h"

#include "field/field_system_decl.h"
#include "overlay005/struct_ov5_021DF47C_decl.h"
#include "overlay005/struct_ov5_021DF84C.h"

#include "overworld_anim_manager.h"
#include "simple3d.h"

FieldEffectManager *FieldEffectManager_New(FieldSystem *fieldSystem, u32 param1, enum HeapID heapID);
void FieldEffectManager_InitAnimManagerList(FieldEffectManager *param0, u32 param1);
void ov5_021DF488(FieldEffectManager *param0, enum HeapID heapID, u32 param2, u32 param3, u32 param4, u32 param5, u32 param6, u32 param7, u32 param8);
void FieldEffectManager_InitRenderer(FieldEffectManager *param0, u32 param1);
void FieldEffectManager_InitRenderers(FieldEffectManager *param0, const u32 *param1);
int FieldEffectManager_HasRenderer(FieldEffectManager *param0, u32 param1);
void FieldEffectManager_Render(FieldEffectManager *param0);
void FieldEffectManager_Free(FieldEffectManager *param0);
void *FieldEffectManager_HeapAlloc(const FieldEffectManager *param0, u32 param1, int param2);
void *FieldEffectManager_HeapAllocInit(const FieldEffectManager *param0, u32 param1, int param2, u32 param3);
void FieldEffectManager_HeapFree(void *param0);
void *FieldEffectManager_GetRendererContext(FieldEffectManager *param0, u32 param1);
FieldSystem *FieldEffectManager_GetFieldSystem(const FieldEffectManager *param0);
FieldEffectManager *MapObject_GetFieldEffectManager(const MapObject *param0);
u32 FieldEffectManager_GetNARCMemberSize(FieldEffectManager *param0, u32 param1);
void FieldEffectManager_ReadNARCWholeMember(FieldEffectManager *param0, u32 param1, void *param2);
void *FieldEffectManager_AllocAndReadNARCWholeMember(FieldEffectManager *param0, u32 param1, int param2);
OverworldAnimManager *FieldEffectManager_InitAnimManager(const FieldEffectManager *param0, const OverworldAnimManagerFuncs *param1, const VecFx32 *param2, int param3, const void *param4, int param5);
void FieldEffectManager_FinishAnimManager(OverworldAnimManager *param0);
UnkStruct_020216E0 *ov5_021DF7F8(FieldEffectManager *param0, const UnkStruct_ov5_021DF84C *param1, const VecFx32 *param2);
UnkStruct_020216E0 *ov5_021DF84C(FieldEffectManager *param0, u32 param1, const VecFx32 *param2);
UnkStruct_ov5_021DF84C *ov5_021DF864(FieldEffectManager *param0, u32 param1, u32 param2, u32 param3, u32 param4, int param5, const UnkStruct_020217F4 *param6);
void ov5_021DF9D4(FieldEffectManager *param0, u32 param1);
void ov5_021DF9E0(FieldEffectManager *param0, u32 param1, u32 param2);
void *ov5_021DF9FC(FieldEffectManager *param0, u32 param1);
void ov5_021DFA08(FieldEffectManager *param0, u32 param1);
void ov5_021DFA14(FieldEffectManager *param0, u32 param1, u32 param2);
void ov5_021DFA30(FieldEffectManager *param0, u32 param1);
void ov5_021DFA3C(FieldEffectManager *param0, u32 param1, u32 param2, u32 param3);
void ov5_021DFA7C(FieldEffectManager *param0, u32 param1);
void FieldEffectManager_LoadModel(FieldEffectManager *param0, Simple3DModel *param1, u32 param2, u32 narcMemberIdx, BOOL allocAtEnd);
void FieldEffectManager_LoadAnimation(FieldEffectManager *param0, Simple3DAnimation *param1, u32 param2, u32 param3, BOOL param4);
void FieldEffectManager_ApplyAnimCopyToModel(FieldEffectManager *param0, Simple3DAnimation *param1, const Simple3DModel *param2, Simple3DAnimation *param3, u32 param4);

#endif // POKEPLATINUM_OV5_021DF440_H
