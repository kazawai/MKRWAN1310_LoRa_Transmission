// size_t
#include <stddef.h>

// Arduino
#include <Arduino.h>
#include <LoRa.h>
#include <SPI.h>

#define LORA_START_FLAG 0x7E
#define LORA_END_FLAG 0x7F
#define LORA_RESPONSE_FLAG 0x7D
#define LORA_INTERRUPT_FLAG 0x7C

#define EOL_MARKER '\x00'

int counter = 0;
bool startReceived = false;
bool finishReceived = false;

void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial.println("LoRa Sender");

  if (!LoRa.begin(868E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  // Set LoRa parameters
  LoRa.setSignalBandwidth(125E3);  // Set signal bandwidth to 125 kHz
  LoRa.setSpreadingFactor(7);      // Set spreading factor (SF) to 7
  LoRa.setCodingRate4(5);          // Set coding rate (CR) to 5 (4/5)
}

void waitForStartFlag() {
  if (Serial.available() == 0) {
    return;
  }

  // Serial.println("Waiting for start flag...");

  byte incomingByte = Serial.read();
  if (incomingByte == EOL_MARKER) {
    return;
  }
  Serial.print("Received : ");
  Serial.println(incomingByte);
  if (incomingByte == LORA_START_FLAG) {
    startReceived = true;
    counter = 0;
    Serial.println("Start flag received. Transmission started.");
    Serial.write(LORA_RESPONSE_FLAG);
    Serial.println("Response sent.");
  }
}

void sendLoRaPacket() {
  LoRa.beginPacket();
  LoRa.print("Hello World");
  LoRa.endPacket();
  Serial.print("Sending packet : ");
  Serial.println(counter);

  // Check for interrupt flag
  if (Serial.available() > 0) {
    byte incomingByte = Serial.read();
    if (incomingByte == LORA_INTERRUPT_FLAG) {
      Serial.println("Interrupt flag received. Stopping transmission.");
      // Send end flag to serial monitor
      Serial.write(LORA_END_FLAG);
      startReceived = false;
      counter = 0;
    }
  }
}

void loop() {
  if (!startReceived) {
    waitForStartFlag();
  } else if (finishReceived) {
    // Reset flags
    startReceived = false;
    finishReceived = false;
    counter = 0;
    Serial.println("Transmission stopped. Waiting for start flag...");
  } else {
    if (counter < 20) {
      // sendLoRaPacket();
      counter++;
      delay(500);  // send packet every 500 ms
    } else {
      Serial.println("Transmission completed. Stopping.");
      // Send end flag to serial monitor
      Serial.write(LORA_END_FLAG);
      finishReceived = true;
    }
  }
}
