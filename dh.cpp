#include "dh.h"
#include "boost/multiprecision/cpp_int.hpp"
#include <random>

namespace dh {

using BigInt = boost::multiprecision::cpp_int;

const uint8_t P[] = {
    0xcf, 0x5c, 0xf5, 0xc3, 0x84, 0x19, 0xa7, 0x24, 0x95, 0x7f, 0xf5, 0xdd,
    0x32, 0x3b, 0x9c, 0x45, 0xc3, 0xcd, 0xd2, 0x61, 0xeb, 0x74, 0x0f, 0x69,
    0xaa, 0x94, 0xb8, 0xbb, 0x1a, 0x5c, 0x96, 0x40, 0x91, 0x53, 0xbd, 0x76,
    0xb2, 0x42, 0x22, 0xd0, 0x32, 0x74, 0xe4, 0x72, 0x5a, 0x54, 0x06, 0x09,
    0x2e, 0x9e, 0x82, 0xe9, 0x13, 0x5c, 0x64, 0x3c, 0xae, 0x98, 0x13, 0x2b,
    0x0d, 0x95, 0xf7, 0xd6, 0x53, 0x47, 0xc6, 0x8a, 0xfc, 0x1e, 0x67, 0x7d,
    0xa9, 0x0e, 0x51, 0xbb, 0xab, 0x5f, 0x5c, 0xf4, 0x29, 0xc2, 0x91, 0xb4,
    0xba, 0x39, 0xc6, 0xb2, 0xdc, 0x5e, 0x8c, 0x72, 0x31, 0xe4, 0x6a, 0xa7,
    0x72, 0x8e, 0x87, 0x66, 0x45, 0x32, 0xcd, 0xf5, 0x47, 0xbe, 0x20, 0xc9,
    0xa3, 0xfa, 0x83, 0x42, 0xbe, 0x6e, 0x34, 0x37, 0x1a, 0x27, 0xc0, 0x6f,
    0x7d, 0xc0, 0xed, 0xdd, 0xd2, 0xf8, 0x63, 0x73};

const uint8_t G = 2;

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

struct DH::Internal {
  BigInt p;
  BigInt g;
  BigInt privKey;
  BigInt pubKey;
};

DH::DH() : internal(new Internal()) {

  this->internal->p = bytes2BigInt(P, sizeof(P));

  this->internal->g = 2;

  // generate private key
  std::random_device dev;
  std::mt19937 rng(dev());
  std::uniform_int_distribution<std::mt19937::result_type> dist(UINT16_MAX,
                                                                UINT32_MAX);
  this->internal->privKey = dist(rng);
  // this->privKey = 19960911;
  // generate publick key
  this->internal->pubKey = boost::multiprecision::powm(
      this->internal->g, this->internal->privKey, this->internal->p);
}

DH::~DH() {}

std::vector<uint8_t> DH::getPBytes() {
  return std::vector<uint8_t>(P, P + sizeof(P));
}
std::vector<uint8_t> DH::getGBytes() { return bigInt2Bytes(this->internal->g); }
std::vector<uint8_t> DH::getPubBytes() {
  return bigInt2Bytes(this->internal->pubKey);
}
std::vector<uint8_t> DH::generateKey(std::span<uint8_t> remotePubKeyBytes) {
  auto remotePubKey =
      bytes2BigInt(remotePubKeyBytes.data(), remotePubKeyBytes.size());
  auto key = boost::multiprecision::powm(remotePubKey, this->internal->privKey,
                                         this->internal->p);
  return bigInt2Bytes(key);
}
} // namespace dh