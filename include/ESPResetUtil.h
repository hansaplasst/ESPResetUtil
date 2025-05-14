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

/**
 * @brief Resets the device by deleting specific files or formatting the entire filesystem.
 *
 * @param format If true, the entire filesystem will be formatted. If false (default), only selected files will be deleted.
 *
 * ESP32 uses LittleFS; ESP8266 uses SPIFFS. After cleanup, the device will restart.
 */
void factoryReset(bool format = false);

/**
 * @brief Performs a clean ESP restart and provides visual feedback.
 *
 * This is a soft reset for ESP32 (`ESP.restart()`) or a hard reset for ESP8266 (`ESP.reset()`).
 * Before resetting, the LED blinks 4 times for visual confirmation.
 */
void espReset();

/**
 * @brief Checks at boot whether a reset button is being held and performs a factory reset if so.
 *
 * @param pin     GPIO pin connected to the reset button. Must use INPUT_PULLUP mode.
 * @param ledPin  GPIO pin for LED feedback. Use 255 to disable LED blinking.
 * @param format  If true, formats the entire filesystem. If false (default), only selected files are deleted.
 *
 * When the button is held longer than FACTORY_RESET_TIME (default 5000 ms), a factory reset is triggered.
 * The LED (if enabled) will blink 20 times before the reset.
 * The reset behavior is controlled by the format parameter.
 */
void checkResetButtonOnStartup(uint8_t pin, uint8_t ledPin, bool format = false);

// --- Implementations ---
inline void espReset() {
  DPRINTF(2, "[ESPReset] Restarting ESP...\n");
  blinkLed(4, 100);
#ifdef ARDUINO_ARCH_ESP8266
  ESP.reset();
#else
  ESP.restart();
#endif
}

inline void factoryReset(bool format) {
  if (format) {
    DPRINTF(2, "[FactoryReset] Formatting filesystem (%s)...\n",
#ifdef ARDUINO_ARCH_ESP32
            "LittleFS");
#else
            "SPIFFS");
#endif
    FILESYSTEM.format();
  } else {
    DPRINTF(2, "[FactoryReset] Deleting selected files...\n");

    const char* filesToDelete[] = {
        // TODO: Make this configurable via a config file
        "/config.json",
        "/user.txt",
    };

    for (const char* path : filesToDelete) {
      if (FILESYSTEM.exists(path)) {
        DPRINTF(1, "[FactoryReset] Removing %s\n", path);
        FILESYSTEM.remove(path);
      } else {
        DPRINTF(0, "[FactoryReset] File not found: %s\n", path);
      }
    }
  }

  delay(500);

#ifdef ARDUINO_ARCH_ESP8266
  ESP.reset();
#else
  ESP.restart();
#endif
}

inline void checkResetButtonOnStartup(uint8_t pin, uint8_t ledPin, bool format) {
  pinMode(pin, INPUT_PULLUP);
  unsigned long startTime = millis();
  DPRINTF(0, "[Startup] Checking reset button state...\n");

  while (digitalRead(pin) == LOW) {
    if (millis() - startTime > FACTORY_RESET_TIME) {
      DPRINTF(2, "[Startup] Button held >5s -> Factory reset triggered.\n");
      if (ledPin != 255) {
        blinkLed(20, 100);
      }
      factoryReset(format);
    }
    delay(10);
  }
}

#endif  // ESP_RESET_UTIL_H
