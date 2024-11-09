#pragma once

#include <SD.h>

#include <memory>

#include "roo_io/fs/arduino/arduino_mount.h"

namespace roo_io {

class ArduinoSDFS : public Filesystem {
 public:
  ArduinoSDFS(uint8_t ss_pin = SS, SDFS& sd = SD, SPIClass& spi = SPI,
              uint32_t freq = 20000000)
      : sd_(sd), spi_(spi), ss_pin_(ss_pin), frequency_(freq) {
    pinMode(ss_pin, OUTPUT);
    digitalWrite(ss_pin, HIGH);
  }

 protected:
  MountImpl::MountResult mountImpl(std::function<void()> unmount_fn) override {
    digitalWrite(ss_pin_, LOW);
    if (!sd_.begin(ss_pin_, spi_, frequency_)) {
      digitalWrite(ss_pin_, HIGH);
      return MountImpl::MountError(kMountError);
    }
    return MountImpl::Mounted(std::unique_ptr<MountImpl>(
        new ArduinoMountImpl(sd_, false, unmount_fn)));
  }

  void unmountImpl() override {
    sd_.end();
    digitalWrite(ss_pin_, HIGH);
  }

  SDFS& sd_;
  SPIClass& spi_;
  uint8_t ss_pin_;
  uint32_t frequency_;
};

}  // namespace roo_io