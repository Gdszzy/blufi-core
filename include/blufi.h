#pragma once

#include "dh.h"
#include "msg.h"
#include <cstdint>
#include <deque>
#include <functional>
#include <span>
#include <string>
#include <variant>

namespace blufi {

enum ErrorCode {
  WrongDataLen = 0x10,
  WrongSeq = 0x20,
  EmptyData = 0x30,
  KeyStateNotMatch = 0x40,
  InvalidType = 0x50,
  RemoteError = 0x70,
  NotImplement = 0x80
};

typedef struct {
  std::string ssid;
  std::int8_t rssi;
} Wifi;

// Callback trigger when a full message received. So this callback must not
// block
using OnMessage = std::function<void(uint8_t type, uint8_t subType,
                                     std::vector<uint8_t> *data)>;

// OnMessage just callback with bytes. Call parseWifi to get the wifi list
std::vector<Wifi> parseWifi(std::vector<uint8_t> &data);

using SendData = std::vector<std::vector<uint8_t>>;

class Core {
public:
  Core(int mtu, OnMessage onMessage);
  ~Core();
  // Call this function when message received.
  uint8_t onReceiveData(std::span<uint8_t>);

  uint8_t negotiateKey(SendData &sendData);
  uint8_t custom(SendData &sendData, std::vector<uint8_t>);
  uint8_t scanWifi(SendData &sendData);
  uint8_t connectWifi(SendData &sendData, std::string ssid, std::string pass);

private:
  int mtu;
  // Message buffer with mtu length
  // When serialization a msg::Msg. This buffer will be used.
  uint8_t *buffer;
  // Wrap buffer to simplify parameters
  std::span<uint8_t> bufferSpan;

  uint8_t sendSeq = 0;
  uint8_t recvSeq = 0;
  // 16 bytes aes key
  uint8_t *key = NULL;

  dh::DH *tmpKey = NULL;
  // message received store here
  std::vector<uint8_t> bodyBuffer;
  // callback
  OnMessage onMessage;

  void fillSendData(msg::Msg &msg, SendData &sendData);
};
} // namespace blufi