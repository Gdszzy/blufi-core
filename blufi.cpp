#include "blufi.h"
#include "dh.h"
#include "md5.h"
#include "uaes.h"

// extern void consoleLog(std::string);

namespace blufi {

const uint8_t NEG_SET_SEC_TOTAL_LEN = 0x00;
const uint8_t NEG_SET_SEC_ALL_DATA = 0x01;

Core::Core(int mtu, OnMessage onMessage) {
  this->mtu = mtu;
  this->buffer = (uint8_t *)malloc(mtu);
  this->bufferSpan = std::span(this->buffer, mtu);
  this->onMessage = onMessage;
}
Core::~Core() {
  free(this->buffer);
  if(this->tmpKey) {
    delete(this->tmpKey);
    this->tmpKey = NULL;
  }
  if(this->key) {
    delete(this->key);
    this->key = NULL;
  }
}

void Core::fillBuffer(msg::Msg &msg, FlattenBuffer &buffer) {
  while(msg.hasNext()) {
    auto len = msg.fillFrame(this->sendSeq++);

    buffer.push_back(len & 0xFF);
    buffer.push_back((len >> 8) & 0xFF);
    buffer.push_back((len >> 16) & 0xFF);
    buffer.push_back((len >> 24) & 0xFF);

    buffer.insert(buffer.end(), this->buffer, this->buffer + len);
  }
}

uint8_t Core::onReceiveData(std::span<uint8_t> data) {
  // consoleLog("onReceive");
  if(data.size() < 4) {
    return ErrorCode::WrongDataLen;
  }
  uint8_t type = data[0] & 0b11;
  uint8_t subType = data[0] >> 2;
  uint8_t seq = data[2];
  if(seq != this->recvSeq) {
    // invalid seq
    return ErrorCode::WrongSeq;
  }
  this->recvSeq++;
  uint8_t bodyLen = data[3];
  if(bodyLen == 0) {
    // empty data
    return ErrorCode::EmptyData;
  }
  bool end = false;
  uint8_t frameCtrl = data[1];
  std::span tmpBody(data.data() + 4, data.size() - 4);
  if(((frameCtrl >> msg::FRAME_CTRL_POSITION_CHECKSUM) & 0x01) > 0) {
    // has checksum
    // skip right now
    tmpBody = std::span(tmpBody.data(), tmpBody.size() - 2);
  }
  if(((frameCtrl >> msg::FRAME_CTRL_POSITION_ENCRYPTED) & 0x01) > 0) {
    // encrypted
    if(this->key == NULL) {
      // need decrypt but no key
      return ErrorCode::KeyStateNotMatch;
    }
    uint8_t iv[16] = {seq};
    UAES_CFB_SimpleDecrypt(128, this->key, 16, iv, tmpBody.data(),
                           tmpBody.data(), tmpBody.size());
  }
  if(((frameCtrl >> msg::FRAME_CTRL_POSITION_FRAG) & 0x01) > 0) {
    // frag pkg. first 2 bytes are total length. remove them
    tmpBody = std::span(tmpBody.data() + 2, tmpBody.size() - 2);
  } else {
    end = true;
  }
  // copy into bodyBuffer
  bodyBuffer.insert(bodyBuffer.end(), tmpBody.begin(), tmpBody.end());
  if(end) {
    // consoleLog("end");
    // process buffer
    if(type == msg::Type::VALUE) {
      if(subType == msg::SubType::NEG) {
        if(this->key) {
          free(this->key);
          this->key = NULL;
        }
        auto key = this->tmpKey->generateKey(bodyBuffer);
        this->key = (uint8_t *)malloc(16);
        MD5Context ctx;
        md5Init(&ctx);
        md5Update(&ctx, key.data(), key.size());
        md5Finalize(&ctx);
        // copy
        std::copy(ctx.digest, ctx.digest + 16, this->key);
        // send result
        // SendData setSecModeBuffer;
        // DataChan *sendData = newDataChan();
        uint8_t data[] = {0b11};
        msg::Msg msg(msg::Type::VALUE, msg::SubType::SET_SEC_MODE,
                     std::span(data, 1), false, NULL, bufferSpan);
        size_t len = msg.fillFrame(this->sendSeq++);
        // This result message must only has one piece
        this->onMessage(type, subType, this->buffer, len);
      } else if(subType == msg::SubType::WIFI_LIST_NEG) {
        // wifi list
        this->onMessage(type, subType, bodyBuffer.data(), bodyBuffer.size());
      } else if(subType == msg::SubType::CUSTOM_DATA) {
        // custom data
        this->onMessage(type, subType, bodyBuffer.data(), bodyBuffer.size());
      } else if(subType == msg::SubType::WIFI_STATUS) {
        // wifi status
        this->onMessage(type, subType, bodyBuffer.data(), bodyBuffer.size());
      } else if(subType == msg::SubType::ERROR) {
        // error
        return ErrorCode::RemoteError | bodyBuffer[0];
      } else {
        // not implement
        return ErrorCode::NotImplement;
      }
    } else {
      // invalid type
      return ErrorCode::InvalidType;
    }
    bodyBuffer.clear();
  }
  return 0;
}

inline void pushBytes2Vec(std::vector<uint8_t> *buff,
                          std::vector<uint8_t> *key) {
  auto len = key->size();
  buff->push_back(len >> 8);
  buff->push_back(len);
  buff->insert(buff->end(), key->begin(), key->end());
}

uint8_t Core::negotiateKey(FlattenBuffer &buffer) {
  if(this->tmpKey) {
    return ErrorCode::KeyStateNotMatch;
  }
  this->tmpKey = new dh::DH();
  auto pBytes = this->tmpKey->getPBytes();
  auto gBytes = this->tmpKey->getGBytes();
  auto pubKeyBytes = this->tmpKey->getPubBytes();
  auto total = pBytes.size() + gBytes.size() + pubKeyBytes.size() + 6;
  // send length
  {
    uint8_t data[] = {NEG_SET_SEC_TOTAL_LEN, static_cast<uint8_t>(total >> 8),
                      static_cast<uint8_t>(total)};
    msg::Msg msg(msg::Type::VALUE, msg::SubType::NEG,
                 std::span(data, sizeof(data)), false, NULL, this->bufferSpan);
    fillBuffer(msg, buffer);
  }
  // send key
  {
    std::vector<uint8_t> keyBuffer;
    keyBuffer.push_back(NEG_SET_SEC_ALL_DATA);
    {
      auto bytes = this->tmpKey->getPBytes();
      pushBytes2Vec(&keyBuffer, &bytes);
    }
    {
      auto bytes = this->tmpKey->getGBytes();
      pushBytes2Vec(&keyBuffer, &bytes);
    }
    {
      auto bytes = this->tmpKey->getPubBytes();
      pushBytes2Vec(&keyBuffer, &bytes);
    }
    msg::Msg msg(msg::Type::VALUE, msg::SubType::NEG, keyBuffer, false, NULL,
                 bufferSpan);
    fillBuffer(msg, buffer);
  }
  return 0;
}
uint8_t Core::custom(FlattenBuffer &buffer, std::span<uint8_t> data) {
  if(this->key == NULL) {
    return ErrorCode::KeyStateNotMatch;
  }
  msg::Msg msg(msg::Type::VALUE, msg::SubType::CUSTOM_DATA, data, true,
               this->key, bufferSpan);
  fillBuffer(msg, buffer);
  return 0;
}
uint8_t Core::scanWifi(FlattenBuffer &buffer) {
  msg::Msg msg(msg::Type::CONTROL_VALUE, msg::SubType::WIFI_NEG,
               std::span<uint8_t>(), false, NULL, bufferSpan);
  fillBuffer(msg, buffer);
  return 0;
}
uint8_t Core::connectWifi(FlattenBuffer &buffer, std::string ssid,
                          std::string pass) {
  if(this->key == NULL) {
    return ErrorCode::KeyStateNotMatch;
  }
  {
    msg::Msg msg(msg::Type::VALUE, msg::SubType::SET_SSID,
                 std::span<uint8_t>((uint8_t *)ssid.data(), ssid.size()), true,
                 this->key, bufferSpan);
    fillBuffer(msg, buffer);
  }
  {
    msg::Msg msg(msg::Type::VALUE, msg::SubType::SET_PWD,
                 std::span<uint8_t>((uint8_t *)pass.data(), pass.size()), true,
                 this->key, bufferSpan);
    fillBuffer(msg, buffer);
  }
  {
    msg::Msg msg(msg::Type::CONTROL_VALUE, msg::SubType::END,
                 std::span<uint8_t>(), false, NULL, bufferSpan);
    fillBuffer(msg, buffer);
  }
  return 0;
}

std::vector<Wifi> parseWifi(std::vector<uint8_t> &data) {
  std::vector<Wifi> list;
  int i = 0;
  while(i < data.size()) {
    auto len = data[i];
    i += 1;
    int8_t rssi = (int8_t)(data[i] - 0xFF);
    std::string ssid((char *)data.data() + i + 1, len - 1);
    list.push_back(Wifi{.ssid = ssid, .rssi = rssi});
    i += len;
  }
  return list;
}
} // namespace blufi

// ============= C implement =============
// void *newCore(int mtu, OnMessageWrapper onMessage) {
//   return new blufi::Core(
//       mtu, [=](uint8_t type, uint8_t subType, uint8_t *data, size_t size) {
//         onMessage(type, subType, data, size);
//       });
// }
// void freeCore(void *core) {
//   blufi::Core *ptr = (blufi::Core *)core;
//   delete(ptr);
// }

// uint8_t onReceiveData(void *core, uint8_t *data, size_t size) {
//   blufi::Core *ptr = (blufi::Core *)core;
//   return ptr->onReceiveData(std::span(data, size));
// }

// uint8_t negotiateKey(void *core, DataChan *sendData) {
//   blufi::Core *ptr = (blufi::Core *)core;
//   return ptr->negotiateKey(sendData);
// }
// uint8_t custom(void *core, DataChan *sendData, uint8_t *data, size_t size) {
//   blufi::Core *ptr = (blufi::Core *)core;
//   return ptr->custom(sendData, std::span(data, size));
// }
// uint8_t scanWifi(void *core, DataChan *sendData) {
//   blufi::Core *ptr = (blufi::Core *)core;
//   return ptr->scanWifi(sendData);
// }
// uint8_t connectWifi(void *core, DataChan *sendData, const char *ssid,
//                     const char *pass) {
//   blufi::Core *ptr = (blufi::Core *)core;
//   return ptr->connectWifi(sendData, ssid, pass);
// }