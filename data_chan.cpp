#include <data_chan.h>

DataChan *newDataChan() {
  DataChan *data = (DataChan *)malloc(sizeof(DataChan));
  memset(data, 0, sizeof(DataChan));
  return data;
}

void freeDataChan(DataChan *datachan) {
  while(datachan != NULL) {
    if(datachan->data != NULL) {
      free(datachan->data);
    }
    DataChan *next = datachan->next;
    free(datachan);
    datachan = next;
  }
}