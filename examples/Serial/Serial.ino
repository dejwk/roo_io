// This example demonstrates a serial link in loopback mode, where
// the server and client are connected to each other using two different UARTs
// on the same device. The server sends a sequence of messages to the client,
// which receives and prints them to the standard output.
//
// Important: to run this example, you need to connect the TX and RX pins of the
// two UARTs to each other using jumper wires. The pin numbers are defined by
// the constants kPinServerTx, kPinServerRx, kPinClientTx, and
// kPinClientRx below. Make sure to cross the wires, i.e. to connect
// kPinServerTx with kPinClientRx, and kPinServerRx with kPinClientTx.
//
// Note: UART is notoriously unreliable: the data may get corrupted or lost, and
// it is difficult to synchronize the transmitter with the receiver. This
// example implements a very simple framing protocol by using the zero character
// as a frame delimiter and the 0xFF character as the 'start frame' marker.
//
// If you're interested in a more robust serial communication protocol, check
// out the http://github.com/dejwk/roo_transport library that implements several
// variants of error recovery protocols that can be used with UART, including a
// near-drop-in replacement for Serial that is fully reliable at multi-Mbps
// transmission speeds.

// Uncomment to switch to ESP32-specific stream implementation.
// #define ESP32_UART

#include "roo_io.h"
#include "roo_io/data/input_stream_reader.h"
#include "roo_io/data/output_stream_writer.h"
#ifdef ESP32_UART
#include "roo_io/uart/esp32/uart_input_stream.h"
#include "roo_io/uart/esp32/uart_output_stream.h"
#else
#include "roo_io/uart/arduino/serial_input_stream.h"
#include "roo_io/uart/arduino/serial_output_stream.h"
#endif
#include "roo_threads.h"

static const int kPinServerTx = 27;
static const int kPinServerRx = 14;
static const int kPinClientTx = 25;
static const int kPinClientRx = 26;

roo::thread server_thread;

void server() {
  LOG(INFO) << "Server connecting...";
  Serial1.begin(115200, SERIAL_8N1, kPinServerRx, kPinServerTx);
  LOG(INFO) << "Server connected.";

#ifdef ESP32_UART
  roo_io::Esp32UartOutputStream serial1_out(UART_NUM_1);
#else
  roo_io::ArduinoSerialOutputStream serial1_out(Serial1);
#endif
  roo_io::OutputStreamWriter serial1_data_out(serial1_out);
  uint32_t i = 0;
  while (true) {
    // Write the frame start marker.
    serial1_data_out.writeU8(0xFF);

    // Write the message body.
    serial1_data_out.writeBeU32(i);
    serial1_data_out.writeString("Hello, world!\n");

    // Write the frame delimiter.
    serial1_data_out.writeU8(0x00);

    LOG(INFO) << "Sent " << i;
    i++;
  }
}

// Start the server in a separate task thread, so that it runs concurrently with
// the client (which executes in the main loop).
void startServer() {
  roo::thread::attributes attrs;
  attrs.set_name("server");
  // Use low priority to avoid starving the (receiving) client thread in the
  // loopback mode.
  attrs.set_priority(1);
  server_thread = roo::thread(attrs, &server);
}

void client() {
  LOG(INFO) << "Client connecting...";
  Serial2.begin(115200, SERIAL_8N1, kPinClientRx, kPinClientTx);
  LOG(INFO) << "Client connected.";

#ifdef ESP32_UART
  roo_io::Esp32UartInputStream serial2_in(UART_NUM_2);
#else
  roo_io::ArduinoSerialInputStream serial2_in(Serial2);
#endif
  roo_io::InputStreamReader serial2_data_in(serial2_in);
  while (true) {
    // Seek past a frame start marker (0xFF) that follows a delimiter (0).
    bool saw_delimiter = false;
    while (true) {
      uint8_t b = serial2_data_in.readU8();
      if (b == 0) {
        saw_delimiter = true;
        continue;
      }
      if (saw_delimiter && b == 0xFF) break;
      saw_delimiter = false;
    }
    // Ok,now we are synchronized.
    uint32_t idx = serial2_data_in.readBeU32();
    std::string msg = serial2_data_in.readString(100);
    LOG(INFO) << "Received " << idx << ": " << msg;
  }
}

void setup() {
  Serial.begin(115200);
  startServer();
  client();
}

void loop() {}
