#include "overlay005/map_prop_animation.h"

#include <nitro.h>
#include <string.h>

#include "constants/heap.h"

#include "heap.h"
#include "narc.h"
#include "unk_02005474.h"

typedef struct {
    BOOL loaded;
    NNSG3dRenderObj *renderObj;
    MapPropAnimation *animation;
    int animeArchiveID;
} BicycleSlopeAnimation;

typedef struct UnkStruct_ov5_021D3CE4_t {
    NNSG3dAnmObj *animationObj;
    BOOL loaded;
    int currentLoop;
    int animeArchiveID;
    BOOL paused;
    void *animationFile;
    int loopCount;
    BOOL looping;
    BOOL reversed;
    int pastoriaGymButtonGroup;
} MapPropAnimation;

typedef struct {
    u8 unk_00;
    u8 flags;
    u8 isBicycleSlope;
    u8 unk_03;
    int animeArchiveIDs[4];
} MapPropAnimeListFile;

typedef struct UnkStruct_ov5_021D3CAC_t {
    NNSFndAllocator allocator;
    MapPropAnimation animations[16];
    BicycleSlopeAnimation bicycleSlopeAnimations[2];
    NARC *animeNARC;
    NARC *animeListNARC;
} MapPropAnimationManager;

typedef struct {
    MapPropAnimation *data[4];
    int count;
} MapPropOneShotAnimationList;

typedef struct UnkStruct_ov5_021D3FD0_t {
    MapPropOneShotAnimationList animations;
    NNSG3dRenderObj *mapPropRenderObjs[6];
    MapPropAnimation *currentAnimation;
    int mapPropModelID;
    u8 tag;
} MapPropOneShotAnimation;

typedef struct UnkStruct_ov5_021D41B4_t {
    MapPropOneShotAnimation items[16];
} MapPropOneShotAnimationManager;

static BOOL BicycleSlopeAnimation_Load(BicycleSlopeAnimation *bicycleSlopeAnims, NNSG3dRenderObj *renderObj, MapPropAnimation *animation, const u8 animeArchiveID)
{
    int i;

    GF_ASSERT(animation->loopCount != -1);

    for (i = 0; i < 2; i++) {
        if (bicycleSlopeAnims[i].loaded) {
            if ((bicycleSlopeAnims[i].animeArchiveID == animeArchiveID) && (bicycleSlopeAnims[i].renderObj == renderObj)) {
                return FALSE;
            }
        }
    }

    for (i = 0; i < 2; i++) {
        if (!bicycleSlopeAnims[i].loaded) {
            bicycleSlopeAnims[i].loaded = TRUE;
            bicycleSlopeAnims[i].animeArchiveID = animeArchiveID;
            bicycleSlopeAnims[i].animation = animation;
            bicycleSlopeAnims[i].renderObj = renderObj;
            break;
        }
    }

    return TRUE;
}

static void *MapPropAnimation_LoadAnimationObj(NARC *animeNARC, const u32 animeArchiveID, NNSG3dResMdl *mapPropModel, NNSG3dAnmObj **animationObj, NNSG3dResTex *mapPropTexture, NNSFndAllocator *allocator)
{
    void *animation;
    void *animationFile = NULL;

    GF_ASSERT(animeArchiveID != 0xFFFFFFFF);
    animationFile = NARC_AllocAndReadWholeMember(animeNARC, animeArchiveID, HEAP_ID_FIELD);
    GF_ASSERT(animationFile != NULL);

    animation = NNS_G3dGetAnmByIdx(animationFile, 0);
    GF_ASSERT(animation != NULL);

    *animationObj = NNS_G3dAllocAnmObj(allocator, animation, mapPropModel);
    GF_ASSERT(animationObj != NULL);

    NNS_G3dAnmObjInit(*animationObj, animation, mapPropModel, mapPropTexture);
    return animationFile;
}

static const BOOL MapPropAnimation_RemoveAnimationObjFromRenderObj(NNSG3dRenderObj *renderObj, NNSG3dAnmObj *animationObj)
{
    if (animationObj == NULL) {
        return FALSE;
    } else if (renderObj == NULL) {
        return FALSE;
    }

    NNS_G3dRenderObjRemoveAnmObj(renderObj, animationObj);
    return TRUE;
}

static void BicycleSlopeAnimation_ResetFinishedAnimations(BicycleSlopeAnimation *bicycleSlopeAnims)
{
    for (int i = 0; i < 2; i++) {
        if (bicycleSlopeAnims[i].loaded) {
            if (MapPropAnimation_IsLoopFinished(bicycleSlopeAnims[i].animation)) {
                MapPropAnimation_RemoveAnimationObjFromRenderObj(bicycleSlopeAnims[i].renderObj, bicycleSlopeAnims[i].animation->animationObj);
                bicycleSlopeAnims[i].loaded = FALSE;
                bicycleSlopeAnims[i].animeArchiveID = 0;
                bicycleSlopeAnims[i].animation = NULL;
                bicycleSlopeAnims[i].renderObj = NULL;
            }
        }
    }
}

MapPropAnimationManager *MapPropAnimationManager_New(void)
{
    int i;
    MapPropAnimationManager *manager;

    manager = Heap_AllocFromHeap(HEAP_ID_FIELD, sizeof(MapPropAnimationManager));

    for (i = 0; i < 16; i++) {
        manager->animations[i].loaded = FALSE;
        manager->animations[i].paused = 0;
        manager->animations[i].animationObj = NULL;
        manager->animations[i].currentLoop = 0;
        manager->animations[i].animeArchiveID = 0;
        manager->animations[i].animationFile = NULL;
        manager->animations[i].loopCount = -1;
        manager->animations[i].looping = FALSE;
        manager->animations[i].reversed = FALSE;
        manager->animations[i].pastoriaGymButtonGroup = 0;
    }

    for (i = 0; i < 2; i++) {
        manager->bicycleSlopeAnimations[i].loaded = FALSE;
        manager->bicycleSlopeAnimations[i].renderObj = NULL;
        manager->bicycleSlopeAnimations[i].animation = NULL;
        manager->bicycleSlopeAnimations[i].animeArchiveID = 0;
    }

    Heap_FndInitAllocatorForExpHeap(&manager->allocator, HEAP_ID_FIELD, 4);

    manager->animeNARC = NARC_ctor(NARC_INDEX_ARC__BM_ANIME, HEAP_ID_FIELD);
    manager->animeListNARC = NARC_ctor(NARC_INDEX_ARC__BM_ANIME_LIST, HEAP_ID_FIELD);

    return manager;
}

static NNSG3dAnmObj *MapPropAnimation_GetAnimationObj(MapPropAnimation *animation)
{
    if (animation == NULL) {
        return NULL;
    }

    return animation->animationObj;
}

static BOOL ov5_021D3944(const u8 animationFlags)
{
    return (animationFlags & 0x1) == 0x1;
}

static BOOL ov5_021D3954(const u8 animationFlags)
{
    return ((animationFlags >> 1) & 0x1) == 0x1;
}

static int MapPropAnimation_IsLooping(const MapPropAnimation *animation)
{
    GF_ASSERT(animation->loaded == TRUE);
    return animation->looping;
}

MapPropAnimation *MapPropAnimationManager_LoadPropAnimations(const int mapPropModelID, const int mapPropAnimationIndex, const int animationLoopCount, const int pastoriaGymButtonGroup, const BOOL animationReversed, const BOOL animationPaused, const BOOL param6, NNSG3dResMdl *mapPropModel, NNSG3dResTex *mapPropTexture, MapPropAnimationManager *manager)
{
    MapPropAnimation *animation;
    int i;
    int animeArchiveID;
    MapPropAnimeListFile animeListFile;

    NARC_ReadWholeMember(manager->animeListNARC, mapPropModelID, &animeListFile);
    GF_ASSERT(mapPropAnimationIndex < 4);

    animeArchiveID = animeListFile.animeArchiveIDs[mapPropAnimationIndex];

    if (animeArchiveID == 0xFFFFFFFF) {
        return NULL;
    }

    if (ov5_021D3944(animeListFile.flags) != param6) {
        return NULL;
    }

    for (i = 0; i < 16; i++) {
        if ((pastoriaGymButtonGroup != 0) && (manager->animations[i].pastoriaGymButtonGroup == pastoriaGymButtonGroup)) {
            GF_ASSERT(FALSE);
        }
    }

    for (i = 0; i < 16; i++) {
        if (!manager->animations[i].loaded) {
            manager->animations[i].loaded = TRUE;
            manager->animations[i].paused = animationPaused;
            manager->animations[i].animationObj = NULL;
            manager->animations[i].currentLoop = 0;
            manager->animations[i].animeArchiveID = animeArchiveID;
            manager->animations[i].reversed = animationReversed;
            manager->animations[i].pastoriaGymButtonGroup = pastoriaGymButtonGroup;
            manager->animations[i].animationFile = MapPropAnimation_LoadAnimationObj(manager->animeNARC, animeArchiveID, mapPropModel, &manager->animations[i].animationObj, mapPropTexture, &manager->allocator);
            manager->animations[i].loopCount = animationLoopCount;
            manager->animations[i].looping = TRUE;

            animation = &manager->animations[i];

            MapPropAnimation_GoToFirstFrame(&manager->animations[i]);
            return animation;
        }
    }

    GF_ASSERT(FALSE);
    return NULL;
}

void MapPropAnimationManager_LoadPropAnimationsSimple(const int mapPropModelID, NNSG3dResMdl *mapPropModel, NNSG3dResTex *mapPropTexture, MapPropAnimationManager *manager)
{
    int animeArchiveID;
    int i;
    int j;
    MapPropAnimeListFile animeListFile;

    NARC_ReadWholeMember(manager->animeListNARC, mapPropModelID, &animeListFile);

    if (!animeListFile.unk_00) {
        return;
    }

    for (i = 0; i < 4; i++) {
        animeArchiveID = animeListFile.animeArchiveIDs[i];

        if (animeArchiveID == 0xFFFFFFFF) {
            return;
        }

        if (ov5_021D3944(animeListFile.flags)) {
            return;
        }

        for (j = 0; j < 16; j++) {
            if (!manager->animations[j].loaded) {
                manager->animations[j].loaded = TRUE;
                manager->animations[j].paused = FALSE;
                manager->animations[j].animationObj = NULL;
                manager->animations[j].currentLoop = 0;
                manager->animations[j].animeArchiveID = animeArchiveID;
                manager->animations[j].reversed = FALSE;
                manager->animations[j].pastoriaGymButtonGroup = 0;
                manager->animations[j].animationFile = MapPropAnimation_LoadAnimationObj(manager->animeNARC, animeArchiveID, mapPropModel, &manager->animations[j].animationObj, mapPropTexture, &manager->allocator);
                manager->animations[j].loopCount = -1;
                manager->animations[j].looping = TRUE;

                MapPropAnimation_GoToFirstFrame(&manager->animations[j]);

                if (animeListFile.isBicycleSlope) {
                    manager->animations[j].paused = TRUE;
                    manager->animations[j].loopCount = 1;
                }

                break;
            }
        }

        GF_ASSERT(j != 16);
    }
}

BOOL MapPropAnimationManager_AddAnimationToRenderObj(const int mapPropModelID, const int mapPropAnimationIndex, const BOOL param2, NNSG3dRenderObj *mapPropRenderObj, MapPropAnimationManager *manager)
{
    int i;
    int animeArchiveID;
    MapPropAnimeListFile animeListFile;
    BOOL addAnimationObj;

    if (manager == NULL) {
        GF_ASSERT(FALSE);
        return FALSE;
    }

    if (mapPropModelID >= MapPropAnimationManager_GetAnimeListNARCFileCount(manager)) {
        return FALSE;
    }

    NARC_ReadWholeMember(manager->animeListNARC, mapPropModelID, &animeListFile);
    GF_ASSERT(mapPropAnimationIndex < 4);

    animeArchiveID = animeListFile.animeArchiveIDs[mapPropAnimationIndex];

    if (animeArchiveID == 0xFFFFFFFF) {
        return FALSE;
    }

    if (ov5_021D3954(animeListFile.flags) != param2) {
        return FALSE;
    }

    for (i = 0; i < 16; i++) {
        if (animeArchiveID == manager->animations[i].animeArchiveID) {
            if (animeListFile.isBicycleSlope) {
                addAnimationObj = BicycleSlopeAnimation_Load(manager->bicycleSlopeAnimations, mapPropRenderObj, &manager->animations[i], animeArchiveID);
            } else {
                addAnimationObj = TRUE;
            }

            if (addAnimationObj) {
                NNS_G3dRenderObjAddAnmObj(mapPropRenderObj, manager->animations[i].animationObj);
            }

            return TRUE;
        }
    }

    return FALSE;
}

BOOL MapPropAnimationManager_AddAllAnimationsToRenderObj(const int mapPropModelID, NNSG3dRenderObj *mapPropRenderObj, MapPropAnimationManager *manager)
{
    int i;
    int j;
    int animeArchiveID;
    MapPropAnimeListFile animeListFile;
    BOOL res;
    BOOL addAnimationObj;

    res = FALSE;

    if (manager == NULL) {
        GF_ASSERT(FALSE);
    }

    if (mapPropModelID >= MapPropAnimationManager_GetAnimeListNARCFileCount(manager)) {
        return FALSE;
    }

    NARC_ReadWholeMember(manager->animeListNARC, mapPropModelID, &animeListFile);

    if (ov5_021D3954(animeListFile.flags)) {
        return FALSE;
    }

    for (i = 0; i < 4; i++) {
        animeArchiveID = animeListFile.animeArchiveIDs[i];

        if (animeArchiveID == 0xFFFFFFFF) {
            return res;
        }

        for (j = 0; j < 16; j++) {
            if (animeArchiveID == manager->animations[j].animeArchiveID) {
                if (animeListFile.isBicycleSlope) {
                    addAnimationObj = BicycleSlopeAnimation_Load(manager->bicycleSlopeAnimations, mapPropRenderObj, &manager->animations[j], animeArchiveID);
                } else {
                    addAnimationObj = TRUE;
                }

                if (addAnimationObj) {
                    NNS_G3dRenderObjAddAnmObj(mapPropRenderObj, manager->animations[j].animationObj);
                    res = TRUE;
                }

                break;
            }
        }
    }

    return res;
}

void MapPropAnimationManager_UnloadAllAnimations(MapPropAnimationManager *manager)
{
    if (manager == NULL) {
        return;
    }

    for (int i = 0; i < 16; i++) {
        if (manager->animations[i].loaded) {
            if (manager->animations[i].loaded == TRUE) {
                NNS_G3dFreeAnmObj(&manager->allocator, manager->animations[i].animationObj);
                manager->animations[i].animationObj = NULL;
                Heap_FreeToHeap(manager->animations[i].animationFile);
            }

            manager->animations[i].loaded = FALSE;
            manager->animations[i].pastoriaGymButtonGroup = 0;
        }
    }
}

void MapPropAnimationManager_UnloadAnimation(MapPropAnimation *animation, MapPropAnimationManager *manager)
{
    if (manager == NULL) {
        return;
    }

    GF_ASSERT(animation != NULL);

    if (animation->loaded) {
        if (animation->loaded == TRUE) {
            NNS_G3dFreeAnmObj(&manager->allocator, animation->animationObj);
            animation->animationObj = NULL;
            Heap_FreeToHeap(animation->animationFile);
        }

        animation->loaded = FALSE;
        animation->pastoriaGymButtonGroup = 0;
    }
}

void MapPropAnimationManager_RemoveAnimationFromRenderObj(MapPropAnimationManager *manager, NNSG3dRenderObj *mapPropRenderObj, const int mapPropModelID, const int mapPropAnimationIndex)
{
    u8 i;
    int animeArchiveID;
    MapPropAnimeListFile animeListFile;

    NARC_ReadWholeMember(manager->animeListNARC, mapPropModelID, &animeListFile);

    GF_ASSERT(mapPropAnimationIndex < 4);
    animeArchiveID = animeListFile.animeArchiveIDs[mapPropAnimationIndex];
    GF_ASSERT(animeArchiveID != 0xFFFFFFFF);

    for (i = 0; i < 16; i++) {
        if (manager->animations[i].loaded == TRUE) {
            if (manager->animations[i].animeArchiveID == animeArchiveID) {
                MapPropAnimation_RemoveAnimationObjFromRenderObj(mapPropRenderObj, manager->animations[i].animationObj);
                return;
            }
        }
    }
}

void MapPropAnimationManager_Free(MapPropAnimationManager *manager)
{
    if (manager == NULL) {
        return;
    }

    NARC_dtor(manager->animeNARC);
    NARC_dtor(manager->animeListNARC);
    Heap_FreeToHeap(manager);

    manager = NULL;
}

MapPropAnimation *MapPropAnimationManager_GetAnimationByPastoriaGymButtonGroup(const int pastoriaGymButtonGroup, MapPropAnimationManager *manager)
{
    GF_ASSERT(pastoriaGymButtonGroup != 0);
    MapPropAnimation *animation = NULL;

    for (int i = 0; i < 16; i++) {
        if (manager->animations[i].pastoriaGymButtonGroup == pastoriaGymButtonGroup) {
            animation = &manager->animations[i];
            GF_ASSERT(animation->loaded);
            break;
        }
    }

    GF_ASSERT(animation != NULL);
    return animation;
}

MapPropAnimation *MapPropAnimationManager_GetAnimation(const int mapPropModelID, const int mapPropAnimationIndex, MapPropAnimationManager *manager)
{
    MapPropAnimation *animation;
    int animeArchiveID;
    int i;
    MapPropAnimeListFile animeListFile;

    NARC_ReadWholeMember(manager->animeListNARC, mapPropModelID, &animeListFile);

    GF_ASSERT(mapPropAnimationIndex < 4);

    animeArchiveID = animeListFile.animeArchiveIDs[mapPropAnimationIndex];
    animation = NULL;

    for (i = 0; i < 16; i++) {
        if (manager->animations[i].animeArchiveID == animeArchiveID) {
            animation = &manager->animations[i];
            GF_ASSERT(animation->loaded);
            break;
        }
    }

    GF_ASSERT(animation != NULL);
    return animation;
}

void MapPropAnimation_SetAnimationPaused(MapPropAnimation *animation, const BOOL paused)
{
    animation->paused = paused;
}

void MapPropAnimation_StartLoop(MapPropAnimation *animation)
{
    animation->looping = TRUE;
}

BOOL MapPropAnimation_IsOnLastFrame(MapPropAnimation *animation)
{
    BOOL onLastFrame;

    if (!animation->reversed) {
        if (animation->animationObj->frame >= NNS_G3dAnmObjGetNumFrame(animation->animationObj) - (FX32_ONE)) {
            onLastFrame = TRUE;
        } else {
            onLastFrame = FALSE;
        }
    } else {
        if (animation->animationObj->frame == 0) {
            onLastFrame = TRUE;
        } else {
            onLastFrame = FALSE;
        }
    }

    return onLastFrame;
}

void MapPropAnimation_GoToFirstFrame(MapPropAnimation *animation)
{
    if (!animation->reversed) {
        animation->animationObj->frame = 0;
    } else {
        animation->animationObj->frame = NNS_G3dAnmObjGetNumFrame(animation->animationObj) - (FX32_ONE);
    }
}

void MapPropAnimation_GoToLastFrame(MapPropAnimation *animation)
{
    if (!animation->reversed) {
        animation->animationObj->frame = NNS_G3dAnmObjGetNumFrame(animation->animationObj) - (FX32_ONE);
    } else {
        animation->animationObj->frame = 0;
    }
}

void MapPropAnimation_AdvanceFrame(MapPropAnimation *animation)
{
    if (!animation->reversed) {
        animation->animationObj->frame += (FX32_ONE);

        if (animation->animationObj->frame == NNS_G3dAnmObjGetNumFrame(animation->animationObj)) {
            animation->animationObj->frame = 0;
        }
    } else {
        if (animation->animationObj->frame <= 0) {
            animation->animationObj->frame = NNS_G3dAnmObjGetNumFrame(animation->animationObj) - (FX32_ONE);
        } else {
            animation->animationObj->frame -= (FX32_ONE);
        }
    }
}

void MapPropAnimation_SetReversed(MapPropAnimation *animation, const BOOL reversed)
{
    animation->reversed = reversed;
}

void MapPropAnimation_SetLoopCount(MapPropAnimation *animation, const int loopCount)
{
    animation->loopCount = loopCount;
}

void MapPropAnimation_SetPastoriaGymButtonGroup(MapPropAnimation *animation, const int pastoriaGymButtonGroup)
{
    animation->pastoriaGymButtonGroup = pastoriaGymButtonGroup;
}

void MapPropAnimationManager_AdvanceAnimations(MapPropAnimationManager *manager)
{
    int i;
    MapPropAnimation *animation;

    if (manager == NULL) {
        return;
    }

    for (i = 0; i < 16; i++) {
        animation = &manager->animations[i];

        if (animation->loaded == TRUE) {
            if ((animation->paused == TRUE) || (animation->looping == FALSE)) {
                continue;
            }

            MapPropAnimation_AdvanceFrame(animation);

            if (animation->loopCount != -1) {
                if (MapPropAnimation_IsOnLastFrame(animation)) {
                    if (animation->currentLoop + 1 >= animation->loopCount) {
                        animation->looping = FALSE;
                    } else {
                        animation->currentLoop++;
                    }
                }
            }
        }
    }

    BicycleSlopeAnimation_ResetFinishedAnimations(manager->bicycleSlopeAnimations);
}

BOOL MapPropAnimation_IsLoopFinished(const MapPropAnimation *animation)
{
    return MapPropAnimation_IsLooping(animation) == FALSE;
}

u16 MapPropAnimationManager_GetAnimeListNARCFileCount(MapPropAnimationManager *manager)
{
    return NARC_GetFileCount(manager->animeListNARC);
}

const u8 MapPropAnimationManager_GetPropAnimationCount(MapPropAnimationManager *manager, const int mapPropModelID)
{
    u8 i;
    MapPropAnimeListFile animeListFile;

    NARC_ReadWholeMember(manager->animeListNARC, mapPropModelID, &animeListFile);

    if (!animeListFile.unk_00) {
        return 0;
    }

    for (i = 0; i < 4; i++) {
        if (animeListFile.animeArchiveIDs[i] == 0xFFFFFFFF) {
            break;
        }
    }

    return i;
}

static MapPropOneShotAnimation *MapPropOneShotAnimationManager_AllocateAnimation(MapPropOneShotAnimationManager *oneShotAnimationManager, const u8 tag)
{
    u8 i;
    u8 itemIndex;
    MapPropOneShotAnimation *item;

    item = NULL;
    GF_ASSERT(tag != 0);
    itemIndex = 16;

    for (i = 0; i < 16; i++) {
        if (itemIndex == 16 && oneShotAnimationManager->items[i].tag == 0) {
            itemIndex = i;
        }
    }

    if (itemIndex != 16) {
        oneShotAnimationManager->items[itemIndex].tag = tag;
    } else {
        GF_ASSERT(FALSE);
        return NULL;
    }

    return &oneShotAnimationManager->items[itemIndex];
}

static void MapPropOneShotAnimation_Reset(MapPropOneShotAnimation *oneShotAnimation)
{
    oneShotAnimation->tag = 0;
    oneShotAnimation->mapPropModelID = 0;
    oneShotAnimation->currentAnimation = NULL;

    for (u8 i = 0; i < 6; i++) {
        oneShotAnimation->mapPropRenderObjs[i] = NULL;
    }
}

static MapPropOneShotAnimation *MapPropOneShotAnimationManager_GetAnimationByTag(MapPropOneShotAnimationManager *oneShotAnimationManager, const int tag)
{
    int i;
    MapPropOneShotAnimation *oneShotAnimation = NULL;

    for (i = 0; i < 16; i++) {
        if (oneShotAnimationManager->items[i].tag == tag) {
            oneShotAnimation = &oneShotAnimationManager->items[i];
            break;
        }
    }

    return oneShotAnimation;
}

static void ov5_021D4068(NNSG3dRenderObj *mapPropRenderObj, const int animationCount, MapPropOneShotAnimation *oneShotAnimation)
{
    GF_ASSERT(animationCount <= 4);

    if (mapPropRenderObj != NULL) {
        oneShotAnimation->mapPropRenderObjs[0] = mapPropRenderObj;
    }

    oneShotAnimation->animations.count = animationCount;
}

static void MapPropOneShotAnimation_SetAnimation(MapPropOneShotAnimation *oneShotAnimation, const int mapPropAnimationIndex, MapPropAnimation *animation)
{
    GF_ASSERT(mapPropAnimationIndex < oneShotAnimation->animations.count);
    oneShotAnimation->animations.data[mapPropAnimationIndex] = animation;
}

static MapPropAnimation *MapPropOneShotAnimation_SwitchAnimation(MapPropOneShotAnimation *oneShotAnimation, const int mapPropAnimationIndex)
{
    u8 i;
    MapPropAnimation *newAnimation;
    NNSG3dAnmObj *newAnimationObj;
    NNSG3dAnmObj *currentAnimationObj;

    GF_ASSERT(mapPropAnimationIndex < oneShotAnimation->animations.count);

    newAnimation = oneShotAnimation->animations.data[mapPropAnimationIndex];
    newAnimationObj = MapPropAnimation_GetAnimationObj(newAnimation);
    currentAnimationObj = MapPropAnimation_GetAnimationObj(oneShotAnimation->currentAnimation);

    for (i = 0; i < 6; i++) {
        if (oneShotAnimation->mapPropRenderObjs[i] != NULL) {
            MapPropAnimation_RemoveAnimationObjFromRenderObj(oneShotAnimation->mapPropRenderObjs[i], currentAnimationObj);
            NNS_G3dRenderObjAddAnmObj(oneShotAnimation->mapPropRenderObjs[i], newAnimationObj);
        }
    }

    oneShotAnimation->currentAnimation = newAnimation;
    return newAnimation;
}

static void ov5_021D40F0(const int mapPropModelID, NNSG3dResMdl *mapPropModel, NNSG3dResTex *mapPropTexture, const int animationCount, const int animationLoopCount, const BOOL animationReversed, MapPropAnimationManager *manager, MapPropOneShotAnimation *oneShotAnimation)
{
    for (int i = 0; i < animationCount; i++) {
        MapPropAnimation *animation = MapPropAnimationManager_LoadPropAnimations(mapPropModelID, i, animationLoopCount, 0, animationReversed, TRUE, TRUE, mapPropModel, mapPropTexture, manager);
        GF_ASSERT(animation != NULL);
        MapPropOneShotAnimation_SetAnimation(oneShotAnimation, i, animation);
    }
}

static void MapPropAnimationManager_UnloadOneShotAnimation(MapPropAnimationManager *manager, MapPropOneShotAnimation *oneShotAnimation)
{
    int i;
    NNSG3dAnmObj *animationObj = MapPropAnimation_GetAnimationObj(oneShotAnimation->currentAnimation);

    for (i = 0; i < 6; i++) {
        if (oneShotAnimation->mapPropRenderObjs[i] != NULL) {
            MapPropAnimation_RemoveAnimationObjFromRenderObj(oneShotAnimation->mapPropRenderObjs[i], animationObj);
        }
    }

    for (i = 0; i < oneShotAnimation->animations.count; i++) {
        MapPropAnimation *animation = oneShotAnimation->animations.data[i];
        MapPropAnimationManager_UnloadAnimation(animation, manager);

        oneShotAnimation->animations.data[i] = NULL;
    }
}

MapPropOneShotAnimationManager *MapPropOneShotAnimationManager_New(void)
{
    int oneShotAnimationManagerSize = sizeof(MapPropOneShotAnimationManager);
    MapPropOneShotAnimationManager *oneShotAnimationManager = Heap_AllocFromHeap(HEAP_ID_FIELD, oneShotAnimationManagerSize);

    MI_CpuClearFast(oneShotAnimationManager, oneShotAnimationManagerSize);

    return oneShotAnimationManager;
}

void MapPropOneShotAnimationManager_Free(MapPropOneShotAnimationManager **oneShotAnimationManager)
{
    if (*oneShotAnimationManager == NULL) {
        return;
    }

    Heap_FreeToHeap(*oneShotAnimationManager);
    *oneShotAnimationManager = NULL;
}

void ov5_021D41C8(MapPropAnimationManager *animationManager, MapPropOneShotAnimationManager *const oneShotAnimationManager, const int tag, const int mapPropModelID, NNSG3dRenderObj *mapPropRenderObj, NNSG3dResMdl *mapPropModel, NNSG3dResTex *mapPropTexture, const int animationCount, const u8 animationLoopCount, const BOOL animationReversed)
{
    u8 fixedAnimationLoopCount;
    MapPropOneShotAnimation *oneShotAnimation = MapPropOneShotAnimationManager_AllocateAnimation(oneShotAnimationManager, tag);

    if (!oneShotAnimation) {
        GF_ASSERT(FALSE);
        return;
    }

    ov5_021D4068(mapPropRenderObj, animationCount, oneShotAnimation);

    fixedAnimationLoopCount = animationLoopCount;
    GF_ASSERT(fixedAnimationLoopCount != 0);

    if (fixedAnimationLoopCount == 0) {
        fixedAnimationLoopCount = 1;
    }

    ov5_021D40F0(mapPropModelID, mapPropModel, mapPropTexture, animationCount, fixedAnimationLoopCount, animationReversed, animationManager, oneShotAnimation);
    oneShotAnimation->mapPropModelID = mapPropModelID;
}

void ov5_021D4220(MapPropOneShotAnimationManager *const oneShotAnimationManager, const int tag, const u8 renderObjIndex, NNSG3dRenderObj *renderObj)
{
    GF_ASSERT(renderObjIndex < 6);
    MapPropOneShotAnimation *oneShotAnimation = MapPropOneShotAnimationManager_GetAnimationByTag(oneShotAnimationManager, tag);

    GF_ASSERT(oneShotAnimation->mapPropRenderObjs[renderObjIndex] == NULL);
    oneShotAnimation->mapPropRenderObjs[renderObjIndex] = renderObj;
}

void MapPropOneShotAnimationManager_PlayAnimation(MapPropOneShotAnimationManager *const oneShotAnimationManager, const int tag, const int mapPropAnimationIndex)
{
    GF_ASSERT(tag != 0);

    MapPropOneShotAnimation *oneShotAnimation = MapPropOneShotAnimationManager_GetAnimationByTag(oneShotAnimationManager, tag);
    MapPropAnimation *animation = MapPropOneShotAnimation_SwitchAnimation(oneShotAnimation, mapPropAnimationIndex);

    MapPropAnimation_SetAnimationPaused(animation, FALSE);
}

void MapPropOneShotAnimationManager_PlayAnimationWithSoundEffect(MapPropOneShotAnimationManager *const oneShotAnimationManager, const int tag, const int mapPropAnimationIndex, const int soundEffectID)
{
    GF_ASSERT(tag != 0);

    MapPropOneShotAnimation *oneShotAnimation = MapPropOneShotAnimationManager_GetAnimationByTag(oneShotAnimationManager, tag);
    MapPropAnimation *animation = MapPropOneShotAnimation_SwitchAnimation(oneShotAnimation, mapPropAnimationIndex);

    if (soundEffectID != 0) {
        Sound_PlayEffect(soundEffectID);
    }

    MapPropAnimation_SetAnimationPaused(animation, FALSE);
}

void MapPropOneShotAnimationManager_UnloadAnimationByTag(MapPropAnimationManager *animationManager, MapPropOneShotAnimationManager *oneShotAnimationManager, const int tag)
{
    GF_ASSERT(tag != 0);
    MapPropOneShotAnimation *oneShotAnimation = MapPropOneShotAnimationManager_GetAnimationByTag(oneShotAnimationManager, tag);

    MapPropAnimationManager_UnloadOneShotAnimation(animationManager, oneShotAnimation);
    MapPropOneShotAnimation_Reset(oneShotAnimation);
}

MapPropAnimation *MapPropOneShotAnimationManager_GetCurrentAnimationByTag(MapPropOneShotAnimationManager *const oneShotAnimationManager, const int tag)
{
    GF_ASSERT(tag != 0);

    MapPropOneShotAnimation *oneShotAnimation = MapPropOneShotAnimationManager_GetAnimationByTag(oneShotAnimationManager, tag);
    return oneShotAnimation->currentAnimation;
}

const BOOL ov5_021D42F0(MapPropOneShotAnimationManager *const oneShotAnimationManager, const int tag)
{
    MapPropAnimation *animation = MapPropOneShotAnimationManager_GetCurrentAnimationByTag(oneShotAnimationManager, tag);
    GF_ASSERT(animation != NULL);

    if (MapPropAnimation_IsLoopFinished(animation)) {
        return TRUE;
    } else {
        return FALSE;
    }
}

const int MapPropOneShotAnimationManager_GetAnimationMapPropModelIDByTag(MapPropOneShotAnimationManager *const oneShotAnimationManager, const int tag)
{
    MapPropOneShotAnimation *oneShotAnimation = MapPropOneShotAnimationManager_GetAnimationByTag(oneShotAnimationManager, tag);
    return oneShotAnimation->mapPropModelID;
}
