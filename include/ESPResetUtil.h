#ifndef ESP_RESET_UTIL_H
#define ESP_RESET_UTIL_H

#include <Arduino.h>
#include <FS.h>
#include <dprintf.h>

#include "LEDBlinkUtil.h"

#ifdef ARDUINO_ARCH_ESP32
  #include <LittleFS.h>
  #define FILESYSTEM LittleFS
#else
  #include <SPIFFS.h>
  #define FILESYSTEM SPIFFS
#endif

#ifndef FACTORY_RESET_TIME
  #define FACTORY_RESET_TIME 5000  // 5 seconds button press = factory reset
#endif

extern const char* FACTORY_RESET_MARKER = "/.factory_reset_marker";

/**
 * @brief Resets the device by deleting specific files or formatting the entire filesystem.
 *
 * @param format If true, the entire filesystem will be formatted. If false (default), only selected files will be deleted.
 *
 * ESP32 uses LittleFS; ESP8266 uses SPIFFS. After cleanup, the device will restart.
 */
void factoryReset(bool format = false);

/**
 * @brief Checks at boot whether a reset button is being held and performs a factory reset if so.
 *
 * @param gpioPin     GPIO pin connected to the reset button. Must use INPUT_PULLUP mode.
 * @param ledPin  GPIO pin for LED feedback. Use 255 to disable LED blinking.
 * @param format  If true, formats the entire filesystem. If false (default), only selected files are deleted.
 *
 * When the button is held longer than FACTORY_RESET_TIME (default 5000 ms), a factory reset is triggered.
 * The LED (if enabled) will blink 20 times before the reset.
 * The reset behavior is controlled by the format parameter.
 */
void checkResetButtonOnStartup(uint8_t gpioPin, uint8_t ledPin, bool format = false);

// --- Implementations ---
inline void espReset(uint8_t LedPin) {
  DPRINTF(2, "[ESPReset] Restarting ESP...");
  blinkLedOnPin(LedPin, 4, 100);
#ifdef ARDUINO_ARCH_ESP8266
  ESP.reset();
#else
  ESP.restart();
#endif
}

/**
 * @brief Performs a clean ESP restart and provides visual feedback.
 *
 * This is a soft reset for ESP32 (`ESP.restart()`) or a hard reset for ESP8266 (`ESP.reset()`).
 * Before resetting, the LED blinks 4 times for visual confirmation.
 */
inline void espReset() {
  espReset(2);  // Default LED pin 2
}

/**
 * @brief Performs a factory reset by either formatting the filesystem or deleting specific files.
 *
 * @param format If true, formats the entire filesystem. If false, deletes selected configuration files.
 * @file /.factory_reset_marker is created to indicate a factory reset has occurred.
 *
 * After performing the reset actions, the device restarts.
 */
inline void factoryReset(bool format) {
  if (format) {
    DPRINTF(2, "[FactoryReset] Formatting filesystem (%s)...",
#ifdef ARDUINO_ARCH_ESP32
            "LittleFS");
#else
            "SPIFFS");
#endif
    FILESYSTEM.format();
  } else {
    DPRINTF(2, "[FactoryReset] Deleting selected files...");

    const char* filesToDelete[] = {
        // TODO: Make this configurable via a config file
        "/config.json",
        "/user.txt",
    };

    for (const char* f : filesToDelete) {
      if (FILESYSTEM.exists(f)) {
        DPRINTF(1, "[FactoryReset] Removing %s", f);
        FILESYSTEM.remove(f);
      } else {
        DPRINTF(0, "[FactoryReset] File not found: %s", f);
      }
    }
  }

  const char* filesToCreate[] = {
      // Files to recreate after deletion to avoid boot issues
      FACTORY_RESET_MARKER,
  };

  for (const char* f : filesToCreate) {
    DPRINTF(0, "[FactoryReset] Creating %s", f);
    File file = FILESYSTEM.open(f, FILE_WRITE);
    if (file) {
      file.close();
    } else {
      DPRINTF(0, "[FactoryReset] Failed to create file: %s", f);
    }
  }
  delay(500);

#ifdef ARDUINO_ARCH_ESP8266
  ESP.reset();
#else
  ESP.restart();
#endif
}

/**
 * @brief Checks if the factory reset marker file exists.
 *
 * @return true if the marker file exists (indicating a factory reset has occurred), false otherwise.
 *
 * If the marker file is found, it is deleted to prevent repeated resets on subsequent boots.
 */
inline bool checkFactoryResetMarker() {
  DPRINTF(0, "[checkFactoryResetMarker] Checking for factory reset marker file...");
  if (FILESYSTEM.exists(FACTORY_RESET_MARKER)) {
    if (FILESYSTEM.remove(FACTORY_RESET_MARKER))
      DPRINTF(0, "[checkFactoryResetMarker] Factory reset marker file exists and was removed.");
    return true;
  }
  return false;
}

inline void checkResetButtonOnStartup(uint8_t gpioPin, uint8_t ledPin, bool format) {
  pinMode(gpioPin, INPUT_PULLUP);
  unsigned long startTime = millis();

  DPRINTF(0, "[Startup] Checking reset button state...");
  while (digitalRead(gpioPin) == LOW) {
    if (millis() - startTime > FACTORY_RESET_TIME) {
      DPRINTF(2, "[Startup] Button held >5s -> Factory reset triggered.");
      if (ledPin != 255) {
        blinkLedOnPin(ledPin, 20, 100);
      }
      factoryReset(format);
    }
    delay(10);
  }
}

#endif  // ESP_RESET_UTIL_H
