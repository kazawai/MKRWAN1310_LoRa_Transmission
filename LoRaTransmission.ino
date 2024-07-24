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
#define LORA_INIT_FLAG 0x7B

#define EOL_MARKER '\x00'

#define MAX_TRANSMISSION 100

int counter = 0;
bool initReceived = false;
bool startReceived = false;
bool finishReceived = false;

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("LoRa Sender");

  if (!LoRa.begin(868E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  // Set LoRa parameters
  LoRa.setSignalBandwidth(125E3);  // Set signal bandwidth to 125 kHz
  LoRa.setSpreadingFactor(8);      // Set spreading factor (SF) to 7
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
  } else if (incomingByte == LORA_INTERRUPT_FLAG) {
    Serial.println("Interrupt flag received. Stopping transmission.");
    // Send end flag to serial monitor
    Serial.write(LORA_END_FLAG);
    initReceived = false;
    startReceived = false;
    counter = 0;
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
      initReceived = false;
      startReceived = false;
      counter = 0;
    }
  }
}

void waitForInitFlag() {
  if (Serial.available() == 0) {
    return;
  }

  byte incomingByte = Serial.read();
  if (incomingByte == EOL_MARKER) {
    return;
  }
  Serial.print("Received : ");
  Serial.println(incomingByte);
  if (incomingByte == LORA_INIT_FLAG) {
    Serial.println("Init flag received. Ready to start transmission.");
    int i = 0;
    while (i < 2) {
      byte incomingByte = Serial.read();
      if (incomingByte == EOL_MARKER) {
        return;
      }
      Serial.print("Received : ");
      Serial.println(incomingByte);
      if (i == 0 && incomingByte >= 7 && incomingByte <= 12) {
        LoRa.setSpreadingFactor(incomingByte);
        Serial.print("Spreading factor set to ");
        Serial.println(incomingByte);
        i++;
      } else if (i == 1 && incomingByte >= 125 && incomingByte <= 500) {
        LoRa.setSignalBandwidth(incomingByte * 1E3);
        Serial.print("Signal bandwidth set to ");
        Serial.println(incomingByte);
        i++;
      }
      delay(1000);
    }
    initReceived = true;
    Serial.write(LORA_RESPONSE_FLAG);
    Serial.println("Response sent.");
  } else if (incomingByte == LORA_INTERRUPT_FLAG) {
    Serial.println("Interrupt flag received. Stopping transmission.");
    // Send end flag to serial monitor
    Serial.write(LORA_END_FLAG);
    initReceived = false;
    startReceived = false;
    counter = 0;
  }
}

void loop() {
  if (!initReceived) {
    waitForInitFlag();
  } else if (!startReceived) {
    waitForStartFlag();
  } else if (finishReceived) {
    // Reset flags
    initReceived = false;
    startReceived = false;
    finishReceived = false;
    counter = 0;
    Serial.println("Transmission stopped. Waiting for start flag...");
  } else {
    if (counter < MAX_TRANSMISSION) {
      sendLoRaPacket();
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
