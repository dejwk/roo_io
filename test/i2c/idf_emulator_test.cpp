#include "gtest/gtest.h"
#include "roo_io/i2c/i2c.h"
#include "roo_testing/devices/clock/ds3231/ds3231.h"
#include "roo_testing/microcontrollers/esp32/fake_esp32.h"

namespace roo_io {
// Verifies native adapter reads/writes a pin-routed RTC and removes owned
// handles.
TEST(IdfI2cEmulator, RtcRegistersAndCleanup) {
  static FakeDs3231 rtc;
  FakeEsp32().attachI2cDevice(rtc, 18, 19);
  {
    I2cMasterBusHandle bus;
    ASSERT_TRUE(bus.init(18, 19));
    I2cSlaveDevice device(bus, 0x68);
    ASSERT_TRUE(device.init());
    // DS3231 user alarm registers do not advance with emulated time.
    const roo::byte payload[] = {roo::byte{0x07}, roo::byte{0x12},
                                 roo::byte{0x34}};
    ASSERT_TRUE(device.transmit(payload, sizeof(payload)));
    roo::byte result[2];
    ASSERT_TRUE(device.transmitReceive(payload, 1, result, 2));
    EXPECT_EQ(payload[1], result[0]);
    EXPECT_EQ(payload[2], result[1]);
    I2cSlaveDevice missing(bus, 0x69);
    ASSERT_TRUE(missing.init());
    EXPECT_FALSE(missing.transmit(payload, sizeof(payload)));
  }
  i2c_master_bus_handle_t handle = nullptr;
  EXPECT_EQ(ESP_ERR_NOT_FOUND, i2c_master_get_bus_handle(0, &handle));
}
}  // namespace roo_io
