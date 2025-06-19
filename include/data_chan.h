#ifndef __BLUFI_TYPES__
#define __BLUFI_TYPES__
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

extern "C" {

typedef struct DataChan {
  uint8_t *data;
  size_t size;
  struct DataChan *next;
} DataChan;

DataChan *newDataChan();

void freeDataChan(DataChan *datachan);
}

#endif