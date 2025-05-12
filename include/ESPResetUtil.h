#ifndef ESP_RESET_UTIL_H
#define ESP_RESET_UTIL_H

#include <Arduino.h>
#include <FS.h>
#include <dprintf.h>

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
 * @brief Formats the filesystem (LittleFS or SPIFFS) and restarts the ESP.
 *
 * ESP32 uses LittleFS. ESP8266 uses SPIFFS.
 */
void factoryReset();

/**
 * @brief Checks if the reset button is held during startup and performs a factory reset if so.
 *
 * @param pin     GPIO pin for the reset button (should use INPUT_PULLUP)
 * @param ledPin  Optional LED pin for visual feedback (255 = no LED)
 */
void checkResetButtonOnStartup(uint8_t pin, uint8_t ledPin = 255);

// --- Implementations ---

inline void factoryReset() {
  DPRINTF(2, "[FactoryReset] Deleting selected files...\n");

  const char* filesToDelete[] = {
      "/config.json",
      "/user.txt",
      // Voeg hier meer paden toe indien gewenst
  };

  for (const char* path : filesToDelete) {
    if (FILESYSTEM.exists(path)) {
      DPRINTF(1, "[FactoryReset] Removing %s\n", path);
      FILESYSTEM.remove(path);
    } else {
      DPRINTF(0, "[FactoryReset] File not found: %s\n", path);
    }
  }

  delay(500);

#ifdef ARDUINO_ARCH_ESP8266
  ESP.reset();
#else
  ESP.restart();
#endif
}

inline void checkResetButtonOnStartup(uint8_t pin, uint8_t ledPin) {
  pinMode(pin, INPUT_PULLUP);
  unsigned long startTime = millis();
  DPRINTF(0, "[Startup] Checking reset button state...\n");

  while (digitalRead(pin) == LOW) {
    if (millis() - startTime > FACTORY_RESET_TIME) {
      DPRINTF(2, "[Startup] Button held >5s -> Factory reset triggered.\n");
      if (ledPin != 255) {
        extern void blinkLed(int16_t count, int16_t msDelay);
        blinkLed(20, 100);
      }
      factoryReset();
    }
    delay(10);
  }
}

#endif  // ESP_RESET_UTIL_H
