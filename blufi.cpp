#include "blufi.h"
#include "dh.h"
#include "md5.h"
#include "uaes.h"

extern void consoleLog(std::string);

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

void Core::fillSendData(msg::Msg &msg, SendData &sendData) {
  while(msg.hasNext()) {
    auto len = msg.fillFrame(this->sendSeq++);
    sendData.push_back(std::vector<uint8_t>(this->buffer, this->buffer + len));
  }
}

uint8_t Core::onReceiveData(std::span<uint8_t> data) {
  // consoleLog("onReceive");
  if(data.size() < 4) {
    // invalid length
    return 0x10;
  }
  uint8_t type = data[0] & 0b11;
  uint8_t subType = data[0] >> 2;
  uint8_t seq = data[2];
  if(seq != this->recvSeq) {
    // invalid seq
    return 0x20;
  }
  this->recvSeq++;
  uint8_t bodyLen = data[3];
  if(bodyLen == 0) {
    // empty data
    return 0x30;
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
      return 0x40;
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
  // consoleLog(std::to_string(frameCtrl));
  // consoleLog(std::to_string(end));
  // consoleLog(std::to_string(type));
  // consoleLog(std::to_string(subType));
  // copy into bodyBuffer
  bodyBuffer.insert(bodyBuffer.end(), tmpBody.begin(), tmpBody.end());
  if(end) {
    // consoleLog("end");
    // process buffer
    if(type == msg::Type::VALUE) {
      if(subType == msg::SubType::WIFI_LIST_NEG) {
        // wifi list
        std::vector<Wifi> list;
        int i = 0;
        while(i < bodyBuffer.size()) {
          auto len = bodyBuffer[i];
          i += 1;
          int8_t rssi = (int8_t)(bodyBuffer[i] - 0xFF);
          std::string ssid((char *)bodyBuffer.data() + i + 1, len - 1);
          list.push_back(Wifi{.ssid = ssid, .rssi = rssi});
          i += len;
        }
        this->onMessage(type, subType, &list);
        // std::get<ScanWifiResult>(this->onResult)(list);
      } else if(subType == msg::SubType::NEG) {
        // consoleLog("neg " + std::to_string(this->task));
        // negotiate result
        // consoleLog("neg 2");
        if(this->key) {
          free(this->key);
          this->key = NULL;
        }
        // consoleLog("body length: " + std::to_string(bodyBuffer.size()));
        auto key = this->tmpKey->generateKey(bodyBuffer);
        this->key = (uint8_t *)malloc(16);
        MD5Context ctx;
        md5Init(&ctx);
        md5Update(&ctx, key.data(), key.size());
        md5Finalize(&ctx);
        // copy
        std::copy(ctx.digest, ctx.digest + 16, this->key);
        // char buf[33];
        // for(int i = 0; i < 16; i++) {
        //   sprintf(buf + (i * 2), "%02x", this->key[i]);
        // }
        // consoleLog(buf);
        // send result
        SendData setSecModeBuffer;
        {
          uint8_t data[] = {0b11};
          msg::Msg msg(msg::Type::VALUE, msg::SubType::SET_SEC_MODE,
                       std::span(data, 1), false, NULL, bufferSpan);
          fillSendData(msg, setSecModeBuffer);
        }
        // 这个消息体正常只有一片
        this->onMessage(type, subType, &setSecModeBuffer[0]);
      } else if(subType == msg::SubType::CUSTOM_DATA) {
        // custom data
        this->onMessage(type, subType, &bodyBuffer);
      } else if(subType == msg::SubType::WIFI_STATUS) {
        // wifi status
        this->onMessage(type, subType, &bodyBuffer);
      } else if(subType == msg::SubType::ERROR) {
        // error
        return 0x70 | bodyBuffer[0];
      } else {
        // not implement
        return 0x80;
      }
    } else {
      // invalid type
      return 0x50;
    }
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

uint8_t Core::negotiateKey(SendData &sendData) {

  if(this->tmpKey) {
    return ErrorCode::KeyStateNotMatch;
  }
  // setup
  this->bodyBuffer.clear();

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
    fillSendData(msg, sendData);
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
    fillSendData(msg, sendData);
  }
  return 0;
}
uint8_t Core::custom(SendData &sendData, std::vector<uint8_t> data) {
  if(this->key == NULL) {
    return ErrorCode::KeyStateNotMatch;
  }
  this->bodyBuffer.clear();

  msg::Msg msg(msg::Type::VALUE, msg::SubType::CUSTOM_DATA, std::span(data),
               true, this->key, bufferSpan);
  fillSendData(msg, sendData);
  return 0;
}
uint8_t Core::scanWifi(SendData &sendData) {

  this->bodyBuffer.clear();

  msg::Msg msg(msg::Type::CONTROL_VALUE, msg::SubType::WIFI_NEG,
               std::span<uint8_t>(), false, NULL, bufferSpan);
  fillSendData(msg, sendData);
  return 0;
}
uint8_t Core::connectWifi(SendData &sendData, std::string ssid,
                          std::string pass) {
  if(this->key == NULL) {
    return ErrorCode::KeyStateNotMatch;
  }

  this->bodyBuffer.clear();

  {
    msg::Msg msg(msg::Type::VALUE, msg::SubType::SET_SSID,
                 std::span<uint8_t>((uint8_t *)ssid.data(), ssid.size()), true,
                 this->key, bufferSpan);
    fillSendData(msg, sendData);
  }
  {
    msg::Msg msg(msg::Type::VALUE, msg::SubType::SET_PWD,
                 std::span<uint8_t>((uint8_t *)pass.data(), pass.size()), true,
                 this->key, bufferSpan);
    fillSendData(msg, sendData);
  }
  {
    msg::Msg msg(msg::Type::CONTROL_VALUE, msg::SubType::END,
                 std::span<uint8_t>(), false, NULL, bufferSpan);
    fillSendData(msg, sendData);
  }
  return 0;
}
} // namespace blufi