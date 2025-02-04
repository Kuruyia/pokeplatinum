#include "overlay005/ov5_021E779C.h"

#include <nitro.h>
#include <string.h>

#include "constants/heap.h"

#include "field/field_system.h"
#include "overlay005/area_data.h"
#include "overlay005/bdhc.h"
#include "overlay005/funcptr_ov5_021E9630.h"
#include "overlay005/map_prop.h"
#include "overlay005/ov5_021D521C.h"
#include "overlay005/ov5_021EEAC8.h"
#include "overlay005/struct_ov5_021D3CAC_decl.h"
#include "overlay005/struct_ov5_021D5894.h"
#include "overlay005/struct_ov5_021EEB48_decl.h"

#include "easy3d.h"
#include "heap.h"
#include "map_matrix.h"
#include "narc.h"
#include "sys_task.h"
#include "sys_task_manager.h"
#include "system.h"
#include "unk_020366A0.h"

typedef struct {
    int unk_00;
    int unk_04;
} UnkStruct_ov5_021E80BC;

typedef struct {
    u16 movementPermissions[1024];
    NNSG3dRenderObj mapRenderObj;
    NNSG3dResFileHeader *nsbmdFile;
    u8 *bdhcBuffer;
    BDHC *bdhc;
    int mapMatrixIndex;
    BOOL present;
    MapPropManager *mapPropManager;
} LoadedMap;

typedef struct {
    LoadedMap *unk_00[2];
    SysTask *unk_08;
    SysTask *unk_0C;
    UnkStruct_ov5_021E80BC unk_10;
    int unk_18[2];
    u8 unk_20[2];
    u8 unk_22;
    BOOL unk_24;
} UnkStruct_ov5_021E7814;

typedef void (*UnkFuncPtr_ov5_021FAE98)(UnkStruct_ov5_021E8F60 *, const int, const int, const int, const int, const int);
typedef void (*UnkFuncPtr_ov5_021FAE98_1)(const u8, AreaDataManager *const, const MapMatrix *, const int, const int, UnkStruct_ov5_021E8F60 *, UnkStruct_ov5_021E7814 *);

typedef struct UnkStruct_ov5_021FAE98_t {
    UnkFuncPtr_ov5_021FAE98 unk_00;
    UnkFuncPtr_ov5_021FAE98_1 unk_04;
} UnkStruct_ov5_021FAE98;

typedef struct {
    UnkStruct_ov5_021E7814 unk_00;
    BOOL unk_28;
    u8 unk_2C;
} UnkStruct_ov5_021E8F60_sub1;

typedef struct {
    LoadedMap *unk_00[2];
    BOOL unk_08;
    u8 unk_0C;
    int unk_10[2];
    u8 unk_18[2];
} UnkStruct_ov5_021E8C8C;

typedef struct {
    VecFx32 unk_00;
    VecFx32 const *unk_0C;
    fx32 const *unk_10;
    fx32 const *unk_14;
    BOOL unk_18;
    u8 unk_1C;
    BOOL unk_20;
} UnkStruct_ov5_021EA174;

typedef struct UnkStruct_ov5_021E8F60_t {
    UnkStruct_ov5_021EEB48 *unk_00;
    UnkStruct_ov5_021E8F60_sub1 unk_04[2];
    UnkStruct_ov5_021E8C8C unk_5C;
    BOOL unk_74[4];
    LoadedMap *loadedMaps[4];
    u8 unk_94;
    u8 unk_95;
    u8 unk_96;
    u32 unk_98;
    u32 unk_9C;
    u8 unk_A0;
    u8 unk_A1;
    BOOL unk_A4;
    int unk_A8;
    AreaDataManager *unk_AC;
    MapMatrix *mapMatrix;
    int mapMatrixWidth;
    int mapMatrixHeight;
    int mapTilesWidth;
    UnkStruct_ov5_021EA174 unk_C0;
    UnkStruct_ov5_021D3CAC *unk_E4;
    UnkStruct_ov5_021FAE98 const *unk_E8;
    NARC *landDataNARC;
    UnkFuncPtr_ov5_021E9630 unk_F0;
    void *unk_F4;
    int unk_F8;
    int unk_FC;
    int unk_100;
    int unk_104;
    int unk_108;
    VecFx32 unk_10C;
} UnkStruct_ov5_021E8F60;

typedef struct {
    int sectionSizeMovementPermission;
    int sectionSizeNSBMD;
    int sectionSizeBDHC;
    int sectionSize3DObject;
} LandDataHeader;

typedef struct {
    NARC *landDataNARC;
    int unk_04;
    NNSG3dRenderObj *unk_08;
    NNSG3dResFileHeader **unk_0C;
    NNSG3dResTex *unk_10;
    u8 unk_14;
    BOOL *unk_18;
    BOOL unk_1C;
    int *unk_20;
    u32 unk_24;
} UnkStruct_ov5_021E9640;

static SysTask *ov5_021E976C(NARC *param0, const int param1, NNSG3dRenderObj *param2, NNSG3dResFileHeader **param3, NNSG3dResTex *param4, BOOL *param5, int *param6);
static NNSG3dResMdl *ov5_021E97AC_LoadNSBMD(NARC *param0, const int param1, NNSG3dRenderObj *param2, NNSG3dResFileHeader **param3, NNSG3dResTex *param4);
static void ov5_021E9640(SysTask *param0);
static void ov5_021E8F60(UnkStruct_ov5_021E8F60 *param0);
static void ov5_021E8E28(UnkStruct_ov5_021E8F60 *param0, const int param1, const int param2, const int param3, const int param4, const int param5);
static void ov5_021E8ECC(UnkStruct_ov5_021E8F60 *param0, const int param1, const int param2, const int param3, const int param4, const int param5);
static void UnkStruct_ov5_021E8F60_Init(UnkStruct_ov5_021E8F60 *param0);
static void ov5_021E7AC4(UnkStruct_ov5_021E8F60 *param0);
static void ov5_021E77E4(UnkStruct_ov5_021E8F60 *param0, const u8 param1);
static BOOL ov5_021E80D0(UnkStruct_ov5_021E8F60 *param0, const int param1, const int param2, const u8 param3, const u8 param4, const u8 param5, const u8 param6);
static void LandDataHeader_Load(NARC *param0, const int param1, LandDataHeader *param2);
static void ov5_021E7C00(const u8 param0, AreaDataManager *const param1, const MapMatrix *param2, const int param3, const int param4, UnkStruct_ov5_021E8F60 *param5, UnkStruct_ov5_021E7814 *param6);
static void ov5_021E7CD4(const u8 param0, AreaDataManager *const param1, const MapMatrix *param2, const int param3, const int param4, UnkStruct_ov5_021E8F60 *param5, UnkStruct_ov5_021E7814 *param6);
static void ov5_021E7E28_LoadLandData(const int param0, const u8 param1, AreaDataManager *const param2, const MapMatrix *param3, const int param4, const int param5, const BOOL param6, const UnkStruct_ov5_021E8F60 *param7);
static void ov5_021E7F1C(const int param0, const u8 param1, AreaDataManager *const param2, const MapMatrix *param3, const int param4, const int param5, const BOOL param6, UnkStruct_ov5_021E8F60 *param7);
static void ov5_021E7FF0(const int param0, const u8 param1, AreaDataManager *const param2, const MapMatrix *param3, const int param4, const int param5, const BOOL param6, UnkStruct_ov5_021E8F60 *param7);
static void ov5_021E86A0(const int param0, const u8 param1, UnkStruct_ov5_021E8F60 *param2);
static void ov5_021E8668(const u8 param0, const u8 param1, const u8 param2, const u8 param3, UnkStruct_ov5_021E8F60 *param4);
static void ov5_021E8614(const u8 param0, UnkStruct_ov5_021E8F60 *param1);
static void ov5_021E89D0(const u8 param0, const UnkStruct_ov5_021E8F60 *param1, UnkStruct_ov5_021D5894 *const param2);
static BOOL ov5_021E8C58(const u8 param0, const UnkStruct_ov5_021E8F60 *param1, u8 *param2);
static BOOL ov5_021E8C8C(UnkStruct_ov5_021E8F60 *param0);
static BOOL ov5_021E8DAC(const UnkStruct_ov5_021E8F60 *param0);
static void ov5_021E8DBC(UnkStruct_ov5_021E8F60 *param0);
static void ov5_021E8DD4(const u8 param0, UnkStruct_ov5_021E8F60 *param1);
static u8 ov5_021E8AF0(const u8 param0);
static u32 ov5_021E8B1C(const u8 param0, const int param1, const int param2, const int param3);
static u8 ov5_021E8B84(const u8 param0, const int param1);
static void ov5_021E8BC0(const u8 param0, const int param1, u8 *param2);
static void ov5_021E8558(const int param0, const int param1, const u8 param2, const u8 param3, const u8 param4, UnkStruct_ov5_021E8F60 *param5);
static int ov5_021E8ABC_CalculatePlayerTile(const int param0, const int param1, const int param2, const int param3, const int param4);
static void ov5_021E8D50(UnkStruct_ov5_021E8F60 *param0);
static void ov5_021E8F90(const int param0, const int param1, const int param2, const int param3, UnkStruct_ov5_021E8F60 *param4);
static void ov5_021E901C(const int param0, const int param1, const MapMatrix *param2, VecFx32 *param3);
static void ov5_021E9A14(UnkStruct_ov5_021E8F60 *param0, const int param1, const int param2, const int param3, const int param4, const int param5);
static void ov5_021E9B70(const int param0, const u8 param1, const AreaDataManager *param2, const MapMatrix *param3, const int param4, const int param5, const BOOL param6, UnkStruct_ov5_021E8F60 *param7);
static void ov5_021E9C40(const u8 param0, const UnkStruct_ov5_021E8F60 *param1, const UnkStruct_ov5_021D5894 *param2);

static const UnkStruct_ov5_021FAE98 Unk_ov5_021FAE98 = {
    ov5_021E8E28,
    ov5_021E7C00
};

static const UnkStruct_ov5_021FAE98 Unk_ov5_021FAE90 = {
    ov5_021E8ECC,
    ov5_021E7CD4
};

static BOOL ov5_021E779C(const int param0, const int param1, const int param2)
{
    if (param0 % param2 != param1 % param2) {
        return 0;
    } else {
        return 1;
    }
}

static BOOL ov5_021E77C0(const int param0, const int param1, const int param2)
{
    if (param0 / param2 != param1 / param2) {
        return 0;
    } else {
        return 1;
    }
}

static void ov5_021E77E4(UnkStruct_ov5_021E8F60 *param0, const u8 param1)
{
    u8 v0;
    u8 v1;

    if (param0->unk_04[param1].unk_00.unk_10.unk_00 != 0) {
        ov5_021E9640(param0->unk_04[param1].unk_00.unk_08);
    }

    if (param0->unk_04[param1].unk_00.unk_10.unk_04 != 0) {
        BDHC_KillLoad(param0->unk_04[param1].unk_00.unk_0C);
    }

    param0->unk_04[param1].unk_00.unk_24 = 1;
}

static void ov5_021E7814(UnkStruct_ov5_021E7814 *param0)
{
    if (param0->unk_10.unk_00 != 0) {
        ov5_021E9640(param0->unk_08);
    }

    if (param0->unk_10.unk_04 != 0) {
        BDHC_KillLoad(param0->unk_0C);
    }

    param0->unk_10.unk_00 = 0;
    param0->unk_10.unk_04 = 0;
}

static void ov5_021E7838_CalculateLoadedMapMatrixIndexes(const int param0, const int playerTileX, const int playerTileY, const int offsetTileX, const int offsetTileY, const int mapMatrixWidth, const int mapMatrixHeight, const int mapTilesWidth, int *loadedMapMatrixIndexes)
{
    int playerTile;
    u8 playerLoadedMapIndex;
    int mapMatrixIndex;
    int mapMatrixX, mapMatrixY;

    if (param0 == 1) {
        int i;

        for (i = 0; i < 4; i++) {
            loadedMapMatrixIndexes[i] = -1;
        }

        if (mapMatrixWidth == 1) {
            if (mapMatrixHeight == 1) {
                // 1x1 map
                // loadedMapMatrixIndexes = [ 0 -1]
                //                          [-1 -1]
                loadedMapMatrixIndexes[0] = 0;
            } else {
                // 1xH map
                // loadedMapMatrixIndexes = [ 0 -1]
                //                          [ 1 -1]
                loadedMapMatrixIndexes[0] = 0;
                loadedMapMatrixIndexes[2] = 1;
            }
        } else {
            // loadedMapMatrixIndexes = [ 0  1]
            //                          [-1 -1]
            loadedMapMatrixIndexes[0] = 0;
            loadedMapMatrixIndexes[1] = 1;

            if (mapMatrixWidth >= 2) {
                // loadedMapMatrixIndexes = [ 0  1]
                //                          [ 2  3]
                loadedMapMatrixIndexes[2] = 2;
                loadedMapMatrixIndexes[3] = 3;
            }
        }

        return;
    }

    GF_ASSERT(playerTileX >= offsetTileX);
    GF_ASSERT(playerTileY >= offsetTileY);

    mapMatrixX = (playerTileX - offsetTileX) / 32;
    mapMatrixY = (playerTileY - offsetTileY) / 32;
    playerTile = ov5_021E8ABC_CalculatePlayerTile(playerTileX, playerTileY, offsetTileX, offsetTileY, mapTilesWidth);
    playerLoadedMapIndex = ov5_021E935C_CalculateLoadedMapIndexForPlayer(playerTile, mapTilesWidth);
    mapMatrixIndex = (mapMatrixY * mapMatrixWidth) + mapMatrixX;

    switch (playerLoadedMapIndex) {
    case 0:
        // Player is on the bottom-right map of the loaded maps
        loadedMapMatrixIndexes[3] = mapMatrixIndex;
        loadedMapMatrixIndexes[0] = mapMatrixIndex - mapMatrixWidth - 1;
        loadedMapMatrixIndexes[1] = mapMatrixIndex - mapMatrixWidth;
        loadedMapMatrixIndexes[2] = mapMatrixIndex - 1;

        if (mapMatrixX - 1 < 0) {
            loadedMapMatrixIndexes[0] = -1;
            loadedMapMatrixIndexes[2] = -1;
        }

        if (mapMatrixY - 1 < 0) {
            loadedMapMatrixIndexes[1] = -1;
        }
        break;
    case 1:
        // Player is on the bottom-left map of the loaded maps
        loadedMapMatrixIndexes[2] = mapMatrixIndex;
        loadedMapMatrixIndexes[0] = mapMatrixIndex - mapMatrixWidth;
        loadedMapMatrixIndexes[1] = mapMatrixIndex - mapMatrixWidth + 1;
        loadedMapMatrixIndexes[3] = mapMatrixIndex + 1;

        if (mapMatrixX + 1 >= mapMatrixWidth) {
            loadedMapMatrixIndexes[1] = -1;
            loadedMapMatrixIndexes[3] = -1;
        }

        if (mapMatrixY - 1 < 0) {
            loadedMapMatrixIndexes[0] = -1;
        }
        break;
    case 2:
        // Player is on the top-right map of the loaded maps
        loadedMapMatrixIndexes[1] = mapMatrixIndex;
        loadedMapMatrixIndexes[0] = mapMatrixIndex - 1;
        loadedMapMatrixIndexes[2] = mapMatrixIndex + mapMatrixWidth - 1;
        loadedMapMatrixIndexes[3] = mapMatrixIndex + mapMatrixWidth;

        if (mapMatrixX - 1 < 0) {
            loadedMapMatrixIndexes[0] = -1;
            loadedMapMatrixIndexes[2] = -1;
        }

        if (mapMatrixY + 1 > mapMatrixHeight) {
            loadedMapMatrixIndexes[3] = -1;
        }
        break;
    case 3:
        // Player is on the top-left map of the loaded maps
        loadedMapMatrixIndexes[0] = mapMatrixIndex;
        loadedMapMatrixIndexes[1] = mapMatrixIndex + 1;
        loadedMapMatrixIndexes[2] = mapMatrixIndex + mapMatrixWidth;
        loadedMapMatrixIndexes[3] = mapMatrixIndex + mapMatrixWidth + 1;

        if (mapMatrixX + 1 >= mapMatrixWidth) {
            loadedMapMatrixIndexes[1] = -1;
            loadedMapMatrixIndexes[3] = -1;
        }

        if (mapMatrixY + 1 > mapMatrixHeight) {
            loadedMapMatrixIndexes[0] = -1;
        }
        break;
    default:
        GF_ASSERT(FALSE);
    }
}

static void UnkStruct_ov5_021E8F60_Init(UnkStruct_ov5_021E8F60 *param0)
{
    u8 i;

    for (i = 0; i < 4; i++) {
        LoadedMap *v1 = Heap_AllocFromHeap(4, sizeof(LoadedMap));
        param0->loadedMaps[i] = v1;
        param0->loadedMaps[i]->present = 0;

        ov5_021EEB84(i, param0->unk_00, (void **)&(param0->loadedMaps[i]->nsbmdFile));
        ov5_021EEB90(i, param0->unk_00, (void **)&(param0->loadedMaps[i]->bdhcBuffer));

        if (param0->unk_FC == 0) {
            param0->loadedMaps[i]->mapPropManager = MapPropManager_New(HEAP_ID_FIELD);
        } else {
            param0->loadedMaps[i]->mapPropManager = NULL;
        }

        param0->loadedMaps[i]->mapMatrixIndex = -1;
        MI_CpuFillFast(param0->loadedMaps[i]->movementPermissions, 0xffffffff, 2 * 32 * 32);
    }
}

void ov5_021E7A54(UnkStruct_ov5_021E8F60 *param0)
{
    u8 v0;

    for (v0 = 0; v0 < 4; v0++) {
        LoadedMap *v1 = NULL;

        v1 = Heap_AllocFromHeap(4, sizeof(LoadedMap));

        param0->loadedMaps[v0] = v1;
        param0->loadedMaps[v0]->present = 0;

        if (param0->unk_FC == 0) {
            param0->loadedMaps[v0]->mapPropManager = MapPropManager_New(HEAP_ID_FIELD);
        } else {
            param0->loadedMaps[v0]->mapPropManager = NULL;
        }

        param0->loadedMaps[v0]->mapMatrixIndex = -1;
    }
}

static void ov5_021E7AC4(UnkStruct_ov5_021E8F60 *param0)
{
    u8 v0;

    for (v0 = 0; v0 < 4; v0++) {
        LoadedMap *v1 = NULL;

        v1 = Heap_AllocFromHeap(4, sizeof(LoadedMap));

        param0->loadedMaps[v0] = v1;
        param0->loadedMaps[v0]->present = 0;

        ov5_021EEB84(v0, param0->unk_00, (void **)&(param0->loadedMaps[v0]->nsbmdFile));

        if (param0->unk_FC == 0) {
            param0->loadedMaps[v0]->mapPropManager = MapPropManager_New(HEAP_ID_FIELD);
        } else {
            param0->loadedMaps[v0]->mapPropManager = NULL;
        }

        param0->loadedMaps[v0]->mapMatrixIndex = -1;
    }
}

static void ov5_021E7B4C(UnkStruct_ov5_021E8F60 *param0)
{
    u8 v0;

    for (v0 = 0; v0 < 4; v0++) {
        param0->loadedMaps[v0]->present = 0;
        ov5_021EEB84(v0, param0->unk_00, (void **)&(param0->loadedMaps[v0]->nsbmdFile));

        if (param0->loadedMaps[v0]->mapPropManager != NULL) {
            MapPropManager_Init(param0->loadedMaps[v0]->mapPropManager);
        }

        param0->loadedMaps[v0]->mapMatrixIndex = -1;
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

    landDataHeader->sectionSizeMovementPermission = *(iter++);
    landDataHeader->sectionSize3DObject = *(iter++);
    landDataHeader->sectionSizeNSBMD = *(iter++);
    landDataHeader->sectionSizeBDHC = *iter;

    Heap_FreeToHeap(buffer);
}

static void ov5_021E7C00(const u8 param0, AreaDataManager *const param1, const MapMatrix *param2, const int param3, const int param4, UnkStruct_ov5_021E8F60 *param5, UnkStruct_ov5_021E7814 *param6)
{
    int v0;
    u8 v1;
    int landDataId;
    LandDataHeader landDataHeader;

    v0 = param6->unk_18[param0];

    if ((v0 < 0) || (v0 >= param3 * param4)) {
        return;
    }

    landDataId = MapMatrix_GetLandDataIdByIndex(v0, param2);

    if (landDataId == 0xffff) {
        return;
    }

    LandDataHeader_Load(param5->landDataNARC, landDataId, &landDataHeader);

    {
        void *v4;

        v4 = &(param6->unk_00[param0]->movementPermissions[0]);
        NARC_ReadFile(param5->landDataNARC, landDataHeader.sectionSizeMovementPermission, v4);
    }

    if (param6->unk_00[param0]->mapPropManager != NULL) {
        MapPropManager_Load(param5->landDataNARC, landDataHeader.sectionSize3DObject, param1, param6->unk_00[param0]->mapPropManager, param5->unk_E4);
    }

    {
        param6->unk_10.unk_00++;
        param6->unk_08 = ov5_021E976C(param5->landDataNARC, landDataHeader.sectionSizeNSBMD, &param6->unk_00[param0]->mapRenderObj, &(param6->unk_00[param0]->nsbmdFile), AreaDataManager_GetMapTexture(param1), &(param6->unk_00[param0]->present), &param6->unk_10.unk_00);
    }

    {
        param6->unk_10.unk_04++;
        param6->unk_0C = BDHC_LazyLoad(param5->landDataNARC, landDataHeader.sectionSizeBDHC, param6->unk_00[param0]->bdhc, &param6->unk_10.unk_04, &param6->unk_00[param0]->bdhcBuffer, &param6->unk_10.unk_00);
    }
}

static void ov5_021E7CD4(const u8 param0, AreaDataManager *const param1, const MapMatrix *param2, const int param3, const int param4, UnkStruct_ov5_021E8F60 *param5, UnkStruct_ov5_021E7814 *param6)
{
    int v0;
    u8 v1;
    int landDataId;
    LandDataHeader landDataHeader;

    v0 = param6->unk_18[param0];

    if ((v0 < 0) || (v0 >= param3 * param4)) {
        return;
    }

    landDataId = MapMatrix_GetLandDataIdByIndex(v0, param2);

    if (landDataId == 0xffff) {
        return;
    }

    LandDataHeader_Load(param5->landDataNARC, landDataId, &landDataHeader);

    {
        u8 v4[0x800];
        NARC_ReadFile(param5->landDataNARC, 0x800, v4);
    }

    if (param6->unk_00[param0]->mapPropManager != NULL) {
        MapPropManager_Load(param5->landDataNARC, landDataHeader.sectionSize3DObject, param1, param6->unk_00[param0]->mapPropManager, param5->unk_E4);
    }

    {
        param6->unk_10.unk_00++;
        param6->unk_08 = ov5_021E976C(param5->landDataNARC, landDataHeader.sectionSizeNSBMD, &param6->unk_00[param0]->mapRenderObj, &(param6->unk_00[param0]->nsbmdFile), AreaDataManager_GetMapTexture(param1), &(param6->unk_00[param0]->present), &param6->unk_10.unk_00);
    }
}

static void ov5_021E7D98(const u8 param0, const AreaDataManager *param1, const MapMatrix *param2, const int param3, const int param4, UnkStruct_ov5_021E8F60 *param5, UnkStruct_ov5_021E7814 *param6)
{
    int v0;
    u8 v1;
    int landDataId;
    LandDataHeader landDataHeader;

    v0 = param6->unk_18[param0];

    if ((v0 < 0) || (v0 >= param3 * param4)) {
        return;
    }

    landDataId = MapMatrix_GetLandDataIdByIndex(v0, param2);

    if (landDataId == 0xffff) {
        return;
    }

    LandDataHeader_Load(param5->landDataNARC, landDataId, &landDataHeader);

    NARC_Seek(param5->landDataNARC, 0x800);
    NARC_Seek(param5->landDataNARC, landDataHeader.sectionSize3DObject);

    {
        param6->unk_10.unk_00++;
        param6->unk_08 = ov5_021E976C(param5->landDataNARC, landDataHeader.sectionSizeNSBMD, &param6->unk_00[param0]->mapRenderObj, &(param6->unk_00[param0]->nsbmdFile), AreaDataManager_GetMapTexture(param1), &(param6->unk_00[param0]->present), &param6->unk_10.unk_00);
    }
}

static void ov5_021E7E28_LoadLandData(const int mapMatrixIndex, const u8 loadedMapIndex, AreaDataManager *const param2, const MapMatrix *mapMatrix, const int mapMatrixWidth, const int mapMatrixHeight, const BOOL param6, const UnkStruct_ov5_021E8F60 *param7)
{
    int landDataId;
    NNSG3dResMdl *mapResMdl;
    LandDataHeader landDataHeader;

    if ((mapMatrixIndex < 0) || (mapMatrixIndex >= mapMatrixWidth * mapMatrixHeight)) {
        return;
    }

    landDataId = MapMatrix_GetLandDataIdByIndex(mapMatrixIndex, mapMatrix);

    if (landDataId == 0xFFFF) {
        return;
    }

    LandDataHeader_Load(param7->landDataNARC, landDataId, &landDataHeader);

    void *movementPermissions = param7->loadedMaps[loadedMapIndex]->movementPermissions;
    NARC_ReadFile(param7->landDataNARC, landDataHeader.sectionSizeMovementPermission, movementPermissions);

    if (param7->loadedMaps[loadedMapIndex]->mapPropManager != NULL) {
        MapPropManager_Load(param7->landDataNARC, landDataHeader.sectionSize3DObject, param2, param7->loadedMaps[loadedMapIndex]->mapPropManager, param7->unk_E4);
    }

    mapResMdl = ov5_021E97AC_LoadNSBMD(param7->landDataNARC, landDataHeader.sectionSizeNSBMD, &param7->loadedMaps[loadedMapIndex]->mapRenderObj, &(param7->loadedMaps[loadedMapIndex]->nsbmdFile), AreaDataManager_GetMapTexture(param2));

    if (param6 == TRUE) {
        ov5_021D53A4(mapResMdl);
    }

    param7->loadedMaps[loadedMapIndex]->present = 1;

    BDHC_Load(param7->landDataNARC, landDataHeader.sectionSizeBDHC, param7->loadedMaps[loadedMapIndex]->bdhc, param7->loadedMaps[loadedMapIndex]->bdhcBuffer);

    param7->loadedMaps[loadedMapIndex]->mapMatrixIndex = mapMatrixIndex;

    if (param7->unk_F0 != NULL) {
        param7->unk_F0(param7->unk_F4, mapMatrixIndex, param7->loadedMaps[loadedMapIndex]->mapPropManager);
    }
}

static void ov5_021E7F1C(const int param0, const u8 param1, AreaDataManager *const param2, const MapMatrix *param3, const int param4, const int param5, const BOOL param6, UnkStruct_ov5_021E8F60 *param7)
{
    int landDataId;
    LandDataHeader landDataHeader;
    NNSG3dResMdl *v2;

    if ((param0 < 0) || (param0 >= param4 * param5)) {
        return;
    }

    landDataId = MapMatrix_GetLandDataIdByIndex(param0, param3);

    if (landDataId == 0xffff) {
        return;
    }

    LandDataHeader_Load(param7->landDataNARC, landDataId, &landDataHeader);

    {
        u8 v3[0x800];
        NARC_ReadFile(param7->landDataNARC, 0x800, v3);
    }

    if (param7->loadedMaps[param1]->mapPropManager != NULL) {
        MapPropManager_Load(param7->landDataNARC, landDataHeader.sectionSize3DObject, param2, param7->loadedMaps[param1]->mapPropManager, param7->unk_E4);
    }

    {
        v2 = ov5_021E97AC_LoadNSBMD(param7->landDataNARC, landDataHeader.sectionSizeNSBMD, &param7->loadedMaps[param1]->mapRenderObj, &(param7->loadedMaps[param1]->nsbmdFile), AreaDataManager_GetMapTexture(param2));

        if (param6 == 1) {
            ov5_021D53A4(v2);
        }

        param7->loadedMaps[param1]->present = 1;
    }

    param7->loadedMaps[param1]->mapMatrixIndex = param0;
}

static void ov5_021E7FF0(const int param0, const u8 param1, AreaDataManager *const param2, const MapMatrix *param3, const int param4, const int param5, const BOOL param6, UnkStruct_ov5_021E8F60 *param7)
{
    int landDataId;
    LandDataHeader landDataHeader;

    if ((param0 < 0) || (param0 >= param4 * param5)) {
        return;
    }

    landDataId = MapMatrix_GetLandDataIdByIndex(param0, param3);

    if (landDataId == 0xffff) {
        return;
    }

    LandDataHeader_Load(param7->landDataNARC, landDataId, &landDataHeader);

    {
        void *v2;

        v2 = &(param7->loadedMaps[param1]->movementPermissions[0]);
        NARC_ReadFile(param7->landDataNARC, landDataHeader.sectionSizeMovementPermission, v2);
    }

    if (param7->loadedMaps[param1]->mapPropManager != NULL) {
        MapPropManager_Load(param7->landDataNARC, landDataHeader.sectionSize3DObject, param2, param7->loadedMaps[param1]->mapPropManager, param7->unk_E4);
    }

    NARC_Seek(param7->landDataNARC, landDataHeader.sectionSizeNSBMD);

    {
        BDHC_Load(param7->landDataNARC, landDataHeader.sectionSizeBDHC, param7->loadedMaps[param1]->bdhc, param7->loadedMaps[param1]->bdhcBuffer);
    }

    param7->loadedMaps[param1]->mapMatrixIndex = param0;

    if (param7->unk_F0 != NULL) {
        param7->unk_F0(param7->unk_F4, param0, param7->loadedMaps[param1]->mapPropManager);
    }
}

static BOOL ov5_021E80BC(const UnkStruct_ov5_021E80BC *param0)
{
    if ((param0->unk_00 == 0) && (param0->unk_04 == 0)) {
        return 1;
    } else {
        return 0;
    }
}

static BOOL ov5_021E80D0(UnkStruct_ov5_021E8F60 *param0, const int param1, const int param2, const u8 param3, const u8 param4, const u8 param5, const u8 param6)
{
    GF_ASSERT(param0->unk_04[param5].unk_28 == 0);

    param0->unk_04[param5].unk_28 = 1;
    param0->unk_04[param5].unk_2C = param6;
    param0->unk_04[param5].unk_00.unk_18[0] = param1;
    param0->unk_04[param5].unk_00.unk_18[1] = param2;
    param0->unk_04[param5].unk_00.unk_20[0] = param3;
    param0->unk_04[param5].unk_00.unk_20[1] = param4;
    param0->unk_04[param5].unk_00.unk_00[0] = param0->loadedMaps[param3];
    param0->unk_04[param5].unk_00.unk_00[1] = param0->loadedMaps[param4];
    param0->unk_04[param5].unk_00.unk_22 = 0;
    param0->unk_04[param5].unk_00.unk_24 = 0;

    if (param0->unk_94 == 0) {
        param0->unk_A8 = 0;
    }

    param0->loadedMaps[param3]->mapMatrixIndex = param1;
    param0->loadedMaps[param4]->mapMatrixIndex = param2;
    param0->unk_94++;
    param0->unk_95 = (param0->unk_95 + 1) % 2;

    return 1;
}

void ov5_021E8188(FieldSystem *fieldSystem, UnkStruct_ov5_021E8F60 *param1)
{
    u8 v0;
    UnkStruct_ov5_021E7814 *v1;
    UnkStruct_ov5_021EA174 *v2;

    v1 = &param1->unk_04[param1->unk_96].unk_00;
    v2 = &param1->unk_C0;

    if ((param1->unk_F8 == 0) && (v2->unk_20 == 1)) {
        if (v2->unk_18) {
            GF_ASSERT((*v2->unk_14) <= (*v2->unk_10));

            if ((*v2->unk_10) - (*v2->unk_14) >= (FX32_ONE * 16)) {
                int v3, v4;

                ov5_021E9230(v2->unk_0C->x, v2->unk_0C->z, &v3, &v4);
                ov5_021E8F90(v3, v4, param1->unk_100, param1->unk_108, param1);

                v2->unk_00 = *v2->unk_0C;

                if (!(v2->unk_00.z % (FX32_ONE * 8) == 0)) {
                    if (Link_SetErrorState(1)) {
                        return;
                    }
                }

                GF_ASSERT(v2->unk_00.z % (FX32_ONE * 8) == 0);

                v2->unk_18 = 0;
                v2->unk_10 = NULL;
                v2->unk_14 = NULL;
            }
        } else {
            if ((v2->unk_00.x != v2->unk_0C->x) && (v2->unk_00.z != v2->unk_0C->z)) {
                GF_ASSERT(FALSE);
            } else if ((v2->unk_00.x == v2->unk_0C->x) && (v2->unk_00.z == v2->unk_0C->z)) {
                (void)0;
            } else {
                v2->unk_18 = 1;

                if (v2->unk_00.x != v2->unk_0C->x) {
                    if (v2->unk_00.x > v2->unk_0C->x) {
                        v2->unk_10 = &v2->unk_00.x;
                        v2->unk_14 = &v2->unk_0C->x;
                        v2->unk_1C = 3;
                    } else {
                        v2->unk_10 = &v2->unk_0C->x;
                        v2->unk_14 = &v2->unk_00.x;
                        v2->unk_1C = 1;
                    }

                    if (!(v2->unk_00.x % (FX32_ONE * 8) == 0)) {
                        if (Link_SetErrorState(1)) {
                            return;
                        }
                    }

                    GF_ASSERT(v2->unk_00.x % (FX32_ONE * 8) == 0);
                } else {
                    if (param1->unk_C0.unk_00.z > param1->unk_C0.unk_0C->z) {
                        v2->unk_10 = &v2->unk_00.z;
                        v2->unk_14 = &v2->unk_0C->z;
                        v2->unk_1C = 4;
                    } else {
                        v2->unk_10 = &v2->unk_0C->z;
                        v2->unk_14 = &v2->unk_00.z;
                        v2->unk_1C = 2;
                    }

                    if (!(v2->unk_00.z % (FX32_ONE * 8) == 0)) {
                        if (Link_SetErrorState(1)) {
                            return;
                        }
                    }

                    GF_ASSERT(v2->unk_00.z % (FX32_ONE * 8) == 0);
                }

                ov5_021E9190(v2->unk_1C, param1);
            }
        }
    }

    switch (param1->unk_A8) {
    case 0:
        if (param1->unk_74[v1->unk_20[v1->unk_22]] == 1) {
            (void)0;
        } else {
            param1->unk_E8->unk_04(v1->unk_22, param1->unk_AC, param1->mapMatrix, param1->mapMatrixWidth, param1->mapMatrixHeight, param1, v1);
        }

        v1->unk_22++;
        param1->unk_A8 = 1;
        break;
    case 1:
        if (param1->unk_74[v1->unk_20[v1->unk_22 - 1]] == 1) {
            ov5_021E7814(v1);
            param1->unk_74[v1->unk_20[v1->unk_22 - 1]] = 0;
        }

        if ((v1->unk_22 <= 2) && (ov5_021E80BC(&v1->unk_10) == 1)) {
            if (v1->unk_22 >= 2) {
                v1->unk_24 = 1;
            } else {
                param1->unk_A8 = 0;
            }

            if (v1->unk_00[v1->unk_22 - 1]->present == 1) {
                NNSG3dResMdlSet *v5 = NNS_G3dGetMdlSet(v1->unk_00[v1->unk_22 - 1]->nsbmdFile);
                NNSG3dResMdl *v6 = NNS_G3dGetMdlByIdx(v5, 0);

                if (AreaDataManager_IsOutdoorsLighting(param1->unk_AC) == 1) {
                    ov5_021D53A4(v6);
                }
            }

            if ((0 <= v1->unk_00[v1->unk_22 - 1]->mapMatrixIndex) && (v1->unk_00[v1->unk_22 - 1]->mapMatrixIndex < param1->mapMatrixWidth * param1->mapMatrixHeight)) {
                if (param1->unk_F0 != NULL) {
                    param1->unk_F0(param1->unk_F4, v1->unk_00[v1->unk_22 - 1]->mapMatrixIndex, v1->unk_00[v1->unk_22 - 1]->mapPropManager);
                }
            }
        }
        break;
    case 2:
        break;
    case 3:
        if (ov5_021E80BC(&v1->unk_10) == 1) {
            param1->unk_94 = 0;
        }
        break;
    }

    if (param1->unk_94 == 0) {
        param1->unk_96 = 0;
        param1->unk_95 = 0;
        param1->unk_A8 = 2;
        return;
    }

    if (ov5_021E8DAC(param1) == 1) {
        param1->unk_04[param1->unk_96].unk_28 = 0;

        ov5_021E8DBC(param1);

        param1->unk_94--;
        param1->unk_96 = (param1->unk_96 + 1) % 2;

        if (param1->unk_94 != 0) {
            param1->unk_A8 = 0;
        } else {
            param1->unk_96 = 0;
            param1->unk_95 = 0;
            param1->unk_A8 = 2;
        }

        if (param1->unk_5C.unk_08 == 1) {
            BOOL v7;

            param1->unk_5C.unk_08 = 0;

            ov5_021E8DD4(param1->unk_5C.unk_0C, param1);
            ov5_021E8D50(param1);
        }
    }
}

static void ov5_021E8558(const int param0, const int param1, const u8 param2, const u8 param3, const u8 param4, UnkStruct_ov5_021E8F60 *param5)
{
    int v0;
    u8 v1;
    BOOL v2;

    GF_ASSERT(param5->unk_5C.unk_08 != 1);

    if (ov5_021E8C58(param4, param5, &v1) == 1) {
        ov5_021E77E4(param5, v1);

        if (param5->unk_94 >= 2) {
            param5->unk_5C.unk_08 = 1;
            param5->unk_5C.unk_10[0] = param0;
            param5->unk_5C.unk_10[1] = param1;
            param5->unk_5C.unk_18[0] = param2;
            param5->unk_5C.unk_18[1] = param3;
            param5->unk_5C.unk_00[0] = param5->loadedMaps[param2];
            param5->unk_5C.unk_00[1] = param5->loadedMaps[param3];
            param5->unk_5C.unk_0C = param4;
            param5->loadedMaps[param2]->mapMatrixIndex = param0;
            param5->loadedMaps[param3]->mapMatrixIndex = param1;
        } else {
            v2 = ov5_021E80D0(param5, param0, param1, param2, param3, param5->unk_95, param4);

            if (v2 == 0) {
                (void)0;
            }
        }
    } else {
        v2 = ov5_021E80D0(param5, param0, param1, param2, param3, param5->unk_95, param4);

        GF_ASSERT(v2 == 1);
    }
}

static void ov5_021E8614(const u8 param0, UnkStruct_ov5_021E8F60 *param1)
{
    param1->loadedMaps[param0]->present = 0;
    BDHC_Reset(param1->loadedMaps[param0]->bdhc);

    if (param1->loadedMaps[param0]->mapPropManager != NULL) {
        MapPropManager_Init(
            param1->loadedMaps[param0]->mapPropManager);
    }

    param1->loadedMaps[param0]->mapMatrixIndex = -1;
    MI_CpuFillFast(param1->loadedMaps[param0]->movementPermissions, 0xffffffff, 2 * 32 * 32);
    param1->unk_74[param0] = 1;
}

static void ov5_021E8668(const u8 param0, const u8 param1, const u8 param2, const u8 param3, UnkStruct_ov5_021E8F60 *param4)
{
    LoadedMap *v0[2];

    ov5_021E8614(param0, param4);
    ov5_021E8614(param1, param4);

    v0[0] = param4->loadedMaps[param0];
    v0[1] = param4->loadedMaps[param1];

    param4->loadedMaps[param0] = param4->loadedMaps[param2];
    param4->loadedMaps[param1] = param4->loadedMaps[param3];

    param4->loadedMaps[param2] = v0[0];
    param4->loadedMaps[param3] = v0[1];
}

static void ov5_021E86A0(const int param0, const u8 param1, UnkStruct_ov5_021E8F60 *param2)
{
    u8 v0, v1;
    int v2, v3;
    int v4;
    int v5, v6;

    switch (param1) {
    case 4:
        ov5_021E8668(2, 3, 0, 1, param2);

        if (param2->unk_A1 == 2) {
            v2 = param0 - param2->mapMatrixWidth;
            v3 = param0 - param2->mapMatrixWidth - 1;
            v5 = 1;
            v6 = 0;

            if (v2 < 0) {
                v2 = -1;
                v3 = -1;
            } else {
                if ((v3 < 0) || (!ov5_021E77C0(v2, v3, param2->mapMatrixWidth))) {
                    v3 = -1;
                }
            }
        } else if (param2->unk_A1 == 3) {
            v2 = param0 - param2->mapMatrixWidth;
            v3 = param0 - param2->mapMatrixWidth + 1;
            v5 = 0;
            v6 = 1;

            if (v2 < 0) {
                v2 = -1;
                v3 = -1;
            } else {
                if (!ov5_021E77C0(v2, v3, param2->mapMatrixWidth)) {
                    v3 = -1;
                }
            }
        } else {
            GF_ASSERT(FALSE);
        }

        ov5_021E8558(v2, v3, v5, v6, param1, param2);
        break;
    case 3:
        ov5_021E8668(1, 3, 0, 2, param2);

        if (param2->unk_A1 == 1) {
            v2 = param0 - 1;
            v3 = param0 - param2->mapMatrixWidth - 1;
            v5 = 2;
            v6 = 0;

            if ((v2 < 0) || (!ov5_021E77C0(v2, param0, param2->mapMatrixWidth))) {
                v2 = -1;
                v3 = -1;
            }

            if ((v3 < 0) || (!ov5_021E779C(v2, v3, param2->mapMatrixWidth))) {
                v3 = -1;
            }
        } else if (param2->unk_A1 == 3) {
            v2 = param0 - 1;
            v3 = param0 + param2->mapMatrixWidth - 1;
            v5 = 0;
            v6 = 2;

            if ((v2 < 0) || (!ov5_021E77C0(v2, param0, param2->mapMatrixWidth))) {
                v2 = -1;
                v3 = -1;
            }

            if ((param2->mapMatrixWidth * param2->mapMatrixHeight <= v3) || (!ov5_021E779C(v2, v3, param2->mapMatrixWidth))) {
                v3 = -1;
            }
        } else {
            GF_ASSERT(FALSE);
        }

        ov5_021E8558(v2, v3, v5, v6, param1, param2);
        break;
    case 1:
        ov5_021E8668(0, 2, 1, 3, param2);

        if (param2->unk_A1 == 0) {
            v2 = param0 + 1;
            v3 = param0 - param2->mapMatrixWidth + 1;
            v5 = 3;
            v6 = 1;

            if ((param2->mapMatrixWidth * param2->mapMatrixHeight <= v2) || (!ov5_021E77C0(v2, param0, param2->mapMatrixWidth))) {
                v2 = -1;
                v3 = -1;
            }

            if ((param2->mapMatrixWidth * param2->mapMatrixHeight <= v3) || (!ov5_021E779C(v2, v3, param2->mapMatrixWidth))) {
                v3 = -1;
            }
        } else if (param2->unk_A1 == 2) {
            v2 = param0 + 1;
            v3 = param0 + param2->mapMatrixWidth + 1;
            v5 = 1;
            v6 = 3;

            if ((param2->mapMatrixWidth * param2->mapMatrixHeight <= v2) || (!ov5_021E77C0(v2, param0, param2->mapMatrixWidth))) {
                v2 = -1;
                v3 = -1;
            }

            if ((v3 < 0) || (!ov5_021E779C(v2, v3, param2->mapMatrixWidth))) {
                v3 = -1;
            }
        } else {
            (void)0;
        }

        ov5_021E8558(v2, v3, v5, v6, param1, param2);
        break;
    case 2:
        ov5_021E8668(0, 1, 2, 3, param2);

        if (param2->unk_A1 == 0) {
            v2 = param0 + param2->mapMatrixWidth;
            v3 = param0 + param2->mapMatrixWidth - 1;
            v5 = 3;
            v6 = 2;

            if (param2->mapMatrixWidth * param2->mapMatrixHeight <= v2) {
                v2 = -1;
                v3 = -1;
            } else {
                if (!ov5_021E77C0(v2, v3, param2->mapMatrixWidth)) {
                    v3 = -1;
                }
            }
        } else if (param2->unk_A1 == 1) {
            v2 = param0 + param2->mapMatrixWidth;
            v3 = param0 + param2->mapMatrixWidth + 1;
            v5 = 2;
            v6 = 3;

            if (param2->mapMatrixWidth * param2->mapMatrixHeight <= v2) {
                v2 = -1;
                v3 = -1;
            } else {
                if ((param2->mapMatrixWidth * param2->mapMatrixHeight <= v3) || (!ov5_021E77C0(v2, v3, param2->mapMatrixWidth))) {
                    v3 = -1;
                }
            }
        } else {
            GF_ASSERT(FALSE);
        }

        ov5_021E8558(v2, v3, v5, v6, param1, param2);
        break;
    default:
        GF_ASSERT(FALSE);
        break;
    }
}

static void ov5_021E89D0(const u8 param0, const UnkStruct_ov5_021E8F60 *param1, UnkStruct_ov5_021D5894 *const param2)
{
    VecFx32 v0 = { 0, 0, 0 };

    ov5_021E901C(param1->loadedMaps[param0]->mapMatrixIndex, param1->mapMatrixWidth, param1->mapMatrix, &v0);

    v0.x += param1->unk_10C.x;
    v0.y += param1->unk_10C.y;
    v0.z += param1->unk_10C.z;

    if (param1->loadedMaps[param0]->present == 1) {
        VecFx32 v1 = { FX32_ONE, FX32_ONE, FX32_ONE };
        MtxFx33 v2 = { FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE };

        Easy3D_DrawRenderObj(&(param1->loadedMaps[param0]->mapRenderObj), &v0, &v2, &v1);
    }

    if (param1->loadedMaps[param0]->present == 1) {
        if (param1->loadedMaps[param0]->mapPropManager != NULL) {
            MapPropManager_Render(&v0, param1->unk_AC, AreaDataManager_IsOutdoorsLighting(param1->unk_AC), param2, param1->loadedMaps[param0]->mapPropManager);
        }
    }
}

static int ov5_021E8A98(const u32 param0, const int param1, const int param2)
{
    u32 v0, v1;
    u32 v2, v3;
    int v4;

    v0 = param0 % param2;
    v1 = param0 / param2;
    v2 = v0 / 32;
    v3 = v1 / 32;
    v4 = v2 + v3 * param1;

    return v4;
}

static int ov5_021E8ABC_CalculatePlayerTile(const int playerTileX, const int playerTileY, const int offsetTileX, const int offsetTileY, const int mapTilesWidth)
{
    int v0;

    v0 = (playerTileX - offsetTileX) + ((playerTileY - offsetTileY) * mapTilesWidth);
    return v0;
}

static u32 ov5_021E8ACC(const u32 param0, const int param1, const int param2)
{
    u32 v0, v1;
    u32 v2, v3;
    u32 v4;

    v0 = param0 % param2;
    v1 = param0 / param2;
    v2 = v0 / 32;
    v3 = v1 / 32;
    v4 = v2 + v3 * param1;

    return v4;
}

static u8 ov5_021E8AF0(const u8 param0)
{
    if (param0 == 0) {
        return 3;
    } else if (param0 == 1) {
        return 2;
    } else if (param0 == 2) {
        return 1;
    } else if (param0 == 3) {
        return 0;
    } else {
        GF_ASSERT(FALSE);
    }

    return 0;
}

static u32 ov5_021E8B1C(const u8 param0, const int param1, const int param2, const int param3)
{
    u32 v0;
    u32 v1;
    int v2;

    switch (param0) {
    case 1:
        v2 = (param3 % param2) + 1;

        if (v2 >= param2) {
            return param3;
        }

        v0 = param3 + 1;
        break;
    case 2:
        v2 = param3 + param2;

        if (v2 >= param1 * 32 * param2) {
            return param3;
        }

        v0 = param3 + param2;
        break;
    case 3:
        v2 = (param3 % param2) - 1;

        if (v2 < 0) {
            return param3;
        }

        v0 = param3 - 1;
        break;
    case 4:
        v2 = param3 - param2;

        if (v2 < 0) {
            return param3;
        }

        v0 = param3 - param2;
        break;
    default:
        GF_ASSERT(FALSE);
        return 0;
    }

    return v0;
}

static u8 ov5_021E8B84(const u8 param0, const int param1)
{
    u8 v0;

    switch (param0) {
    case 4:
        v0 = param1 - 2;
        break;
    case 2:
        v0 = param1 + 2;
        break;
    case 3:
        v0 = param1 - 1;
        break;
    case 1:
        v0 = param1 + 1;
        break;
    }

    return v0;
}

static void ov5_021E8BC0(const u8 param0, const int param1, u8 *param2)
{
    u8 v0;
    u8 v1;

    v0 = ov5_021E8B84(param0, param1);

    if (param1 < v0) {
        if (v0 - param1 == 1) {
            v1 = (*param2) - 1;
        } else if (v0 - param1 == 2) {
            v1 = (*param2) - 2;
        } else {
            GF_ASSERT(FALSE);
        }
    } else if (param1 > v0) {
        if (param1 - v0 == 1) {
            v1 = (*param2) + 1;
        } else if (param1 - v0 == 2) {
            v1 = (*param2) + 2;
        } else {
            GF_ASSERT(FALSE);
        }
    } else {
        GF_ASSERT(FALSE);
    }

    (*param2) = v1;
}

static u8 ov5_021E8C24(const u8 param0)
{
    switch (param0) {
    case 4:
        return 2;
    case 2:
        return 4;
    case 1:
        return 3;
    case 3:
        return 1;
    default:
        GF_ASSERT(FALSE);
        return 0;
    }
}

static BOOL ov5_021E8C58(const u8 param0, const UnkStruct_ov5_021E8F60 *param1, u8 *param2)
{
    int v0;

    for (v0 = 0; v0 < 2; v0++) {
        if (param1->unk_04[v0].unk_28 == 1) {
            if (ov5_021E8C24(param0) == param1->unk_04[v0].unk_2C) {
                *param2 = v0;
                return 1;
            }
        }
    }

    return 0;
}

static BOOL ov5_021E8C8C(UnkStruct_ov5_021E8F60 *param0)
{
    UnkStruct_ov5_021E8C8C *v0 = &param0->unk_5C;

    if (param0->unk_04[param0->unk_95].unk_28 != 0) {
        GF_ASSERT(FALSE);
    }

    param0->unk_04[param0->unk_95].unk_28 = 1;
    param0->unk_04[param0->unk_95].unk_2C = v0->unk_0C;

    param0->unk_04[param0->unk_95].unk_00.unk_18[0] = v0->unk_10[0];
    param0->unk_04[param0->unk_95].unk_00.unk_18[1] = v0->unk_10[1];

    param0->unk_04[param0->unk_95].unk_00.unk_20[0] = v0->unk_18[0];
    param0->unk_04[param0->unk_95].unk_00.unk_20[1] = v0->unk_18[1];

    param0->unk_04[param0->unk_95].unk_00.unk_00[0] = v0->unk_00[0];
    param0->unk_04[param0->unk_95].unk_00.unk_00[1] = v0->unk_00[1];

    param0->unk_04[param0->unk_95].unk_00.unk_22 = 0;
    param0->unk_04[param0->unk_95].unk_00.unk_24 = 0;

    return 1;
}

static void ov5_021E8D50(UnkStruct_ov5_021E8F60 *param0)
{
    int v0;
    BOOL v1;
    UnkStruct_ov5_021E8C8C *v2 = &param0->unk_5C;

    if (param0->unk_94 >= 2) {
        GF_ASSERT(FALSE);
    }

    v1 = ov5_021E8C8C(param0);

    GF_ASSERT(v1 == 1);

    v2->unk_00[0]->mapMatrixIndex = v2->unk_10[0];
    v2->unk_00[1]->mapMatrixIndex = v2->unk_10[1];

    param0->unk_94++;
    param0->unk_95 = (param0->unk_95 + 1) % 2;
}

static BOOL ov5_021E8DAC(const UnkStruct_ov5_021E8F60 *param0)
{
    return param0->unk_04[param0->unk_96].unk_00.unk_24;
}

static void ov5_021E8DBC(UnkStruct_ov5_021E8F60 *param0)
{
    u8 v0;

    for (v0 = 0; v0 < 4; v0++) {
        param0->unk_74[v0] = 0;
    }
}

static void ov5_021E8DD4(const u8 param0, UnkStruct_ov5_021E8F60 *param1)
{
    switch (param0) {
    case 4:
        param1->unk_74[2] = 1;
        param1->unk_74[3] = 1;
        break;
    case 1:
        param1->unk_74[0] = 1;
        param1->unk_74[2] = 1;
        break;
    case 2:
        param1->unk_74[0] = 1;
        param1->unk_74[1] = 1;
        break;
    case 3:
        param1->unk_74[1] = 1;
        param1->unk_74[3] = 1;
        break;
    }
}

static void ov5_021E8E28(UnkStruct_ov5_021E8F60 *param0, const int playerTileX, const int playerTileY, const int offsetTileX, const int offsetTileY, const int mapTilesWidth)
{
    u8 i;
    int loadedMapMatrixIndexes[4];

    UnkStruct_ov5_021E8F60_Init(param0);
    ov5_021E7838_CalculateLoadedMapMatrixIndexes(param0->unk_F8, playerTileX, playerTileY, offsetTileX, offsetTileY, param0->mapMatrixWidth, param0->mapMatrixHeight, mapTilesWidth, loadedMapMatrixIndexes);

    for (i = 0; i < 4; i++) {
        param0->loadedMaps[i]->bdhc = BDHC_New();

        BDHC_MarkNotLoaded(param0->loadedMaps[i]->bdhc);
        ov5_021E7E28_LoadLandData(loadedMapMatrixIndexes[i], i, param0->unk_AC, param0->mapMatrix, param0->mapMatrixWidth, param0->mapMatrixHeight, AreaDataManager_IsOutdoorsLighting(param0->unk_AC), param0);
    }
}

static void ov5_021E8ECC(UnkStruct_ov5_021E8F60 *param0, const int param1, const int param2, const int param3, const int param4, const int param5)
{
    u8 v0;
    int v1[4];

    ov5_021E7AC4(param0);
    ov5_021E7838_CalculateLoadedMapMatrixIndexes(param0->unk_F8, param1, param2, param3, param4, param0->mapMatrixWidth, param0->mapMatrixHeight, param5, v1);

    for (v0 = 0; v0 < 4; v0++) {
        param0->loadedMaps[v0]->bdhc = NULL;
        ov5_021E7F1C(v1[v0], v0, param0->unk_AC, param0->mapMatrix, param0->mapMatrixWidth, param0->mapMatrixHeight, AreaDataManager_IsOutdoorsLighting(param0->unk_AC), param0);
    }
}

static void ov5_021E8F60(UnkStruct_ov5_021E8F60 *param0)
{
    int v0;

    param0->unk_96 = 0;
    param0->unk_95 = 0;
    param0->unk_94 = 0;

    for (v0 = 0; v0 < 2; v0++) {
        param0->unk_04[v0].unk_28 = 0;
        param0->unk_04[v0].unk_00.unk_24 = 1;
    }

    ov5_021E8DBC(param0);
}

static void ov5_021E8F90(const int playerTileX, const int playerTileY, const int param2, const int param3, UnkStruct_ov5_021E8F60 *param4)
{
    int v0 = playerTileX, v1 = playerTileY;

    if (param4->unk_F8 == 1) {
        if (param4->mapMatrixWidth > 1) {
            v0 = param2 + 31;
        } else {
            v0 = param2 + 16;
        }

        if (param4->mapMatrixHeight > 1) {
            v1 = param3 + 31;
        } else {
            v1 = param3 + 16;
        }
    }

    param4->unk_9C = ov5_021E8ABC_CalculatePlayerTile(v0, v1, param2, param3, param4->mapTilesWidth);
    param4->unk_A1 = ov5_021E935C_CalculateLoadedMapIndexForPlayer(param4->unk_9C, param4->mapTilesWidth);
    param4->unk_A0 = ov5_021E8AF0(param4->unk_A1);
    param4->unk_98 = ov5_021E8ACC(param4->unk_9C, param4->mapMatrixWidth, param4->mapTilesWidth);
}

static void ov5_021E901C(const int param0, const int param1, const MapMatrix *param2, VecFx32 *param3)
{
    u16 v0;
    u16 v1;

    param3->x = (32 / 2) * 16 * FX32_ONE;
    param3->z = (32 / 2) * 16 * FX32_ONE;

    if (param0 == -1) {
        return;
    }

    v0 = param0 % param1;
    v1 = param0 / param1;

    {
        int v2;
        int v3;
        u16 v4;

        v3 = MapMatrix_GetWidth(param2);
        v4 = MapMatrix_GetMatrixID(param2);
        v2 = MapMatrix_GetAltitudeAtCoords(param2, v4, v0, v1, v3);
        param3->y = v2 * (16 / 2) * FX32_ONE;
    }

    param3->x += v0 * 32 * 16 * FX32_ONE;
    param3->z += v1 * 32 * 16 * FX32_ONE;
}

UnkStruct_ov5_021E8F60 *ov5_021E9084(MapMatrix *mapMatrix, AreaDataManager *param1, UnkStruct_ov5_021D3CAC *param2, const int param3)
{
    UnkStruct_ov5_021E8F60 *v0;
    BOOL v1;

    v0 = Heap_AllocFromHeap(4, sizeof(UnkStruct_ov5_021E8F60));
    MI_CpuClear8(v0, sizeof(UnkStruct_ov5_021E8F60));

    if (param3 == 0) {
        v0->unk_E8 = &Unk_ov5_021FAE98;
        v1 = 1;
    } else if (param3 == 1) {
        v0->unk_E8 = &Unk_ov5_021FAE90;
        v1 = 0;
    } else {
        GF_ASSERT(FALSE);
    }

    v0->unk_00 = ov5_021EEAC8(v1);
    v0->unk_AC = param1;
    v0->mapMatrix = mapMatrix;
    v0->mapMatrixWidth = MapMatrix_GetWidth(mapMatrix);
    v0->mapMatrixHeight = MapMatrix_GetHeight(mapMatrix);
    v0->mapTilesWidth = v0->mapMatrixWidth * 32;
    v0->unk_E4 = param2;
    v0->unk_C0.unk_20 = 1;

    ov5_021E8F60(v0);

    v0->unk_A4 = 0;
    v0->unk_A8 = 2;
    v0->landDataNARC = NARC_ctor(NARC_INDEX_FIELDDATA__LAND_DATA__LAND_DATA, 4);
    v0->unk_F0 = NULL;

    return v0;
}

void ov5_021E9150(UnkStruct_ov5_021E8F60 *param0, const int playerTileX, const int playerTileY)
{
    param0->unk_E8->unk_00(param0, playerTileX, playerTileY, param0->unk_100, param0->unk_108, param0->mapTilesWidth);
    ov5_021E8F90(playerTileX, playerTileY, param0->unk_100, param0->unk_108, param0);
}

void ov5_021E9190(const u8 param0, UnkStruct_ov5_021E8F60 *param1)
{
    int v0;
    u32 v1;
    u8 v2;

    v1 = ov5_021E8B1C(param0, param1->mapMatrixHeight, param1->mapTilesWidth, param1->unk_9C);
    v0 = ov5_021E8A98(v1, param1->mapMatrixWidth, param1->mapTilesWidth);
    v2 = ov5_021E935C_CalculateLoadedMapIndexForPlayer(v1, param1->mapTilesWidth);

    if (param1->unk_98 != v0) {
        return;
    } else {
        if (param1->unk_A1 != v2) {
            ov5_021E86A0(v0, param0, param1);
            ov5_021E8BC0(param0, param1->unk_A1, &(param1->unk_A0));
        }
    }
}

void ov5_021E91FC(const UnkStruct_ov5_021E8F60 *param0, UnkStruct_ov5_021D5894 *const param1)
{
    u8 v0;

    for (v0 = 0; v0 < 4; v0++) {
        if (param0->loadedMaps[v0]->mapMatrixIndex != -1) {
            ov5_021E89D0(v0, param0, param1);
        }
    }
}

void ov5_021E9230(fx32 param0, fx32 param1, int *param2, int *param3)
{
    *param2 = param0 / (16 * FX32_ONE);
    *param3 = param1 / (16 * FX32_ONE);
}

void ov5_021E924C(UnkStruct_ov5_021E8F60 *param0)
{
    u8 v0;

    ClearUnusedSystemCache();

    NNS_GfdResetLnkTexVramState();
    NNS_GfdResetLnkPlttVramState();

    for (v0 = 0; v0 < 2; v0++) {
        ov5_021E7814(&param0->unk_04[v0].unk_00);
    }

    for (v0 = 0; v0 < 4; v0++) {
        param0->loadedMaps[v0]->present = 0;

        BDHC_Free(param0->loadedMaps[v0]->bdhc);

        if (param0->loadedMaps[v0]->mapPropManager != NULL) {
            MapPropManager_Free(param0->loadedMaps[v0]->mapPropManager);
        }

        Heap_FreeToHeap(param0->loadedMaps[v0]);
        param0->loadedMaps[v0] = NULL;
    }

    param0->unk_A8 = 3;
    param0->unk_A4 = 1;
    param0->unk_C0.unk_20 = 0;
}

void ov5_021E92E4(UnkStruct_ov5_021E8F60 *param0)
{
    NARC_dtor(param0->landDataNARC);
    ov5_021EEB48(param0->unk_00);
    Heap_FreeToHeap((void *)param0);
}

BOOL ov5_021E9300(const UnkStruct_ov5_021E8F60 *param0)
{
    if ((param0->unk_A4 == 1) && (param0->unk_94 == 0)) {
        return 1;
    } else {
        return 0;
    }
}

void ov5_021E931C(VecFx32 const *param0, UnkStruct_ov5_021E8F60 *param1)
{
    param1->unk_C0.unk_00 = *param0;
    param1->unk_C0.unk_0C = param0;
}

void ov5_021E9338(UnkStruct_ov5_021E8F60 *param0)
{
    param0->unk_C0.unk_0C = NULL;
}

void ov5_021E9340(const u8 param0, const UnkStruct_ov5_021E8F60 *param1, MapPropManager **param2)
{
    *param2 = param1->loadedMaps[param0]->mapPropManager;
}

u8 ov5_021E9354(const UnkStruct_ov5_021E8F60 *param0)
{
    return param0->unk_A0;
}

u8 ov5_021E935C_CalculateLoadedMapIndexForPlayer(const u32 playerTile, const int mapTilesWidth)
{
    u8 playerLoadedMapIndex;
    int playerTileX, playerTileYMod;
    int playerTileY;

    playerTileX = playerTile % 32;
    playerTileY = playerTile / mapTilesWidth;
    playerTileYMod = playerTileY % 32;

    if (playerTileX < 32 / 2) {
        if (playerTileYMod < 32 / 2) {
            // Player is on the top-left corner of the map square
            playerLoadedMapIndex = 0;
        } else {
            // Player is on the bottom-left corner of the map square
            playerLoadedMapIndex = 2;
        }
    } else {
        if (playerTileYMod < 32 / 2) {
            // Player is on the top-right corner of the map square
            playerLoadedMapIndex = 1;
        } else {
            // Player is on the bottom-right corner of the map square
            playerLoadedMapIndex = 3;
        }
    }

    return playerLoadedMapIndex;
}

u8 ov5_021E9390(const int param0, const u8 param1, const UnkStruct_ov5_021E8F60 *param2)
{
    u8 v0;

    if (param2->unk_98 == param0) {
        return param2->unk_A0;
    }

    GF_ASSERT(param0 != -1);

    switch (param2->unk_A0) {
    case 0:
        if (param0 - param2->unk_98 == param2->mapMatrixWidth) {
            v0 = param2->unk_A0 + 2;
        } else if ((param0 - param2->unk_98 == 1) && (ov5_021E77C0(param2->unk_98, param0, param2->mapMatrixWidth))) {
            v0 = param2->unk_A0 + 1;
        } else if ((param0 - param2->unk_98 == param2->mapMatrixWidth + 1) && (!ov5_021E77C0(param2->unk_98, param0, param2->mapMatrixWidth))) {
            v0 = param2->unk_A0 + 3;
        } else {
            v0 = 4;
        }
        break;
    case 1:
        if (param0 - param2->unk_98 == param2->mapMatrixWidth) {
            v0 = param2->unk_A0 + 2;
        } else if ((param2->unk_98 - param0 == 1) && (ov5_021E77C0(param2->unk_98, param0, param2->mapMatrixWidth))) {
            v0 = param2->unk_A0 - 1;
        } else if ((param0 - param2->unk_98 == param2->mapMatrixWidth - 1) && (!ov5_021E77C0(param2->unk_98, param0, param2->mapMatrixWidth))) {
            v0 = param2->unk_A0 + 1;
        } else {
            v0 = 4;
        }
        break;
    case 2:
        if (param2->unk_98 - param0 == param2->mapMatrixWidth) {
            v0 = param2->unk_A0 - 2;
        } else if ((param2->unk_98 - param0 == param2->mapMatrixWidth - 1) && (!ov5_021E77C0(param2->unk_98, param0, param2->mapMatrixWidth))) {
            v0 = param2->unk_A0 - 1;
        } else if ((param0 - param2->unk_98 == 1) && (ov5_021E77C0(param2->unk_98, param0, param2->mapMatrixWidth))) {
            v0 = param2->unk_A0 + 1;
        } else {
            v0 = 4;
        }
        break;
    case 3:
        if (param2->unk_98 - param0 == param2->mapMatrixWidth) {
            v0 = param2->unk_A0 - 2;
        } else if ((param2->unk_98 - param0 == param2->mapMatrixWidth + 1) && (!ov5_021E77C0(param2->unk_98, param0, param2->mapMatrixWidth))) {
            v0 = param2->unk_A0 - 3;
        } else if ((param2->unk_98 - param0 == 1) && (ov5_021E77C0(param2->unk_98, param0, param2->mapMatrixWidth))) {
            v0 = param2->unk_A0 - 1;
        } else {
            v0 = 4;
        }
        break;
    default:
        v0 = 4;
    }

    return v0;
}

int ov5_021E9560_GetMapMatrixIndex(const UnkStruct_ov5_021E8F60 *param0, const u8 loadedMapIndex)
{
    GF_ASSERT(loadedMapIndex < 4);
    return param0->loadedMaps[loadedMapIndex]->mapMatrixIndex;
}

BOOL ov5_021E9580(const UnkStruct_ov5_021E8F60 *param0, const int param1, const int param2, u8 *param3)
{
    int v0, v1;
    u32 v2, v3;
    u32 v4;
    u32 v5;
    u32 v6;
    u8 v7;

    v0 = param1 - param0->unk_100;
    v1 = param2 - param0->unk_108;
    v2 = v0 / 32;
    v3 = v1 / 32;
    v4 = v2 + (v3 * param0->mapMatrixWidth);

    if (!(v4 < param0->mapMatrixWidth * param0->mapMatrixHeight)) {
        if (Link_SetErrorState(1)) {
            return 0;
        }
    }

    GF_ASSERT(v4 < param0->mapMatrixWidth * param0->mapMatrixHeight);

    v6 = v0 + (v1 * param0->mapTilesWidth);
    v7 = ov5_021E935C_CalculateLoadedMapIndexForPlayer(v6, param0->mapTilesWidth);
    v5 = ov5_021E9390(v4, v7, param0);

    if (param3 != NULL) {
        *param3 = v5;
    }

    if (v5 > 3) {
        return 0;
    } else {
        return 1;
    }
}

const BDHC *LoadedMaps_GetBDHC(const UnkStruct_ov5_021E8F60 *param0, const u8 loadedMapIndex)
{
    return param0->loadedMaps[loadedMapIndex]->bdhc;
}

u16 const *LoadedMaps_GetMovementPermissions(const UnkStruct_ov5_021E8F60 *param0, const u8 loadedMapIndex)
{
    return param0->loadedMaps[loadedMapIndex]->movementPermissions;
}

void ov5_021E9630(UnkStruct_ov5_021E8F60 *param0, UnkFuncPtr_ov5_021E9630 param1, void *param2)
{
    param0->unk_F0 = param1;
    param0->unk_F4 = param2;
}

static void ov5_021E9640(SysTask *param0)
{
    UnkStruct_ov5_021E9640 *v0;

    v0 = (UnkStruct_ov5_021E9640 *)SysTask_GetParam(param0);
    v0->unk_1C = 1;
}

static void ov5_021E964C(SysTask *param0, void *param1)
{
    int v0;
    UnkStruct_ov5_021E9640 *v1;
    BOOL v2;
    int v3;
    void *v4;

    v1 = (UnkStruct_ov5_021E9640 *)param1;

    if (v1->unk_1C == 1) {
        v1->unk_14 = 5;
    }

    switch (v1->unk_14) {
    case 0:
        v1->unk_24 = 0;

        if (v1->unk_04 <= 0xe000) {
            v3 = v1->unk_04;
            v1->unk_14 = 2;
        } else {
            v3 = 0xe000;
            v1->unk_14 = 1;
        }

        v4 = &((u8 *)(*v1->unk_0C))[v1->unk_24];
        NARC_ReadFile(v1->landDataNARC, v3, v4);
        v1->unk_24 += v3;
        break;
    case 1: {
        int v5;

        v3 = v1->unk_04 - v1->unk_24;

        if (v3 > 0xe000) {
            v3 = 0xe000;
            v5 = 0;
        } else {
            v5 = 1;
        }

        v4 = &((u8 *)(*v1->unk_0C))[v1->unk_24];
        NARC_ReadFile(v1->landDataNARC, v3, v4);

        if (v5) {
            v1->unk_14 = 2;
        } else {
            v1->unk_24 += v3;
        }
    } break;
    case 2:
        if (v1->unk_10 != NULL) {
            if (Easy3D_IsTextureUploadedToVRAM(v1->unk_10) == 1) {
                v2 = Easy3D_BindTextureToResource(*v1->unk_0C, v1->unk_10);
            } else {
                (void)0;
            }
        }

        v1->unk_14 = 3;
        break;
    case 3: {
        NNSG3dResMdlSet *v6 = NNS_G3dGetMdlSet(*v1->unk_0C);
        NNSG3dResMdl *v7;

        GF_ASSERT(v6->dict.numEntry == 1);

        v7 = NNS_G3dGetMdlByIdx(v6, 0);
        GF_ASSERT(v7);

        NNS_G3dRenderObjInit(v1->unk_08, v7);
    }
        *v1->unk_18 = 1;
        v1->unk_14 = 5;
        break;
    case 5:
        *v1->unk_20 = 0;
        Heap_FreeToHeap((void *)(param1));
        SysTask_Done(param0);
        return;
    }
}

SysTask *ov5_021E976C(NARC *landDataNARC, const int param1, NNSG3dRenderObj *param2, NNSG3dResFileHeader **param3, NNSG3dResTex *param4, BOOL *param5, int *param6)
{
    SysTask *v0;
    UnkStruct_ov5_021E9640 *v1;

    v1 = (UnkStruct_ov5_021E9640 *)Heap_AllocFromHeapAtEnd(4, sizeof(UnkStruct_ov5_021E9640));

    v1->landDataNARC = landDataNARC;
    v1->unk_04 = param1;
    v1->unk_08 = &param2[0];
    v1->unk_0C = param3;
    v1->unk_10 = param4;
    v1->unk_14 = 0;
    v1->unk_18 = param5;
    *v1->unk_18 = 0;
    v1->unk_20 = param6;
    v1->unk_1C = 0;

    v0 = SysTask_Start(ov5_021E964C, v1, 1);
    return v0;
}

NNSG3dResMdl *ov5_021E97AC_LoadNSBMD(NARC *landDataNARC, const int nsbmdSize, NNSG3dRenderObj *mapRenderObj, NNSG3dResFileHeader **nsbmdBuffer, NNSG3dResTex *mapTexture)
{
    BOOL textureBound;
    NNSG3dResMdl *mapResMdl;

    NARC_ReadFile(landDataNARC, nsbmdSize, *nsbmdBuffer);

    if (mapTexture != NULL) {
        if (Easy3D_IsTextureUploadedToVRAM(mapTexture) == 1) {
            textureBound = Easy3D_BindTextureToResource(*nsbmdBuffer, mapTexture);
            GF_ASSERT(textureBound);
        }
    }

    NNSG3dResMdlSet *mapResMdlSet = NNS_G3dGetMdlSet(*nsbmdBuffer);

    GF_ASSERT(mapResMdlSet->dict.numEntry == 1);
    mapResMdl = NNS_G3dGetMdlByIdx(NNS_G3dGetMdlSet(*nsbmdBuffer), 0);

    GF_ASSERT(mapResMdl);
    NNS_G3dRenderObjInit(mapRenderObj, mapResMdl);

    return mapResMdl;
}

NARC *ov5_021E9828_GetLandDataNARC(UnkStruct_ov5_021E8F60 *param0)
{
    return param0->landDataNARC;
}

UnkStruct_ov5_021E8F60 *ov5_021E9830(MapMatrix *mapMatrix, AreaDataManager *param1, NARC *landDataNARC)
{
    UnkStruct_ov5_021E8F60 *v0;
    BOOL v1;

    v0 = Heap_AllocFromHeap(4, sizeof(UnkStruct_ov5_021E8F60));
    MI_CpuClear8(v0, sizeof(UnkStruct_ov5_021E8F60));

    v0->unk_00 = ov5_021EEBC0(0);
    v0->unk_AC = param1;
    v0->mapMatrix = mapMatrix;

    if (mapMatrix != NULL) {
        v0->mapMatrixWidth = MapMatrix_GetWidth(mapMatrix);
        v0->mapMatrixHeight = MapMatrix_GetHeight(mapMatrix);
        v0->mapTilesWidth = v0->mapMatrixWidth * 32;
    }

    v0->unk_C0.unk_20 = 0;
    ov5_021E8F60(v0);
    v0->unk_A4 = 0;
    v0->unk_A8 = 2;
    v0->landDataNARC = landDataNARC;

    if (landDataNARC == NULL) {
        v0->landDataNARC = NARC_ctor(NARC_INDEX_FIELDDATA__LAND_DATA__LAND_DATA, 4);
    }

    return v0;
}

void ov5_021E98C8(UnkStruct_ov5_021E8F60 *param0, MapMatrix *mapMatrix, AreaDataManager *param2, NARC *landDataNARC)
{
    BOOL v0;

    param0->unk_AC = param2;
    param0->mapMatrix = mapMatrix;
    param0->mapMatrixWidth = MapMatrix_GetWidth(mapMatrix);
    param0->mapMatrixHeight = MapMatrix_GetHeight(mapMatrix);
    param0->mapTilesWidth = param0->mapMatrixWidth * 32;
    param0->unk_C0.unk_20 = 0;

    ov5_021E8F60(param0);

    param0->unk_A4 = 0;
    param0->unk_A8 = 2;
    param0->landDataNARC = landDataNARC;

    if (landDataNARC == NULL) {
        param0->landDataNARC = NARC_ctor(NARC_INDEX_FIELDDATA__LAND_DATA__LAND_DATA, 4);
    }
}

void ov5_021E9938(UnkStruct_ov5_021E8F60 *param0)
{
    u8 v0;

    for (v0 = 0; v0 < 4; v0++) {
        param0->loadedMaps[v0]->present = 0;

        if (param0->loadedMaps[v0]->mapPropManager != NULL) {
            MapPropManager_Free(param0->loadedMaps[v0]->mapPropManager);
        }

        Heap_FreeToHeap(param0->loadedMaps[v0]);
        param0->loadedMaps[v0] = NULL;
    }

    param0->unk_A8 = 3;
    param0->unk_A4 = 1;
    param0->unk_C0.unk_20 = 0;
}

void ov5_021E9998(UnkStruct_ov5_021E8F60 *param0)
{
    u8 v0;

    for (v0 = 0; v0 < 4; v0++) {
        if (param0->loadedMaps[v0]->mapPropManager != NULL) {
            MapPropManager_Init(
                param0->loadedMaps[v0]->mapPropManager);
        }
    }
}

void ov5_021E99C4(UnkStruct_ov5_021E8F60 *param0)
{
    ov5_021EEB48(param0->unk_00);
    Heap_FreeToHeap((void *)param0);
}

void ov5_021E99D8(UnkStruct_ov5_021E8F60 *param0, const int param1, const int param2)
{
    ov5_021E9A14(param0, param1, param2, param0->unk_100, param0->unk_108, param0->mapTilesWidth);
    ov5_021E8F90(param1, param2, param0->unk_100, param0->unk_108, param0);
}

static void ov5_021E9A14(UnkStruct_ov5_021E8F60 *param0, const int param1, const int param2, const int param3, const int param4, const int param5)
{
    u8 v0;
    int v1[4];

    ov5_021E7AC4(param0);
    ov5_021E7838_CalculateLoadedMapMatrixIndexes(param0->unk_F8, param1, param2, param0->unk_100, param0->unk_108, param0->mapMatrixWidth, param0->mapMatrixHeight, param5, v1);

    for (v0 = 0; v0 < 4; v0++) {
        param0->loadedMaps[v0]->bdhc = NULL;
        ov5_021E9B70(v1[v0], v0, param0->unk_AC, param0->mapMatrix, param0->mapMatrixWidth, param0->mapMatrixHeight, AreaDataManager_IsOutdoorsLighting(param0->unk_AC), param0);
    }
}

void ov5_021E9AAC(UnkStruct_ov5_021E8F60 *param0, const int param1, const int param2, int *param3)
{
    int v0;

    ov5_021E7B4C(param0);
    ov5_021E7838_CalculateLoadedMapMatrixIndexes(param0->unk_F8, param1, param2, param0->unk_100, param0->unk_108, param0->mapMatrixWidth, param0->mapMatrixHeight, param0->mapTilesWidth, param3);

    for (v0 = 0; v0 < 4; v0++) {
        param0->loadedMaps[v0]->present = 0;
    }
}

void ov5_021E9B10(UnkStruct_ov5_021E8F60 *param0, int param1, int param2)
{
    param0->loadedMaps[param1]->bdhc = NULL;
    ov5_021E9B70(param2, param1, param0->unk_AC, param0->mapMatrix, param0->mapMatrixWidth, param0->mapMatrixHeight, AreaDataManager_IsOutdoorsLighting(param0->unk_AC), param0);
    param0->loadedMaps[param1]->present = 0;
}

static void ov5_021E9B70(const int param0, const u8 param1, const AreaDataManager *param2, const MapMatrix *param3, const int param4, const int param5, const BOOL param6, UnkStruct_ov5_021E8F60 *param7)
{
    int landDataId, v1;
    LandDataHeader landDataHeader;
    NNSG3dResMdl *v3;

    if ((param0 < 0) || (param0 >= param4 * param5)) {
        return;
    }

    landDataId = MapMatrix_GetLandDataIdByIndex(param0, param3);
    LandDataHeader_Load(param7->landDataNARC, landDataId, &landDataHeader);

    v1 = 0x800 + landDataHeader.sectionSize3DObject;
    NARC_Seek(param7->landDataNARC, v1);

    DC_FlushRange((void *)param7->loadedMaps[param1]->nsbmdFile, landDataHeader.sectionSizeNSBMD);
    v3 = ov5_021E97AC_LoadNSBMD(param7->landDataNARC, landDataHeader.sectionSizeNSBMD, &param7->loadedMaps[param1]->mapRenderObj, &(param7->loadedMaps[param1]->nsbmdFile), AreaDataManager_GetMapTexture(param2));

    if (param6 == 1) {
        ov5_021D53A4(v3);
    }

    param7->loadedMaps[param1]->present = 1;
    param7->loadedMaps[param1]->mapMatrixIndex = param0;
}

void ov5_021E9C0C(const UnkStruct_ov5_021E8F60 *param0, const UnkStruct_ov5_021D5894 *param1)
{
    int v0;

    for (v0 = 0; v0 < 4; v0++) {
        if (param0->loadedMaps[v0]->mapMatrixIndex != -1) {
            ov5_021E9C40(v0, param0, param1);
        }
    }
}

static void ov5_021E9C40(const u8 param0, const UnkStruct_ov5_021E8F60 *param1, const UnkStruct_ov5_021D5894 *param2)
{
    VecFx32 v0 = { 0, 0, 0 };
    VecFx32 v1 = { FX32_ONE, FX32_ONE, FX32_ONE };
    MtxFx33 v2 = { FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE };

    ov5_021E901C(param1->loadedMaps[param0]->mapMatrixIndex, param1->mapMatrixWidth, param1->mapMatrix, &v0);

    v0.x += param1->unk_10C.x;
    v0.y += param1->unk_10C.y;
    v0.z += param1->unk_10C.z;

    if (param1->loadedMaps[param0]->present == 1) {
        Easy3D_DrawRenderObj(&(param1->loadedMaps[param0]->mapRenderObj), &v0, &v2, &v1);
    }
}

void ov5_021E9CD8(UnkStruct_ov5_021E8F60 *param0)
{
    int v0;

    for (v0 = 0; v0 < 2; v0++) {
        ov5_021E7814(&param0->unk_04[v0].unk_00);
    }

    for (v0 = 0; v0 < 4; v0++) {
        param0->loadedMaps[v0]->present = 0;
    }

    param0->unk_A8 = 3;
    param0->unk_A4 = 1;
    param0->unk_C0.unk_20 = 0;

    for (v0 = 0; v0 < 4; v0++) {
        MI_CpuFillFast(param0->loadedMaps[v0]->movementPermissions, 0xffffffff, 2 * 32 * 32);
    }
}

void ov5_021E9D3C(MapMatrix *param0, AreaDataManager *param1, UnkStruct_ov5_021E8F60 *param2, UnkStruct_ov5_021E8F60 *param3, const int param4, const int param5)
{
    int v0;
    int v1[4];

    ov5_021EEC24(param2->unk_00, param3->unk_00);
    MapMatrix_Copy(param0, param3->mapMatrix);

    for (v0 = 0; v0 < 2; v0++) {
        param3->unk_04[v0] = param2->unk_04[v0];
    }

    for (v0 = 0; v0 < 4; v0++) {
        param3->unk_74[v0] = param2->unk_74[v0];
    }

    param3->unk_94 = param2->unk_94;
    param3->unk_95 = param2->unk_95;
    param3->unk_96 = param2->unk_96;
    param3->unk_98 = param2->unk_98;
    param3->unk_9C = param2->unk_9C;
    param3->unk_A0 = param2->unk_A0;
    param3->unk_A1 = param2->unk_A1;

    for (v0 = 0; v0 < 4; v0++) {
        param3->loadedMaps[v0]->mapRenderObj = param2->loadedMaps[v0]->mapRenderObj;
        param3->loadedMaps[v0]->nsbmdFile = param2->loadedMaps[v0]->nsbmdFile;
        param3->loadedMaps[v0]->present = param2->loadedMaps[v0]->present;
        param3->loadedMaps[v0]->mapMatrixIndex = param2->loadedMaps[v0]->mapMatrixIndex;
    }

    param3->unk_AC = param1;
    param3->mapMatrixWidth = param2->mapMatrixWidth;
    param3->mapMatrixHeight = param2->mapMatrixHeight;
    param3->mapTilesWidth = param2->mapMatrixWidth * 32;
    param3->unk_C0 = param2->unk_C0;
    param3->unk_F8 = param2->unk_F8;
    param3->unk_100 = param2->unk_100;
    param3->unk_104 = param2->unk_104;
    param3->unk_108 = param2->unk_108;
    param3->unk_10C = param2->unk_10C;

    ov5_021E8F90(param4, param5, param3->unk_100, param3->unk_108, param3);

    param3->unk_A4 = 0;
    param3->unk_A8 = param2->unk_A8;

    for (v0 = 0; v0 < 4; v0++) {
        v1[v0] = param3->loadedMaps[v0]->mapMatrixIndex;
        BDHC_MarkNotLoaded(param3->loadedMaps[v0]->bdhc);
    }

    for (v0 = 0; v0 < 4; v0++) {
        ov5_021E7FF0(v1[v0], v0, param3->unk_AC, param3->mapMatrix, param3->mapMatrixWidth, param3->mapMatrixHeight, AreaDataManager_IsOutdoorsLighting(param3->unk_AC), param3);
    }
}

void ov5_021E9F98_Copy(UnkStruct_ov5_021E8F60 *param0, UnkStruct_ov5_021E8F60 *param1)
{
    int v0;
    int v1[4];

    ov5_021EEC24(param1->unk_00, param0->unk_00);
    MapMatrix_Copy(param1->mapMatrix, param0->mapMatrix);

    for (v0 = 0; v0 < 2; v0++) {
        param0->unk_04[v0] = param1->unk_04[v0];
    }

    for (v0 = 0; v0 < 4; v0++) {
        param0->unk_74[v0] = param1->unk_74[v0];
    }

    param0->unk_94 = param1->unk_94;
    param0->unk_95 = param1->unk_95;
    param0->unk_96 = param1->unk_96;
    param0->unk_98 = param1->unk_98;
    param0->unk_9C = param1->unk_9C;
    param0->unk_A0 = param1->unk_A0;
    param0->unk_A1 = param1->unk_A1;

    for (v0 = 0; v0 < 4; v0++) {
        param0->loadedMaps[v0]->mapRenderObj = param1->loadedMaps[v0]->mapRenderObj;
        param0->loadedMaps[v0]->nsbmdFile = param1->loadedMaps[v0]->nsbmdFile;
        param0->loadedMaps[v0]->present = param1->loadedMaps[v0]->present;
        param0->loadedMaps[v0]->mapMatrixIndex = param1->loadedMaps[v0]->mapMatrixIndex;
    }

    param0->unk_AC = param1->unk_AC;
    param0->mapMatrixWidth = param1->mapMatrixWidth;
    param0->mapMatrixHeight = param1->mapMatrixHeight;
    param0->mapTilesWidth = param1->mapMatrixWidth * 32;
    param0->unk_C0 = param1->unk_C0;
    param0->unk_F8 = param1->unk_F8;
    param0->unk_100 = param1->unk_100;
    param0->unk_104 = param1->unk_104;
    param0->unk_108 = param1->unk_108;
    param0->unk_10C = param1->unk_10C;
    param0->unk_A4 = 0;
    param0->unk_A8 = param1->unk_A8;
}

void ov5_021EA174(FieldSystem *fieldSystem, UnkStruct_ov5_021E8F60 *param1)
{
    u8 v0;
    UnkStruct_ov5_021E7814 *v1;
    UnkStruct_ov5_021EA174 *v2;

    v1 = &param1->unk_04[param1->unk_96].unk_00;
    v2 = &param1->unk_C0;

    if ((param1->unk_F8 == 0) && (v2->unk_20 == 1)) {
        if (v2->unk_18) {
            GF_ASSERT((*v2->unk_14) <= (*v2->unk_10));

            if ((*v2->unk_10) - (*v2->unk_14) >= (FX32_ONE * 16)) {
                int v3, v4;

                ov5_021E9230(v2->unk_0C->x, v2->unk_0C->z, &v3, &v4);
                ov5_021E8F90(v3, v4, param1->unk_100, param1->unk_108, param1);

                v2->unk_00 = *v2->unk_0C;

                if (!(v2->unk_00.z % (FX32_ONE * 8) == 0)) {
                    if (Link_SetErrorState(1)) {
                        return;
                    }
                }

                GF_ASSERT(v2->unk_00.z % (FX32_ONE * 8) == 0);

                v2->unk_18 = 0;
                v2->unk_10 = NULL;
                v2->unk_14 = NULL;
            }
        } else {
            if ((v2->unk_00.x != v2->unk_0C->x) && (v2->unk_00.z != v2->unk_0C->z)) {
                GF_ASSERT(FALSE);
            } else if ((v2->unk_00.x == v2->unk_0C->x) && (v2->unk_00.z == v2->unk_0C->z)) {
                (void)0;
            } else {
                v2->unk_18 = 1;

                if (v2->unk_00.x != v2->unk_0C->x) {
                    if (v2->unk_00.x > v2->unk_0C->x) {
                        v2->unk_10 = &v2->unk_00.x;
                        v2->unk_14 = &v2->unk_0C->x;
                        v2->unk_1C = 3;
                    } else {
                        v2->unk_10 = &v2->unk_0C->x;
                        v2->unk_14 = &v2->unk_00.x;
                        v2->unk_1C = 1;
                    }

                    if (!(v2->unk_00.x % (FX32_ONE * 8) == 0)) {
                        if (Link_SetErrorState(1)) {
                            return;
                        }
                    }

                    GF_ASSERT(v2->unk_00.x % (FX32_ONE * 8) == 0);
                } else {
                    if (param1->unk_C0.unk_00.z > param1->unk_C0.unk_0C->z) {
                        v2->unk_10 = &v2->unk_00.z;
                        v2->unk_14 = &v2->unk_0C->z;
                        v2->unk_1C = 4;
                    } else {
                        v2->unk_10 = &v2->unk_0C->z;
                        v2->unk_14 = &v2->unk_00.z;
                        v2->unk_1C = 2;
                    }

                    if (!(v2->unk_00.z % (FX32_ONE * 8) == 0)) {
                        if (Link_SetErrorState(1)) {
                            return;
                        }
                    }

                    GF_ASSERT(v2->unk_00.z % (FX32_ONE * 8) == 0);
                }

                ov5_021E9190(v2->unk_1C, param1);
            }
        }
    }

    switch (param1->unk_A8) {
    case 0:
        if (param1->unk_74[v1->unk_20[v1->unk_22]] == 1) {
            (void)0;
        } else {
            ov5_021E7D98(v1->unk_22, param1->unk_AC, param1->mapMatrix, param1->mapMatrixWidth, param1->mapMatrixHeight, param1, v1);
        }

        v1->unk_22++;
        param1->unk_A8 = 1;
        break;
    case 1:
        if (param1->unk_74[v1->unk_20[v1->unk_22 - 1]] == 1) {
            ov5_021E7814(v1);
            param1->unk_74[v1->unk_20[v1->unk_22 - 1]] = 0;
        }

        if ((v1->unk_22 <= 2) && (ov5_021E80BC(&v1->unk_10) == 1)) {
            if (v1->unk_22 >= 2) {
                v1->unk_24 = 1;
            } else {
                param1->unk_A8 = 0;
            }

            if (v1->unk_00[v1->unk_22 - 1]->present == 1) {
                NNSG3dResMdlSet *v5 = NNS_G3dGetMdlSet(v1->unk_00[v1->unk_22 - 1]->nsbmdFile);
                NNSG3dResMdl *v6 = NNS_G3dGetMdlByIdx(v5, 0);

                if (AreaDataManager_IsOutdoorsLighting(param1->unk_AC) == 1) {
                    ov5_021D53A4(v6);
                }
            }

            if (param1->unk_F0 != NULL) {
                if ((0 <= v1->unk_00[v1->unk_22 - 1]->mapMatrixIndex) && (v1->unk_00[v1->unk_22 - 1]->mapMatrixIndex < param1->mapMatrixWidth * param1->mapMatrixHeight)) {
                    param1->unk_F0(param1->unk_F4, v1->unk_00[v1->unk_22 - 1]->mapMatrixIndex, v1->unk_00[v1->unk_22 - 1]->mapPropManager);
                }
            }
        }
        break;
    case 2:
        break;
    case 3:
        if (ov5_021E80BC(&v1->unk_10) == 1) {
            param1->unk_94 = 0;
        }
        break;
    }

    if (param1->unk_94 == 0) {
        param1->unk_96 = 0;
        param1->unk_95 = 0;
        param1->unk_A8 = 2;
        return;
    }

    if (ov5_021E8DAC(param1) == 1) {
        param1->unk_04[param1->unk_96].unk_28 = 0;
        ov5_021E8DBC(param1);
        param1->unk_94--;
        param1->unk_96 = (param1->unk_96 + 1) % 2;

        if (param1->unk_94 != 0) {
            param1->unk_A8 = 0;
        } else {
            param1->unk_96 = 0;
            param1->unk_95 = 0;
            param1->unk_A8 = 2;
        }

        if (param1->unk_5C.unk_08 == 1) {
            BOOL v7;

            param1->unk_5C.unk_08 = 0;

            ov5_021E8DD4(param1->unk_5C.unk_0C, param1);
            ov5_021E8D50(param1);
        }
    }
}

void ov5_021EA540(UnkStruct_ov5_021E8F60 *param0, MapMatrix *mapMatrix, AreaDataManager *param2)
{
    param0->unk_AC = param2;
    param0->mapMatrix = mapMatrix;
    param0->mapMatrixWidth = MapMatrix_GetWidth(mapMatrix);
    param0->mapMatrixHeight = MapMatrix_GetHeight(mapMatrix);
    param0->mapTilesWidth = param0->mapMatrixWidth * 32;

    ov5_021E8F60(param0);

    param0->unk_A4 = 0;
    param0->unk_A8 = 2;
}

void ov5_021EA58C(UnkStruct_ov5_021E8F60 *param0, const int param1, const int param2, int *param3)
{
    int v0;

    ov5_021E7838_CalculateLoadedMapMatrixIndexes(param0->unk_F8, param1, param2, param0->unk_100, param0->unk_108, param0->mapMatrixWidth, param0->mapMatrixHeight, param0->mapTilesWidth, param3);

    for (v0 = 0; v0 < 4; v0++) {
        param0->loadedMaps[v0]->present = 0;
    }
}

void ov5_021EA5E0(UnkStruct_ov5_021E8F60 *param0, int param1, int param2)
{
    param0->loadedMaps[param1]->present = 0;
    ov5_021EEB84(param1, param0->unk_00, (void **)&(param0->loadedMaps[param1]->nsbmdFile));
    ov5_021EEB90(param1, param0->unk_00, (void **)&(param0->loadedMaps[param1]->bdhcBuffer));
    param0->loadedMaps[param1]->mapMatrixIndex = -1;

    MI_CpuFillFast(param0->loadedMaps[param1]->movementPermissions, 0xffffffff, 2 * 32 * 32);

    BDHC_MarkNotLoaded(param0->loadedMaps[param1]->bdhc);
    ov5_021E7E28_LoadLandData(param2, param1, param0->unk_AC, param0->mapMatrix, param0->mapMatrixWidth, param0->mapMatrixHeight, AreaDataManager_IsOutdoorsLighting(param0->unk_AC), param0);
}

void ov5_021EA678(UnkStruct_ov5_021E8F60 *param0, int param1, int param2, int param3)
{
    param0->unk_100 = param1;
    param0->unk_104 = param2;
    param0->unk_108 = param3;
    param0->unk_10C.x = param1 * 16 * FX32_ONE;
    param0->unk_10C.y = param2 * 16 * FX32_ONE;
    param0->unk_10C.z = param3 * 16 * FX32_ONE;
}

void ov5_021EA6A4(UnkStruct_ov5_021E8F60 *param0, int param1)
{
    param0->unk_F8 = param1;
}

int ov5_021EA6AC(const UnkStruct_ov5_021E8F60 *param0)
{
    return param0->unk_100;
}

int ov5_021EA6B4(const UnkStruct_ov5_021E8F60 *param0)
{
    return param0->unk_108;
}

void ov5_021EA6BC(const UnkStruct_ov5_021E8F60 *param0, VecFx32 *param1)
{
    *param1 = param0->unk_10C;
}

void ov5_021EA6D0(UnkStruct_ov5_021E8F60 *param0, int param1)
{
    param0->unk_FC = param1;
}

void ov5_021EA6D8_SetMapMatrix(UnkStruct_ov5_021E8F60 *param0, MapMatrix *mapMatrix)
{
    param0->mapMatrix = mapMatrix;
}

void ov5_021EA6E0(UnkStruct_ov5_021E8F60 *param0, int param1, BOOL param2)
{
    param0->loadedMaps[param1]->present = param2;
}

void ov5_021EA6F4(UnkStruct_ov5_021E8F60 *param0, int param1, int param2)
{
    ov5_021E8F90(param1, param2, param0->unk_100, param0->unk_108, param0);
}
