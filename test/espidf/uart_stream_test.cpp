#include "driver/uart.h"
#include "gtest/gtest.h"
#include "roo_backport/byte.h"
#include "roo_io/uart/esp32/uart_output_stream.h"

#ifndef ESP_PLATFORM
#error "This test requires the roo_testing ESP-IDF profile"
#endif

#ifdef ARDUINO
#error "This is an ESP-IDF-only test; ARDUINO must not be defined"
#endif

namespace {

TEST(EspIdfUartStream, WritesThroughTheIdfDriverShim) {
  uart_config_t config = {};
  config.baud_rate = 115200;
  config.data_bits = UART_DATA_8_BITS;
  config.parity = UART_PARITY_DISABLE;
  config.stop_bits = UART_STOP_BITS_1;
  config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
  config.source_clk = UART_SCLK_DEFAULT;

  ASSERT_EQ(ESP_OK, uart_param_config(UART_NUM_0, &config));
  ASSERT_EQ(ESP_OK, uart_driver_install(UART_NUM_0, 256, 0, 0, nullptr, 0));

  uint32_t baud_rate = 0;
  ASSERT_EQ(ESP_OK, uart_get_baudrate(UART_NUM_0, &baud_rate));
  EXPECT_EQ(115200u, baud_rate);

  roo_io::Esp32UartOutputStream stream(UART_NUM_0);
  const roo::byte payload[] = {roo::byte{'I'}, roo::byte{'D'}, roo::byte{'F'},
                               roo::byte{'\n'}};
  EXPECT_EQ(sizeof(payload), stream.writeFully(payload, sizeof(payload)));
  EXPECT_TRUE(stream.isOpen());
  stream.close();
  EXPECT_FALSE(stream.isOpen());

  EXPECT_EQ(ESP_OK, uart_driver_delete(UART_NUM_0));
}

}  // namespace
