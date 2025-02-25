#ifndef _BLUFI_H
#define _BLUFI_H

#include "dh.h"
#include "msg.h"
#include <atomic>
#include <cstdint>
#include <functional>
#include <span>
#include <string>
#include <variant>

namespace blufi {

using namespace std;

typedef struct {
  std::string ssid;
  std::int8_t rssi;
} Wifi;

using NegotiateResult = std::function<void(int result)>;
using ScanWifiResult = std::function<void(const std::vector<Wifi> &)>;
using BytesResult = std::function<void(const std::span<uint8_t> &)>;

using OnResult = std::variant<BytesResult, NegotiateResult, ScanWifiResult>;

enum Task { Unknown = 0, Negotiate, Custom, ScanWifi, ConnectWifi };

class Core {
public:
  Core(int mtu, std::function<int(std::span<uint8_t>)> onSendData);
  ~Core();
  uint16_t onReceiveData(std::span<uint8_t>);

  // function are
  int negotiateKey(NegotiateResult onResult);
  int custom(std::vector<uint8_t>, BytesResult onResult);
  int scanWifi(ScanWifiResult onResult);
  int connectWifi(std::string ssid, std::string pass, BytesResult onResult);

private:
  std::atomic<bool> sendLock = false;
  uint8_t *buffer;
  int mtu;
  std::span<uint8_t> bufferSpan;
  std::function<int(std::span<uint8_t>)> onSendData;
  uint8_t sendSeq = 0;
  uint8_t recvSeq = 0;
  // 16 bytes aes key
  uint8_t *key = NULL;

  dh::DH *tmpKey = NULL;

  Task task = Task::Unknown;
  std::vector<uint8_t> bodyBuffer;
  OnResult onResult;

  int sendMsg(msg::Msg &msg);
};
} // namespace blufi

#endif //_BLUFI_H