#ifndef LED_BLINK_UTIL_H
#define LED_BLINK_UTIL_H

#include <Adafruit_NeoPixel.h>
#include <Arduino.h>

#ifndef LEDPIN
  #define LEDPIN 2  // Default built-in LED pin (e.g. GPIO2 on Wemos D1 Mini ESP32)
#endif

/**
 * @brief Blink the default LED defined by LEDPIN.
 *
 * @param count     Number of times to blink (default = 1)
 * @param msDelay   Duration of a full blink in milliseconds (default = 1000)
 */
void blinkLed(int16_t count = 1, int16_t msDelay = 1000);

/**
 * @brief Blink a custom LED on a specified GPIO pin.
 *
 * @param pin       GPIO pin number to blink
 * @param count     Number of times to blink (default = 1)
 * @param msDelay   Duration of a full blink in milliseconds (default = 1000)
 */
void blinkLedOnPin(uint8_t pin, int16_t count = 1, int16_t msDelay = 1000, bool rgb = false, uint8_t brightness = 50);

// --- Implementations ---

inline void blinkLed(int16_t count, int16_t msDelay) {
  blinkLedOnPin(LEDPIN, count, msDelay);
}

inline void blinkLedOnPin(uint8_t pin, int16_t count, int16_t msDelay, bool rgb, uint8_t brightness) {
  pinMode(pin, OUTPUT);
  if (msDelay < 100) msDelay = 100;

  for (int i = 0; i < count; i++) {
    if (rgb) {
      // For RGB LEDs (e.g., NeoPixel), turn on white color
      Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, pin, NEO_GRB + NEO_KHZ800);
      strip.begin();
      strip.setPixelColor(0, strip.Color(0, 0, brightness));  // Blue color
      strip.show();
      delay(msDelay / 2);
      strip.setPixelColor(0, strip.Color(0, 0, 0));  // Turn off
      strip.show();
      delay(msDelay / 2);
      continue;
    } else {
      digitalWrite(pin, HIGH);
      delay(msDelay / 2);
      digitalWrite(pin, LOW);
      delay(msDelay / 2);
    }
  }
}

#endif  // LED_BLINK_UTIL_H
