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
  checkResetButtonOnStartup(RESET_PIN, LEDPIN, true);
  DPRINTF(1, " Done... Setup Continued.");

  // Second file system
  DPRINTF(1, "Checking second file system..")
  if (!fs2.begin(true, "/fsDev", 10, "fsDev")) {  // See: partitions.csv
    DPRINTF(3, "[LittleFS] Initialization failed! Formatting...");
    factoryReset(true, fs2);  // After formatting fs2 a marker file exists on the fs2 partition
  }
  if (checkFactoryResetMarker(fs2)) {
    DPRINTF(1, "[Setup] Factory reset marker found on second file system...");
  }
  DPRINTF(1, "Press button 8s to factory reset the second partition.")
  delay(3000);
  checkResetButtonOnStartup(RESET_PIN, LEDPIN, true, fs2);
  DPRINTF(1, " Done... Setup Continued.");

  DPRINTF(1, "[Setup] Initialization would go here...");
  blinkLed(2, 1000);  // Indicate setup completion
  DPRINTF(1, "Press button 5s to factory reset the first (standard) partition.")
}

void loop() {
  if (digitalRead(RESET_PIN) == LOW) {
    DPRINTF(2, "[Loop] Reset button pressed during runtime");
    espReset(LEDPIN);
  }

  delay(10);
}
