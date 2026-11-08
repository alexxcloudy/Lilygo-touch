#include <Arduino.h>
#include "LGFX_Config.hpp"
#include "CST816Touch.hpp"
#include "TrokoEmotions.hpp"

// Physical buttons on T-Display-S3
#define BTN_BOOT 0
#define BTN_USER 14

LGFX lcd;
LGFX_Sprite canvas(&lcd);
CST816Touch touch;
TrokoEngine troko;

uint32_t lastBtn14Press = 0;
uint32_t lastBtn0Press = 0;
uint32_t lastFrameTime = 0;

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("======================================");
  Serial.println("  TROKO ESP32-S3 COMPANION (V2 BIG)   ");
  Serial.println("   LilyGO T-Display-S3 Touch (1.9\")   ");
  Serial.println("======================================");

  // Initialize Hardware Buttons
  pinMode(BTN_BOOT, INPUT_PULLUP);
  pinMode(BTN_USER, INPUT_PULLUP);

  // Initialize ST7789 Parallel Display
  lcd.init();
  lcd.setRotation(1); // Landscape 320x170
  lcd.setBrightness(230);
  lcd.fillScreen(COLOR_BG);

  // Initialize CST816 Touch Screen
  bool touchOk = touch.begin();
  Serial.printf("CST816 Touch Status: %s\n", touchOk ? "ONLINE" : "OFFLINE");

  // Create Double-Buffered Canvas (320 x 170 x 16-bit)
  canvas.setColorDepth(16);
  if (!canvas.createSprite(320, 170)) {
    Serial.println("Allocating canvas sprite in PSRAM...");
    canvas.setPsram(true);
    canvas.createSprite(320, 170);
  }
  canvas.fillScreen(COLOR_BG);

  // Initialize Troko Engine
  troko.init();

  Serial.println("Troko V2 ready! Large, expressive eyes active.");
}

void handleButtons() {
  uint32_t now = millis();

  // Button 14 -> Next emotion
  if (digitalRead(BTN_USER) == LOW) {
    if (now - lastBtn14Press > 260) {
      lastBtn14Press = now;
      troko.nextState();
      Serial.printf("Button 14 -> Emotion: %s\n", TROKO_NAMES[troko.currentState]);
    }
  }

  // Button 0 -> Toggle Auto-Cycle / Showcase mode
  if (digitalRead(BTN_BOOT) == LOW) {
    if (now - lastBtn0Press > 320) {
      lastBtn0Press = now;
      troko.autoPlay = !troko.autoPlay;
      troko.nextAutoCycle = now + 4000;
      Serial.printf("Boot Button -> AutoPlay: %s\n", troko.autoPlay ? "ENABLED" : "DISABLED");
    }
  }
}

void handleTouch() {
  bool active = touch.update(1);
  uint32_t now = millis();
  static uint32_t lastGestureTime = 0;

  if (active) {
    troko.lookAt(touch.x, touch.y);

    if (touch.gesture != GESTURE_NONE && (now - lastGestureTime > 350)) {
      if (touch.gesture == GESTURE_SLIDE_LEFT || touch.gesture == GESTURE_SLIDE_UP) {
        lastGestureTime = now;
        troko.nextState();
        troko.resetGaze();
        Serial.printf("Swipe Next -> %s\n", TROKO_NAMES[troko.currentState]);
      } else if (touch.gesture == GESTURE_SLIDE_RIGHT || touch.gesture == GESTURE_SLIDE_DOWN) {
        lastGestureTime = now;
        troko.prevState();
        troko.resetGaze();
        Serial.printf("Swipe Prev -> %s\n", TROKO_NAMES[troko.currentState]);
      } else if (touch.gesture == GESTURE_LONG_PRESS) {
        lastGestureTime = now;
        troko.setState(STATE_REPOSO);
        troko.resetGaze();
        Serial.println("Long Press -> Back to REPOSO");
      } else if (touch.gesture == GESTURE_DOUBLE_CLICK) {
        lastGestureTime = now;
        troko.autoPlay = !troko.autoPlay;
        troko.nextAutoCycle = now + 4000;
        Serial.printf("Double Click -> AutoPlay: %s\n", troko.autoPlay ? "ON" : "OFF");
      }
    }
  } else {
    // When finger is released, gently return gaze to center
    troko.resetGaze();
  }
}

void loop() {
  handleButtons();
  handleTouch();

  uint32_t now = millis();
  if (now - lastFrameTime >= 16) { // Silky smooth 60 FPS
    lastFrameTime = now;
    troko.update();
    troko.render(canvas);
    canvas.pushSprite(0, 0);
  }
  delay(1);
}
