#include "ESPResetUtil.h"

/**
 * @brief Performs a clean ESP restart and provides visual feedback.
 *
 * This is a soft reset for ESP32 (`ESP.restart()`) or a hard reset for ESP8266 (`ESP.reset()`).
 * Before resetting, the a LED (default LedPin 2) blinks 4 times for visual confirmation.
 */
void espResetUtil::espReset(uint8_t LedPin) {
  DPRINTF(2, "[ESPReset] Restarting ESP...");
  blinkLedOnPin(LedPin, 4, 100);
#ifdef ARDUINO_ARCH_ESP8266
  ESP.reset();
#else
  ESP.restart();
#endif
}

/**
 * @brief Performs a factory reset by either formatting the filesystem or deleting specific files.
 *
 * @param format If true, formats the entire filesystem. If false, deletes selected configuration files.
 * @file /.factory_reset_marker is created to indicate a factory reset has occurred.
 *
 * After performing the reset actions, the device restarts.
 */
void espResetUtil::factoryReset(bool format, fs::LittleFSFS& fileSystem, std::initializer_list<const char*> filesToDelete) {
  DPRINTF(0, "[factoryReset]");

  // 1. Delete files
  for (const char* filename : filesToDelete) {
    if (fileSystem.exists(filename)) {
      if (fileSystem.remove(filename)) {
        DPRINTF(1, " Deleted file: %s", filename);
      } else {
        DPRINTF(3, " Failed to delete file: %s", filename);
      }
    } else {
      DPRINTF(1, " File not found (skip): %s", filename);
    }
  }

  if (format) {
    DPRINTF(1, " Formatting filesystem...");
    fileSystem.format();
  }

  const char* filesToCreate[] = {
      // Files to recreate after deletion to avoid boot issues
      "/.factory_reset_marker",
  };

  for (const char* f : filesToCreate) {
    DPRINTF(1, " Creating %s", f);
    File file = fileSystem.open(f, FILE_WRITE);
    if (file) {
      file.close();
    } else {
      DPRINTF(0, " Failed to create file: %s", f);
    }
  }
  DPRINTF(0, "Rebooting...")
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
bool espResetUtil::checkFactoryResetMarker(fs::LittleFSFS& fileSystem, const char* filename) {
  DPRINTF(0, "[checkFactoryResetMarker] Checking for factory reset marker file...");
  if (fileSystem.exists(filename)) {
    if (fileSystem.remove(filename))
      DPRINTF(0, " Factory reset marker file exists and was removed.");
    return true;
  }
  DPRINTF(0, " Factory reset marker not found.");
  return false;
}

/**
 * @brief Returns true if reset button is being held longer than FACTORY_RESET_TIME
 *
 * @param gpioPin    GPIO pin connected to the reset button. Must use INPUT_PULLUP mode.
 * @param ledPin     GPIO pin for LED feedback. Use 255 to disable LED blinking.
 *
 * When the button is held longer than FACTORY_RESET_TIME (default 5000 ms), the LED
 * (if enabled) will blink 20 times before the reset.
 *
 * @return true if the button is held longer than FACTORY_RESET_TIME (default 5000 ms)
 */
bool espResetUtil::factoryResetRequest(uint8_t gpioPin, uint8_t ledPin) {
  DPRINTF(0, "[factoryResetRequest]");
  pinMode(gpioPin, INPUT_PULLUP);
  unsigned long startTime = millis();

  DPRINTF(0, "[Startup] Checking reset button state...");
  while (digitalRead(gpioPin) == LOW) {
    if (millis() - startTime > FACTORY_RESET_TIME) {
      DPRINTF(2, "[Startup] Button held >5s -> Factory reset triggered.");
      if (ledPin != 255) {
        blinkLedOnPin(ledPin, 20, 100);
      }
      // factoryReset(format, fileSystem);
      return true;
    }
    delay(10);
  }
  return false;
}
