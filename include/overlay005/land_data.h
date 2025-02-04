#ifndef POKEPLATINUM_OV5_LAND_DATA_H
#define POKEPLATINUM_OV5_LAND_DATA_H

#include <nitro/fx/fx.h>
#include <nitro/types.h>

#include "field/field_system_decl.h"
#include "overlay005/area_data.h"
#include "overlay005/bdhc.h"
#include "overlay005/map_prop.h"
#include "overlay005/map_prop_animation.h"
#include "overlay005/model_attributes.h"
#include "overlay005/struct_ov5_021E8F60_decl.h"

#include "map_matrix.h"
#include "narc.h"

typedef void (*MapLoadedCallback)(void *, const int, MapPropManager *const);

void LandDataManager_Tick(FieldSystem *fieldSystem, LandDataManager *param1);
LandDataManager *LandDataManager_New(MapMatrix *param0, AreaDataManager *param1, MapPropAnimationManager *param2, const BOOL param3);
void LandDataManager_InitialLoad(LandDataManager *param0, const int param1, const int param2);
void LandDataManager_CheckLazyLoadNextMapPair(const u8 param0, LandDataManager *param1);
void LandDataManager_RenderLoadedMaps(const LandDataManager *param0, ModelAttributes *const param1);
void LandData_ObjectPosToTilePos(fx32 param0, fx32 param1, int *param2, int *param3);
void LandDataManager_End(LandDataManager *param0);
void LandDataManager_FreeNARCAndLoadedMapBuffers(LandDataManager *param0);
BOOL LandDataManager_HasEnded(const LandDataManager *param0);
void LandDataManager_TrackPosition(VecFx32 const *param0, LandDataManager *param1);
void LandDataManager_ForgetTrackedPosition(LandDataManager *param0);
void LandDataManager_GetLoadedMapPropManager(const u8 param0, const LandDataManager *param1, MapPropManager **param2);
u8 LandDataManager_GetTrackedObjectLoadedMapsQuadrant(const LandDataManager *param0);
u8 LandDataManager_CalculateMapQuadrantOfTile(const u32 param0, const int param1);
u8 LandDataManager_GetRelativeLoadedMapsQuadrant(const int param0, const u8 param1, const LandDataManager *param2);
int LandDataManager_GetLoadedMapMatrixIndex(const LandDataManager *param0, const u8 param1);
BOOL LandDataManager_GetRelativeLoadedMapsQuadrantOfTile(const LandDataManager *param0, const int param1, const int param2, u8 *param3);
const BDHC *LandDataManager_GetLoadedMapBDHC(const LandDataManager *param0, const u8 param1);
u16 const *LandDataManager_GetLoadedMapTerrainAttributes(const LandDataManager *param0, const u8 param1);
void LandDataManager_SetMapLoadedCallback(LandDataManager *param0, MapLoadedCallback param1, void *param2);
NARC *LandDataManager_GetLandDataNARC(LandDataManager *param0);
void LandDataManager_SetInDistortionWorld(LandDataManager *param0, BOOL param1);
int LandDataManager_GetOffsetTileX(const LandDataManager *param0);
int LandDataManager_GetOffsetTileY(const LandDataManager *param0);
void LandDataManager_GetOffset(const LandDataManager *param0, VecFx32 *param1);
void LandDataManager_SetSkipMapProps(LandDataManager *param0, BOOL param1);
void LandDataManager_SetMapMatrix(LandDataManager *param0, MapMatrix *param1);
void LandDataManager_SetLoadedMapValid(LandDataManager *param0, int param1, BOOL param2);

void LandDataManager_DistortionWorldNewLoadedMapsWithoutCollisionsAndModel(LandDataManager *param0);
LandDataManager *LandDataManager_DistortionWorldNew(MapMatrix *param0, AreaDataManager *param1, NARC *param2);
void LandDataManager_DistortionWorldInit(LandDataManager *param0, MapMatrix *param1, AreaDataManager *param2, NARC *param3);
void LandDataManager_DistortionWorldEnd(LandDataManager *param0);
void LandDataManager_DistortionWorldInitLoadedMapPropManagers(LandDataManager *param0);
void LandDataManager_DistortionWorldFreeLoadedMapBuffers(LandDataManager *param0);
void LandDataManager_DistortionWorldInitialLoad(LandDataManager *param0, const int param1, const int param2);
void LandDataManager_DistortionWorldInitLoadedMaps(LandDataManager *param0, const int param1, const int param2, int *param3);
void LandDataManager_DistortionWorldLoadAndInvalidate(LandDataManager *param0, int param1, int param2);
void LandDataManager_DistortionWorldRenderNextFloorMaps(const LandDataManager *param0, const ModelAttributes *param1);
void LandDataManager_DistortionWorldEndWithoutFreeing(LandDataManager *param0);
void LandDataManager_DistortionWorldPrepareGoingToNextFloor(MapMatrix *param0, AreaDataManager *param1, LandDataManager *param2, LandDataManager *param3, const int param4, const int param5);
void LandDataManager_DistortionWorldPrepareGoingToPreviousFloor(LandDataManager *param0, LandDataManager *param1);
void LandDataManager_DistortionWorldTick(FieldSystem *fieldSystem, LandDataManager *param1);
void LandDataManager_DistortionWorldInitWithoutNARC(LandDataManager *param0, MapMatrix *param1, AreaDataManager *param2);
void LandDataManager_DistortionWorldInvalidateLoadedMaps(LandDataManager *param0, const int param1, const int param2, int *param3);
void LandDataManager_DistortionWorldLoadEntire(LandDataManager *param0, int param1, int param2);
void LandDataManager_DistortionWorldSetOffsets(LandDataManager *param0, int param1, int param2, int param3);
void LandDataManager_DistortionWorldUpdateTrackedObjectValues(LandDataManager *param0, int param1, int param2);

#endif // POKEPLATINUM_OV5_LAND_DATA_H
