#pragma once

#include <string>
#include <array>
#include <chrono>
#include <openssl/sha.h>
#include <openssl/hmac.h>

#include <unistd.h>
#include <sstream>
#include <cstring>
#include <vector>
#include <iostream>

template<typename v>
static std::string val_to_str(v val) {
  if constexpr (std::is_same_v<v, bool>) {
    return std::format("{}", val);
  }
  std::ostringstream strm_v;
  strm_v << val;
  return strm_v.str();
};

static uint64_t current_ms_epoch() {
  return duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

static std::string since_epoch_dttm(uint64_t dttm, std::string format) {
  int epoch_time = dttm / 1000;
  struct tm * timeinfo;
  time_t epoch_time_as_time_t = epoch_time;
  timeinfo = localtime(&epoch_time_as_time_t);
  std::ostringstream oss;
  oss << std::put_time(timeinfo, "%d.%m.%Y %H:%M:%S");
  auto str = oss.str();
  return str;
}

static std::string b2a_hex( char *byte_arr, int n ) {

    const static std::string HexCodes = "0123456789abcdef";
    std::string HexString;
    for ( int i = 0; i < n ; ++i ) {
        unsigned char BinValue = byte_arr[i];
        HexString += HexCodes[( BinValue >> 4 ) & 0x0F];
        HexString += HexCodes[BinValue & 0x0F];
    }
    return HexString;
}

static std::string hmac_sha256( const char *key, const char *data) {
    unsigned char* digest;
    digest = HMAC(EVP_sha256(), key, std::strlen(key), (unsigned char*)data, std::strlen(data), NULL, NULL);
    return b2a_hex( (char *)digest, 32 );
}

