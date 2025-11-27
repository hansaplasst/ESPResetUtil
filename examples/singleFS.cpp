#include <Arduino.h>
#include <dprintf.h>

#include "ESPResetUtil.h"

// --- Project settings ---
#ifndef RESET_PIN
  #define RESET_PIN 4
#endif
#ifndef LEDPIN
  #define LEDPIN 2
#endif

fs::LittleFSFS fs2;

void setup() {
  Serial.begin(BAUDRATE);
  pinMode(LEDPIN, OUTPUT);           // Todo: Include a Config.h for configurable settings
  pinMode(RESET_PIN, INPUT_PULLUP);  // Todo: Include a Config.h for configurable settings

  // First file system (default)
  if (!LittleFS.begin(false)) {
    DPRINTF(3, "[LittleFS] Initialization failed! Formatting...");
    factoryReset(true);  // after formatting a marker file exists on the partition
  }

  if (checkFactoryResetMarker()) {
    DPRINTF(1, "[Setup] Factory reset marker was found...");
  }

  DPRINTF(1, "Checking reset button.")
  if (factoryResetRequest(RESET_PIN, LEDPIN)) {
    factoryReset(true);
  }
  DPRINTF(1, " Done...")

  DPRINTF(1, "[Setup] Initialization would go here...");
  blinkLed(2, 1000);  // Indicate setup completion
}

void loop() {
  if (digitalRead(RESET_PIN) == LOW) {
    DPRINTF(2, "[Loop] Reset button pressed during runtime");
    espReset(LEDPIN);
  }

  delay(10);
}
