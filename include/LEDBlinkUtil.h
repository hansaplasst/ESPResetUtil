#ifndef LED_BLINK_UTIL_H
#define LED_BLINK_UTIL_H

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
void blinkLedOnPin(uint8_t pin, int16_t count = 1, int16_t msDelay = 1000);

// --- Implementations ---

inline void blinkLed(int16_t count, int16_t msDelay) {
  blinkLedOnPin(LEDPIN, count, msDelay);
}

inline void blinkLedOnPin(uint8_t pin, int16_t count, int16_t msDelay) {
  pinMode(pin, OUTPUT);
  if (msDelay < 100) msDelay = 100;

  for (int i = 0; i < count; i++) {
    digitalWrite(pin, HIGH);
    delay(msDelay / 2);
    digitalWrite(pin, LOW);
    delay(msDelay / 2);
  }
}

#endif  // LED_BLINK_UTIL_H
