#pragma once

#include <Arduino.h>
#include <Wire.h>

#define CST816_I2C_ADDR 0x15
#define PIN_TOUCH_SDA   18
#define PIN_TOUCH_SCL   17
#define PIN_TOUCH_RST   21
#define PIN_TOUCH_INT   16

enum TouchGesture {
  GESTURE_NONE = 0x00,
  GESTURE_SLIDE_UP = 0x01,
  GESTURE_SLIDE_DOWN = 0x02,
  GESTURE_SLIDE_LEFT = 0x03,
  GESTURE_SLIDE_RIGHT = 0x04,
  GESTURE_SINGLE_CLICK = 0x05,
  GESTURE_DOUBLE_CLICK = 0x0B,
  GESTURE_LONG_PRESS = 0x0C
};

static volatile bool s_touchFlag = false;
static void IRAM_ATTR touchISR() {
  s_touchFlag = true;
}

class CST816Touch {
public:
  int16_t x = 0;
  int16_t y = 0;
  TouchGesture gesture = GESTURE_NONE;
  TouchGesture lastRawGesture = GESTURE_NONE;
  bool touched = false;

  bool begin() {
    pinMode(PIN_TOUCH_INT, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PIN_TOUCH_INT), touchISR, FALLING);

    pinMode(PIN_TOUCH_RST, OUTPUT);
    digitalWrite(PIN_TOUCH_RST, LOW);
    delay(20);
    digitalWrite(PIN_TOUCH_RST, HIGH);
    delay(50);

    Wire.begin(PIN_TOUCH_SDA, PIN_TOUCH_SCL, 400000U);

    // Verify presence
    Wire.beginTransmission(CST816_I2C_ADDR);
    return (Wire.endTransmission() == 0);
  }

  bool update(uint8_t rotation = 1) {
    bool hasEvent = s_touchFlag || (digitalRead(PIN_TOUCH_INT) == LOW);
    s_touchFlag = false;

    if (!hasEvent) {
      touched = false;
      gesture = GESTURE_NONE;
      lastRawGesture = GESTURE_NONE;
      return false;
    }

    Wire.beginTransmission(CST816_I2C_ADDR);
    Wire.write(0x01);
    if (Wire.endTransmission() != 0) {
      touched = false;
      return false;
    }

    if (Wire.requestFrom((uint8_t)CST816_I2C_ADDR, (uint8_t)6) != 6) {
      touched = false;
      return false;
    }

    uint8_t gest = Wire.read();
    uint8_t points = Wire.read();
    uint8_t x_hi = Wire.read();
    uint8_t x_lo = Wire.read();
    uint8_t y_hi = Wire.read();
    uint8_t y_lo = Wire.read();

    touched = (points > 0);

    // Only fire gesture event on rising edge (single fire per swipe)
    uint8_t effectiveGest = gest;
    if (gest == (uint8_t)lastRawGesture) {
      effectiveGest = 0; // Already fired for this swipe gesture
    } else {
      lastRawGesture = (TouchGesture)gest;
    }

    // Map hardware gestures based on rotation
    if (rotation == 1) {
      switch (effectiveGest) {
        case 0x01: gesture = GESTURE_SLIDE_LEFT; break;
        case 0x02: gesture = GESTURE_SLIDE_RIGHT; break;
        case 0x03: gesture = GESTURE_SLIDE_UP; break;
        case 0x04: gesture = GESTURE_SLIDE_DOWN; break;
        default:   gesture = (TouchGesture)effectiveGest; break;
      }
    } else {
      gesture = (TouchGesture)effectiveGest;
    }

    if (touched) {
      uint16_t raw_x = ((x_hi & 0x0F) << 8) | x_lo;
      uint16_t raw_y = ((y_hi & 0x0F) << 8) | y_lo;

      // CST816 native is portrait (170 x 320)
      if (rotation == 1) {
        // Landscape rotation 1 (320x170)
        x = raw_y;
        y = 170 - 1 - raw_x;
        if (x < 0) x = 0; if (x >= 320) x = 319;
        if (y < 0) y = 0; if (y >= 170) y = 169;
      } else if (rotation == 3) {
        // Landscape rotation 3 (320x170)
        x = 320 - 1 - raw_y;
        y = raw_x;
        if (x < 0) x = 0; if (x >= 320) x = 319;
        if (y < 0) y = 0; if (y >= 170) y = 169;
      } else {
        x = raw_x;
        y = raw_y;
      }
    }

    return touched;
  }
};
