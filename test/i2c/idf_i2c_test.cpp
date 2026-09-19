#include <cstdlib>
#include <new>

#include "gtest/gtest.h"
#include "idf_fake.h"
#include "roo_io/i2c/i2c.h"

namespace {
thread_local size_t allocation_count = 0;
}

// Counts C++ allocations without exceptions; assertions run outside the
// measured interval so GoogleTest's own allocations do not affect the
// regression check.
void* operator new(size_t size) {
  ++allocation_count;
  void* memory = std::malloc(size == 0 ? 1 : size);
  if (memory != nullptr) {
    return memory;
  }
  std::abort();
}

void operator delete(void* memory) noexcept { std::free(memory); }

void operator delete(void* memory, size_t) noexcept { std::free(memory); }

namespace roo_io {
namespace {
/// Resets the scripted IDF boundary between independent lifecycle tests.
class I2cTest : public testing::Test {
  void SetUp() override { i2c_test::state() = i2c_test::State(); }
};
// Verifies copies observe initialization and ownership lasts through device
// cleanup.
TEST_F(I2cTest, CopiesShareInitializationAndDeviceKeepsOwnedBusAlive) {
  std::unique_ptr<I2cSlaveDevice> device;
  {
    I2cMasterBusHandle bus;
    device.reset(new I2cSlaveDevice(bus, 0x68));
    ASSERT_TRUE(bus.init(18, 19, 400000));
    EXPECT_FALSE(bus.init(18, 19));
    ASSERT_TRUE(device->init());
    EXPECT_TRUE(device->init());
    EXPECT_EQ(1, i2c_test::state().added);
    EXPECT_EQ(400000u, i2c_test::state().frequency);
  }
  EXPECT_EQ(0, i2c_test::state().deleted);
  device.reset();
  EXPECT_EQ(1, i2c_test::state().removed);
  EXPECT_EQ(1, i2c_test::state().deleted);
}
// Verifies native-handle construction, copies, and device wrappers allocate no
// state.
TEST_F(I2cTest, NativeHandleBorrowingDoesNotAllocate) {
  const i2c_master_bus_handle_t handle = i2c_test::handle();
  const size_t before = allocation_count;
  I2cMasterBusHandle bus(handle, 400000);
  I2cMasterBusHandle copy = bus;
  I2cMasterBusHandle assigned(handle);
  assigned = copy;
  I2cSlaveDevice device(assigned, 0x68);
  const size_t after = allocation_count;
  EXPECT_EQ(before, after);
  EXPECT_FALSE(bus.init(18, 19));
  ASSERT_TRUE(device.init());
  EXPECT_EQ(400000u, i2c_test::state().frequency);
}

// Verifies a null borrowed handle never falls back to an unrelated bus by port.
TEST_F(I2cTest, NullBorrowedHandleFailsInitialization) {
  i2c_test::state().available = true;
  I2cMasterBusHandle bus(static_cast<i2c_master_bus_handle_t>(nullptr));
  I2cSlaveDevice device(bus, 0x68);
  EXPECT_FALSE(bus.init(18, 19));
  EXPECT_FALSE(device.init());
  EXPECT_EQ(0, i2c_test::state().created);
  EXPECT_EQ(0, i2c_test::state().added);
}

// Verifies a device removes its registration without deleting an external bus.
TEST_F(I2cTest, BorrowedBusIsNeverDeleted) {
  {
    I2cMasterBusHandle bus(i2c_test::handle());
    I2cSlaveDevice device(bus, 0x68);
    ASSERT_TRUE(device.init());
  }
  EXPECT_EQ(1, i2c_test::state().removed);
  EXPECT_EQ(0, i2c_test::state().deleted);
}
// Verifies transfers fail before initialization and setup failures permit
// retry.
TEST_F(I2cTest, InitializationFailuresCanBeRetried) {
  I2cMasterBusHandle bus;
  I2cSlaveDevice device(bus, 0x68);
  roo::byte byte{};
  EXPECT_FALSE(device.transmit(&byte, 1));
  EXPECT_EQ(0u, device.receive(&byte, 1));
  EXPECT_FALSE(device.init());
  i2c_test::state().create_result = ESP_ERR_NO_MEM;
  EXPECT_FALSE(bus.init(18, 19));
  i2c_test::state().create_result = ESP_OK;
  ASSERT_TRUE(bus.init(18, 19));
  i2c_test::state().add_result = ESP_ERR_NO_MEM;
  EXPECT_FALSE(device.init());
  i2c_test::state().add_result = ESP_OK;
  EXPECT_TRUE(device.init());
}
// Verifies combined transfer dispatch, timeout forwarding, and failure
// recovery.
TEST_F(I2cTest, CombinedTransferAndTimeoutFailure) {
  i2c_test::state().available = true;
  I2cSlaveDevice device(I2cMasterBusHandle(), 0x68, 25);
  ASSERT_TRUE(device.init());
  const roo::byte reg{};
  roo::byte data[7];
  EXPECT_TRUE(device.transmitReceive(&reg, 1, data, 7));
  EXPECT_EQ(1, i2c_test::state().combined);
  EXPECT_EQ(25, i2c_test::state().timeout);
  EXPECT_EQ(0x68, i2c_test::state().address);
  EXPECT_EQ(0x56, static_cast<int>(data[0]));
  i2c_test::state().transfer_result = ESP_ERR_TIMEOUT;
  EXPECT_FALSE(device.transmitReceive(&reg, 1, data, 7));
  EXPECT_FALSE(device.transmit(&reg, 1));
  EXPECT_EQ(0u, device.receive(data, 7));
  i2c_test::state().transfer_result = ESP_OK;
  EXPECT_TRUE(device.transmitReceive(&reg, 1, data, 7));
}
// Verifies invalid addresses and infinite timeouts cannot register a device.
TEST_F(I2cTest, InvalidConfigurationDoesNotRegisterDevice) {
  I2cMasterBusHandle bus(i2c_test::handle());
  I2cSlaveDevice invalid_address(bus, 0x80);
  I2cSlaveDevice invalid_timeout(bus, 0x68, -1);
  EXPECT_FALSE(invalid_address.init());
  EXPECT_FALSE(invalid_timeout.init());
  EXPECT_EQ(0, i2c_test::state().added);
}
}  // namespace
}  // namespace roo_io
