#include <data_chan.h>
#include <stdint.h>
#include <stdlib.h>

extern "C" {

typedef void (*OnMessageWrapper)(uint8_t type, uint8_t subType, uint8_t *data,
                                 size_t dataSize);

void *newCore(int mtu, OnMessageWrapper onMessage);
void freeCore(void *core);

uint8_t onReceiveData(void *core, uint8_t *data, size_t size);

uint8_t negotiateKey(void *core, DataChan *sendData);
uint8_t custom(void *core, DataChan *sendData, uint8_t *data, size_t size);
uint8_t scanWifi(void *core, DataChan *sendData);
uint8_t connectWifi(void *core, DataChan *sendData, const char *ssid,
                    const char *pass);
}