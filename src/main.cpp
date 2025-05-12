#include <Arduino.h>

#define DEBUG_LEVEL 1  // 0 = VERBOSE, 1 = INFO, 2 = WARNING, 3 = ERROR
#include <dprintf.h>

#ifdef BROWNOUT_HACK
  #include "soc/rtc_cntl_reg.h"
  #include "soc/soc.h"
#endif

#include "ESPResetUtil.h"

// --- Project settings ---
#define RESET_PIN 4
#define LEDPIN 2

void setup() {
  delay(1000);
  Serial.begin(BAUDRATE);

  pinMode(LEDPIN, OUTPUT);
  pinMode(RESET_PIN, INPUT_PULLUP);

#ifdef ARDUINO_ARCH_ESP32
  if (!LittleFS.begin(false)) {
#elif defined(ARDUINO_ARCH_ESP8266)
  if (!LittleFS.begin()) {
#endif
    DPRINTF(3, "[LittleFS] Initialization failed! Formatting...\n");
    factoryReset();
  }

  checkResetButtonOnStartup(RESET_PIN, LEDPIN);

#ifdef BROWNOUT_HACK
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);  // Disable brownout detector
#endif

  DPRINTF(1, "[Setup] WiFi/Bluetooth initialization would go here...\n");

#ifdef BROWNOUT_HACK
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 1);  // Re-enable brownout detector
#endif
}

void loop() {
  if (digitalRead(RESET_PIN) == LOW) {
    DPRINTF(2, "[Loop] Reset button pressed during runtime\n");
    espReset();
  }

  delay(10);
}
