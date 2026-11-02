#include <array>

#include "Arduino.h"
#include "SPI.h"
#include "Wire.h"
#include "gtest/gtest.h"
#include "roo_io/i2c/i2c.h"
#include "roo_io/spi/spi.h"
#include "roo_io/uart/arduino/serial_input_stream.h"
#include "roo_io/uart/arduino/serial_output_stream.h"
#include "roo_testing/buses/i2c/fake_i2c.h"
#include "roo_testing/buses/spi/fake_spi.h"
#include "roo_testing/buses/uart/fake_uart.h"
#include "roo_testing/microcontrollers/rp2040/fake_pico.h"

namespace {
class Registers : public FakeI2cDevice {
 public:
  Registers() : FakeI2cDevice("Roo registers", 0x68) {}
  Result write(const uint8_t* data, uint16_t size, bool stop,
               uint16_t) override {
    if (size == 0) return I2C_ERROR_DEV;
    pointer = data[0];
    for (uint16_t i = 1; i < size; ++i)
      values[pointer++ % values.size()] = data[i];
    stopped = stop;
    return I2C_ERROR_OK;
  }
  Result read(uint8_t* data, uint16_t size, bool stop, uint16_t) override {
    for (uint16_t i = 0; i < size; ++i)
      data[i] = values[pointer++ % values.size()];
    stopped = stop;
    return I2C_ERROR_OK;
  }
  std::array<uint8_t, 256> values{};
  size_t pointer = 0;
  bool stopped = true;
};

class Echo : public SimpleFakeSpiDevice {
 public:
  Echo() : SimpleFakeSpiDevice("Roo SPI echo") {}
  void transfer(const FakeSpiInterface&, uint8_t* data,
                uint16_t bits) override {
    EXPECT_TRUE(isSelected());
    for (uint16_t i = 0; i < bits / 8; ++i) data[i] ^= 0xa5;
    ++calls;
  }
  int calls = 0;
};

struct Wiring {
  Wiring() {
    FakePico().attachI2cDevice(registers[0], 4, 5);
    FakePico().attachI2cDevice(registers[1], 6, 7);
    FakePico().attachSpiDevice(echo[0], 18, 16, 19);
    FakePico().attachSpiDevice(echo[1], 14, 12, 15);
    FakePico().attachOutput(17, echo[0].cs());
    FakePico().attachOutput(13, echo[1].cs());
    FakePico().attachUartDevice(cable.end_a(), 0, 1);
    FakePico().attachUartDevice(cable.end_b(), 8, 9);
  }
  Registers registers[2];
  Echo echo[2];
  FakeUartCable cable;
} wiring;

// Verifies both Wire controllers use Pico routing and repeated START through
// the public Roo abstraction, preserving device register and STOP state.
TEST(PicoArduinoTransport, I2cControllers) {
  for (int i = 0; i < 2; ++i) {
    TwoWire& wire = i == 0 ? Wire : Wire1;
    wire.setSDA(i == 0 ? 4 : 6);
    wire.setSCL(i == 0 ? 5 : 7);
    roo_io::I2cSlaveDevice device(roo_io::I2cMasterBusHandle(wire), 0x68);
    roo_io::I2cMasterBusHandle(wire).init();
    ASSERT_TRUE(device.init());
    const roo::byte tx[] = {roo::byte{7}, roo::byte{0x5a}};
    ASSERT_TRUE(device.transmit(tx, 2));
    roo::byte rx{};
    ASSERT_TRUE(device.transmitReceive(tx, 1, &rx, 1));
    EXPECT_EQ(rx, roo::byte{0x5a});
    EXPECT_TRUE(wiring.registers[i].stopped);
    wire.end();
  }
}

// Verifies scoped multi-buffer exchanges keep CS asserted through both Pico
// SPI controllers and retain source bytes and independent receive data.
TEST(PicoArduinoTransport, SpiControllers) {
  for (int i = 0; i < 2; ++i) {
    decltype(SPI)& spi = i == 0 ? SPI : SPI1;
    spi.begin();
    roo_io::SpiSlaveDevice device(roo_io::SpiMasterBusHandle(spi),
                                  i == 0 ? 17 : 13);
    ASSERT_TRUE(device.init());
    const uint8_t tx = 0x33;
    uint8_t rx;
    {
      roo_io::SpiTransaction scope = device.transaction();
      ASSERT_TRUE(scope.isOpen());
      ASSERT_TRUE(scope.transfer(&tx, &rx, 1));
      EXPECT_EQ(rx, 0x33 ^ 0xa5);
      ASSERT_TRUE(scope.transfer(&tx, &rx, 1));
      EXPECT_TRUE(wiring.echo[i].isSelected());
    }
    EXPECT_FALSE(wiring.echo[i].isSelected());
    EXPECT_EQ(wiring.echo[i].calls, 2);
    spi.end();
  }
}

// Verifies Arduino serial wrappers exchange bytes through Pico UART models
// without a filesystem dependency or frontend-specific HardwareSerial type.
TEST(PicoArduinoTransport, SerialStreams) {
  Serial1.begin(115200);
  Serial2.begin(115200);
  roo_io::ArduinoSerialOutputStream output(Serial1);
  roo_io::ArduinoSerialInputStream input(Serial2);
  const roo_io::byte bytes[] = {roo_io::byte{1}, roo_io::byte{2},
                                roo_io::byte{3}};
  EXPECT_EQ(output.writeFully(bytes, 3), 3u);
  roo_io::byte received[3];
  EXPECT_EQ(input.readFully(received, 3), 3u);
  EXPECT_EQ(received[2], bytes[2]);
  Serial1.end();
  Serial2.end();
}
}  // namespace
