#include "driver/uart.h"
#include "roo_backport/byte.h"
#include "roo_io/uart/esp32/uart_output_stream.h"

extern "C" void app_main() {
  uart_config_t config = {};
  config.baud_rate = 115200;
  config.data_bits = UART_DATA_8_BITS;
  config.parity = UART_PARITY_DISABLE;
  config.stop_bits = UART_STOP_BITS_1;
  config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
  config.source_clk = UART_SCLK_DEFAULT;

  ESP_ERROR_CHECK(uart_param_config(UART_NUM_0, &config));
  ESP_ERROR_CHECK(uart_driver_install(UART_NUM_0, 256, 0, 0, nullptr, 0));

  roo_io::Esp32UartOutputStream stream(UART_NUM_0);
  const roo::byte message[] = {
      roo::byte{'H'}, roo::byte{'e'}, roo::byte{'l'},  roo::byte{'l'},
      roo::byte{'o'}, roo::byte{' '}, roo::byte{'I'},  roo::byte{'D'},
      roo::byte{'F'}, roo::byte{'!'}, roo::byte{'\n'},
  };
  stream.writeFully(message, sizeof(message));
  ESP_ERROR_CHECK(uart_driver_delete(UART_NUM_0));
}
