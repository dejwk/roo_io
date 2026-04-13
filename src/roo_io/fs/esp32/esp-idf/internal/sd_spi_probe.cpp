#include "roo_io/fs/esp32/esp-idf/internal/sd_spi_probe.h"

#if defined(ESP_PLATFORM) && !defined(ARDUINO) && !defined(ROO_TESTING)

#include <cstring>

#include "driver/gpio.h"
#include "driver/spi_master.h"

namespace roo_io {
namespace internal {
namespace {

// Temporarily add an SPI device at low speed for card probing, run `fn`, then
// remove the device.  Returns false if the device could not be added.
template <typename Fn>
bool WithSpiDevice(spi_host_device_t spi_host, gpio_num_t cs_pin, Fn fn) {
  spi_device_interface_config_t dev_cfg = {};
  dev_cfg.clock_speed_hz = 400000;  // 400 kHz for SD init
  dev_cfg.mode = 0;
  dev_cfg.spics_io_num = cs_pin;
  dev_cfg.queue_size = 1;

  spi_device_handle_t handle = nullptr;
  if (spi_bus_add_device(spi_host, &dev_cfg, &handle) != ESP_OK) {
    return false;
  }
  fn(handle);
  spi_bus_remove_device(handle);
  return true;
}

// Clock out `count` bytes of 0xFF with CS deasserted (HIGH).
void SendClocks(spi_device_handle_t handle, int count) {
  uint8_t buf[16];
  memset(buf, 0xFF, sizeof(buf));
  while (count > 0) {
    int n = (count > (int)sizeof(buf)) ? (int)sizeof(buf) : count;
    spi_transaction_t t = {};
    t.length = n * 8;
    t.tx_buffer = buf;
    spi_device_polling_transmit(handle, &t);
    count -= n;
  }
}

// Send a 6-byte SD command and read the R1 response.
// The command and response read are done in a single SPI transaction so that
// CS stays asserted (LOW) throughout, as the SD protocol requires.
// Returns the R1 byte, or 0xFF if no response.
constexpr int kMaxRead = 16;

uint8_t SendCmd(spi_device_handle_t handle, const uint8_t* cmd) {
  uint8_t tx[6 + kMaxRead];
  uint8_t rx[6 + kMaxRead];
  memcpy(tx, cmd, 6);
  memset(tx + 6, 0xFF, kMaxRead);  // clock out 0xFF while reading response

  spi_transaction_t t = {};
  t.length = sizeof(tx) * 8;
  t.tx_buffer = tx;
  t.rx_buffer = rx;
  spi_device_polling_transmit(handle, &t);

  // Scan the receive buffer starting after the 6 command bytes.
  for (int i = 6; i < 6 + kMaxRead; i++) {
    if (!(rx[i] & 0x80)) return rx[i];
  }
  return 0xFF;
}

}  // namespace

bool SdSpiProbeCard(spi_host_device_t spi_host, gpio_num_t cs_pin) {
  bool present = false;

  // Ensure CS pin is configured as output and driven HIGH before we add the
  // SPI device (which will take over the pin).
  gpio_set_direction(cs_pin, GPIO_MODE_OUTPUT);
  gpio_set_level(cs_pin, 1);

  WithSpiDevice(spi_host, GPIO_NUM_NC, [&](spi_device_handle_t handle) {
    // Power-up: >=74 clocks with CS HIGH.
    // We use spics_io_num = GPIO_NUM_NC for this device so CS stays HIGH
    // during clocks, but we need to manually deassert CS.
    gpio_set_level(cs_pin, 1);
    SendClocks(handle, 10);  // 80 clocks
  });

  WithSpiDevice(spi_host, cs_pin, [&](spi_device_handle_t handle) {
    // CMD0 (GO_IDLE_STATE) with pre-computed CRC.
    // Retry up to 5 times: a freshly inserted card may need a few CMD0s to
    // transition from native mode to SPI mode.
    //  - 0xFF means no card (MISO pulled high, or no device driving it).
    //  - 0x00 means no card (another SPI device, e.g. ILI9488 with
    //    non-tri-state SDO, is driving MISO low).
    //  - 0x01 means card in idle state (present).
    //  - Anything else: retry.
    const uint8_t cmd0[] = {0x40, 0x00, 0x00, 0x00, 0x00, 0x95};
    constexpr int kMaxRetries = 5;
    for (int attempt = 0; attempt < kMaxRetries; attempt++) {
      uint8_t token = SendCmd(handle, cmd0);
      if (token == 0xFF || token == 0x00) {
        // No card — MISO is pulled high or driven low by another bus device.
        break;
      }
      if (token == 0x01) {
        present = true;
        break;
      }
    }
  });

  return present;
}

}  // namespace internal
}  // namespace roo_io

#endif  // ESP_PLATFORM && !ARDUINO && !ROO_TESTING
