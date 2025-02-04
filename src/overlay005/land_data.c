#include "overlay005/land_data.h"

#include <nitro.h>
#include <string.h>

#include "constants/heap.h"
#include "constants/map.h"
#include "constants/map_object_move.h"

#include "field/field_system.h"
#include "overlay005/area_data.h"
#include "overlay005/area_light.h"
#include "overlay005/bdhc.h"
#include "overlay005/loaded_map_buffers.h"
#include "overlay005/map_prop.h"
#include "overlay005/map_prop_animation.h"
#include "overlay005/model_attributes.h"

#include "easy3d.h"
#include "heap.h"
#include "map_matrix.h"
#include "narc.h"
#include "sys_task.h"
#include "sys_task_manager.h"
#include "system.h"
#include "unk_020366A0.h"

#define INVALID_LAND_DATA_ID       0xFFFF
#define INVALID_TERRAIN_ATTRIBUTES -1
#define INVALID_MAP_MATRIX_INDEX   -1

#define TERRAIN_ATTRIBUTES_SIZE  0x800
#define TERRAIN_ATTRIBUTES_COUNT 1024

#define MAP_LAZY_LOADER_SLOT_COUNT    2
#define MAP_LAZY_LOADER_MANAGER_COUNT 2

#define QUADRANT_COUNT 4

typedef struct MapLoadTasksState {
    BOOL mapModelTaskRunning;
    BOOL bdhcTaskRunning;
} MapLoadTasksState;

typedef struct LoadedMap {
    u16 terrainAttributes[TERRAIN_ATTRIBUTES_COUNT];
    NNSG3dRenderObj mapRenderObj;
    NNSG3dResFileHeader *mapModelFile;
    u8 *bdhcFile;
    BDHC *bdhc;
    int mapMatrixIndex;
    BOOL valid;
    MapPropManager *mapPropManager;
} LoadedMap;

typedef struct MapLazyLoader {
    LoadedMap *loadSlots[MAP_LAZY_LOADER_SLOT_COUNT];
    SysTask *mapModelLoadSysTask;
    SysTask *bdhcLoadSysTask;
    MapLoadTasksState mapLoadTasksState;
    int mapMatrixIndexes[MAP_LAZY_LOADER_SLOT_COUNT];
    u8 quadrants[MAP_LAZY_LOADER_SLOT_COUNT];
    u8 nextIndexToLoad;
    BOOL available;
} MapLazyLoader;

typedef void (*InitialLoadMapsFn)(LandDataManager *, const int, const int, const int, const int, const int);
typedef void (*LazyLoadMapsFn)(const u8, AreaDataManager *const, const MapMatrix *, const int, const int, LandDataManager *, MapLazyLoader *);

typedef struct MapLoadFunctions {
    InitialLoadMapsFn initialLoad;
    LazyLoadMapsFn lazyLoad;
} MapLoadFunctions;

typedef struct MapLazyLoaderManager {
    MapLazyLoader loader;
    BOOL loading;
    u8 loadDirection;
} MapLazyLoaderManager;

typedef struct MapLazyLoaderPending {
    LoadedMap *loadSlots[MAP_LAZY_LOADER_SLOT_COUNT];
    BOOL pending;
    u8 trackedObjectDirection;
    int mapMatrixIndexes[MAP_LAZY_LOADER_SLOT_COUNT];
    u8 quadrants[MAP_LAZY_LOADER_SLOT_COUNT];
} MapLazyLoaderPending;

typedef struct MapLazyLoaderTrackedObject {
    VecFx32 prevPosition;
    VecFx32 const *position;
    fx32 const *higherMovementCoord;
    fx32 const *lowerMovementCoord;
    BOOL moving;
    u8 movingDirection;
    BOOL tracking;
} MapLazyLoaderTrackedObject;

typedef struct LandDataManager_t {
    LoadedMapBuffers *loadedMapBufs;
    MapLazyLoaderManager mapLazyLoaderMan[MAP_LAZY_LOADER_MANAGER_COUNT];
    MapLazyLoaderPending lazyLoaderPending;
    BOOL mapQuadrantsToCancelLazyLoad[QUADRANT_COUNT];
    LoadedMap *loadedMaps[QUADRANT_COUNT];
    u8 runningLazyLoaderManCount;
    u8 nextLazyLoaderManIndex;
    u8 activeLazyLoaderManIndex;
    u32 trackedObjectMapMatrixIndex;
    u32 trackedObjectTileIndex;
    u8 trackedObjectLoadedMapsQuadrant;
    u8 trackedObjectMapQuadrant;
    BOOL ending;
    int lazyLoaderSubTask;
    AreaDataManager *areaDataMan;
    MapMatrix *mapMatrix;
    int mapMatrixWidth;
    int mapMatrixHeight;
    int mapMatrixWidthTiles;
    MapLazyLoaderTrackedObject lazyLoaderTrackedObject;
    MapPropAnimationManager *mapPropAnimMan;
    MapLoadFunctions const *mapLoadFns;
    NARC *landDataNARC;
    MapLoadedCallback mapLoadedCb;
    void *mapLoadedCbUserData;
    BOOL inDistortionWorld;
    BOOL skipMapProps;
    int offsetTileX;
    int offsetAltitude;
    int offsetTileY;
    VecFx32 offset;
} LandDataManager;

typedef struct LandDataHeader {
    int terrainAttributesSize;
    int mapModelSize;
    int bdhcSize;
    int mapPropsSize;
} LandDataHeader;

typedef struct MapModelLoaderTaskContext {
    NARC *landDataNARC;
    int mapModelDataSize;
    NNSG3dRenderObj *mapRenderObj;
    NNSG3dResFileHeader **mapModelFile;
    NNSG3dResTex *mapTexture;
    u8 currentSubTask;
    BOOL *loadedMapValid;
    BOOL killLoadTask;
    int *loadTaskRunning;
    u32 bytesRead;
} MapModelLoaderTaskContext;

enum LazyLoaderSubTask {
    LAZY_LOADER_SUBTASK_LOAD_MAP,
    LAZY_LOADER_SUBTASK_FINISH_MAP_LOAD,
    LAZY_LOADER_SUBTASK_IDLE,
    LAZY_LOADER_SUBTASK_GRACEFUL_SHUTDOWN
};

enum MapModelLoadSubTask {
    MAP_MODEL_LOADER_SUBTASK_FIRST_FILE_READ,
    MAP_MODEL_LOADER_SUBTASK_FILE_READ,
    MAP_MODEL_LOADER_SUBTASK_BIND_TEXTURE,
    MAP_MODEL_LOADER_SUBTASK_INIT_RENDER_OBJ,
    MAP_MODEL_LOADER_SUBTASK_END_TASK = 5
};

enum Direction {
    DIRECTION_INVALID,
    DIRECTION_EAST,
    DIRECTION_SOUTH,
    DIRECTION_WEST,
    DIRECTION_NORTH
};

enum Quadrant {
    QUADRANT_TOP_LEFT,
    QUADRANT_TOP_RIGHT,
    QUADRANT_BOTTOM_LEFT,
    QUADRANT_BOTTOM_RIGHT,
    QUADRANT_INVALID
};

static SysTask *LandDataManager_LazyLoadMapModel(NARC *param0, const int param1, NNSG3dRenderObj *param2, NNSG3dResFileHeader **param3, NNSG3dResTex *param4, BOOL *param5, int *param6);
static NNSG3dResMdl *LandDataManager_LoadMapModel(NARC *param0, const int param1, NNSG3dRenderObj *param2, NNSG3dResFileHeader **param3, NNSG3dResTex *param4);
static void LandDataManager_KillLoadMapModel(SysTask *param0);
static void LandDataManager_InitMapLazyLoader(LandDataManager *param0);
static void LandDataManager_InitAndLoad(LandDataManager *param0, const int param1, const int param2, const int param3, const int param4, const int param5);
static void LandDataManager_InitAndLoadWithoutCollisions(LandDataManager *param0, const int param1, const int param2, const int param3, const int param4, const int param5);
static void LandDataManager_NewLoadedMaps(LandDataManager *param0);
static void LandDataManager_NewLoadedMapsWithoutCollisions(LandDataManager *param0);
static void LandDataManager_KillLoadTasks(LandDataManager *param0, const u8 param1);
static BOOL LandDataManager_SetMapsToLazyLoaderManager(LandDataManager *param0, const int param1, const int param2, const u8 param3, const u8 param4, const u8 param5, const u8 param6);
static void LandDataHeader_Load(NARC *param0, const int param1, LandDataHeader *param2);
static void LandDataManager_LazyLoad(const u8 param0, AreaDataManager *const param1, const MapMatrix *param2, const int param3, const int param4, LandDataManager *param5, MapLazyLoader *param6);
static void LandDataManager_LazyLoadWithoutCollisions(const u8 param0, AreaDataManager *const param1, const MapMatrix *param2, const int param3, const int param4, LandDataManager *param5, MapLazyLoader *param6);
static void LandDataManager_Load(const int param0, const u8 param1, AreaDataManager *const param2, const MapMatrix *param3, const int param4, const int param5, const BOOL param6, const LandDataManager *param7);
static void LandDataManager_LoadWithoutCollisions(const int param0, const u8 param1, AreaDataManager *const param2, const MapMatrix *param3, const int param4, const int param5, const BOOL param6, LandDataManager *param7);
static void LandDataManager_LoadWithoutModel(const int param0, const u8 param1, AreaDataManager *const param2, const MapMatrix *param3, const int param4, const int param5, const BOOL param6, LandDataManager *param7);
static void LandDataManager_QueueLazyLoadNextMapPair(const int param0, const u8 param1, LandDataManager *param2);
static void LandDataManager_PrepareLoadingNextMapPair(const u8 param0, const u8 param1, const u8 param2, const u8 param3, LandDataManager *param4);
static void LandDataManager_ResetLoadedMap(const u8 param0, LandDataManager *param1);
static void LandDataManager_RenderLoadedMap(const u8 param0, const LandDataManager *param1, ModelAttributes *const param2);
static BOOL LandDataManager_FindCancellableLazyLoaderManager(const u8 param0, const LandDataManager *param1, u8 *param2);
static BOOL LandDataManager_SetPendingMapsToNextLazyLoaderManager(LandDataManager *param0);
static BOOL LandDataManager_IsActiveLazyLoaderAvailable(const LandDataManager *param0);
static void LandDataManager_ResetMapQuadrantsToCancelLazyLoad(LandDataManager *param0);
static void LandDataManager_CancelMapQuadrantsToLazyLoad(const u8 param0, LandDataManager *param1);
static u8 LandData_GetOppositeQuadrant(const u8 param0);
static u32 LandData_CalculateNextTileIndex(const u8 param0, const int param1, const int param2, const int param3);
static u8 LandData_CalculateNextQuadrant(const u8 param0, const int param1);
static void LandData_CalculateNextOppositeQuadrant(const u8 param0, const int param1, u8 *param2);
static void LandDataManager_QueueLazyLoadMapPair(const int param0, const int param1, const u8 param2, const u8 param3, const u8 param4, LandDataManager *param5);
static int LandData_CalculateTileIndex(const int param0, const int param1, const int param2, const int param3, const int param4);
static void LandDataManager_QueueLazyLoadPendingMaps(LandDataManager *param0);
static void LandDataManager_UpdateTrackedObjectValues(const int param0, const int param1, const int param2, const int param3, LandDataManager *param4);
static void LandDataManager_CalculateRenderingPosition(const int param0, const int param1, const MapMatrix *param2, VecFx32 *param3);
static void LandDataManager_DistortionWorldInitAndLoad(LandDataManager *param0, const int param1, const int param2, const int param3, const int param4, const int param5);
static void LandDataManager_DistortionWorldLoad(const int param0, const u8 param1, const AreaDataManager *param2, const MapMatrix *param3, const int param4, const int param5, const BOOL param6, LandDataManager *param7);
static void LandDataManager_DistortionWorldRenderNextFloorMap(const u8 param0, const LandDataManager *param1, const ModelAttributes *param2);

static const MapLoadFunctions sMapLoadFns = {
    LandDataManager_InitAndLoad,
    LandDataManager_LazyLoad
};

static const MapLoadFunctions sMapLoadWithoutCollisionsFns = {
    LandDataManager_InitAndLoadWithoutCollisions,
    LandDataManager_LazyLoadWithoutCollisions
};

static BOOL LandData_AreMapsInSameColumn(const int lhsX, const int rhsX, const int mapMatrixWidth)
{
    return lhsX % mapMatrixWidth == rhsX % mapMatrixWidth;
}

static BOOL LandData_AreMapsInSameLine(const int lhsY, const int rhsY, const int mapMatrixWidth)
{
    return lhsY / mapMatrixWidth == rhsY / mapMatrixWidth;
}

static void LandDataManager_KillLoadTasks(LandDataManager *landDataMan, const u8 index)
{
    if (landDataMan->mapLazyLoaderMan[index].loader.mapLoadTasksState.mapModelTaskRunning != FALSE) {
        LandDataManager_KillLoadMapModel(landDataMan->mapLazyLoaderMan[index].loader.mapModelLoadSysTask);
    }

    if (landDataMan->mapLazyLoaderMan[index].loader.mapLoadTasksState.bdhcTaskRunning != FALSE) {
        BDHC_KillLoad(landDataMan->mapLazyLoaderMan[index].loader.bdhcLoadSysTask);
    }

    landDataMan->mapLazyLoaderMan[index].loader.available = TRUE;
}

static void MapLazyLoader_KillTasks(MapLazyLoader *loader)
{
    if (loader->mapLoadTasksState.mapModelTaskRunning != FALSE) {
        LandDataManager_KillLoadMapModel(loader->mapModelLoadSysTask);
    }

    if (loader->mapLoadTasksState.bdhcTaskRunning != FALSE) {
        BDHC_KillLoad(loader->bdhcLoadSysTask);
    }

    loader->mapLoadTasksState.mapModelTaskRunning = FALSE;
    loader->mapLoadTasksState.bdhcTaskRunning = FALSE;
}

static void LandData_CalculateLoadedMapMatrixIndexes(const BOOL inDistortionWorld, const int trackedObjectTileX, const int trackedObjectTileY, const int offsetTileX, const int offsetTileY, const int mapMatrixWidth, const int mapMatrixHeight, const int mapMatrixWidthTiles, int *loadedMapMatrixIndexes)
{
    int trackedObjectTileIndex;
    u8 trackedObjectMapQuadrant;
    int mapMatrixIndex;
    int mapMatrixX, mapMatrixY;

    if (inDistortionWorld == TRUE) {
        for (int i = 0; i < 4; i++) {
            loadedMapMatrixIndexes[i] = INVALID_MAP_MATRIX_INDEX;
        }

        if (mapMatrixWidth == 1) {
            if (mapMatrixHeight == 1) {
                // 1x1 map
                // loadedMapMatrixIndexes = [ 0 -1]
                //                          [-1 -1]
                loadedMapMatrixIndexes[QUADRANT_TOP_LEFT] = 0;
            } else {
                // 1xH map
                // loadedMapMatrixIndexes = [ 0 -1]
                //                          [ 1 -1]
                loadedMapMatrixIndexes[QUADRANT_TOP_LEFT] = 0;
                loadedMapMatrixIndexes[QUADRANT_BOTTOM_LEFT] = 1;
            }
        } else {
            // WxH map
            // loadedMapMatrixIndexes = [ 0  1]
            //                          [-1 -1]
            loadedMapMatrixIndexes[QUADRANT_TOP_LEFT] = 0;
            loadedMapMatrixIndexes[QUADRANT_TOP_RIGHT] = 1;

            if (mapMatrixWidth >= 2) {
                // loadedMapMatrixIndexes = [ 0  1]
                //                          [ 2  3]
                loadedMapMatrixIndexes[QUADRANT_BOTTOM_LEFT] = 2;
                loadedMapMatrixIndexes[QUADRANT_BOTTOM_RIGHT] = 3;
            }
        }

        return;
    }

    GF_ASSERT(trackedObjectTileX >= offsetTileX);
    GF_ASSERT(trackedObjectTileY >= offsetTileY);

    mapMatrixX = (trackedObjectTileX - offsetTileX) / MAP_TILES_COUNT_X;
    mapMatrixY = (trackedObjectTileY - offsetTileY) / MAP_TILES_COUNT_Y;
    trackedObjectTileIndex = LandData_CalculateTileIndex(trackedObjectTileX, trackedObjectTileY, offsetTileX, offsetTileY, mapMatrixWidthTiles);
    trackedObjectMapQuadrant = LandDataManager_CalculateMapQuadrantOfTile(trackedObjectTileIndex, mapMatrixWidthTiles);
    mapMatrixIndex = (mapMatrixY * mapMatrixWidth) + mapMatrixX;

    switch (trackedObjectMapQuadrant) {
    case QUADRANT_TOP_LEFT:
        loadedMapMatrixIndexes[QUADRANT_BOTTOM_RIGHT] = mapMatrixIndex;
        loadedMapMatrixIndexes[QUADRANT_TOP_LEFT] = mapMatrixIndex - mapMatrixWidth - 1;
        loadedMapMatrixIndexes[QUADRANT_TOP_RIGHT] = mapMatrixIndex - mapMatrixWidth;
        loadedMapMatrixIndexes[QUADRANT_BOTTOM_LEFT] = mapMatrixIndex - 1;

        if (mapMatrixX - 1 < 0) {
            loadedMapMatrixIndexes[QUADRANT_TOP_LEFT] = INVALID_MAP_MATRIX_INDEX;
            loadedMapMatrixIndexes[QUADRANT_BOTTOM_LEFT] = INVALID_MAP_MATRIX_INDEX;
        }

        if (mapMatrixY - 1 < 0) {
            loadedMapMatrixIndexes[QUADRANT_TOP_RIGHT] = INVALID_MAP_MATRIX_INDEX;
        }
        break;
    case QUADRANT_TOP_RIGHT:
        loadedMapMatrixIndexes[QUADRANT_BOTTOM_LEFT] = mapMatrixIndex;
        loadedMapMatrixIndexes[QUADRANT_TOP_LEFT] = mapMatrixIndex - mapMatrixWidth;
        loadedMapMatrixIndexes[QUADRANT_TOP_RIGHT] = mapMatrixIndex - mapMatrixWidth + 1;
        loadedMapMatrixIndexes[QUADRANT_BOTTOM_RIGHT] = mapMatrixIndex + 1;

        if (mapMatrixX + 1 >= mapMatrixWidth) {
            loadedMapMatrixIndexes[QUADRANT_TOP_RIGHT] = INVALID_MAP_MATRIX_INDEX;
            loadedMapMatrixIndexes[QUADRANT_BOTTOM_RIGHT] = INVALID_MAP_MATRIX_INDEX;
        }

        if (mapMatrixY - 1 < 0) {
            loadedMapMatrixIndexes[QUADRANT_TOP_LEFT] = INVALID_MAP_MATRIX_INDEX;
        }
        break;
    case QUADRANT_BOTTOM_LEFT:
        loadedMapMatrixIndexes[QUADRANT_TOP_RIGHT] = mapMatrixIndex;
        loadedMapMatrixIndexes[QUADRANT_TOP_LEFT] = mapMatrixIndex - 1;
        loadedMapMatrixIndexes[QUADRANT_BOTTOM_LEFT] = mapMatrixIndex + mapMatrixWidth - 1;
        loadedMapMatrixIndexes[QUADRANT_BOTTOM_RIGHT] = mapMatrixIndex + mapMatrixWidth;

        if (mapMatrixX - 1 < 0) {
            loadedMapMatrixIndexes[QUADRANT_TOP_LEFT] = INVALID_MAP_MATRIX_INDEX;
            loadedMapMatrixIndexes[QUADRANT_BOTTOM_LEFT] = INVALID_MAP_MATRIX_INDEX;
        }

        if (mapMatrixY + 1 > mapMatrixHeight) {
            loadedMapMatrixIndexes[QUADRANT_BOTTOM_RIGHT] = INVALID_MAP_MATRIX_INDEX;
        }
        break;
    case QUADRANT_BOTTOM_RIGHT:
        loadedMapMatrixIndexes[QUADRANT_TOP_LEFT] = mapMatrixIndex;
        loadedMapMatrixIndexes[QUADRANT_TOP_RIGHT] = mapMatrixIndex + 1;
        loadedMapMatrixIndexes[QUADRANT_BOTTOM_LEFT] = mapMatrixIndex + mapMatrixWidth;
        loadedMapMatrixIndexes[QUADRANT_BOTTOM_RIGHT] = mapMatrixIndex + mapMatrixWidth + 1;

        if (mapMatrixX + 1 >= mapMatrixWidth) {
            loadedMapMatrixIndexes[QUADRANT_TOP_RIGHT] = INVALID_MAP_MATRIX_INDEX;
            loadedMapMatrixIndexes[QUADRANT_BOTTOM_RIGHT] = INVALID_MAP_MATRIX_INDEX;
        }

        if (mapMatrixY + 1 > mapMatrixHeight) {
            loadedMapMatrixIndexes[QUADRANT_TOP_LEFT] = INVALID_MAP_MATRIX_INDEX;
        }
        break;
    default:
        GF_ASSERT(FALSE);
    }
}

static void LandDataManager_NewLoadedMaps(LandDataManager *landDataMan)
{
    for (u8 i = 0; i < 4; i++) {
        LoadedMap *loadedMap = Heap_AllocFromHeap(4, sizeof(LoadedMap));
        landDataMan->loadedMaps[i] = loadedMap;
        landDataMan->loadedMaps[i]->valid = FALSE;

        LoadedMapBuffers_GetMapModelFileBufPtr(i, landDataMan->loadedMapBufs, (void **)&(landDataMan->loadedMaps[i]->mapModelFile));
        LoadedMapBuffers_GetBDHCFileBufPtr(i, landDataMan->loadedMapBufs, (void **)&(landDataMan->loadedMaps[i]->bdhcFile));

        if (landDataMan->skipMapProps == FALSE) {
            landDataMan->loadedMaps[i]->mapPropManager = MapPropManager_New(HEAP_ID_FIELD);
        } else {
            landDataMan->loadedMaps[i]->mapPropManager = NULL;
        }

        landDataMan->loadedMaps[i]->mapMatrixIndex = INVALID_MAP_MATRIX_INDEX;
        MI_CpuFillFast(landDataMan->loadedMaps[i]->terrainAttributes, INVALID_TERRAIN_ATTRIBUTES, sizeof(u16) * MAP_TILES_COUNT_X * MAP_TILES_COUNT_Y);
    }
}

void LandDataManager_DistortionWorldNewLoadedMapsWithoutCollisionsAndModel(LandDataManager *landDataMan)
{
    for (u8 i = 0; i < 4; i++) {
        LoadedMap *loadedMap = Heap_AllocFromHeap(4, sizeof(LoadedMap));
        landDataMan->loadedMaps[i] = loadedMap;
        landDataMan->loadedMaps[i]->valid = FALSE;

        if (landDataMan->skipMapProps == FALSE) {
            landDataMan->loadedMaps[i]->mapPropManager = MapPropManager_New(HEAP_ID_FIELD);
        } else {
            landDataMan->loadedMaps[i]->mapPropManager = NULL;
        }

        landDataMan->loadedMaps[i]->mapMatrixIndex = INVALID_MAP_MATRIX_INDEX;
    }
}

static void LandDataManager_NewLoadedMapsWithoutCollisions(LandDataManager *landDataMan)
{
    for (u8 i = 0; i < 4; i++) {
        LoadedMap *loadedMap = Heap_AllocFromHeap(4, sizeof(LoadedMap));
        landDataMan->loadedMaps[i] = loadedMap;
        landDataMan->loadedMaps[i]->valid = FALSE;

        LoadedMapBuffers_GetMapModelFileBufPtr(i, landDataMan->loadedMapBufs, (void **)&(landDataMan->loadedMaps[i]->mapModelFile));

        if (landDataMan->skipMapProps == FALSE) {
            landDataMan->loadedMaps[i]->mapPropManager = MapPropManager_New(HEAP_ID_FIELD);
        } else {
            landDataMan->loadedMaps[i]->mapPropManager = NULL;
        }

        landDataMan->loadedMaps[i]->mapMatrixIndex = INVALID_MAP_MATRIX_INDEX;
    }
}

static void LandDataManager_InitLoadedMapsWithoutCollisions(LandDataManager *landDataMan)
{
    for (u8 i = 0; i < 4; i++) {
        landDataMan->loadedMaps[i]->valid = FALSE;
        LoadedMapBuffers_GetMapModelFileBufPtr(i, landDataMan->loadedMapBufs, (void **)&(landDataMan->loadedMaps[i]->mapModelFile));

        if (landDataMan->loadedMaps[i]->mapPropManager != NULL) {
            MapPropManager_Init(landDataMan->loadedMaps[i]->mapPropManager);
        }

        landDataMan->loadedMaps[i]->mapMatrixIndex = INVALID_MAP_MATRIX_INDEX;
    }
}

static void LandDataHeader_Load(NARC *landDataNARC, const int landDataID, LandDataHeader *landDataHeader)
{
    void *buffer;
    int *iter;
    int bufferSize = sizeof(int) * 4;

    if (landDataNARC != NULL) {
        buffer = Heap_AllocFromHeapAtEnd(4, bufferSize);
        NARC_ReadFromMember(landDataNARC, landDataID, 0, bufferSize, buffer);
    } else {
        GF_ASSERT(FALSE);
        buffer = NARC_AllocAtEndAndReadFromMemberByIndexPair(NARC_INDEX_FIELDDATA__LAND_DATA__LAND_DATA, landDataID, 4, 0, bufferSize);
    }

    iter = (int *)buffer;

    landDataHeader->terrainAttributesSize = *(iter++);
    landDataHeader->mapPropsSize = *(iter++);
    landDataHeader->mapModelSize = *(iter++);
    landDataHeader->bdhcSize = *iter;

    Heap_FreeToHeap(buffer);
}

static void LandDataManager_LazyLoad(const u8 index, AreaDataManager *const areaDataMan, const MapMatrix *mapMatrix, const int mapMatrixWidth, const int mapMatrixHeight, LandDataManager *landDataMan, MapLazyLoader *loader)
{
    int mapMatrixIndex;
    int landDataID;
    LandDataHeader landDataHeader;

    mapMatrixIndex = loader->mapMatrixIndexes[index];

    if ((mapMatrixIndex < 0) || (mapMatrixIndex >= mapMatrixWidth * mapMatrixHeight)) {
        return;
    }

    landDataID = MapMatrix_GetLandDataIdByIndex(mapMatrixIndex, mapMatrix);

    if (landDataID == INVALID_LAND_DATA_ID) {
        return;
    }

    LandDataHeader_Load(landDataMan->landDataNARC, landDataID, &landDataHeader);

    void *terrainAttributes = &loader->loadSlots[index]->terrainAttributes;
    NARC_ReadFile(landDataMan->landDataNARC, landDataHeader.terrainAttributesSize, terrainAttributes);

    if (loader->loadSlots[index]->mapPropManager != NULL) {
        MapPropManager_Load(landDataMan->landDataNARC, landDataHeader.mapPropsSize, areaDataMan, loader->loadSlots[index]->mapPropManager, landDataMan->mapPropAnimMan);
    }

    loader->mapLoadTasksState.mapModelTaskRunning++;
    loader->mapModelLoadSysTask = LandDataManager_LazyLoadMapModel(landDataMan->landDataNARC, landDataHeader.mapModelSize, &loader->loadSlots[index]->mapRenderObj, &(loader->loadSlots[index]->mapModelFile), AreaDataManager_GetMapTexture(areaDataMan), &(loader->loadSlots[index]->valid), &loader->mapLoadTasksState.mapModelTaskRunning);

    loader->mapLoadTasksState.bdhcTaskRunning++;
    loader->bdhcLoadSysTask = BDHC_LazyLoad(landDataMan->landDataNARC, landDataHeader.bdhcSize, loader->loadSlots[index]->bdhc, &loader->mapLoadTasksState.bdhcTaskRunning, &loader->loadSlots[index]->bdhcFile, &loader->mapLoadTasksState.mapModelTaskRunning);
}

static void LandDataManager_LazyLoadWithoutCollisions(const u8 index, AreaDataManager *const areaDataMan, const MapMatrix *mapMatrix, const int mapMatrixWidth, const int mapMatrixHeight, LandDataManager *landDataMan, MapLazyLoader *loader)
{
    int mapMatrixIndex;
    int landDataID;
    LandDataHeader landDataHeader;

    mapMatrixIndex = loader->mapMatrixIndexes[index];

    if ((mapMatrixIndex < 0) || (mapMatrixIndex >= mapMatrixWidth * mapMatrixHeight)) {
        return;
    }

    landDataID = MapMatrix_GetLandDataIdByIndex(mapMatrixIndex, mapMatrix);

    if (landDataID == INVALID_LAND_DATA_ID) {
        return;
    }

    LandDataHeader_Load(landDataMan->landDataNARC, landDataID, &landDataHeader);

    u8 terrainAttributes[TERRAIN_ATTRIBUTES_SIZE];
    NARC_ReadFile(landDataMan->landDataNARC, TERRAIN_ATTRIBUTES_SIZE, terrainAttributes);

    if (loader->loadSlots[index]->mapPropManager != NULL) {
        MapPropManager_Load(landDataMan->landDataNARC, landDataHeader.mapPropsSize, areaDataMan, loader->loadSlots[index]->mapPropManager, landDataMan->mapPropAnimMan);
    }

    loader->mapLoadTasksState.mapModelTaskRunning++;
    loader->mapModelLoadSysTask = LandDataManager_LazyLoadMapModel(landDataMan->landDataNARC, landDataHeader.mapModelSize, &loader->loadSlots[index]->mapRenderObj, &(loader->loadSlots[index]->mapModelFile), AreaDataManager_GetMapTexture(areaDataMan), &(loader->loadSlots[index]->valid), &loader->mapLoadTasksState.mapModelTaskRunning);
}

static void LandDataManager_LazyLoadWithoutCollisionsAndProps(const u8 index, const AreaDataManager *areaDataMan, const MapMatrix *mapMatrix, const int mapMatrixWidth, const int mapMatrixHeight, LandDataManager *landDataMan, MapLazyLoader *loader)
{
    int mapMatrixIndex;
    int landDataID;
    LandDataHeader landDataHeader;

    mapMatrixIndex = loader->mapMatrixIndexes[index];

    if ((mapMatrixIndex < 0) || (mapMatrixIndex >= mapMatrixWidth * mapMatrixHeight)) {
        return;
    }

    landDataID = MapMatrix_GetLandDataIdByIndex(mapMatrixIndex, mapMatrix);

    if (landDataID == INVALID_LAND_DATA_ID) {
        return;
    }

    LandDataHeader_Load(landDataMan->landDataNARC, landDataID, &landDataHeader);

    NARC_Seek(landDataMan->landDataNARC, TERRAIN_ATTRIBUTES_SIZE);
    NARC_Seek(landDataMan->landDataNARC, landDataHeader.mapPropsSize);

    loader->mapLoadTasksState.mapModelTaskRunning++;
    loader->mapModelLoadSysTask = LandDataManager_LazyLoadMapModel(landDataMan->landDataNARC, landDataHeader.mapModelSize, &loader->loadSlots[index]->mapRenderObj, &(loader->loadSlots[index]->mapModelFile), AreaDataManager_GetMapTexture(areaDataMan), &(loader->loadSlots[index]->valid), &loader->mapLoadTasksState.mapModelTaskRunning);
}

static void LandDataManager_Load(const int mapMatrixIndex, const u8 loadedMapIndex, AreaDataManager *const areaDataMan, const MapMatrix *mapMatrix, const int mapMatrixWidth, const int mapMatrixHeight, const BOOL isOutdoorsLighting, const LandDataManager *landDataMan)
{
    int landDataID;
    NNSG3dResMdl *mapModel;
    LandDataHeader landDataHeader;

    if ((mapMatrixIndex < 0) || (mapMatrixIndex >= mapMatrixWidth * mapMatrixHeight)) {
        return;
    }

    landDataID = MapMatrix_GetLandDataIdByIndex(mapMatrixIndex, mapMatrix);

    if (landDataID == INVALID_LAND_DATA_ID) {
        return;
    }

    LandDataHeader_Load(landDataMan->landDataNARC, landDataID, &landDataHeader);

    void *terrainAttributes = landDataMan->loadedMaps[loadedMapIndex]->terrainAttributes;
    NARC_ReadFile(landDataMan->landDataNARC, landDataHeader.terrainAttributesSize, terrainAttributes);

    if (landDataMan->loadedMaps[loadedMapIndex]->mapPropManager != NULL) {
        MapPropManager_Load(landDataMan->landDataNARC, landDataHeader.mapPropsSize, areaDataMan, landDataMan->loadedMaps[loadedMapIndex]->mapPropManager, landDataMan->mapPropAnimMan);
    }

    mapModel = LandDataManager_LoadMapModel(landDataMan->landDataNARC, landDataHeader.mapModelSize, &landDataMan->loadedMaps[loadedMapIndex]->mapRenderObj, &(landDataMan->loadedMaps[loadedMapIndex]->mapModelFile), AreaDataManager_GetMapTexture(areaDataMan));

    if (isOutdoorsLighting == TRUE) {
        AreaLight_UseGlobalModelAttributes(mapModel);
    }

    landDataMan->loadedMaps[loadedMapIndex]->valid = TRUE;

    BDHC_Load(landDataMan->landDataNARC, landDataHeader.bdhcSize, landDataMan->loadedMaps[loadedMapIndex]->bdhc, landDataMan->loadedMaps[loadedMapIndex]->bdhcFile);

    landDataMan->loadedMaps[loadedMapIndex]->mapMatrixIndex = mapMatrixIndex;

    if (landDataMan->mapLoadedCb != NULL) {
        landDataMan->mapLoadedCb(landDataMan->mapLoadedCbUserData, mapMatrixIndex, landDataMan->loadedMaps[loadedMapIndex]->mapPropManager);
    }
}

static void LandDataManager_LoadWithoutCollisions(const int mapMatrixIndex, const u8 loadedMapIndex, AreaDataManager *const areaDataMan, const MapMatrix *mapMatrix, const int mapMatrixWidth, const int mapMatrixHeight, const BOOL isOutdoorsLighting, LandDataManager *landDataMan)
{
    int landDataID;
    LandDataHeader landDataHeader;
    NNSG3dResMdl *mapModel;

    if ((mapMatrixIndex < 0) || (mapMatrixIndex >= mapMatrixWidth * mapMatrixHeight)) {
        return;
    }

    landDataID = MapMatrix_GetLandDataIdByIndex(mapMatrixIndex, mapMatrix);

    if (landDataID == INVALID_LAND_DATA_ID) {
        return;
    }

    LandDataHeader_Load(landDataMan->landDataNARC, landDataID, &landDataHeader);

    u8 terrainAttributes[TERRAIN_ATTRIBUTES_SIZE];
    NARC_ReadFile(landDataMan->landDataNARC, TERRAIN_ATTRIBUTES_SIZE, terrainAttributes);

    if (landDataMan->loadedMaps[loadedMapIndex]->mapPropManager != NULL) {
        MapPropManager_Load(landDataMan->landDataNARC, landDataHeader.mapPropsSize, areaDataMan, landDataMan->loadedMaps[loadedMapIndex]->mapPropManager, landDataMan->mapPropAnimMan);
    }

    mapModel = LandDataManager_LoadMapModel(landDataMan->landDataNARC, landDataHeader.mapModelSize, &landDataMan->loadedMaps[loadedMapIndex]->mapRenderObj, &(landDataMan->loadedMaps[loadedMapIndex]->mapModelFile), AreaDataManager_GetMapTexture(areaDataMan));

    if (isOutdoorsLighting == TRUE) {
        AreaLight_UseGlobalModelAttributes(mapModel);
    }

    landDataMan->loadedMaps[loadedMapIndex]->valid = TRUE;
    landDataMan->loadedMaps[loadedMapIndex]->mapMatrixIndex = mapMatrixIndex;
}

static void LandDataManager_LoadWithoutModel(const int mapMatrixIndex, const u8 loadedMapIndex, AreaDataManager *const areaDataMan, const MapMatrix *mapMatrix, const int mapMatrixWidth, const int mapMatrixHeight, const BOOL isOutdoorsLighting, LandDataManager *landDataMan)
{
    int landDataID;
    LandDataHeader landDataHeader;

    if ((mapMatrixIndex < 0) || (mapMatrixIndex >= mapMatrixWidth * mapMatrixHeight)) {
        return;
    }

    landDataID = MapMatrix_GetLandDataIdByIndex(mapMatrixIndex, mapMatrix);

    if (landDataID == INVALID_LAND_DATA_ID) {
        return;
    }

    LandDataHeader_Load(landDataMan->landDataNARC, landDataID, &landDataHeader);

    void *terrainAttributes = landDataMan->loadedMaps[loadedMapIndex]->terrainAttributes;
    NARC_ReadFile(landDataMan->landDataNARC, landDataHeader.terrainAttributesSize, terrainAttributes);

    if (landDataMan->loadedMaps[loadedMapIndex]->mapPropManager != NULL) {
        MapPropManager_Load(landDataMan->landDataNARC, landDataHeader.mapPropsSize, areaDataMan, landDataMan->loadedMaps[loadedMapIndex]->mapPropManager, landDataMan->mapPropAnimMan);
    }

    NARC_Seek(landDataMan->landDataNARC, landDataHeader.mapModelSize);

    BDHC_Load(landDataMan->landDataNARC, landDataHeader.bdhcSize, landDataMan->loadedMaps[loadedMapIndex]->bdhc, landDataMan->loadedMaps[loadedMapIndex]->bdhcFile);

    landDataMan->loadedMaps[loadedMapIndex]->mapMatrixIndex = mapMatrixIndex;

    if (landDataMan->mapLoadedCb != NULL) {
        landDataMan->mapLoadedCb(landDataMan->mapLoadedCbUserData, mapMatrixIndex, landDataMan->loadedMaps[loadedMapIndex]->mapPropManager);
    }
}

static BOOL LandDataManager_LazyLoadFinished(const MapLoadTasksState *tasksState)
{
    return tasksState->mapModelTaskRunning == FALSE && tasksState->bdhcTaskRunning == FALSE;
}

static BOOL LandDataManager_SetMapsToLazyLoaderManager(LandDataManager *landDataMan, const int firstMapMatrixIndex, const int secondMapMatrixIndex, const u8 firstQuadrantToLoad, const u8 secondQuadrantToLoad, const u8 lazyLoaderManIndex, const u8 trackedObjectDirection)
{
    GF_ASSERT(landDataMan->mapLazyLoaderMan[lazyLoaderManIndex].loading == FALSE);

    landDataMan->mapLazyLoaderMan[lazyLoaderManIndex].loading = TRUE;
    landDataMan->mapLazyLoaderMan[lazyLoaderManIndex].loadDirection = trackedObjectDirection;
    landDataMan->mapLazyLoaderMan[lazyLoaderManIndex].loader.mapMatrixIndexes[0] = firstMapMatrixIndex;
    landDataMan->mapLazyLoaderMan[lazyLoaderManIndex].loader.mapMatrixIndexes[1] = secondMapMatrixIndex;
    landDataMan->mapLazyLoaderMan[lazyLoaderManIndex].loader.quadrants[0] = firstQuadrantToLoad;
    landDataMan->mapLazyLoaderMan[lazyLoaderManIndex].loader.quadrants[1] = secondQuadrantToLoad;
    landDataMan->mapLazyLoaderMan[lazyLoaderManIndex].loader.loadSlots[0] = landDataMan->loadedMaps[firstQuadrantToLoad];
    landDataMan->mapLazyLoaderMan[lazyLoaderManIndex].loader.loadSlots[1] = landDataMan->loadedMaps[secondQuadrantToLoad];
    landDataMan->mapLazyLoaderMan[lazyLoaderManIndex].loader.nextIndexToLoad = 0;
    landDataMan->mapLazyLoaderMan[lazyLoaderManIndex].loader.available = FALSE;

    if (landDataMan->runningLazyLoaderManCount == 0) {
        landDataMan->lazyLoaderSubTask = LAZY_LOADER_SUBTASK_LOAD_MAP;
    }

    landDataMan->loadedMaps[firstQuadrantToLoad]->mapMatrixIndex = firstMapMatrixIndex;
    landDataMan->loadedMaps[secondQuadrantToLoad]->mapMatrixIndex = secondMapMatrixIndex;
    landDataMan->runningLazyLoaderManCount++;
    landDataMan->nextLazyLoaderManIndex = (landDataMan->nextLazyLoaderManIndex + 1) % 2;

    return TRUE;
}

void LandDataManager_Tick(FieldSystem *fieldSystem, LandDataManager *landDataMan)
{
    MapLazyLoader *activeLazyLoader;
    MapLazyLoaderTrackedObject *trackedObject;

    activeLazyLoader = &landDataMan->mapLazyLoaderMan[landDataMan->activeLazyLoaderManIndex].loader;
    trackedObject = &landDataMan->lazyLoaderTrackedObject;

    // Here, we check for movements from the tracked object and start lazy loading maps if necessary.
    if (landDataMan->inDistortionWorld == FALSE && trackedObject->tracking == TRUE) {
        if (trackedObject->moving) {
            GF_ASSERT((*trackedObject->lowerMovementCoord) <= (*trackedObject->higherMovementCoord));

            if ((*trackedObject->higherMovementCoord) - (*trackedObject->lowerMovementCoord) >= MAP_OBJECT_MOVE_TILE_SIZE) {
                // The tracked object has finished moving a tile.
                int trackedObjectTileX, trackedObjectTileY;

                LandData_ObjectPosToTilePos(trackedObject->position->x, trackedObject->position->z, &trackedObjectTileX, &trackedObjectTileY);
                LandDataManager_UpdateTrackedObjectValues(trackedObjectTileX, trackedObjectTileY, landDataMan->offsetTileX, landDataMan->offsetTileY, landDataMan);

                trackedObject->prevPosition = *trackedObject->position;

                if (!(trackedObject->prevPosition.z % (FX32_ONE * 8) == 0)) {
                    if (Link_SetErrorState(1)) {
                        return;
                    }
                }

                GF_ASSERT(trackedObject->prevPosition.z % (FX32_ONE * 8) == 0);

                trackedObject->moving = FALSE;
                trackedObject->higherMovementCoord = NULL;
                trackedObject->lowerMovementCoord = NULL;
            }
        } else {
            if (trackedObject->prevPosition.x != trackedObject->position->x && trackedObject->prevPosition.z != trackedObject->position->z) {
                GF_ASSERT(FALSE);
            } else if (trackedObject->prevPosition.x != trackedObject->position->x || trackedObject->prevPosition.z != trackedObject->position->z) {
                // The tracked object has started moving: check in which direction it is moving,
                // and start lazy loading the next maps in that direction if necessary.
                trackedObject->moving = TRUE;

                if (trackedObject->prevPosition.x != trackedObject->position->x) {
                    if (trackedObject->prevPosition.x > trackedObject->position->x) {
                        trackedObject->higherMovementCoord = &trackedObject->prevPosition.x;
                        trackedObject->lowerMovementCoord = &trackedObject->position->x;
                        trackedObject->movingDirection = DIRECTION_WEST;
                    } else {
                        trackedObject->higherMovementCoord = &trackedObject->position->x;
                        trackedObject->lowerMovementCoord = &trackedObject->prevPosition.x;
                        trackedObject->movingDirection = DIRECTION_EAST;
                    }

                    if (trackedObject->prevPosition.x % (FX32_ONE * 8) != 0) {
                        if (Link_SetErrorState(1)) {
                            return;
                        }
                    }

                    GF_ASSERT(trackedObject->prevPosition.x % (FX32_ONE * 8) == 0);
                } else {
                    if (landDataMan->lazyLoaderTrackedObject.prevPosition.z > landDataMan->lazyLoaderTrackedObject.position->z) {
                        trackedObject->higherMovementCoord = &trackedObject->prevPosition.z;
                        trackedObject->lowerMovementCoord = &trackedObject->position->z;
                        trackedObject->movingDirection = DIRECTION_NORTH;
                    } else {
                        trackedObject->higherMovementCoord = &trackedObject->position->z;
                        trackedObject->lowerMovementCoord = &trackedObject->prevPosition.z;
                        trackedObject->movingDirection = DIRECTION_SOUTH;
                    }

                    if (trackedObject->prevPosition.z % (FX32_ONE * 8) != 0) {
                        if (Link_SetErrorState(1)) {
                            return;
                        }
                    }

                    GF_ASSERT(trackedObject->prevPosition.z % (FX32_ONE * 8) == 0);
                }

                LandDataManager_CheckLazyLoadNextMapPair(trackedObject->movingDirection, landDataMan);
            }
        }
    }

    // Here, we run the logic for lazy loading maps.
    switch (landDataMan->lazyLoaderSubTask) {
    case LAZY_LOADER_SUBTASK_LOAD_MAP:
        if (landDataMan->mapQuadrantsToCancelLazyLoad[activeLazyLoader->quadrants[activeLazyLoader->nextIndexToLoad]] != TRUE) {
            landDataMan->mapLoadFns->lazyLoad(activeLazyLoader->nextIndexToLoad, landDataMan->areaDataMan, landDataMan->mapMatrix, landDataMan->mapMatrixWidth, landDataMan->mapMatrixHeight, landDataMan, activeLazyLoader);
        }

        activeLazyLoader->nextIndexToLoad++;
        landDataMan->lazyLoaderSubTask = LAZY_LOADER_SUBTASK_FINISH_MAP_LOAD;
        break;
    case LAZY_LOADER_SUBTASK_FINISH_MAP_LOAD:
        if (landDataMan->mapQuadrantsToCancelLazyLoad[activeLazyLoader->quadrants[activeLazyLoader->nextIndexToLoad - 1]] == TRUE) {
            MapLazyLoader_KillTasks(activeLazyLoader);
            landDataMan->mapQuadrantsToCancelLazyLoad[activeLazyLoader->quadrants[activeLazyLoader->nextIndexToLoad - 1]] = FALSE;
        }

        if (activeLazyLoader->nextIndexToLoad <= 2 && LandDataManager_LazyLoadFinished(&activeLazyLoader->mapLoadTasksState) == TRUE) {
            if (activeLazyLoader->nextIndexToLoad >= 2) {
                activeLazyLoader->available = TRUE;
            } else {
                landDataMan->lazyLoaderSubTask = LAZY_LOADER_SUBTASK_LOAD_MAP;
            }

            if (activeLazyLoader->loadSlots[activeLazyLoader->nextIndexToLoad - 1]->valid == TRUE) {
                NNSG3dResMdlSet *mapModelSet = NNS_G3dGetMdlSet(activeLazyLoader->loadSlots[activeLazyLoader->nextIndexToLoad - 1]->mapModelFile);
                NNSG3dResMdl *mapModel = NNS_G3dGetMdlByIdx(mapModelSet, 0);

                if (AreaDataManager_IsOutdoorsLighting(landDataMan->areaDataMan) == TRUE) {
                    AreaLight_UseGlobalModelAttributes(mapModel);
                }
            }

            const int loadedMapMatrixIndex = activeLazyLoader->loadSlots[activeLazyLoader->nextIndexToLoad - 1]->mapMatrixIndex;

            if (0 <= loadedMapMatrixIndex && loadedMapMatrixIndex < landDataMan->mapMatrixWidth * landDataMan->mapMatrixHeight && landDataMan->mapLoadedCb != NULL) {
                landDataMan->mapLoadedCb(landDataMan->mapLoadedCbUserData, activeLazyLoader->loadSlots[activeLazyLoader->nextIndexToLoad - 1]->mapMatrixIndex, activeLazyLoader->loadSlots[activeLazyLoader->nextIndexToLoad - 1]->mapPropManager);
            }
        }
        break;
    case LAZY_LOADER_SUBTASK_IDLE:
        break;
    case LAZY_LOADER_SUBTASK_GRACEFUL_SHUTDOWN:
        if (LandDataManager_LazyLoadFinished(&activeLazyLoader->mapLoadTasksState) == TRUE) {
            landDataMan->runningLazyLoaderManCount = 0;
        }
        break;
    }

    if (landDataMan->runningLazyLoaderManCount == 0) {
        landDataMan->activeLazyLoaderManIndex = 0;
        landDataMan->nextLazyLoaderManIndex = 0;
        landDataMan->lazyLoaderSubTask = LAZY_LOADER_SUBTASK_IDLE;
        return;
    }

    // Here, we retire the active lazy loader if it has finished loading maps.
    if (LandDataManager_IsActiveLazyLoaderAvailable(landDataMan) == TRUE) {
        landDataMan->mapLazyLoaderMan[landDataMan->activeLazyLoaderManIndex].loading = FALSE;

        LandDataManager_ResetMapQuadrantsToCancelLazyLoad(landDataMan);

        landDataMan->runningLazyLoaderManCount--;
        landDataMan->activeLazyLoaderManIndex = (landDataMan->activeLazyLoaderManIndex + 1) % 2;

        if (landDataMan->runningLazyLoaderManCount != 0) {
            landDataMan->lazyLoaderSubTask = LAZY_LOADER_SUBTASK_LOAD_MAP;
        } else {
            landDataMan->activeLazyLoaderManIndex = 0;
            landDataMan->nextLazyLoaderManIndex = 0;
            landDataMan->lazyLoaderSubTask = LAZY_LOADER_SUBTASK_IDLE;
        }

        if (landDataMan->lazyLoaderPending.pending == TRUE) {
            landDataMan->lazyLoaderPending.pending = FALSE;

            LandDataManager_CancelMapQuadrantsToLazyLoad(landDataMan->lazyLoaderPending.trackedObjectDirection, landDataMan);
            LandDataManager_QueueLazyLoadPendingMaps(landDataMan);
        }
    }
}

static void LandDataManager_QueueLazyLoadMapPair(const int firstMapMatrixIndex, const int secondMapMatrixIndex, const u8 firstQuadrantToLoad, const u8 secondQuadrantToLoad, const u8 trackedObjectDirection, LandDataManager *landDataMan)
{
    u8 managerIndex;
    BOOL res;

    GF_ASSERT(landDataMan->lazyLoaderPending.pending != TRUE);

    if (LandDataManager_FindCancellableLazyLoaderManager(trackedObjectDirection, landDataMan, &managerIndex) == TRUE) {
        LandDataManager_KillLoadTasks(landDataMan, managerIndex);

        if (landDataMan->runningLazyLoaderManCount >= 2) {
            landDataMan->lazyLoaderPending.pending = TRUE;
            landDataMan->lazyLoaderPending.mapMatrixIndexes[0] = firstMapMatrixIndex;
            landDataMan->lazyLoaderPending.mapMatrixIndexes[1] = secondMapMatrixIndex;
            landDataMan->lazyLoaderPending.quadrants[0] = firstQuadrantToLoad;
            landDataMan->lazyLoaderPending.quadrants[1] = secondQuadrantToLoad;
            landDataMan->lazyLoaderPending.loadSlots[0] = landDataMan->loadedMaps[firstQuadrantToLoad];
            landDataMan->lazyLoaderPending.loadSlots[1] = landDataMan->loadedMaps[secondQuadrantToLoad];
            landDataMan->lazyLoaderPending.trackedObjectDirection = trackedObjectDirection;
            landDataMan->loadedMaps[firstQuadrantToLoad]->mapMatrixIndex = firstMapMatrixIndex;
            landDataMan->loadedMaps[secondQuadrantToLoad]->mapMatrixIndex = secondMapMatrixIndex;
        } else {
            res = LandDataManager_SetMapsToLazyLoaderManager(landDataMan, firstMapMatrixIndex, secondMapMatrixIndex, firstQuadrantToLoad, secondQuadrantToLoad, landDataMan->nextLazyLoaderManIndex, trackedObjectDirection);
        }
    } else {
        res = LandDataManager_SetMapsToLazyLoaderManager(landDataMan, firstMapMatrixIndex, secondMapMatrixIndex, firstQuadrantToLoad, secondQuadrantToLoad, landDataMan->nextLazyLoaderManIndex, trackedObjectDirection);
        GF_ASSERT(res == TRUE);
    }
}

static void LandDataManager_ResetLoadedMap(const u8 index, LandDataManager *landDataMan)
{
    landDataMan->loadedMaps[index]->valid = FALSE;
    BDHC_Reset(landDataMan->loadedMaps[index]->bdhc);

    if (landDataMan->loadedMaps[index]->mapPropManager != NULL) {
        MapPropManager_Init(landDataMan->loadedMaps[index]->mapPropManager);
    }

    landDataMan->loadedMaps[index]->mapMatrixIndex = INVALID_MAP_MATRIX_INDEX;
    MI_CpuFillFast(landDataMan->loadedMaps[index]->terrainAttributes, INVALID_TERRAIN_ATTRIBUTES, sizeof(u16) * MAP_TILES_COUNT_X * MAP_TILES_COUNT_Y);
    landDataMan->mapQuadrantsToCancelLazyLoad[index] = TRUE;
}

static void LandDataManager_PrepareLoadingNextMapPair(const u8 firstResetLoadedMap, const u8 secondResetLoadedMap, const u8 firstKeptLoadedMap, const u8 secondKeptLoadedMap, LandDataManager *landDataMan)
{
    LoadedMap *tmp[2];

    LandDataManager_ResetLoadedMap(firstResetLoadedMap, landDataMan);
    LandDataManager_ResetLoadedMap(secondResetLoadedMap, landDataMan);

    tmp[0] = landDataMan->loadedMaps[firstResetLoadedMap];
    tmp[1] = landDataMan->loadedMaps[secondResetLoadedMap];

    landDataMan->loadedMaps[firstResetLoadedMap] = landDataMan->loadedMaps[firstKeptLoadedMap];
    landDataMan->loadedMaps[secondResetLoadedMap] = landDataMan->loadedMaps[secondKeptLoadedMap];

    landDataMan->loadedMaps[firstKeptLoadedMap] = tmp[0];
    landDataMan->loadedMaps[secondKeptLoadedMap] = tmp[1];
}

static void LandDataManager_QueueLazyLoadNextMapPair(const int trackedObjectMapMatrixIndex, const u8 trackedObjectDirection, LandDataManager *landDataMan)
{
    int firstMapMatrixIndexToLoad, secondMapMatrixIndexToLoad;
    int firstQuadrantToLoad, secondQuadrantToLoad;

    switch (trackedObjectDirection) {
    case DIRECTION_NORTH:
        // The tracked object is going north, and is currently in one of the bottom quadrants within its map,
        // which means that it will end up in the top quadrants of its map.
        //
        // In this configuration, the map pair to load is the one that is above the current map pair.
        //
        // 1) First, we "push down" the top map pair.
        //
        // loadedMaps = [1 2] ==> [X X]
        //              [3 4]     [1 2]
        LandDataManager_PrepareLoadingNextMapPair(QUADRANT_BOTTOM_LEFT, QUADRANT_BOTTOM_RIGHT, QUADRANT_TOP_LEFT, QUADRANT_TOP_RIGHT, landDataMan);

        if (landDataMan->trackedObjectMapQuadrant == QUADRANT_BOTTOM_LEFT) {
            // The tracked object is in the bottom left quadrant of its map, which means that it will end up
            // in the top left quadrant.
            //
            // In this configuration, the maps to load are the ones that are above, to the left,
            // and to the top-left of the map that the tracked object is currently in.
            //
            // loadedMaps = [ X  X ]
            //              [ 1 (2)]
            // (between parentheses is the loaded map that the tracked object is currently in)
            //
            // 2) Then, we only need to load the map above (a.k.a. the map on the "top-right" of the loaded maps quadrant),
            // and the map to the top-left (a.k.a. the map on the "top-left" of the loaded maps quadrant) of the map that
            // the tracked object is currently in.
            // The map on the left is already loaded.
            firstMapMatrixIndexToLoad = trackedObjectMapMatrixIndex - landDataMan->mapMatrixWidth;
            secondMapMatrixIndexToLoad = trackedObjectMapMatrixIndex - landDataMan->mapMatrixWidth - 1;

            firstQuadrantToLoad = QUADRANT_TOP_RIGHT;
            secondQuadrantToLoad = QUADRANT_TOP_LEFT;

            if (firstMapMatrixIndexToLoad < 0) {
                firstMapMatrixIndexToLoad = INVALID_MAP_MATRIX_INDEX;
                secondMapMatrixIndexToLoad = INVALID_MAP_MATRIX_INDEX;
            } else if (secondMapMatrixIndexToLoad < 0 || !LandData_AreMapsInSameLine(firstMapMatrixIndexToLoad, secondMapMatrixIndexToLoad, landDataMan->mapMatrixWidth)) {
                secondMapMatrixIndexToLoad = INVALID_MAP_MATRIX_INDEX;
            }
        } else if (landDataMan->trackedObjectMapQuadrant == QUADRANT_BOTTOM_RIGHT) {
            // The tracked object is in the bottom right quadrant of its map, which means that it will end up
            // in the top right quadrant.
            //
            // In this configuration, the maps to load are the ones that are above, to the right,
            // and to the top-right of the map that the tracked object is currently in.
            //
            // loadedMaps = [ X  X ]
            //              [(1) 2 ]
            // (between parentheses is the loaded map that the tracked object is currently in)
            //
            // 2) Then, we only need to load the map above (a.k.a. the map on the "top-left" of the loaded maps quadrant),
            // and the map to the top-right (a.k.a. the map on the "top-right" of the loaded maps quadrant) of the map that
            // the tracked object is currently in.
            // The map on the right is already loaded.
            firstMapMatrixIndexToLoad = trackedObjectMapMatrixIndex - landDataMan->mapMatrixWidth;
            secondMapMatrixIndexToLoad = trackedObjectMapMatrixIndex - landDataMan->mapMatrixWidth + 1;

            firstQuadrantToLoad = QUADRANT_TOP_LEFT;
            secondQuadrantToLoad = QUADRANT_TOP_RIGHT;

            if (firstMapMatrixIndexToLoad < 0) {
                firstMapMatrixIndexToLoad = INVALID_MAP_MATRIX_INDEX;
                secondMapMatrixIndexToLoad = INVALID_MAP_MATRIX_INDEX;
            } else if (!LandData_AreMapsInSameLine(firstMapMatrixIndexToLoad, secondMapMatrixIndexToLoad, landDataMan->mapMatrixWidth)) {
                secondMapMatrixIndexToLoad = INVALID_MAP_MATRIX_INDEX;
            }
        } else {
            GF_ASSERT(FALSE);
        }

        LandDataManager_QueueLazyLoadMapPair(firstMapMatrixIndexToLoad, secondMapMatrixIndexToLoad, firstQuadrantToLoad, secondQuadrantToLoad, trackedObjectDirection, landDataMan);
        break;
    case DIRECTION_WEST:
        // The tracked object is going west, and is currently in one of the right quadrants within its map,
        // which means that it will end up in the left quadrants of its map.
        //
        // In this configuration, the map pair to load is the one that is on the left of the current map pair.
        //
        // 1) First, we "push right" the top map pair.
        //
        // loadedMaps = [1 2] ==> [X 1]
        //              [3 4]     [X 3]
        LandDataManager_PrepareLoadingNextMapPair(QUADRANT_TOP_RIGHT, QUADRANT_BOTTOM_RIGHT, QUADRANT_TOP_LEFT, QUADRANT_BOTTOM_LEFT, landDataMan);

        if (landDataMan->trackedObjectMapQuadrant == QUADRANT_TOP_RIGHT) {
            // The tracked object is in the top right quadrant of its map, which means that it will end up
            // in the top left quadrant.
            //
            // In this configuration, the maps to load are the ones that are above, to the left,
            // and to the top-left of the map that the tracked object is currently in.
            //
            // loadedMaps = [ X  1 ]
            //              [ X (3)]
            // (between parentheses is the loaded map that the tracked object is currently in)
            //
            // 2) Then, we only need to load the map to the left (a.k.a. the map on the "bottom-left" of the loaded maps quadrant),
            // and the map to the top-left (a.k.a. the map on the "top-left" of the loaded maps quadrant) of the map that
            // the tracked object is currently in.
            // The map above is already loaded.
            firstMapMatrixIndexToLoad = trackedObjectMapMatrixIndex - 1;
            secondMapMatrixIndexToLoad = trackedObjectMapMatrixIndex - landDataMan->mapMatrixWidth - 1;

            firstQuadrantToLoad = QUADRANT_BOTTOM_LEFT;
            secondQuadrantToLoad = QUADRANT_TOP_LEFT;

            if (firstMapMatrixIndexToLoad < 0 || !LandData_AreMapsInSameLine(firstMapMatrixIndexToLoad, trackedObjectMapMatrixIndex, landDataMan->mapMatrixWidth)) {
                firstMapMatrixIndexToLoad = INVALID_MAP_MATRIX_INDEX;
                secondMapMatrixIndexToLoad = INVALID_MAP_MATRIX_INDEX;
            }

            if (secondMapMatrixIndexToLoad < 0 || !LandData_AreMapsInSameColumn(firstMapMatrixIndexToLoad, secondMapMatrixIndexToLoad, landDataMan->mapMatrixWidth)) {
                secondMapMatrixIndexToLoad = INVALID_MAP_MATRIX_INDEX;
            }
        } else if (landDataMan->trackedObjectMapQuadrant == QUADRANT_BOTTOM_RIGHT) {
            // The tracked object is in the bottom right quadrant of its map, which means that it will end up
            // in the bottom left quadrant.
            //
            // In this configuration, the maps to load are the ones that are below, to the left,
            // and to the bottom-left of the map that the tracked object is currently in.
            //
            // loadedMaps = [ X (1)]
            //              [ X  3 ]
            // (between parentheses is the loaded map that the tracked object is currently in)
            //
            // 2) Then, we only need to load the map to the left (a.k.a. the map on the "top-left" of the loaded maps quadrant),
            // and the map to the bottom-left (a.k.a. the map on the "bottom-left" of the loaded maps quadrant) of the map that
            // the tracked object is currently in.
            // The map below is already loaded.
            firstMapMatrixIndexToLoad = trackedObjectMapMatrixIndex - 1;
            secondMapMatrixIndexToLoad = trackedObjectMapMatrixIndex + landDataMan->mapMatrixWidth - 1;

            firstQuadrantToLoad = QUADRANT_TOP_LEFT;
            secondQuadrantToLoad = QUADRANT_BOTTOM_LEFT;

            if (firstMapMatrixIndexToLoad < 0 || !LandData_AreMapsInSameLine(firstMapMatrixIndexToLoad, trackedObjectMapMatrixIndex, landDataMan->mapMatrixWidth)) {
                firstMapMatrixIndexToLoad = INVALID_MAP_MATRIX_INDEX;
                secondMapMatrixIndexToLoad = INVALID_MAP_MATRIX_INDEX;
            }

            if (landDataMan->mapMatrixWidth * landDataMan->mapMatrixHeight <= secondMapMatrixIndexToLoad || !LandData_AreMapsInSameColumn(firstMapMatrixIndexToLoad, secondMapMatrixIndexToLoad, landDataMan->mapMatrixWidth)) {
                secondMapMatrixIndexToLoad = INVALID_MAP_MATRIX_INDEX;
            }
        } else {
            GF_ASSERT(FALSE);
        }

        LandDataManager_QueueLazyLoadMapPair(firstMapMatrixIndexToLoad, secondMapMatrixIndexToLoad, firstQuadrantToLoad, secondQuadrantToLoad, trackedObjectDirection, landDataMan);
        break;
    case DIRECTION_EAST:
        // The tracked object is going east, and is currently in one of the left quadrants within its map,
        // which means that it will end up in the right quadrants of its map.
        //
        // In this configuration, the map pair to load is the one that is on the right of the current map pair.
        //
        // 1) First, we "push left" the top map pair.
        //
        // loadedMaps = [1 2] ==> [2 X]
        //              [3 4]     [4 X]
        LandDataManager_PrepareLoadingNextMapPair(QUADRANT_TOP_LEFT, QUADRANT_BOTTOM_LEFT, QUADRANT_TOP_RIGHT, QUADRANT_BOTTOM_RIGHT, landDataMan);

        if (landDataMan->trackedObjectMapQuadrant == QUADRANT_TOP_LEFT) {
            // The tracked object is in the top left quadrant of its map, which means that it will end up
            // in the top right quadrant.
            //
            // In this configuration, the maps to load are the ones that are above, to the right,
            // and to the top-right of the map that the tracked object is currently in.
            //
            // loadedMaps = [ 2  X ]
            //              [(4) X ]
            // (between parentheses is the loaded map that the tracked object is currently in)
            //
            // 2) Then, we only need to load the map to the right (a.k.a. the map on the "bottom-right" of the loaded maps quadrant),
            // and the map to the top-right (a.k.a. the map on the "top-right" of the loaded maps quadrant) of the map that
            // the tracked object is currently in.
            // The map above is already loaded.
            firstMapMatrixIndexToLoad = trackedObjectMapMatrixIndex + 1;
            secondMapMatrixIndexToLoad = trackedObjectMapMatrixIndex - landDataMan->mapMatrixWidth + 1;

            firstQuadrantToLoad = QUADRANT_BOTTOM_RIGHT;
            secondQuadrantToLoad = QUADRANT_TOP_RIGHT;

            if (landDataMan->mapMatrixWidth * landDataMan->mapMatrixHeight <= firstMapMatrixIndexToLoad || !LandData_AreMapsInSameLine(firstMapMatrixIndexToLoad, trackedObjectMapMatrixIndex, landDataMan->mapMatrixWidth)) {
                firstMapMatrixIndexToLoad = INVALID_MAP_MATRIX_INDEX;
                secondMapMatrixIndexToLoad = INVALID_MAP_MATRIX_INDEX;
            }

            if (landDataMan->mapMatrixWidth * landDataMan->mapMatrixHeight <= secondMapMatrixIndexToLoad || !LandData_AreMapsInSameColumn(firstMapMatrixIndexToLoad, secondMapMatrixIndexToLoad, landDataMan->mapMatrixWidth)) {
                secondMapMatrixIndexToLoad = INVALID_MAP_MATRIX_INDEX;
            }
        } else if (landDataMan->trackedObjectMapQuadrant == QUADRANT_BOTTOM_LEFT) {
            // The tracked object is in the botton left quadrant of its map, which means that it will end up
            // in the bottom right quadrant.
            //
            // In this configuration, the maps to load are the ones that are below, to the right,
            // and to the bottom-right of the map that the tracked object is currently in.
            //
            // loadedMaps = [(2) X ]
            //              [ 4  X ]
            // (between parentheses is the loaded map that the tracked object is currently in)
            //
            // 2) Then, we only need to load the map to the right (a.k.a. the map on the "top-right" of the loaded maps quadrant),
            // and the map to the bottom-right (a.k.a. the map on the "bottom-right" of the loaded maps quadrant) of the map that
            // the tracked object is currently in.
            // The map below is already loaded.
            firstMapMatrixIndexToLoad = trackedObjectMapMatrixIndex + 1;
            secondMapMatrixIndexToLoad = trackedObjectMapMatrixIndex + landDataMan->mapMatrixWidth + 1;

            firstQuadrantToLoad = QUADRANT_TOP_RIGHT;
            secondQuadrantToLoad = QUADRANT_BOTTOM_RIGHT;

            if (landDataMan->mapMatrixWidth * landDataMan->mapMatrixHeight <= firstMapMatrixIndexToLoad || !LandData_AreMapsInSameLine(firstMapMatrixIndexToLoad, trackedObjectMapMatrixIndex, landDataMan->mapMatrixWidth)) {
                firstMapMatrixIndexToLoad = INVALID_MAP_MATRIX_INDEX;
                secondMapMatrixIndexToLoad = INVALID_MAP_MATRIX_INDEX;
            }

            if (secondMapMatrixIndexToLoad < 0 || !LandData_AreMapsInSameColumn(firstMapMatrixIndexToLoad, secondMapMatrixIndexToLoad, landDataMan->mapMatrixWidth)) {
                secondMapMatrixIndexToLoad = INVALID_MAP_MATRIX_INDEX;
            }
        }

        LandDataManager_QueueLazyLoadMapPair(firstMapMatrixIndexToLoad, secondMapMatrixIndexToLoad, firstQuadrantToLoad, secondQuadrantToLoad, trackedObjectDirection, landDataMan);
        break;
    case DIRECTION_SOUTH:
        // The tracked object is going south, and is currently in one of the top quadrants within its map,
        // which means that it will end up in the bottom quadrants of its map.
        //
        // In this configuration, the map pair to load is the one that is on the bottom of the current map pair.
        //
        // 1) First, we "push up" the top map pair.
        //
        // loadedMaps = [1 2] ==> [3 4]
        //              [3 4]     [X X]
        LandDataManager_PrepareLoadingNextMapPair(QUADRANT_TOP_LEFT, QUADRANT_TOP_RIGHT, QUADRANT_BOTTOM_LEFT, QUADRANT_BOTTOM_RIGHT, landDataMan);

        if (landDataMan->trackedObjectMapQuadrant == QUADRANT_TOP_LEFT) {
            // The tracked object is in the top left quadrant of its map, which means that it will end up
            // in the bottom left quadrant.
            //
            // In this configuration, the maps to load are the ones that are below, to the left,
            // and to the bottom-left of the map that the tracked object is currently in.
            //
            // loadedMaps = [ 3 (4)]
            //              [ X  X ]
            // (between parentheses is the loaded map that the tracked object is currently in)
            //
            // 2) Then, we only need to load the map below (a.k.a. the map on the "bottom-right" of the loaded maps quadrant),
            // and the map to the bottom-left (a.k.a. the map on the "bottom-left" of the loaded maps quadrant) of the map that
            // the tracked object is currently in.
            // The map on the left is already loaded.
            firstMapMatrixIndexToLoad = trackedObjectMapMatrixIndex + landDataMan->mapMatrixWidth;
            secondMapMatrixIndexToLoad = trackedObjectMapMatrixIndex + landDataMan->mapMatrixWidth - 1;

            firstQuadrantToLoad = QUADRANT_BOTTOM_RIGHT;
            secondQuadrantToLoad = QUADRANT_BOTTOM_LEFT;

            if (landDataMan->mapMatrixWidth * landDataMan->mapMatrixHeight <= firstMapMatrixIndexToLoad) {
                firstMapMatrixIndexToLoad = INVALID_MAP_MATRIX_INDEX;
                secondMapMatrixIndexToLoad = INVALID_MAP_MATRIX_INDEX;
            } else if (!LandData_AreMapsInSameLine(firstMapMatrixIndexToLoad, secondMapMatrixIndexToLoad, landDataMan->mapMatrixWidth)) {
                secondMapMatrixIndexToLoad = INVALID_MAP_MATRIX_INDEX;
            }
        } else if (landDataMan->trackedObjectMapQuadrant == QUADRANT_TOP_RIGHT) {
            // The tracked object is in the top right quadrant of its map, which means that it will end up
            // in the bottom right quadrant.
            //
            // In this configuration, the maps to load are the ones that are below, to the right,
            // and to the bottom-right of the map that the tracked object is currently in.
            //
            // loadedMaps = [(3) 4 ]
            //              [ X  X ]
            // (between parentheses is the loaded map that the tracked object is currently in)
            //
            // 2) Then, we only need to load the map below (a.k.a. the map on the "bottom-left" of the loaded maps quadrant),
            // and the map to the bottom-right (a.k.a. the map on the "bottom-right" of the loaded maps quadrant) of the map that
            // the tracked object is currently in.
            // The map on the right is already loaded.
            firstMapMatrixIndexToLoad = trackedObjectMapMatrixIndex + landDataMan->mapMatrixWidth;
            secondMapMatrixIndexToLoad = trackedObjectMapMatrixIndex + landDataMan->mapMatrixWidth + 1;

            firstQuadrantToLoad = QUADRANT_BOTTOM_LEFT;
            secondQuadrantToLoad = QUADRANT_BOTTOM_RIGHT;

            if (landDataMan->mapMatrixWidth * landDataMan->mapMatrixHeight <= firstMapMatrixIndexToLoad) {
                firstMapMatrixIndexToLoad = INVALID_MAP_MATRIX_INDEX;
                secondMapMatrixIndexToLoad = INVALID_MAP_MATRIX_INDEX;
            } else if (landDataMan->mapMatrixWidth * landDataMan->mapMatrixHeight <= secondMapMatrixIndexToLoad || !LandData_AreMapsInSameLine(firstMapMatrixIndexToLoad, secondMapMatrixIndexToLoad, landDataMan->mapMatrixWidth)) {
                secondMapMatrixIndexToLoad = INVALID_MAP_MATRIX_INDEX;
            }
        } else {
            GF_ASSERT(FALSE);
        }

        LandDataManager_QueueLazyLoadMapPair(firstMapMatrixIndexToLoad, secondMapMatrixIndexToLoad, firstQuadrantToLoad, secondQuadrantToLoad, trackedObjectDirection, landDataMan);
        break;
    default:
        GF_ASSERT(FALSE);
        break;
    }
}

static void LandDataManager_RenderLoadedMap(const u8 index, const LandDataManager *landDataMan, ModelAttributes *const modelAttrs)
{
    VecFx32 position = { 0, 0, 0 };

    LandDataManager_CalculateRenderingPosition(landDataMan->loadedMaps[index]->mapMatrixIndex, landDataMan->mapMatrixWidth, landDataMan->mapMatrix, &position);

    position.x += landDataMan->offset.x;
    position.y += landDataMan->offset.y;
    position.z += landDataMan->offset.z;

    if (landDataMan->loadedMaps[index]->valid == TRUE) {
        VecFx32 scale = { FX32_ONE, FX32_ONE, FX32_ONE };
        MtxFx33 rotationMatrix = { FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE };

        Easy3D_DrawRenderObj(&landDataMan->loadedMaps[index]->mapRenderObj, &position, &rotationMatrix, &scale);
    }

    if (landDataMan->loadedMaps[index]->valid == TRUE) {
        if (landDataMan->loadedMaps[index]->mapPropManager != NULL) {
            MapPropManager_Render(&position, landDataMan->areaDataMan, AreaDataManager_IsOutdoorsLighting(landDataMan->areaDataMan), modelAttrs, landDataMan->loadedMaps[index]->mapPropManager);
        }
    }
}

static int LandData_CalculateMapMatrixIndex(const u32 tileIndex, const int mapMatrixWidth, const int mapMatrixWidthTiles)
{
    u32 tileX, tileY;
    u32 mapMatrixX, mapMatrixY;

    tileX = tileIndex % mapMatrixWidthTiles;
    tileY = tileIndex / mapMatrixWidthTiles;
    mapMatrixX = tileX / MAP_TILES_COUNT_X;
    mapMatrixY = tileY / MAP_TILES_COUNT_Y;

    return mapMatrixX + mapMatrixY * mapMatrixWidth;
}

static int LandData_CalculateTileIndex(const int tileX, const int tileY, const int offsetTileX, const int offsetTileY, const int mapMatrixWidthTiles)
{
    return (tileX - offsetTileX) + ((tileY - offsetTileY) * mapMatrixWidthTiles);
}

static u32 LandData_CalculateMapMatrixIndexU32(const u32 tileIndex, const int mapMatrixWidth, const int mapMatrixWidthTiles)
{
    u32 tileX, tileY;
    u32 mapMatrixX, mapMatrixY;

    tileX = tileIndex % mapMatrixWidthTiles;
    tileY = tileIndex / mapMatrixWidthTiles;
    mapMatrixX = tileX / MAP_TILES_COUNT_X;
    mapMatrixY = tileY / MAP_TILES_COUNT_Y;

    return mapMatrixX + mapMatrixY * mapMatrixWidth;
}

static u8 LandData_GetOppositeQuadrant(const u8 quadrant)
{
    if (quadrant == QUADRANT_TOP_LEFT) {
        return QUADRANT_BOTTOM_RIGHT;
    } else if (quadrant == QUADRANT_TOP_RIGHT) {
        return QUADRANT_BOTTOM_LEFT;
    } else if (quadrant == QUADRANT_BOTTOM_LEFT) {
        return QUADRANT_TOP_RIGHT;
    } else if (quadrant == QUADRANT_BOTTOM_RIGHT) {
        return QUADRANT_TOP_LEFT;
    } else {
        GF_ASSERT(FALSE);
    }

    return 0;
}

static u32 LandData_CalculateNextTileIndex(const u8 trackedObjectDirection, const int mapMatrixHeight, const int mapMatrixWidthTiles, const int trackedObjectTileIndex)
{
    u32 nextTileIndex;
    int nextTileIndexCandidate;

    switch (trackedObjectDirection) {
    case DIRECTION_EAST:
        nextTileIndexCandidate = (trackedObjectTileIndex % mapMatrixWidthTiles) + 1;

        if (nextTileIndexCandidate >= mapMatrixWidthTiles) {
            return trackedObjectTileIndex;
        }

        nextTileIndex = trackedObjectTileIndex + 1;
        break;
    case DIRECTION_SOUTH:
        nextTileIndexCandidate = trackedObjectTileIndex + mapMatrixWidthTiles;

        if (nextTileIndexCandidate >= mapMatrixHeight * MAP_TILES_COUNT_Y * mapMatrixWidthTiles) {
            return trackedObjectTileIndex;
        }

        nextTileIndex = trackedObjectTileIndex + mapMatrixWidthTiles;
        break;
    case DIRECTION_WEST:
        nextTileIndexCandidate = (trackedObjectTileIndex % mapMatrixWidthTiles) - 1;

        if (nextTileIndexCandidate < 0) {
            return trackedObjectTileIndex;
        }

        nextTileIndex = trackedObjectTileIndex - 1;
        break;
    case DIRECTION_NORTH:
        nextTileIndexCandidate = trackedObjectTileIndex - mapMatrixWidthTiles;

        if (nextTileIndexCandidate < 0) {
            return trackedObjectTileIndex;
        }

        nextTileIndex = trackedObjectTileIndex - mapMatrixWidthTiles;
        break;
    default:
        GF_ASSERT(FALSE);
        return 0;
    }

    return nextTileIndex;
}

static u8 LandData_CalculateNextQuadrant(const u8 direction, const int quadrant)
{
    switch (direction) {
    case DIRECTION_NORTH:
        return quadrant - 2;
    case DIRECTION_SOUTH:
        return quadrant + 2;
    case DIRECTION_WEST:
        return quadrant - 1;
    case DIRECTION_EAST:
        return quadrant + 1;
    }
}

static void LandData_CalculateNextOppositeQuadrant(const u8 direction, const int quadrant, u8 *oppositeQuadrant)
{
    u8 nextQuadrant;
    u8 nextOppositeQuadrant;

    nextQuadrant = LandData_CalculateNextQuadrant(direction, quadrant);

    if (quadrant < nextQuadrant) {
        if (nextQuadrant - quadrant == 1) {
            nextOppositeQuadrant = (*oppositeQuadrant) - 1;
        } else if (nextQuadrant - quadrant == 2) {
            nextOppositeQuadrant = (*oppositeQuadrant) - 2;
        } else {
            GF_ASSERT(FALSE);
        }
    } else if (quadrant > nextQuadrant) {
        if (quadrant - nextQuadrant == 1) {
            nextOppositeQuadrant = (*oppositeQuadrant) + 1;
        } else if (quadrant - nextQuadrant == 2) {
            nextOppositeQuadrant = (*oppositeQuadrant) + 2;
        } else {
            GF_ASSERT(FALSE);
        }
    } else {
        GF_ASSERT(FALSE);
    }

    (*oppositeQuadrant) = nextOppositeQuadrant;
}

static u8 LandData_GetOppositeDirection(const u8 direction)
{
    switch (direction) {
    case DIRECTION_NORTH:
        return DIRECTION_SOUTH;
    case DIRECTION_SOUTH:
        return DIRECTION_NORTH;
    case DIRECTION_EAST:
        return DIRECTION_WEST;
    case DIRECTION_WEST:
        return DIRECTION_EAST;
    default:
        GF_ASSERT(FALSE);
        return DIRECTION_INVALID;
    }
}

static BOOL LandDataManager_FindCancellableLazyLoaderManager(const u8 trackedObjectDirection, const LandDataManager *landDataMan, u8 *managerIndex)
{
    for (int i = 0; i < 2; i++) {
        if (landDataMan->mapLazyLoaderMan[i].loading == TRUE) {
            if (LandData_GetOppositeDirection(trackedObjectDirection) == landDataMan->mapLazyLoaderMan[i].loadDirection) {
                *managerIndex = i;
                return TRUE;
            }
        }
    }

    return FALSE;
}

static BOOL LandDataManager_SetPendingMapsToNextLazyLoaderManager(LandDataManager *landDataMan)
{
    MapLazyLoaderPending *lazyLoaderPending = &landDataMan->lazyLoaderPending;

    if (landDataMan->mapLazyLoaderMan[landDataMan->nextLazyLoaderManIndex].loading != FALSE) {
        GF_ASSERT(FALSE);
    }

    landDataMan->mapLazyLoaderMan[landDataMan->nextLazyLoaderManIndex].loading = TRUE;
    landDataMan->mapLazyLoaderMan[landDataMan->nextLazyLoaderManIndex].loadDirection = lazyLoaderPending->trackedObjectDirection;

    landDataMan->mapLazyLoaderMan[landDataMan->nextLazyLoaderManIndex].loader.mapMatrixIndexes[0] = lazyLoaderPending->mapMatrixIndexes[0];
    landDataMan->mapLazyLoaderMan[landDataMan->nextLazyLoaderManIndex].loader.mapMatrixIndexes[1] = lazyLoaderPending->mapMatrixIndexes[1];

    landDataMan->mapLazyLoaderMan[landDataMan->nextLazyLoaderManIndex].loader.quadrants[0] = lazyLoaderPending->quadrants[0];
    landDataMan->mapLazyLoaderMan[landDataMan->nextLazyLoaderManIndex].loader.quadrants[1] = lazyLoaderPending->quadrants[1];

    landDataMan->mapLazyLoaderMan[landDataMan->nextLazyLoaderManIndex].loader.loadSlots[0] = lazyLoaderPending->loadSlots[0];
    landDataMan->mapLazyLoaderMan[landDataMan->nextLazyLoaderManIndex].loader.loadSlots[1] = lazyLoaderPending->loadSlots[1];

    landDataMan->mapLazyLoaderMan[landDataMan->nextLazyLoaderManIndex].loader.nextIndexToLoad = 0;
    landDataMan->mapLazyLoaderMan[landDataMan->nextLazyLoaderManIndex].loader.available = FALSE;

    return TRUE;
}

static void LandDataManager_QueueLazyLoadPendingMaps(LandDataManager *landDataMan)
{
    BOOL res;
    MapLazyLoaderPending *lazyLoaderPending = &landDataMan->lazyLoaderPending;

    if (landDataMan->runningLazyLoaderManCount >= 2) {
        GF_ASSERT(FALSE);
    }

    res = LandDataManager_SetPendingMapsToNextLazyLoaderManager(landDataMan);
    GF_ASSERT(res == TRUE);

    lazyLoaderPending->loadSlots[0]->mapMatrixIndex = lazyLoaderPending->mapMatrixIndexes[0];
    lazyLoaderPending->loadSlots[1]->mapMatrixIndex = lazyLoaderPending->mapMatrixIndexes[1];

    landDataMan->runningLazyLoaderManCount++;
    landDataMan->nextLazyLoaderManIndex = (landDataMan->nextLazyLoaderManIndex + 1) % 2;
}

static BOOL LandDataManager_IsActiveLazyLoaderAvailable(const LandDataManager *landDataMan)
{
    return landDataMan->mapLazyLoaderMan[landDataMan->activeLazyLoaderManIndex].loader.available;
}

static void LandDataManager_ResetMapQuadrantsToCancelLazyLoad(LandDataManager *landDataMan)
{
    for (u8 i = 0; i < 4; i++) {
        landDataMan->mapQuadrantsToCancelLazyLoad[i] = FALSE;
    }
}

static void LandDataManager_CancelMapQuadrantsToLazyLoad(const u8 trackedObjectDirection, LandDataManager *landDataMan)
{
    switch (trackedObjectDirection) {
    case DIRECTION_NORTH:
        landDataMan->mapQuadrantsToCancelLazyLoad[QUADRANT_BOTTOM_LEFT] = TRUE;
        landDataMan->mapQuadrantsToCancelLazyLoad[QUADRANT_BOTTOM_RIGHT] = TRUE;
        break;
    case DIRECTION_EAST:
        landDataMan->mapQuadrantsToCancelLazyLoad[QUADRANT_TOP_LEFT] = TRUE;
        landDataMan->mapQuadrantsToCancelLazyLoad[QUADRANT_BOTTOM_LEFT] = TRUE;
        break;
    case DIRECTION_SOUTH:
        landDataMan->mapQuadrantsToCancelLazyLoad[QUADRANT_TOP_LEFT] = TRUE;
        landDataMan->mapQuadrantsToCancelLazyLoad[QUADRANT_TOP_RIGHT] = TRUE;
        break;
    case DIRECTION_WEST:
        landDataMan->mapQuadrantsToCancelLazyLoad[QUADRANT_TOP_RIGHT] = TRUE;
        landDataMan->mapQuadrantsToCancelLazyLoad[QUADRANT_BOTTOM_RIGHT] = TRUE;
        break;
    }
}

static void LandDataManager_InitAndLoad(LandDataManager *landDataMan, const int trackedObjectTileX, const int trackedObjectTileY, const int offsetTileX, const int offsetTileY, const int mapMatrixWidthTiles)
{
    u8 i;
    int loadedMapMatrixIndexes[4];

    LandDataManager_NewLoadedMaps(landDataMan);
    LandData_CalculateLoadedMapMatrixIndexes(landDataMan->inDistortionWorld, trackedObjectTileX, trackedObjectTileY, offsetTileX, offsetTileY, landDataMan->mapMatrixWidth, landDataMan->mapMatrixHeight, mapMatrixWidthTiles, loadedMapMatrixIndexes);

    for (i = 0; i < 4; i++) {
        landDataMan->loadedMaps[i]->bdhc = BDHC_New();

        BDHC_MarkNotLoaded(landDataMan->loadedMaps[i]->bdhc);
        LandDataManager_Load(loadedMapMatrixIndexes[i], i, landDataMan->areaDataMan, landDataMan->mapMatrix, landDataMan->mapMatrixWidth, landDataMan->mapMatrixHeight, AreaDataManager_IsOutdoorsLighting(landDataMan->areaDataMan), landDataMan);
    }
}

static void LandDataManager_InitAndLoadWithoutCollisions(LandDataManager *landDataMan, const int trackedObjectTileX, const int trackedObjectTileY, const int offsetTileX, const int offsetTileY, const int mapMatrixWidthTiles)
{
    u8 i;
    int loadedMapMatrixIndexes[4];

    LandDataManager_NewLoadedMapsWithoutCollisions(landDataMan);
    LandData_CalculateLoadedMapMatrixIndexes(landDataMan->inDistortionWorld, trackedObjectTileX, trackedObjectTileY, offsetTileX, offsetTileY, landDataMan->mapMatrixWidth, landDataMan->mapMatrixHeight, mapMatrixWidthTiles, loadedMapMatrixIndexes);

    for (i = 0; i < 4; i++) {
        landDataMan->loadedMaps[i]->bdhc = NULL;
        LandDataManager_LoadWithoutCollisions(loadedMapMatrixIndexes[i], i, landDataMan->areaDataMan, landDataMan->mapMatrix, landDataMan->mapMatrixWidth, landDataMan->mapMatrixHeight, AreaDataManager_IsOutdoorsLighting(landDataMan->areaDataMan), landDataMan);
    }
}

static void LandDataManager_InitMapLazyLoader(LandDataManager *landDataMan)
{
    landDataMan->activeLazyLoaderManIndex = 0;
    landDataMan->nextLazyLoaderManIndex = 0;
    landDataMan->runningLazyLoaderManCount = 0;

    for (int i = 0; i < 2; i++) {
        landDataMan->mapLazyLoaderMan[i].loading = FALSE;
        landDataMan->mapLazyLoaderMan[i].loader.available = TRUE;
    }

    LandDataManager_ResetMapQuadrantsToCancelLazyLoad(landDataMan);
}

static void LandDataManager_UpdateTrackedObjectValues(const int trackedObjectTileX, const int trackedObjectTileY, const int offsetTileX, const int offsetTileY, LandDataManager *landDataMan)
{
    int fixedTrackedObjectTileX = trackedObjectTileX, fixedTrackedObjectTileY = trackedObjectTileY;

    if (landDataMan->inDistortionWorld == TRUE) {
        if (landDataMan->mapMatrixWidth > 1) {
            fixedTrackedObjectTileX = offsetTileX + 31;
        } else {
            fixedTrackedObjectTileX = offsetTileX + 16;
        }

        if (landDataMan->mapMatrixHeight > 1) {
            fixedTrackedObjectTileY = offsetTileY + 31;
        } else {
            fixedTrackedObjectTileY = offsetTileY + 16;
        }
    }

    landDataMan->trackedObjectTileIndex = LandData_CalculateTileIndex(fixedTrackedObjectTileX, fixedTrackedObjectTileY, offsetTileX, offsetTileY, landDataMan->mapMatrixWidthTiles);
    landDataMan->trackedObjectMapQuadrant = LandDataManager_CalculateMapQuadrantOfTile(landDataMan->trackedObjectTileIndex, landDataMan->mapMatrixWidthTiles);
    landDataMan->trackedObjectLoadedMapsQuadrant = LandData_GetOppositeQuadrant(landDataMan->trackedObjectMapQuadrant);
    landDataMan->trackedObjectMapMatrixIndex = LandData_CalculateMapMatrixIndexU32(landDataMan->trackedObjectTileIndex, landDataMan->mapMatrixWidth, landDataMan->mapMatrixWidthTiles);
}

static void LandDataManager_CalculateRenderingPosition(const int mapMatrixIndex, const int mapMatrixWidth, const MapMatrix *mapMatrix, VecFx32 *position)
{
    u16 mapMatrixX;
    u16 mapMatrixY;

    position->x = (MAP_TILES_COUNT_X / 2) * MAP_OBJECT_MOVE_TILE_SIZE;
    position->z = (MAP_TILES_COUNT_Y / 2) * MAP_OBJECT_MOVE_TILE_SIZE;

    if (mapMatrixIndex == INVALID_MAP_MATRIX_INDEX) {
        return;
    }

    mapMatrixX = mapMatrixIndex % mapMatrixWidth;
    mapMatrixY = mapMatrixIndex / mapMatrixWidth;

    int altitude;
    int mapMatrixWidth2;
    u16 mapMatrixID;

    mapMatrixWidth2 = MapMatrix_GetWidth(mapMatrix);
    mapMatrixID = MapMatrix_GetMatrixID(mapMatrix);
    altitude = MapMatrix_GetAltitudeAtCoords(mapMatrix, mapMatrixID, mapMatrixX, mapMatrixY, mapMatrixWidth2);
    position->y = altitude * (MAP_OBJECT_MOVE_TILE_SIZE / 2);

    position->x += mapMatrixX * MAP_TILES_COUNT_X * MAP_OBJECT_MOVE_TILE_SIZE;
    position->z += mapMatrixY * MAP_TILES_COUNT_Y * MAP_OBJECT_MOVE_TILE_SIZE;
}

LandDataManager *LandDataManager_New(MapMatrix *mapMatrix, AreaDataManager *areaDataMan, MapPropAnimationManager *mapPropAnimMan, const BOOL skipCollisions)
{
    LandDataManager *landDataMan;
    BOOL allocBDHCFiles;

    landDataMan = Heap_AllocFromHeap(4, sizeof(LandDataManager));
    MI_CpuClear8(landDataMan, sizeof(LandDataManager));

    if (skipCollisions == FALSE) {
        landDataMan->mapLoadFns = &sMapLoadFns;
        allocBDHCFiles = TRUE;
    } else if (skipCollisions == TRUE) {
        landDataMan->mapLoadFns = &sMapLoadWithoutCollisionsFns;
        allocBDHCFiles = FALSE;
    } else {
        GF_ASSERT(FALSE);
    }

    landDataMan->loadedMapBufs = LoadedMapBuffers_New(allocBDHCFiles);
    landDataMan->areaDataMan = areaDataMan;
    landDataMan->mapMatrix = mapMatrix;
    landDataMan->mapMatrixWidth = MapMatrix_GetWidth(mapMatrix);
    landDataMan->mapMatrixHeight = MapMatrix_GetHeight(mapMatrix);
    landDataMan->mapMatrixWidthTiles = landDataMan->mapMatrixWidth * MAP_TILES_COUNT_X;
    landDataMan->mapPropAnimMan = mapPropAnimMan;
    landDataMan->lazyLoaderTrackedObject.tracking = TRUE;

    LandDataManager_InitMapLazyLoader(landDataMan);

    landDataMan->ending = FALSE;
    landDataMan->lazyLoaderSubTask = LAZY_LOADER_SUBTASK_IDLE;
    landDataMan->landDataNARC = NARC_ctor(NARC_INDEX_FIELDDATA__LAND_DATA__LAND_DATA, 4);
    landDataMan->mapLoadedCb = NULL;

    return landDataMan;
}

void LandDataManager_InitialLoad(LandDataManager *landDataMan, const int playerTileX, const int playerTileY)
{
    landDataMan->mapLoadFns->initialLoad(landDataMan, playerTileX, playerTileY, landDataMan->offsetTileX, landDataMan->offsetTileY, landDataMan->mapMatrixWidthTiles);
    LandDataManager_UpdateTrackedObjectValues(playerTileX, playerTileY, landDataMan->offsetTileX, landDataMan->offsetTileY, landDataMan);
}

void LandDataManager_CheckLazyLoadNextMapPair(const u8 trackedObjectDirection, LandDataManager *landDataMan)
{
    int nextTrackedObjectMapMatrixIndex;
    u32 nextTrackedObjectTileIndex;
    u8 nextTrackedObjectMapQuadrant;

    nextTrackedObjectTileIndex = LandData_CalculateNextTileIndex(trackedObjectDirection, landDataMan->mapMatrixHeight, landDataMan->mapMatrixWidthTiles, landDataMan->trackedObjectTileIndex);
    nextTrackedObjectMapMatrixIndex = LandData_CalculateMapMatrixIndex(nextTrackedObjectTileIndex, landDataMan->mapMatrixWidth, landDataMan->mapMatrixWidthTiles);
    nextTrackedObjectMapQuadrant = LandDataManager_CalculateMapQuadrantOfTile(nextTrackedObjectTileIndex, landDataMan->mapMatrixWidthTiles);

    if (landDataMan->trackedObjectMapMatrixIndex != nextTrackedObjectMapMatrixIndex) {
        // If the tracked object is about to change maps, don't do anything: the tracked object will change
        // quadrant within the same map, and change quadrant within the loaded maps, but the loaded maps
        // themselves are still the expected ones in the expected order.
        return;
    } else if (landDataMan->trackedObjectMapQuadrant != nextTrackedObjectMapQuadrant) {
        // If the tracked object is about to change quadrant within the same map, queue the next map pair to load.
        LandDataManager_QueueLazyLoadNextMapPair(nextTrackedObjectMapMatrixIndex, trackedObjectDirection, landDataMan);
        LandData_CalculateNextOppositeQuadrant(trackedObjectDirection, landDataMan->trackedObjectMapQuadrant, &landDataMan->trackedObjectLoadedMapsQuadrant);
    }
}

void LandDataManager_RenderLoadedMaps(const LandDataManager *landDataMan, ModelAttributes *const modelAttrs)
{
    for (u8 i = 0; i < 4; i++) {
        if (landDataMan->loadedMaps[i]->mapMatrixIndex != INVALID_MAP_MATRIX_INDEX) {
            LandDataManager_RenderLoadedMap(i, landDataMan, modelAttrs);
        }
    }
}

void LandData_ObjectPosToTilePos(fx32 objectPositionX, fx32 objectPositionZ, int *tileX, int *tileY)
{
    *tileX = objectPositionX / MAP_OBJECT_MOVE_TILE_SIZE;
    *tileY = objectPositionZ / MAP_OBJECT_MOVE_TILE_SIZE;
}

void LandDataManager_End(LandDataManager *landDataMan)
{
    u8 i;

    ClearUnusedSystemCache();

    NNS_GfdResetLnkTexVramState();
    NNS_GfdResetLnkPlttVramState();

    for (i = 0; i < 2; i++) {
        MapLazyLoader_KillTasks(&landDataMan->mapLazyLoaderMan[i].loader);
    }

    for (i = 0; i < 4; i++) {
        landDataMan->loadedMaps[i]->valid = 0;

        BDHC_Free(landDataMan->loadedMaps[i]->bdhc);

        if (landDataMan->loadedMaps[i]->mapPropManager != NULL) {
            MapPropManager_Free(landDataMan->loadedMaps[i]->mapPropManager);
        }

        Heap_FreeToHeap(landDataMan->loadedMaps[i]);
        landDataMan->loadedMaps[i] = NULL;
    }

    landDataMan->lazyLoaderSubTask = LAZY_LOADER_SUBTASK_GRACEFUL_SHUTDOWN;
    landDataMan->ending = TRUE;
    landDataMan->lazyLoaderTrackedObject.tracking = FALSE;
}

void LandDataManager_FreeNARCAndLoadedMapBuffers(LandDataManager *landDataMan)
{
    NARC_dtor(landDataMan->landDataNARC);
    LoadedMapBuffers_Free(landDataMan->loadedMapBufs);
    Heap_FreeToHeap(&landDataMan->loadedMapBufs);
}

BOOL LandDataManager_HasEnded(const LandDataManager *landDataMan)
{
    return landDataMan->ending == TRUE && landDataMan->runningLazyLoaderManCount == 0;
}

void LandDataManager_TrackPosition(VecFx32 const *position, LandDataManager *landDataMan)
{
    landDataMan->lazyLoaderTrackedObject.prevPosition = *position;
    landDataMan->lazyLoaderTrackedObject.position = position;
}

void LandDataManager_ForgetTrackedPosition(LandDataManager *landDataMan)
{
    landDataMan->lazyLoaderTrackedObject.position = NULL;
}

void LandDataManager_GetLoadedMapPropManager(const u8 index, const LandDataManager *landDataMan, MapPropManager **mapPropMan)
{
    *mapPropMan = landDataMan->loadedMaps[index]->mapPropManager;
}

u8 LandDataManager_GetTrackedObjectLoadedMapsQuadrant(const LandDataManager *landDataMan)
{
    return landDataMan->trackedObjectLoadedMapsQuadrant;
}

u8 LandDataManager_CalculateMapQuadrantOfTile(const u32 tileIndex, const int mapMatrixWidthTiles)
{
    u8 mapQuadrant;
    int tileX, tileYMod;
    int tileY;

    tileX = tileIndex % MAP_TILES_COUNT_X;
    tileY = tileIndex / mapMatrixWidthTiles;
    tileYMod = tileY % MAP_TILES_COUNT_Y;

    if (tileX < MAP_TILES_COUNT_X / 2) {
        if (tileYMod < MAP_TILES_COUNT_Y / 2) {
            mapQuadrant = QUADRANT_TOP_LEFT;
        } else {
            mapQuadrant = QUADRANT_BOTTOM_LEFT;
        }
    } else {
        if (tileYMod < MAP_TILES_COUNT_Y / 2) {
            mapQuadrant = QUADRANT_TOP_RIGHT;
        } else {
            mapQuadrant = QUADRANT_BOTTOM_RIGHT;
        }
    }

    return mapQuadrant;
}

u8 LandDataManager_GetRelativeLoadedMapsQuadrant(const int mapMatrixIndex, const u8 unused1, const LandDataManager *landDataMan)
{
    u8 mapMatrixIndexQuadrant;

    if (landDataMan->trackedObjectMapMatrixIndex == mapMatrixIndex) {
        return landDataMan->trackedObjectLoadedMapsQuadrant;
    }

    GF_ASSERT(mapMatrixIndex != INVALID_MAP_MATRIX_INDEX);

    switch (landDataMan->trackedObjectLoadedMapsQuadrant) {
    case QUADRANT_TOP_LEFT:
        if (mapMatrixIndex - landDataMan->trackedObjectMapMatrixIndex == landDataMan->mapMatrixWidth) {
            mapMatrixIndexQuadrant = landDataMan->trackedObjectLoadedMapsQuadrant + 2;
        } else if (mapMatrixIndex - landDataMan->trackedObjectMapMatrixIndex == 1 && LandData_AreMapsInSameLine(landDataMan->trackedObjectMapMatrixIndex, mapMatrixIndex, landDataMan->mapMatrixWidth)) {
            mapMatrixIndexQuadrant = landDataMan->trackedObjectLoadedMapsQuadrant + 1;
        } else if (mapMatrixIndex - landDataMan->trackedObjectMapMatrixIndex == landDataMan->mapMatrixWidth + 1 && !LandData_AreMapsInSameLine(landDataMan->trackedObjectMapMatrixIndex, mapMatrixIndex, landDataMan->mapMatrixWidth)) {
            mapMatrixIndexQuadrant = landDataMan->trackedObjectLoadedMapsQuadrant + 3;
        } else {
            mapMatrixIndexQuadrant = QUADRANT_INVALID;
        }
        break;
    case QUADRANT_TOP_RIGHT:
        if (mapMatrixIndex - landDataMan->trackedObjectMapMatrixIndex == landDataMan->mapMatrixWidth) {
            mapMatrixIndexQuadrant = landDataMan->trackedObjectLoadedMapsQuadrant + 2;
        } else if (landDataMan->trackedObjectMapMatrixIndex - mapMatrixIndex == 1 && LandData_AreMapsInSameLine(landDataMan->trackedObjectMapMatrixIndex, mapMatrixIndex, landDataMan->mapMatrixWidth)) {
            mapMatrixIndexQuadrant = landDataMan->trackedObjectLoadedMapsQuadrant - 1;
        } else if (mapMatrixIndex - landDataMan->trackedObjectMapMatrixIndex == landDataMan->mapMatrixWidth - 1 && !LandData_AreMapsInSameLine(landDataMan->trackedObjectMapMatrixIndex, mapMatrixIndex, landDataMan->mapMatrixWidth)) {
            mapMatrixIndexQuadrant = landDataMan->trackedObjectLoadedMapsQuadrant + 1;
        } else {
            mapMatrixIndexQuadrant = QUADRANT_INVALID;
        }
        break;
    case QUADRANT_BOTTOM_LEFT:
        if (landDataMan->trackedObjectMapMatrixIndex - mapMatrixIndex == landDataMan->mapMatrixWidth) {
            mapMatrixIndexQuadrant = landDataMan->trackedObjectLoadedMapsQuadrant - 2;
        } else if (landDataMan->trackedObjectMapMatrixIndex - mapMatrixIndex == landDataMan->mapMatrixWidth - 1 && !LandData_AreMapsInSameLine(landDataMan->trackedObjectMapMatrixIndex, mapMatrixIndex, landDataMan->mapMatrixWidth)) {
            mapMatrixIndexQuadrant = landDataMan->trackedObjectLoadedMapsQuadrant - 1;
        } else if (mapMatrixIndex - landDataMan->trackedObjectMapMatrixIndex == 1 && LandData_AreMapsInSameLine(landDataMan->trackedObjectMapMatrixIndex, mapMatrixIndex, landDataMan->mapMatrixWidth)) {
            mapMatrixIndexQuadrant = landDataMan->trackedObjectLoadedMapsQuadrant + 1;
        } else {
            mapMatrixIndexQuadrant = QUADRANT_INVALID;
        }
        break;
    case QUADRANT_BOTTOM_RIGHT:
        if (landDataMan->trackedObjectMapMatrixIndex - mapMatrixIndex == landDataMan->mapMatrixWidth) {
            mapMatrixIndexQuadrant = landDataMan->trackedObjectLoadedMapsQuadrant - 2;
        } else if (landDataMan->trackedObjectMapMatrixIndex - mapMatrixIndex == landDataMan->mapMatrixWidth + 1 && !LandData_AreMapsInSameLine(landDataMan->trackedObjectMapMatrixIndex, mapMatrixIndex, landDataMan->mapMatrixWidth)) {
            mapMatrixIndexQuadrant = landDataMan->trackedObjectLoadedMapsQuadrant - 3;
        } else if (landDataMan->trackedObjectMapMatrixIndex - mapMatrixIndex == 1 && LandData_AreMapsInSameLine(landDataMan->trackedObjectMapMatrixIndex, mapMatrixIndex, landDataMan->mapMatrixWidth)) {
            mapMatrixIndexQuadrant = landDataMan->trackedObjectLoadedMapsQuadrant - 1;
        } else {
            mapMatrixIndexQuadrant = QUADRANT_INVALID;
        }
        break;
    default:
        mapMatrixIndexQuadrant = QUADRANT_INVALID;
    }

    return mapMatrixIndexQuadrant;
}

int LandDataManager_GetLoadedMapMatrixIndex(const LandDataManager *landDataMan, const u8 loadedMapIndex)
{
    GF_ASSERT(loadedMapIndex < 4);
    return landDataMan->loadedMaps[loadedMapIndex]->mapMatrixIndex;
}

BOOL LandDataManager_GetRelativeLoadedMapsQuadrantOfTile(const LandDataManager *landDataMan, const int tileX, const int tileY, u8 *result)
{
    int fixedTileX, fixedTileY;
    u32 mapMatrixX, mapMatrixY;
    u32 mapMatrixIndex;
    u32 loadedMapsQuadrant;
    u32 tileIndex;
    u8 tileMapQuadrant;

    fixedTileX = tileX - landDataMan->offsetTileX;
    fixedTileY = tileY - landDataMan->offsetTileY;
    mapMatrixX = fixedTileX / MAP_TILES_COUNT_X;
    mapMatrixY = fixedTileY / MAP_TILES_COUNT_Y;
    mapMatrixIndex = mapMatrixX + (mapMatrixY * landDataMan->mapMatrixWidth);

    if (mapMatrixIndex >= landDataMan->mapMatrixWidth * landDataMan->mapMatrixHeight) {
        if (Link_SetErrorState(1)) {
            return FALSE;
        }
    }

    GF_ASSERT(mapMatrixIndex < landDataMan->mapMatrixWidth * landDataMan->mapMatrixHeight);

    tileIndex = fixedTileX + (fixedTileY * landDataMan->mapMatrixWidthTiles);
    tileMapQuadrant = LandDataManager_CalculateMapQuadrantOfTile(tileIndex, landDataMan->mapMatrixWidthTiles);
    loadedMapsQuadrant = LandDataManager_GetRelativeLoadedMapsQuadrant(mapMatrixIndex, tileMapQuadrant, landDataMan);

    if (result != NULL) {
        *result = loadedMapsQuadrant;
    }

    if (loadedMapsQuadrant > 3) {
        return FALSE;
    } else {
        return TRUE;
    }
}

const BDHC *LandDataManager_GetLoadedMapBDHC(const LandDataManager *landDataMan, const u8 loadedMapIndex)
{
    return landDataMan->loadedMaps[loadedMapIndex]->bdhc;
}

u16 const *LandDataManager_GetLoadedMapTerrainAttributes(const LandDataManager *landDataMan, const u8 loadedMapIndex)
{
    return landDataMan->loadedMaps[loadedMapIndex]->terrainAttributes;
}

void LandDataManager_SetMapLoadedCallback(LandDataManager *landDataMan, MapLoadedCallback mapLoadedCb, void *cbUserData)
{
    landDataMan->mapLoadedCb = mapLoadedCb;
    landDataMan->mapLoadedCbUserData = cbUserData;
}

static void LandDataManager_KillLoadMapModel(SysTask *sysTask)
{
    MapModelLoaderTaskContext *ctx = SysTask_GetParam(sysTask);
    ctx->killLoadTask = TRUE;
}

static void LandDataManager_LazyLoadMapModelTask(SysTask *sysTask, void *sysTaskParam)
{
    MapModelLoaderTaskContext *ctx;
    BOOL textureBound;
    int bytesToRead;
    void *buffer;

    ctx = (MapModelLoaderTaskContext *)sysTaskParam;

    if (ctx->killLoadTask == TRUE) {
        ctx->currentSubTask = MAP_MODEL_LOADER_SUBTASK_END_TASK;
    }

    switch (ctx->currentSubTask) {
    case MAP_MODEL_LOADER_SUBTASK_FIRST_FILE_READ:
        ctx->bytesRead = 0;

        if (ctx->mapModelDataSize <= 0xE000) {
            bytesToRead = ctx->mapModelDataSize;
            ctx->currentSubTask = MAP_MODEL_LOADER_SUBTASK_BIND_TEXTURE;
        } else {
            bytesToRead = 0xE000;
            ctx->currentSubTask = MAP_MODEL_LOADER_SUBTASK_FILE_READ;
        }

        buffer = &((u8 *)*ctx->mapModelFile)[ctx->bytesRead];
        NARC_ReadFile(ctx->landDataNARC, bytesToRead, buffer);
        ctx->bytesRead += bytesToRead;
        break;
    case MAP_MODEL_LOADER_SUBTASK_FILE_READ:
        BOOL finishedReading;

        bytesToRead = ctx->mapModelDataSize - ctx->bytesRead;

        if (bytesToRead > 0xE000) {
            bytesToRead = 0xE000;
            finishedReading = FALSE;
        } else {
            finishedReading = TRUE;
        }

        buffer = &((u8 *)*ctx->mapModelFile)[ctx->bytesRead];
        NARC_ReadFile(ctx->landDataNARC, bytesToRead, buffer);

        if (finishedReading) {
            ctx->currentSubTask = MAP_MODEL_LOADER_SUBTASK_BIND_TEXTURE;
        } else {
            ctx->bytesRead += bytesToRead;
        }

        break;
    case MAP_MODEL_LOADER_SUBTASK_BIND_TEXTURE:
        if (ctx->mapTexture != NULL) {
            if (Easy3D_IsTextureUploadedToVRAM(ctx->mapTexture) == TRUE) {
                textureBound = Easy3D_BindTextureToResource(*ctx->mapModelFile, ctx->mapTexture);
            }
        }

        ctx->currentSubTask = MAP_MODEL_LOADER_SUBTASK_INIT_RENDER_OBJ;
        break;
    case MAP_MODEL_LOADER_SUBTASK_INIT_RENDER_OBJ:
        NNSG3dResMdlSet *mapModelSet = NNS_G3dGetMdlSet(*ctx->mapModelFile);
        NNSG3dResMdl *mapModel;

        GF_ASSERT(mapModelSet->dict.numEntry == 1);

        mapModel = NNS_G3dGetMdlByIdx(mapModelSet, 0);
        GF_ASSERT(mapModel);

        NNS_G3dRenderObjInit(ctx->mapRenderObj, mapModel);

        *ctx->loadedMapValid = TRUE;
        ctx->currentSubTask = MAP_MODEL_LOADER_SUBTASK_END_TASK;
        break;
    case MAP_MODEL_LOADER_SUBTASK_END_TASK:
        *ctx->loadTaskRunning = FALSE;
        Heap_FreeToHeap(sysTaskParam);
        SysTask_Done(sysTask);
        return;
    }
}

SysTask *LandDataManager_LazyLoadMapModel(NARC *landDataNARC, const int mapModelDataSize, NNSG3dRenderObj *mapRenderObj, NNSG3dResFileHeader **mapModelFile, NNSG3dResTex *mapTexture, BOOL *loadedMapValid, BOOL *loadTaskRunning)
{
    MapModelLoaderTaskContext *ctx = Heap_AllocFromHeapAtEnd(4, sizeof(MapModelLoaderTaskContext));

    ctx->landDataNARC = landDataNARC;
    ctx->mapModelDataSize = mapModelDataSize;
    ctx->mapRenderObj = mapRenderObj;
    ctx->mapModelFile = mapModelFile;
    ctx->mapTexture = mapTexture;
    ctx->currentSubTask = MAP_MODEL_LOADER_SUBTASK_FIRST_FILE_READ;
    ctx->loadedMapValid = loadedMapValid;
    *ctx->loadedMapValid = FALSE;
    ctx->loadTaskRunning = loadTaskRunning;
    ctx->killLoadTask = FALSE;

    return SysTask_Start(LandDataManager_LazyLoadMapModelTask, ctx, 1);
}

NNSG3dResMdl *LandDataManager_LoadMapModel(NARC *landDataNARC, const int mapModelFileSize, NNSG3dRenderObj *mapRenderObj, NNSG3dResFileHeader **mapModelFile, NNSG3dResTex *mapTexture)
{
    BOOL textureBound;
    NNSG3dResMdl *mapModel;

    NARC_ReadFile(landDataNARC, mapModelFileSize, *mapModelFile);

    if (mapTexture != NULL) {
        if (Easy3D_IsTextureUploadedToVRAM(mapTexture) == TRUE) {
            textureBound = Easy3D_BindTextureToResource(*mapModelFile, mapTexture);
            GF_ASSERT(textureBound);
        }
    }

    NNSG3dResMdlSet *mapResMdlSet = NNS_G3dGetMdlSet(*mapModelFile);

    GF_ASSERT(mapResMdlSet->dict.numEntry == 1);
    mapModel = NNS_G3dGetMdlByIdx(NNS_G3dGetMdlSet(*mapModelFile), 0);

    GF_ASSERT(mapModel);
    NNS_G3dRenderObjInit(mapRenderObj, mapModel);

    return mapModel;
}

NARC *LandDataManager_GetLandDataNARC(LandDataManager *landDataMan)
{
    return landDataMan->landDataNARC;
}

LandDataManager *LandDataManager_DistortionWorldNew(MapMatrix *mapMatrix, AreaDataManager *areaDataMan, NARC *landDataNARC)
{
    LandDataManager *landDataMan = Heap_AllocFromHeap(4, sizeof(LandDataManager));
    MI_CpuClear8(landDataMan, sizeof(LandDataManager));

    landDataMan->loadedMapBufs = LoadedMapBuffers_NewWithHeapMapModelFiles(0);
    landDataMan->areaDataMan = areaDataMan;
    landDataMan->mapMatrix = mapMatrix;

    if (mapMatrix != NULL) {
        landDataMan->mapMatrixWidth = MapMatrix_GetWidth(mapMatrix);
        landDataMan->mapMatrixHeight = MapMatrix_GetHeight(mapMatrix);
        landDataMan->mapMatrixWidthTiles = landDataMan->mapMatrixWidth * MAP_TILES_COUNT_X;
    }

    landDataMan->lazyLoaderTrackedObject.tracking = FALSE;
    LandDataManager_InitMapLazyLoader(landDataMan);
    landDataMan->ending = FALSE;
    landDataMan->lazyLoaderSubTask = LAZY_LOADER_SUBTASK_IDLE;
    landDataMan->landDataNARC = landDataNARC;

    if (landDataNARC == NULL) {
        landDataMan->landDataNARC = NARC_ctor(NARC_INDEX_FIELDDATA__LAND_DATA__LAND_DATA, 4);
    }

    return landDataMan;
}

void LandDataManager_DistortionWorldInit(LandDataManager *landDataMan, MapMatrix *mapMatrix, AreaDataManager *areaDataMan, NARC *landDataNARC)
{
    landDataMan->areaDataMan = areaDataMan;
    landDataMan->mapMatrix = mapMatrix;
    landDataMan->mapMatrixWidth = MapMatrix_GetWidth(mapMatrix);
    landDataMan->mapMatrixHeight = MapMatrix_GetHeight(mapMatrix);
    landDataMan->mapMatrixWidthTiles = landDataMan->mapMatrixWidth * MAP_TILES_COUNT_X;
    landDataMan->lazyLoaderTrackedObject.tracking = FALSE;

    LandDataManager_InitMapLazyLoader(landDataMan);

    landDataMan->ending = FALSE;
    landDataMan->lazyLoaderSubTask = LAZY_LOADER_SUBTASK_IDLE;
    landDataMan->landDataNARC = landDataNARC;

    if (landDataNARC == NULL) {
        landDataMan->landDataNARC = NARC_ctor(NARC_INDEX_FIELDDATA__LAND_DATA__LAND_DATA, 4);
    }
}

void LandDataManager_DistortionWorldEnd(LandDataManager *landDataMan)
{
    for (u8 i = 0; i < 4; i++) {
        landDataMan->loadedMaps[i]->valid = FALSE;

        if (landDataMan->loadedMaps[i]->mapPropManager != NULL) {
            MapPropManager_Free(landDataMan->loadedMaps[i]->mapPropManager);
        }

        Heap_FreeToHeap(landDataMan->loadedMaps[i]);
        landDataMan->loadedMaps[i] = NULL;
    }

    landDataMan->lazyLoaderSubTask = LAZY_LOADER_SUBTASK_GRACEFUL_SHUTDOWN;
    landDataMan->ending = TRUE;
    landDataMan->lazyLoaderTrackedObject.tracking = FALSE;
}

void LandDataManager_DistortionWorldInitLoadedMapPropManagers(LandDataManager *landDataMan)
{
    for (u8 i = 0; i < 4; i++) {
        if (landDataMan->loadedMaps[i]->mapPropManager != NULL) {
            MapPropManager_Init(landDataMan->loadedMaps[i]->mapPropManager);
        }
    }
}

void LandDataManager_DistortionWorldFreeLoadedMapBuffers(LandDataManager *landDataMan)
{
    LoadedMapBuffers_Free(landDataMan->loadedMapBufs);
    Heap_FreeToHeap(&landDataMan->loadedMapBufs);
}

void LandDataManager_DistortionWorldInitialLoad(LandDataManager *landDataMan, const int playerTileX, const int playerTileY)
{
    LandDataManager_DistortionWorldInitAndLoad(landDataMan, playerTileX, playerTileY, landDataMan->offsetTileX, landDataMan->offsetTileY, landDataMan->mapMatrixWidthTiles);
    LandDataManager_UpdateTrackedObjectValues(playerTileX, playerTileY, landDataMan->offsetTileX, landDataMan->offsetTileY, landDataMan);
}

static void LandDataManager_DistortionWorldInitAndLoad(LandDataManager *landDataMan, const int trackedObjectTileX, const int trackedObjectTileY, const int unused3, const int unused4, const int mapMatrixWidthTiles)
{
    u8 i;
    int loadedMapMatrixIndexes[4];

    LandDataManager_NewLoadedMapsWithoutCollisions(landDataMan);
    LandData_CalculateLoadedMapMatrixIndexes(landDataMan->inDistortionWorld, trackedObjectTileX, trackedObjectTileY, landDataMan->offsetTileX, landDataMan->offsetTileY, landDataMan->mapMatrixWidth, landDataMan->mapMatrixHeight, mapMatrixWidthTiles, loadedMapMatrixIndexes);

    for (i = 0; i < 4; i++) {
        landDataMan->loadedMaps[i]->bdhc = NULL;
        LandDataManager_DistortionWorldLoad(loadedMapMatrixIndexes[i], i, landDataMan->areaDataMan, landDataMan->mapMatrix, landDataMan->mapMatrixWidth, landDataMan->mapMatrixHeight, AreaDataManager_IsOutdoorsLighting(landDataMan->areaDataMan), landDataMan);
    }
}

void LandDataManager_DistortionWorldInitLoadedMaps(LandDataManager *landDataMan, const int trackedObjectTileX, const int trackedObjectTileY, int *loadedMapMatrixIndexes)
{
    int i;

    LandDataManager_InitLoadedMapsWithoutCollisions(landDataMan);
    LandData_CalculateLoadedMapMatrixIndexes(landDataMan->inDistortionWorld, trackedObjectTileX, trackedObjectTileY, landDataMan->offsetTileX, landDataMan->offsetTileY, landDataMan->mapMatrixWidth, landDataMan->mapMatrixHeight, landDataMan->mapMatrixWidthTiles, loadedMapMatrixIndexes);

    for (i = 0; i < 4; i++) {
        landDataMan->loadedMaps[i]->valid = FALSE;
    }
}

void LandDataManager_DistortionWorldLoadAndInvalidate(LandDataManager *landDataMan, int loadedMapIndex, int mapMatrixIndex)
{
    landDataMan->loadedMaps[loadedMapIndex]->bdhc = NULL;
    LandDataManager_DistortionWorldLoad(mapMatrixIndex, loadedMapIndex, landDataMan->areaDataMan, landDataMan->mapMatrix, landDataMan->mapMatrixWidth, landDataMan->mapMatrixHeight, AreaDataManager_IsOutdoorsLighting(landDataMan->areaDataMan), landDataMan);
    landDataMan->loadedMaps[loadedMapIndex]->valid = FALSE;
}

static void LandDataManager_DistortionWorldLoad(const int mapMatrixIndex, const u8 loadedMapIndex, const AreaDataManager *areaDataMan, const MapMatrix *mapMatrix, const int mapMatrixWidth, const int mapMatrixHeight, const BOOL isOutdoorsLighting, LandDataManager *landDataMan)
{
    int landDataID, bytesToSkip;
    LandDataHeader landDataHeader;
    NNSG3dResMdl *mapModel;

    if ((mapMatrixIndex < 0) || (mapMatrixIndex >= mapMatrixWidth * mapMatrixHeight)) {
        return;
    }

    landDataID = MapMatrix_GetLandDataIdByIndex(mapMatrixIndex, mapMatrix);
    LandDataHeader_Load(landDataMan->landDataNARC, landDataID, &landDataHeader);

    bytesToSkip = TERRAIN_ATTRIBUTES_SIZE + landDataHeader.mapPropsSize;
    NARC_Seek(landDataMan->landDataNARC, bytesToSkip);

    DC_FlushRange(landDataMan->loadedMaps[loadedMapIndex]->mapModelFile, landDataHeader.mapModelSize);
    mapModel = LandDataManager_LoadMapModel(landDataMan->landDataNARC, landDataHeader.mapModelSize, &landDataMan->loadedMaps[loadedMapIndex]->mapRenderObj, &(landDataMan->loadedMaps[loadedMapIndex]->mapModelFile), AreaDataManager_GetMapTexture(areaDataMan));

    if (isOutdoorsLighting == TRUE) {
        AreaLight_UseGlobalModelAttributes(mapModel);
    }

    landDataMan->loadedMaps[loadedMapIndex]->valid = TRUE;
    landDataMan->loadedMaps[loadedMapIndex]->mapMatrixIndex = mapMatrixIndex;
}

void LandDataManager_DistortionWorldRenderNextFloorMaps(const LandDataManager *landDataMan, const ModelAttributes *modelAttrs)
{
    for (int i = 0; i < 4; i++) {
        if (landDataMan->loadedMaps[i]->mapMatrixIndex != INVALID_MAP_MATRIX_INDEX) {
            LandDataManager_DistortionWorldRenderNextFloorMap(i, landDataMan, modelAttrs);
        }
    }
}

static void LandDataManager_DistortionWorldRenderNextFloorMap(const u8 index, const LandDataManager *landDataMan, const ModelAttributes *modelAttrs)
{
    VecFx32 position = { 0, 0, 0 };
    VecFx32 scale = { FX32_ONE, FX32_ONE, FX32_ONE };
    MtxFx33 rotationMatrix = { FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE };

    LandDataManager_CalculateRenderingPosition(landDataMan->loadedMaps[index]->mapMatrixIndex, landDataMan->mapMatrixWidth, landDataMan->mapMatrix, &position);

    position.x += landDataMan->offset.x;
    position.y += landDataMan->offset.y;
    position.z += landDataMan->offset.z;

    if (landDataMan->loadedMaps[index]->valid == TRUE) {
        Easy3D_DrawRenderObj(&(landDataMan->loadedMaps[index]->mapRenderObj), &position, &rotationMatrix, &scale);
    }
}

void LandDataManager_DistortionWorldEndWithoutFreeing(LandDataManager *landDataMan)
{
    int i;

    for (i = 0; i < 2; i++) {
        MapLazyLoader_KillTasks(&landDataMan->mapLazyLoaderMan[i].loader);
    }

    for (i = 0; i < 4; i++) {
        landDataMan->loadedMaps[i]->valid = FALSE;
    }

    landDataMan->lazyLoaderSubTask = LAZY_LOADER_SUBTASK_GRACEFUL_SHUTDOWN;
    landDataMan->ending = TRUE;
    landDataMan->lazyLoaderTrackedObject.tracking = FALSE;

    for (i = 0; i < 4; i++) {
        MI_CpuFillFast(landDataMan->loadedMaps[i]->terrainAttributes, INVALID_TERRAIN_ATTRIBUTES, sizeof(u16) * MAP_TILES_COUNT_X * MAP_TILES_COUNT_Y);
    }
}

void LandDataManager_DistortionWorldPrepareGoingToNextFloor(MapMatrix *mapMatrix, AreaDataManager *areaDataMan, LandDataManager *src, LandDataManager *dest, const int trackedObjectTileX, const int trackedObjectTileY)
{
    int i;
    int mapMatrixIndexes[4];

    LoadedMapBuffers_SwapMapModelData(src->loadedMapBufs, dest->loadedMapBufs);
    MapMatrix_Copy(mapMatrix, dest->mapMatrix);

    for (i = 0; i < 2; i++) {
        dest->mapLazyLoaderMan[i] = src->mapLazyLoaderMan[i];
    }

    for (i = 0; i < 4; i++) {
        dest->mapQuadrantsToCancelLazyLoad[i] = src->mapQuadrantsToCancelLazyLoad[i];
    }

    dest->runningLazyLoaderManCount = src->runningLazyLoaderManCount;
    dest->nextLazyLoaderManIndex = src->nextLazyLoaderManIndex;
    dest->activeLazyLoaderManIndex = src->activeLazyLoaderManIndex;
    dest->trackedObjectMapMatrixIndex = src->trackedObjectMapMatrixIndex;
    dest->trackedObjectTileIndex = src->trackedObjectTileIndex;
    dest->trackedObjectLoadedMapsQuadrant = src->trackedObjectLoadedMapsQuadrant;
    dest->trackedObjectMapQuadrant = src->trackedObjectMapQuadrant;

    for (i = 0; i < 4; i++) {
        dest->loadedMaps[i]->mapRenderObj = src->loadedMaps[i]->mapRenderObj;
        dest->loadedMaps[i]->mapModelFile = src->loadedMaps[i]->mapModelFile;
        dest->loadedMaps[i]->valid = src->loadedMaps[i]->valid;
        dest->loadedMaps[i]->mapMatrixIndex = src->loadedMaps[i]->mapMatrixIndex;
    }

    dest->areaDataMan = areaDataMan;
    dest->mapMatrixWidth = src->mapMatrixWidth;
    dest->mapMatrixHeight = src->mapMatrixHeight;
    dest->mapMatrixWidthTiles = src->mapMatrixWidth * MAP_TILES_COUNT_X;
    dest->lazyLoaderTrackedObject = src->lazyLoaderTrackedObject;
    dest->inDistortionWorld = src->inDistortionWorld;
    dest->offsetTileX = src->offsetTileX;
    dest->offsetAltitude = src->offsetAltitude;
    dest->offsetTileY = src->offsetTileY;
    dest->offset = src->offset;

    LandDataManager_UpdateTrackedObjectValues(trackedObjectTileX, trackedObjectTileY, dest->offsetTileX, dest->offsetTileY, dest);

    dest->ending = FALSE;
    dest->lazyLoaderSubTask = src->lazyLoaderSubTask;

    for (i = 0; i < 4; i++) {
        mapMatrixIndexes[i] = dest->loadedMaps[i]->mapMatrixIndex;
        BDHC_MarkNotLoaded(dest->loadedMaps[i]->bdhc);
    }

    for (i = 0; i < 4; i++) {
        LandDataManager_LoadWithoutModel(mapMatrixIndexes[i], i, dest->areaDataMan, dest->mapMatrix, dest->mapMatrixWidth, dest->mapMatrixHeight, AreaDataManager_IsOutdoorsLighting(dest->areaDataMan), dest);
    }
}

void LandDataManager_DistortionWorldPrepareGoingToPreviousFloor(LandDataManager *dest, LandDataManager *src)
{
    int i;

    LoadedMapBuffers_SwapMapModelData(src->loadedMapBufs, dest->loadedMapBufs);
    MapMatrix_Copy(src->mapMatrix, dest->mapMatrix);

    for (i = 0; i < 2; i++) {
        dest->mapLazyLoaderMan[i] = src->mapLazyLoaderMan[i];
    }

    for (i = 0; i < 4; i++) {
        dest->mapQuadrantsToCancelLazyLoad[i] = src->mapQuadrantsToCancelLazyLoad[i];
    }

    dest->runningLazyLoaderManCount = src->runningLazyLoaderManCount;
    dest->nextLazyLoaderManIndex = src->nextLazyLoaderManIndex;
    dest->activeLazyLoaderManIndex = src->activeLazyLoaderManIndex;
    dest->trackedObjectMapMatrixIndex = src->trackedObjectMapMatrixIndex;
    dest->trackedObjectTileIndex = src->trackedObjectTileIndex;
    dest->trackedObjectLoadedMapsQuadrant = src->trackedObjectLoadedMapsQuadrant;
    dest->trackedObjectMapQuadrant = src->trackedObjectMapQuadrant;

    for (i = 0; i < 4; i++) {
        dest->loadedMaps[i]->mapRenderObj = src->loadedMaps[i]->mapRenderObj;
        dest->loadedMaps[i]->mapModelFile = src->loadedMaps[i]->mapModelFile;
        dest->loadedMaps[i]->valid = src->loadedMaps[i]->valid;
        dest->loadedMaps[i]->mapMatrixIndex = src->loadedMaps[i]->mapMatrixIndex;
    }

    dest->areaDataMan = src->areaDataMan;
    dest->mapMatrixWidth = src->mapMatrixWidth;
    dest->mapMatrixHeight = src->mapMatrixHeight;
    dest->mapMatrixWidthTiles = src->mapMatrixWidth * MAP_TILES_COUNT_X;
    dest->lazyLoaderTrackedObject = src->lazyLoaderTrackedObject;
    dest->inDistortionWorld = src->inDistortionWorld;
    dest->offsetTileX = src->offsetTileX;
    dest->offsetAltitude = src->offsetAltitude;
    dest->offsetTileY = src->offsetTileY;
    dest->offset = src->offset;
    dest->ending = FALSE;
    dest->lazyLoaderSubTask = src->lazyLoaderSubTask;
}

void LandDataManager_DistortionWorldTick(FieldSystem *fieldSystem, LandDataManager *landDataMan)
{
    MapLazyLoader *activeLazyLoader;
    MapLazyLoaderTrackedObject *trackedObject;

    activeLazyLoader = &landDataMan->mapLazyLoaderMan[landDataMan->activeLazyLoaderManIndex].loader;
    trackedObject = &landDataMan->lazyLoaderTrackedObject;

    if (landDataMan->inDistortionWorld == FALSE && trackedObject->tracking == TRUE) {
        if (trackedObject->moving) {
            GF_ASSERT((*trackedObject->lowerMovementCoord) <= (*trackedObject->higherMovementCoord));

            if ((*trackedObject->higherMovementCoord) - (*trackedObject->lowerMovementCoord) >= MAP_OBJECT_MOVE_TILE_SIZE) {
                int trackedObjectTileX, trackedObjectTileY;

                LandData_ObjectPosToTilePos(trackedObject->position->x, trackedObject->position->z, &trackedObjectTileX, &trackedObjectTileY);
                LandDataManager_UpdateTrackedObjectValues(trackedObjectTileX, trackedObjectTileY, landDataMan->offsetTileX, landDataMan->offsetTileY, landDataMan);

                trackedObject->prevPosition = *trackedObject->position;

                if (!(trackedObject->prevPosition.z % (FX32_ONE * 8) == 0)) {
                    if (Link_SetErrorState(1)) {
                        return;
                    }
                }

                GF_ASSERT(trackedObject->prevPosition.z % (FX32_ONE * 8) == 0);

                trackedObject->moving = FALSE;
                trackedObject->higherMovementCoord = NULL;
                trackedObject->lowerMovementCoord = NULL;
            }
        } else {
            if (trackedObject->prevPosition.x != trackedObject->position->x && trackedObject->prevPosition.z != trackedObject->position->z) {
                GF_ASSERT(FALSE);
            } else if (trackedObject->prevPosition.x != trackedObject->position->x || trackedObject->prevPosition.z != trackedObject->position->z) {
                trackedObject->moving = TRUE;

                if (trackedObject->prevPosition.x != trackedObject->position->x) {
                    if (trackedObject->prevPosition.x > trackedObject->position->x) {
                        trackedObject->higherMovementCoord = &trackedObject->prevPosition.x;
                        trackedObject->lowerMovementCoord = &trackedObject->position->x;
                        trackedObject->movingDirection = DIRECTION_WEST;
                    } else {
                        trackedObject->higherMovementCoord = &trackedObject->position->x;
                        trackedObject->lowerMovementCoord = &trackedObject->prevPosition.x;
                        trackedObject->movingDirection = DIRECTION_EAST;
                    }

                    if (trackedObject->prevPosition.x % (FX32_ONE * 8) != 0) {
                        if (Link_SetErrorState(1)) {
                            return;
                        }
                    }

                    GF_ASSERT(trackedObject->prevPosition.x % (FX32_ONE * 8) == 0);
                } else {
                    if (landDataMan->lazyLoaderTrackedObject.prevPosition.z > landDataMan->lazyLoaderTrackedObject.position->z) {
                        trackedObject->higherMovementCoord = &trackedObject->prevPosition.z;
                        trackedObject->lowerMovementCoord = &trackedObject->position->z;
                        trackedObject->movingDirection = DIRECTION_NORTH;
                    } else {
                        trackedObject->higherMovementCoord = &trackedObject->position->z;
                        trackedObject->lowerMovementCoord = &trackedObject->prevPosition.z;
                        trackedObject->movingDirection = DIRECTION_SOUTH;
                    }

                    if (trackedObject->prevPosition.z % (FX32_ONE * 8) != 0) {
                        if (Link_SetErrorState(1)) {
                            return;
                        }
                    }

                    GF_ASSERT(trackedObject->prevPosition.z % (FX32_ONE * 8) == 0);
                }

                LandDataManager_CheckLazyLoadNextMapPair(trackedObject->movingDirection, landDataMan);
            }
        }
    }

    switch (landDataMan->lazyLoaderSubTask) {
    case LAZY_LOADER_SUBTASK_LOAD_MAP:
        if (landDataMan->mapQuadrantsToCancelLazyLoad[activeLazyLoader->quadrants[activeLazyLoader->nextIndexToLoad]] != TRUE) {
            LandDataManager_LazyLoadWithoutCollisionsAndProps(activeLazyLoader->nextIndexToLoad, landDataMan->areaDataMan, landDataMan->mapMatrix, landDataMan->mapMatrixWidth, landDataMan->mapMatrixHeight, landDataMan, activeLazyLoader);
        }

        activeLazyLoader->nextIndexToLoad++;
        landDataMan->lazyLoaderSubTask = LAZY_LOADER_SUBTASK_FINISH_MAP_LOAD;
        break;
    case LAZY_LOADER_SUBTASK_FINISH_MAP_LOAD:
        if (landDataMan->mapQuadrantsToCancelLazyLoad[activeLazyLoader->quadrants[activeLazyLoader->nextIndexToLoad - 1]] == TRUE) {
            MapLazyLoader_KillTasks(activeLazyLoader);
            landDataMan->mapQuadrantsToCancelLazyLoad[activeLazyLoader->quadrants[activeLazyLoader->nextIndexToLoad - 1]] = FALSE;
        }

        if (activeLazyLoader->nextIndexToLoad <= 2 && LandDataManager_LazyLoadFinished(&activeLazyLoader->mapLoadTasksState) == TRUE) {
            if (activeLazyLoader->nextIndexToLoad >= 2) {
                activeLazyLoader->available = TRUE;
            } else {
                landDataMan->lazyLoaderSubTask = LAZY_LOADER_SUBTASK_LOAD_MAP;
            }

            if (activeLazyLoader->loadSlots[activeLazyLoader->nextIndexToLoad - 1]->valid == TRUE) {
                NNSG3dResMdlSet *mapModelSet = NNS_G3dGetMdlSet(activeLazyLoader->loadSlots[activeLazyLoader->nextIndexToLoad - 1]->mapModelFile);
                NNSG3dResMdl *mapModel = NNS_G3dGetMdlByIdx(mapModelSet, 0);

                if (AreaDataManager_IsOutdoorsLighting(landDataMan->areaDataMan) == TRUE) {
                    AreaLight_UseGlobalModelAttributes(mapModel);
                }
            }

            if (landDataMan->mapLoadedCb != NULL) {
                const int loadedMapMatrixIndex = activeLazyLoader->loadSlots[activeLazyLoader->nextIndexToLoad - 1]->mapMatrixIndex;

                if (0 <= loadedMapMatrixIndex && loadedMapMatrixIndex < landDataMan->mapMatrixWidth * landDataMan->mapMatrixHeight) {
                    landDataMan->mapLoadedCb(landDataMan->mapLoadedCbUserData, activeLazyLoader->loadSlots[activeLazyLoader->nextIndexToLoad - 1]->mapMatrixIndex, activeLazyLoader->loadSlots[activeLazyLoader->nextIndexToLoad - 1]->mapPropManager);
                }
            }
        }
        break;
    case LAZY_LOADER_SUBTASK_IDLE:
        break;
    case LAZY_LOADER_SUBTASK_GRACEFUL_SHUTDOWN:
        if (LandDataManager_LazyLoadFinished(&activeLazyLoader->mapLoadTasksState) == TRUE) {
            landDataMan->runningLazyLoaderManCount = 0;
        }
        break;
    }

    if (landDataMan->runningLazyLoaderManCount == 0) {
        landDataMan->activeLazyLoaderManIndex = 0;
        landDataMan->nextLazyLoaderManIndex = 0;
        landDataMan->lazyLoaderSubTask = LAZY_LOADER_SUBTASK_IDLE;
        return;
    }

    if (LandDataManager_IsActiveLazyLoaderAvailable(landDataMan) == TRUE) {
        landDataMan->mapLazyLoaderMan[landDataMan->activeLazyLoaderManIndex].loading = FALSE;

        LandDataManager_ResetMapQuadrantsToCancelLazyLoad(landDataMan);

        landDataMan->runningLazyLoaderManCount--;
        landDataMan->activeLazyLoaderManIndex = (landDataMan->activeLazyLoaderManIndex + 1) % 2;

        if (landDataMan->runningLazyLoaderManCount != 0) {
            landDataMan->lazyLoaderSubTask = LAZY_LOADER_SUBTASK_LOAD_MAP;
        } else {
            landDataMan->activeLazyLoaderManIndex = 0;
            landDataMan->nextLazyLoaderManIndex = 0;
            landDataMan->lazyLoaderSubTask = LAZY_LOADER_SUBTASK_IDLE;
        }

        if (landDataMan->lazyLoaderPending.pending == TRUE) {
            landDataMan->lazyLoaderPending.pending = FALSE;

            LandDataManager_CancelMapQuadrantsToLazyLoad(landDataMan->lazyLoaderPending.trackedObjectDirection, landDataMan);
            LandDataManager_QueueLazyLoadPendingMaps(landDataMan);
        }
    }
}

void LandDataManager_DistortionWorldInitWithoutNARC(LandDataManager *landDataMan, MapMatrix *mapMatrix, AreaDataManager *areaDataMan)
{
    landDataMan->areaDataMan = areaDataMan;
    landDataMan->mapMatrix = mapMatrix;
    landDataMan->mapMatrixWidth = MapMatrix_GetWidth(mapMatrix);
    landDataMan->mapMatrixHeight = MapMatrix_GetHeight(mapMatrix);
    landDataMan->mapMatrixWidthTiles = landDataMan->mapMatrixWidth * MAP_TILES_COUNT_X;

    LandDataManager_InitMapLazyLoader(landDataMan);

    landDataMan->ending = FALSE;
    landDataMan->lazyLoaderSubTask = LAZY_LOADER_SUBTASK_IDLE;
}

void LandDataManager_DistortionWorldInvalidateLoadedMaps(LandDataManager *landDataMan, const int trackedObjectTileX, const int trackedObjectTileY, int *loadedMapMatrixIndexes)
{
    int i;

    LandData_CalculateLoadedMapMatrixIndexes(landDataMan->inDistortionWorld, trackedObjectTileX, trackedObjectTileY, landDataMan->offsetTileX, landDataMan->offsetTileY, landDataMan->mapMatrixWidth, landDataMan->mapMatrixHeight, landDataMan->mapMatrixWidthTiles, loadedMapMatrixIndexes);

    for (i = 0; i < 4; i++) {
        landDataMan->loadedMaps[i]->valid = FALSE;
    }
}

void LandDataManager_DistortionWorldLoadEntire(LandDataManager *landDataMan, int loadedMapIndex, int mapMatrixIndex)
{
    landDataMan->loadedMaps[loadedMapIndex]->valid = FALSE;
    LoadedMapBuffers_GetMapModelFileBufPtr(loadedMapIndex, landDataMan->loadedMapBufs, (void **)&(landDataMan->loadedMaps[loadedMapIndex]->mapModelFile));
    LoadedMapBuffers_GetBDHCFileBufPtr(loadedMapIndex, landDataMan->loadedMapBufs, (void **)&(landDataMan->loadedMaps[loadedMapIndex]->bdhcFile));
    landDataMan->loadedMaps[loadedMapIndex]->mapMatrixIndex = INVALID_MAP_MATRIX_INDEX;

    MI_CpuFillFast(landDataMan->loadedMaps[loadedMapIndex]->terrainAttributes, INVALID_TERRAIN_ATTRIBUTES, sizeof(u16) * MAP_TILES_COUNT_X * MAP_TILES_COUNT_Y);

    BDHC_MarkNotLoaded(landDataMan->loadedMaps[loadedMapIndex]->bdhc);
    LandDataManager_Load(mapMatrixIndex, loadedMapIndex, landDataMan->areaDataMan, landDataMan->mapMatrix, landDataMan->mapMatrixWidth, landDataMan->mapMatrixHeight, AreaDataManager_IsOutdoorsLighting(landDataMan->areaDataMan), landDataMan);
}

void LandDataManager_DistortionWorldSetOffsets(LandDataManager *landDataMan, int offsetTileX, int offsetAltitude, int offsetTileY)
{
    landDataMan->offsetTileX = offsetTileX;
    landDataMan->offsetAltitude = offsetAltitude;
    landDataMan->offsetTileY = offsetTileY;
    landDataMan->offset.x = offsetTileX * MAP_OBJECT_MOVE_TILE_SIZE;
    landDataMan->offset.y = offsetAltitude * MAP_OBJECT_MOVE_TILE_SIZE;
    landDataMan->offset.z = offsetTileY * MAP_OBJECT_MOVE_TILE_SIZE;
}

void LandDataManager_SetInDistortionWorld(LandDataManager *landDataMan, BOOL inDistortionWorld)
{
    landDataMan->inDistortionWorld = inDistortionWorld;
}

int LandDataManager_GetOffsetTileX(const LandDataManager *landDataMan)
{
    return landDataMan->offsetTileX;
}

int LandDataManager_GetOffsetTileY(const LandDataManager *landDataMan)
{
    return landDataMan->offsetTileY;
}

void LandDataManager_GetOffset(const LandDataManager *landDataMan, VecFx32 *offset)
{
    *offset = landDataMan->offset;
}

void LandDataManager_SetSkipMapProps(LandDataManager *landDataMan, BOOL skipMapProps)
{
    landDataMan->skipMapProps = skipMapProps;
}

void LandDataManager_SetMapMatrix(LandDataManager *landDataMan, MapMatrix *mapMatrix)
{
    landDataMan->mapMatrix = mapMatrix;
}

void LandDataManager_SetLoadedMapValid(LandDataManager *landDataMan, int index, BOOL valid)
{
    landDataMan->loadedMaps[index]->valid = valid;
}

void LandDataManager_DistortionWorldUpdateTrackedObjectValues(LandDataManager *landDataMan, int trackedObjectTileX, int trackedObjectTileY)
{
    LandDataManager_UpdateTrackedObjectValues(trackedObjectTileX, trackedObjectTileY, landDataMan->offsetTileX, landDataMan->offsetTileY, landDataMan);
}
