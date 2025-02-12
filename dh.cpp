#include "dh.h"
#include <random>

namespace dh {

BigInt bytes2BigInt(const uint8_t *buf, size_t len) {
  BigInt result = 0;
  for(size_t i = 0; i < len; i++) {
    result <<= 8;
    result |= buf[i];
  }
  return result;
}

std::vector<uint8_t> bigInt2Bytes(const BigInt &num) {
  std::vector<uint8_t> bytes;
  auto size = num.backend().size();
  auto p = num.backend().limbs();
  auto byteCount = sizeof(*p);

  for(auto i = 0; i < size; ++i) {
    for(auto j = 0; j < byteCount; j++) {
      bytes.push_back((*p) >> (8 * j));
    }
    ++p;
  }
  // remote zoer at end
  while(bytes[bytes.size() - 1] == 0) {
    bytes.pop_back();
  }
  std::reverse(bytes.begin(), bytes.end());
  return bytes;
}

DH::DH() {
  this->p = bytes2BigInt(P, sizeof(P));
  this->g = 2;
  // generate private key
  std::random_device dev;
  std::mt19937 rng(dev());
  std::uniform_int_distribution<std::mt19937::result_type> dist(UINT16_MAX,
                                                                UINT32_MAX);
  this->privKey = dist(rng);
  // this->privKey = 19960911;
  // generate publick key
  this->pubKey = boost::multiprecision::powm(this->g, this->privKey, this->p);
}

std::vector<uint8_t> DH::getPBytes() {
  return std::vector<uint8_t>(P, P + sizeof(P));
}
std::vector<uint8_t> DH::getGBytes() { return bigInt2Bytes(this->g); }
std::vector<uint8_t> DH::getPubBytes() { return bigInt2Bytes(this->pubKey); }
std::vector<uint8_t> DH::generateKey(std::span<uint8_t> remotePubKeyBytes) {
  auto remotePubKey =
      bytes2BigInt(remotePubKeyBytes.data(), remotePubKeyBytes.size());
  auto key = boost::multiprecision::powm(remotePubKey, this->privKey, this->p);
  return bigInt2Bytes(key);
}
} // namespace dh