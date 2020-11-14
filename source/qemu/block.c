#include "qemu/block.h"

#define SECTOR_SIZE 512

BlockDevice* blk_open(char* filename, bool readOnly) {
  FILE* fp = fopen(filename, "rb+");
  if(fp == NULL) {
    fprintf(stderr, "Error opening %s\n", filename);
    return NULL;
  }

  BlockDevice* device = (BlockDevice*) malloc(sizeof(BlockDevice));
  device->fp = fp;
  device->readOnly = readOnly;
  device->inserted = true;

  fseek(fp, 0, SEEK_END);
  device->sectors = ftell(fp)/SECTOR_SIZE;
  
  return device;
}

void blk_close(BlockDevice* device) {
  fclose(device->fp);
  free(device);
}

int blk_read(BlockDevice* device, uint64_t address, void* dest, uint32_t len) {
  #ifdef DEBUG
  printf("Reading %d bytes from 0x%x in SD file\n", len, address);
  #endif
  fseek(device->fp, address, SEEK_SET);
  return fread(dest, 1, len, device->fp);
}

int blk_write(BlockDevice* device, uint64_t address, void* src, uint32_t len) {
  if(device->readOnly) {
    return -1;
  }
  fseek(device->fp, address, SEEK_SET);
  int r = fwrite(src, 1, len, device->fp);
  fflush(device->fp);
  return r;
}

bool blk_is_inserted(BlockDevice* device) {
  return device->inserted;
}

void blk_get_geometry(BlockDevice* device, uint64_t* sectors) {
  *sectors = device->sectors;
}

bool blk_is_read_only(BlockDevice* device) {
  return device->readOnly;
}

