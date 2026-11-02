#include <hardware/gpio.h>
#include <hardware/i2c.h>
#include <hardware/spi.h>
#include <hardware/uart.h>

#include <array>
#if defined(__FREERTOS)
#include <atomic>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#endif

#include "gtest/gtest.h"
#include "roo_io/i2c/pico/i2c.h"
#include "roo_io/spi/pico/spi.h"
#include "roo_io/uart/pico/uart.h"
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

// Verifies SDK I2C repeated START, NACK recovery and independent controllers.
TEST(PicoSdkTransport, I2cControllersAndErrors) {
  for (int i = 0; i < 2; ++i) {
    i2c_inst_t* bus = i == 0 ? i2c0 : i2c1;
    i2c_init(bus, 400000);
    gpio_set_function(i == 0 ? 4 : 6, GPIO_FUNC_I2C);
    gpio_set_function(i == 0 ? 5 : 7, GPIO_FUNC_I2C);
    roo_io::PicoI2cMasterBusHandle handle(bus);
    roo_io::PicoI2cSlaveDevice device(handle, 0x68);
    ASSERT_TRUE(device.init());
    const roo::byte tx[] = {roo::byte{9}, roo::byte{0x42}};
    roo::byte rx;
    ASSERT_TRUE(device.transmit(tx, 2));
    ASSERT_TRUE(device.transmitReceive(tx, 1, &rx, 1));
    EXPECT_EQ(rx, roo::byte{0x42});
    roo_io::PicoI2cSlaveDevice absent(handle, 0x69);
    EXPECT_FALSE(absent.transmit(tx, 2));
    EXPECT_TRUE(device.transmitReceive(tx, 1, &rx, 1));
    EXPECT_TRUE(wiring.registers[i].stopped);
    i2c_deinit(bus);
  }
  EXPECT_FALSE(
      roo_io::PicoI2cSlaveDevice(roo_io::PicoI2cMasterBusHandle(nullptr), 0x68)
          .init());
}

// Verifies SDK-reserved addresses and invalid buffers fail before entering
// hardware APIs whose parameter checks may otherwise assert on physical Pico.
TEST(PicoSdkTransport, InvalidArguments) {
  const roo_io::PicoI2cMasterBusHandle bus(i2c0);
  for (uint8_t address : {0x00, 0x07, 0x78, 0x7f, 0x80}) {
    roo_io::PicoI2cSlaveDevice device(bus, address);
    EXPECT_FALSE(device.init());
    const roo::byte byte{0};
    EXPECT_FALSE(device.transmit(&byte, 1));
  }
  roo_io::PicoI2cSlaveDevice device(bus, 0x68);
  EXPECT_FALSE(device.transmit(nullptr, 1));
  EXPECT_EQ(device.receive(nullptr, 1), 0u);
}

// Verifies SDK scopes retain CS across exchanges, preserve const TX and
// support exactly aliased buffers on both initialized controllers.
TEST(PicoSdkTransport, ScopedSpi) {
  for (int i = 0; i < 2; ++i) {
    spi_inst_t* bus = i == 0 ? spi0 : spi1;
    spi_init(bus, 1000000);
    gpio_set_function(i == 0 ? 18 : 14, GPIO_FUNC_SPI);
    gpio_set_function(i == 0 ? 16 : 12, GPIO_FUNC_SPI);
    gpio_set_function(i == 0 ? 19 : 15, GPIO_FUNC_SPI);
    roo_io::PicoSpiSlaveDevice device(roo_io::PicoSpiMasterBusHandle(bus),
                                      i == 0 ? 17 : 13);
    ASSERT_TRUE(device.init());
    const uint8_t tx = 0x33;
    uint8_t rx;
    {
      auto scope = device.transaction();
      ASSERT_TRUE(scope.isOpen());
      EXPECT_FALSE(scope.transfer(nullptr, nullptr, 1));
      ASSERT_TRUE(scope.transfer(&tx, &rx, 1));
      EXPECT_EQ(rx, 0x33 ^ 0xa5);
      ASSERT_TRUE(scope.transfer(&rx, &rx, 1));
      EXPECT_EQ(rx, tx);
      EXPECT_TRUE(wiring.echo[i].isSelected());
    }
    EXPECT_FALSE(wiring.echo[i].isSelected());
    spi_deinit(bus);
  }
}

#if defined(__FREERTOS)
struct Contender {
  roo_io::PicoSpiSlaveDevice* device;
  std::atomic<bool> started{false};
  std::atomic<bool> done{false};
  bool success = false;
};

void Contend(void* argument) {
  Contender& context = *static_cast<Contender*>(argument);
  context.started.store(true);
  const uint8_t tx = 0;
  uint8_t rx;
  context.success = context.device->transfer(&tx, &rx, 1);
  context.done.store(true);
  vTaskDelete(nullptr);
}

// Verifies a contender cannot change configuration/CS between calls in an
// SDK scope, and proceeds when that scope releases the shared controller.
TEST(PicoSdkTransport, FreeRtosScopeSerialization) {
  spi_init(spi0, 1000000);
  for (int pin : {18, 16, 19}) gpio_set_function(pin, GPIO_FUNC_SPI);
  roo_io::PicoSpiSlaveDevice first(roo_io::PicoSpiMasterBusHandle(spi0), 17);
  roo_io::PicoSpiSlaveDevice second(first);
  ASSERT_TRUE(first.init());
  ASSERT_TRUE(second.init());
  auto scope = first.transaction();
  ASSERT_TRUE(scope.isOpen());
  Contender context{&second};
  TaskHandle_t task = nullptr;
  ASSERT_EQ(xTaskCreate(Contend, "spi-contender", 4096, &context, 1, &task),
            pdPASS);
  for (int i = 0; i < 100 && !context.started.load(); ++i) vTaskDelay(1);
  EXPECT_TRUE(context.started.load());
  EXPECT_FALSE(context.done.load());
  EXPECT_TRUE(wiring.echo[0].isSelected());
  EXPECT_TRUE(scope.close());
  for (int i = 0; i < 100 && !context.done.load(); ++i) vTaskDelay(1);
  if (!context.done.load()) {
    vTaskDelete(task);
    FAIL() << "SPI scope retained controller ownership";
  }
  EXPECT_TRUE(context.success);
  spi_deinit(spi0);
}
#endif

// Verifies SDK UART streams exchange bytes and close only the borrowed view.
TEST(PicoSdkTransport, UartStreams) {
  uart_init(uart0, 115200);
  uart_init(uart1, 115200);
  for (int pin : {0, 1, 8, 9}) gpio_set_function(pin, GPIO_FUNC_UART);
  roo_io::PicoUartOutputStream output(uart0);
  roo_io::PicoUartInputStream input(uart1);
  const roo_io::byte tx[] = {roo_io::byte{1}, roo_io::byte{2}, roo_io::byte{3}};
  roo_io::byte rx[3];
  EXPECT_EQ(output.write(nullptr, 0), 0u);
  ASSERT_EQ(output.writeFully(tx, 3), 3u);
  output.flush();
  ASSERT_EQ(input.readFully(rx, 3), 3u);
  EXPECT_EQ(rx[2], tx[2]);
  input.close();
  EXPECT_EQ(input.status(), roo_io::kClosed);
  EXPECT_TRUE(uart_is_enabled(uart1));
  uart_deinit(uart0);
  uart_deinit(uart1);
}
}  // namespace
