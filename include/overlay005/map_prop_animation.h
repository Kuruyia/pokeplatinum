#ifndef POKEPLATINUM_OV5_MAP_PROP_ANIMATION_H
#define POKEPLATINUM_OV5_MAP_PROP_ANIMATION_H

#include <nnsys.h>

#include "overlay005/struct_ov5_021D3CAC_decl.h"
#include "overlay005/struct_ov5_021D3CE4_decl.h"
#include "overlay005/struct_ov5_021D41B4_decl.h"

MapPropAnimationManager *MapPropAnimationManager_New(void);
MapPropAnimation *MapPropAnimationManager_LoadPropAnimations(const int param0, const int param1, const int param2, const int param3, const BOOL param4, const BOOL param5, const BOOL param6, NNSG3dResMdl *param7, NNSG3dResTex *param8, MapPropAnimationManager *param9);
void MapPropAnimationManager_LoadPropAnimationsSimple(const int param0, NNSG3dResMdl *param1, NNSG3dResTex *param2, MapPropAnimationManager *param3);
BOOL MapPropAnimationManager_AddAnimationToRenderObj(const int param0, const int param1, const BOOL param2, NNSG3dRenderObj *param3, MapPropAnimationManager *param4);
BOOL MapPropAnimationManager_AddAllAnimationsToRenderObj(const int param0, NNSG3dRenderObj *param1, MapPropAnimationManager *param2);
void MapPropAnimationManager_UnloadAllAnimations(MapPropAnimationManager *param0);
void MapPropAnimationManager_UnloadAnimation(MapPropAnimation *param0, MapPropAnimationManager *param1);
void MapPropAnimationManager_RemoveAnimationFromRenderObj(MapPropAnimationManager *param0, NNSG3dRenderObj *param1, const int param2, const int param3);
void MapPropAnimationManager_Free(MapPropAnimationManager *param0);
MapPropAnimation *MapPropAnimationManager_GetAnimationByPastoriaGymButtonGroup(const int param0, MapPropAnimationManager *param1);
MapPropAnimation *MapPropAnimationManager_GetAnimation(const int param0, const int param1, MapPropAnimationManager *param2);
void MapPropAnimation_SetAnimationPaused(MapPropAnimation *param0, const BOOL param1);
void MapPropAnimation_StartLoop(MapPropAnimation *param0);
BOOL MapPropAnimation_IsOnLastFrame(MapPropAnimation *param0);
void MapPropAnimation_GoToFirstFrame(MapPropAnimation *param0);
void MapPropAnimation_GoToLastFrame(MapPropAnimation *param0);
void MapPropAnimation_AdvanceFrame(MapPropAnimation *param0);
void MapPropAnimation_SetReversed(MapPropAnimation *param0, const BOOL param1);
void MapPropAnimation_SetLoopCount(MapPropAnimation *param0, const int param1);
void MapPropAnimation_SetPastoriaGymButtonGroup(MapPropAnimation *param0, const int param1);
void MapPropAnimationManager_AdvanceAnimations(MapPropAnimationManager *param0);
BOOL MapPropAnimation_IsLoopFinished(const MapPropAnimation *param0);
u16 MapPropAnimationManager_GetAnimeListNARCFileCount(MapPropAnimationManager *param0);
const u8 MapPropAnimationManager_GetPropAnimationCount(MapPropAnimationManager *param0, const int param1);
MapPropOneShotAnimationManager *MapPropOneShotAnimationManager_New(void);
void MapPropOneShotAnimationManager_Free(MapPropOneShotAnimationManager **param0);
void ov5_021D41C8(MapPropAnimationManager *param0, MapPropOneShotAnimationManager *const param1, const int param2, const int param3, NNSG3dRenderObj *param4, NNSG3dResMdl *param5, NNSG3dResTex *param6, const int param7, const u8 param8, const BOOL param9);
void ov5_021D4220(MapPropOneShotAnimationManager *const param0, const int param1, const u8 param2, NNSG3dRenderObj *param3);
void MapPropOneShotAnimationManager_PlayAnimation(MapPropOneShotAnimationManager *const param0, const int param1, const int param2);
void MapPropOneShotAnimationManager_PlayAnimationWithSoundEffect(MapPropOneShotAnimationManager *const param0, const int param1, const int param2, const int param3);
void MapPropOneShotAnimationManager_UnloadAnimationByTag(MapPropAnimationManager *param0, MapPropOneShotAnimationManager *param1, const int param2);
MapPropAnimation *MapPropOneShotAnimationManager_GetCurrentAnimationByTag(MapPropOneShotAnimationManager *const param0, const int param1);
const BOOL ov5_021D42F0(MapPropOneShotAnimationManager *const param0, const int param1);
const int MapPropOneShotAnimationManager_GetAnimationMapPropModelIDByTag(MapPropOneShotAnimationManager *const param0, const int param1);

#endif // POKEPLATINUM_OV5_MAP_PROP_ANIMATION_H
