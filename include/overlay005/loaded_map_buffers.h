#ifndef POKEPLATINUM_OV5_LOADED_MAP_BUFFERS_H
#define POKEPLATINUM_OV5_LOADED_MAP_BUFFERS_H

#include <nitro/types.h>

typedef struct LoadedMapBuffers {
    u16 mapModelAllocType;
    u16 bdhcAllocType;
    void *mapModelFiles[4];
    void *bdhcFiles[4];
} LoadedMapBuffers;

LoadedMapBuffers *LoadedMapBuffers_New(const BOOL allocBDHCFiles);
void LoadedMapBuffers_Free(LoadedMapBuffers *buffers);
void LoadedMapBuffers_GetMapModelFileBufPtr(const u8 index, const LoadedMapBuffers *buffers, void **mapModelFile);
void LoadedMapBuffers_GetBDHCFileBufPtr(const u8 index, const LoadedMapBuffers *buffers, void **bdhcFile);
LoadedMapBuffers *LoadedMapBuffers_NewWithHeapMapModelFiles(const BOOL allocBDHCFiles);
void LoadedMapBuffers_SwapMapModelData(LoadedMapBuffers *a, LoadedMapBuffers *b);

#endif // POKEPLATINUM_OV5_LOADED_MAP_BUFFERS_H
