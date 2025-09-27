/**
 * @file StreamEx_Example.ino
 * @brief Example usage of the StreamEx library on Arduino.
 *
 * This sketch shows:
 *  - How to initialize StreamEx with user-provided buffers.
 *  - How to write data into TX and RX buffers.
 *  - How to read data back using the Stream API (read/peek/available).
 *  - How to use pop/push buffer functions directly.
 */

#include "StreamEx.h"

// Allocate static buffers for TX and RX
constexpr size_t TX_BUFFER_SIZE = 64;
constexpr size_t RX_BUFFER_SIZE = 64;
char txBuffer[TX_BUFFER_SIZE];
char rxBuffer[RX_BUFFER_SIZE];

// Create a StreamEx object
StreamEx myStream(txBuffer, TX_BUFFER_SIZE, rxBuffer, RX_BUFFER_SIZE);

void setup() {
  Serial.begin(115200);
  while (!Serial) { ; } // Wait for serial monitor

  Serial.println(F("StreamEx Example Starting..."));

  // --- TX EXAMPLE: write to TX and retrieve it ---
  myStream.write("Hello TX World!", 15);
  Serial.print(F("TX buffer content: "));
  Serial.println(myStream.getTxBuffer());

  // Pop all TX content into a buffer
  char txOut[TX_BUFFER_SIZE];
  myStream.popAllTxBuffer(txOut, sizeof(txOut));
  Serial.print(F("Popped TX content: "));
  Serial.println(txOut);

  // --- RX EXAMPLE: feed RX buffer and consume via Stream API ---
  myStream.writeRxBuffer("ABC123", 6);

  Serial.print(F("RX available: "));
  Serial.println(myStream.available());

  Serial.print(F("Peek RX: "));
  Serial.println((char)myStream.peek());

  Serial.print(F("Read RX: "));
  while (myStream.available() > 0) {
    char c = (char)myStream.read();
    Serial.print(c);
  }
  Serial.println();

  // --- Push/pop with sliding window ---
  myStream.pushBackRxBuffer("HelloWorld", 10);

  char rxOut[RX_BUFFER_SIZE];
  myStream.popFrontRxBuffer(rxOut, 5); // Get first 5 chars
  rxOut[5] = '\0';
  Serial.print(F("First 5 RX chars: "));
  Serial.println(rxOut);

  // Show remaining RX buffer
  Serial.print(F("Remaining RX buffer: "));
  Serial.println(myStream.getRxBuffer());
}

void loop() {
  // Nothing here, just one-time demo in setup()
}
