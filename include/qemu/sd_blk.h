#ifndef __SD_BLK_H__
#define __SD_BLK_H__

#include <stdint.h>

typedef struct {
} BlockBackend;

extern int blk_pread(BlockBackend* device, uint64_t address, void* dest, uint32_t len);
extern int blk_pwrite(BlockBackend* device, uint64_t address, void* src, uint32_t len);
extern bool blk_is_inserted(BlockBackend* device);
extern void blk_get_geometry(BlockBackend* device, uint64_t* sectors);
extern bool blk_is_read_only(BlockBackend* device);

#endif
