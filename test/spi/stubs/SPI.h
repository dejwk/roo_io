#pragma once
#include <stdint.h>

#include <vector>
#define MSBFIRST 1
#define SPI_MODE0 0
#define SPI_MODE1 0x10
#define SPI_MODE2 0x20
#define SPI_MODE3 0x30
struct SPISettings {
  SPISettings(uint32_t frequency, uint8_t order, uint8_t mode)
      : frequency(frequency), order(order), mode(mode) {}
  uint32_t frequency;
  uint8_t order;
  uint8_t mode;
};
class SPIClass {
 public:
  void beginTransaction(SPISettings s) {
    settings = s;
    ++begins;
  }
  uint8_t transfer(uint8_t b) {
    sent.push_back(b);
    return b ^ 0xff;
  }
  void endTransaction() { ++ends; }
  SPISettings settings{0, 0, 0};
  int begins = 0;
  int ends = 0;
  std::vector<uint8_t> sent;
};
extern SPIClass SPI;
