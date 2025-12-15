#ifndef ESP_RESET_UTIL_H
#define ESP_RESET_UTIL_H

#include <Arduino.h>
#include <FS.h>
#include <dprintf.h>

#include "LEDBlinkUtil.h"

#ifdef ARDUINO_ARCH_ESP32
  #include <LittleFS.h>
#else
  #include <SPIFFS.h>
  #define FILESYSTEM SPIFFS
#endif

#ifndef FACTORY_RESET_TIME
  #define FACTORY_RESET_TIME 5000  // 5 seconds button press = factory reset
#endif

namespace espResetUtil {

/**
 * @brief Performs a clean ESP restart and provides visual feedback.
 *
 * This is a soft reset for ESP32 (`ESP.restart()`) or a hard reset for ESP8266 (`ESP.reset()`).
 * Before resetting, the a LED (default LedPin 2) blinks 4 times for visual confirmation.
 *
 * @param LedPin   GPIO pin for LED feedback (default = 2)
 * @param rgb       Set to true if using an RGB LED (e.g., NeoPixel) (default = false) (colour is blue)
 * @param brightness Brightness level for RGB LED (0-255) (default = 128)
 */
void espReset(uint8_t LedPin = 2, bool rgb = false, uint8_t brightness = 128);

/**
 * @brief Performs a factory reset by either formatting the filesystem or deleting specific files.
 *
 * @param format If true, formats the entire filesystem. If false, deletes selected configuration files.
 * @param fileSystem Reference to the filesystem to operate on (default = LittleFS)
 * @param filesToDelete List of file paths to delete during factory reset (default = empty)
 *
 * @post  /.factory_reset_marker is created to indicate a factory reset has occurred.
 *        After performing the reset actions, the device restarts.
 */
void factoryReset(bool format = false, fs::LittleFSFS& fileSystem = LittleFS,
                  std::initializer_list<const char*> filesToDelete = {});  // TODO: Add SPIFFS variant

/**
 * @brief Checks if the factory reset marker file exists.
 *
 * @param fileSystem Reference to the filesystem to check (default = LittleFS)
 * @param filename   Path to the factory reset marker file (default = "/.factory_reset_marker")
 *
 * @post if the marker file is found, it is deleted to prevent repeated resets on subsequent boots.
 * @return true if the marker file exists (indicating a factory reset has occurred), false otherwise.
 */
bool checkFactoryResetMarker(fs::LittleFSFS& fileSystem = LittleFS,
                             const char* filename = "/.factory_reset_marker");  // TODO: Add SPIFFS variant

/**
 * @brief Returns true if reset button is being held longer than FACTORY_RESET_TIME
 *
 * @param gpioPin    GPIO pin connected to the reset button. Must use INPUT_PULLUP mode.
 * @param ledPin     GPIO pin for LED feedback. Use 255 to disable LED blinking.
 * @param rgb       Set to true if using an RGB LED (e.g., NeoPixel) (default = false) (colour is blue)
 * @param brightness Brightness level for RGB LED (0-255) (default = 128)
 *
 * When the button is held longer than FACTORY_RESET_TIME (default 5000 ms), the LED
 * (if enabled) will blink 20 times before the reset.
 *
 * @return true if the button is held longer than FACTORY_RESET_TIME (default 5000 ms)
 */
bool factoryResetRequest(uint8_t gpioPin, uint8_t ledPin, bool rgb = false, uint8_t brightness = 128);

}  // namespace espResetUtil

#endif  // ESP_RESET_UTIL_H