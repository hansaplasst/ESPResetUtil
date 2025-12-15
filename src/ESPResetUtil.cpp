#include "ESPResetUtil.h"

void espResetUtil::espReset(uint8_t LedPin, bool rgb, uint8_t brightness) {
  DPRINTF(2, "[ESPReset] Restarting ESP...");
  blinkLedOnPin(LedPin, 4, 100, rgb, brightness);
#ifdef ARDUINO_ARCH_ESP8266
  ESP.reset();
#else
  ESP.restart();
#endif
}

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

bool espResetUtil::factoryResetRequest(uint8_t gpioPin, uint8_t ledPin, bool rgb, uint8_t brightness) {
  DPRINTF(0, "[factoryResetRequest]");
  pinMode(gpioPin, INPUT_PULLUP);
  unsigned long startTime = millis();

  DPRINTF(0, "[Startup] Checking reset button state...");
  while (digitalRead(gpioPin) == LOW) {
    if (millis() - startTime > FACTORY_RESET_TIME) {
      DPRINTF(2, "[Startup] Button held >5s -> Factory reset triggered.");
      if (ledPin != 255) {
        blinkLedOnPin(ledPin, 20, 100, rgb, brightness);
      }
      // factoryReset(format, fileSystem);
      return true;
    }
    delay(10);
  }
  return false;
}
