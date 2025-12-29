#include <Arduino.h>
#include <dprintf.h>

#include "ESPResetUtil.h"
#include "esp_partition.h"

// --- Project settings ---
#ifndef RESET_PIN
  #define RESET_PIN 4
#endif

fs::LittleFSFS fs2;

static void ResetPinMonitorTask(void* arg) {
  pinMode(LEDPIN, OUTPUT);
  for (;;) {
    if (digitalRead(RESET_PIN) == LOW) {
      DPRINTF(2, "[Loop] Reset button pressed during runtime");
      espResetUtil::espReset(LEDPIN, true, 128);
    }
    delay(10);
  }
}

void printPartitions() {
  const esp_partition_t* it = nullptr;
  esp_partition_iterator_t iterator =
      esp_partition_find(ESP_PARTITION_TYPE_ANY, ESP_PARTITION_SUBTYPE_ANY, nullptr);

  while (iterator != nullptr) {
    it = esp_partition_get(iterator);
    if (it != nullptr) {
      Serial.printf("Label: %-10s  Type: 0x%02x  Subtype: 0x%02x  Offset: 0x%06x  Size: 0x%06x\n",
                    it->label, it->type, it->subtype, it->address, it->size);
    }
    iterator = esp_partition_next(iterator);
  }
  esp_partition_iterator_release(iterator);
}

void setup() {
  Serial.begin(BAUDRATE);
  pinMode(LEDPIN, OUTPUT);           // Todo: Include a Config.h for configurable settings
  pinMode(RESET_PIN, INPUT_PULLUP);  // Todo: Include a Config.h for configurable settings

  // First file system (default)
  if (!LittleFS.begin(false)) {
    DPRINTF(3, "/littlefs Initialization failed! Formatting...");
    espResetUtil::factoryReset(true);  // after formatting a marker file exists on the partition
  }

  if (espResetUtil::checkFactoryResetMarker()) {
    DPRINTF(1, "[Setup] Factory reset marker was found...");
  }

  DPRINTF(1, "Checking reset button.")
  if (espResetUtil::factoryResetRequest(RESET_PIN, LEDPIN, true)) {
    espResetUtil::factoryReset(true);
  }
  DPRINTF(1, " Done... Setup Continued.");

  // Second file system
  DPRINTF(1, "Checking second file system..")
  if (!fs2.begin(true, "/devffs", 10, "devffs")) {  // See: partitions.csv
    DPRINTF(3, "/devffs Initialization failed! Formatting...");
    espResetUtil::factoryReset(true, fs2);  // After formatting fs2 a marker file exists on the fs2 partition
  }
  if (espResetUtil::checkFactoryResetMarker(fs2)) {
    DPRINTF(1, "[Setup] Factory reset marker found on second file system...");
  }
  DPRINTF(1, "Press button 8s to factory reset the second partition.")
  delay(3000);
  if (espResetUtil::factoryResetRequest(RESET_PIN, LEDPIN, true)) {
    espResetUtil::factoryReset(true, fs2);
  }
  DPRINTF(1, " Done... Setup Continued.");

  DPRINTF(1, "[Setup] Initialization would go here...");
  DPRINTF(1, "Press button 5s to factory reset the first (standard) partition.")
  printPartitions();

  // Create a task to monitor the reset pin during normal operation
  BaseType_t res = xTaskCreatePinnedToCore(
      ResetPinMonitorTask,    // task function
      "ResetPinMonitorTask",  // task name
      2048,                   // stack size
      nullptr,                // parameters
      1,                      // priority
      nullptr,                // task handle
      APP_CPU_NUM             // Run on APP CPU
  );

  blinkLed(2, 1000, true);  // Indicate setup completion
}

void loop() {
  DPRINTF(1, "Main loop running and holding APP CPU core busy for 10 seconds...");
  for (uint8_t i = 10; i > 0; i--) {
    DPRINTF(1, "%d", i);
    delay(1000);
  }
}
