#ifndef __SD_BLK_H__
#define __SD_BLK_H__

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct {
  FILE* fp;
  bool readOnly;
  bool inserted;
  uint64_t sectors;
} BlockDevice;

extern BlockDevice* blk_open(char* filename, bool readOnly);
extern void blk_close(BlockDevice* device);
extern int blk_read(BlockDevice* device, uint64_t address, void* dest, uint32_t len);
extern int blk_write(BlockDevice* device, uint64_t address, void* src, uint32_t len);
extern bool blk_is_inserted(BlockDevice* device);
extern void blk_get_geometry(BlockDevice* device, uint64_t* sectors);
extern bool blk_is_read_only(BlockDevice* device);

#endif
