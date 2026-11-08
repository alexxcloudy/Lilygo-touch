#pragma once

#include <Arduino.h>
#include <LovyanGFX.hpp>

// Color definitions (RGB565)
#define COLOR_BG          0x0000 // Deep OLED black
#define COLOR_EYES        0xFFFF // Pure bright white
#define COLOR_LIME        0x87F0 // Kimo signature neon lime (#84FF00)
#define COLOR_PINK        0xFA6F // Soft blushing pink (#FF4E78)
#define COLOR_CYAN        0x067F // Neon tear cyan (#00D2FF)
#define COLOR_RED         0xF947 // Enraged anime red (#FF2A38)
#define COLOR_COOL_DARK   0x18E3 // Sunglasses dark tint
#define COLOR_COOL_GLINT  0xFFFF // Sunglasses white reflection

enum KimoState {
  STATE_REPOSO = 0,
  STATE_ESCUCHA,
  STATE_PENSANDO,
  STATE_EXITO,
  STATE_CARINO,
  STATE_COOL,
  STATE_FELIZ,
  STATE_RISA,
  STATE_RISA_OJOS,
  STATE_COMPLICIDAD,
  STATE_SEGURO,
  STATE_OOPS,
  STATE_VERGUENZA,
  STATE_RISA_APENADA,
  STATE_SORPRESA,
  STATE_SHOCK,
  STATE_ENFADO,
  STATE_TRISTEZA,
  STATE_COUNT
};

const char* const KIMO_NAMES[STATE_COUNT] = {
  "REPOSO", "ESCUCHA", "PENSANDO", "EXITO", "CARINO", "COOL",
  "FELIZ", "RISA", "RISA DE OJOS", "COMPLICIDAD", "SEGURO", "OOPS",
  "VERGUENZA", "RISA APENADA", "SORPRESA", "SHOCK", "ENFADO", "TRISTEZA"
};

struct HeartParticle {
  float x, y;
  float speedY;
  float sway;
  float size;
  bool active;
};

struct TearParticle {
  float x, y;
  float speedY;
  float len;
  bool isLeft;
  bool active;
};

class KimoEngine {
public:
  KimoState currentState = STATE_REPOSO;

  // Screen metrics (Landscape 320x170)
  const int16_t screenW = 320;
  const int16_t screenH = 170;
  const int16_t eyeBaseLeftX = 100;  // Left eye center
  const int16_t eyeBaseRightX = 220; // Right eye center
  const int16_t eyeBaseY = 85;       // Vertical center

  // Eye morph variables
  float eyeLX, eyeLY;
  float eyeRX, eyeRY;
  float eyeLW, eyeLH, eyeLR;
  float eyeRW, eyeRH, eyeRR;
  float eyeAngle; // angle tilt
  float isArch;   // 0.0 = capsule, 1.0 = inverted arch (^ ^)
  float blinkScale = 1.0f;

  // Target variables
  float targetLW, targetLH, targetLR;
  float targetRW, targetRH, targetRR;
  float targetAngle;
  float targetArch;

  // Gaze tracking
  float gazeX = 0, gazeY = 0;
  float targetGazeX = 0, targetGazeY = 0;
  uint32_t lastInteractionTime = 0;
  uint32_t nextIdleGlance = 0;
  uint32_t nextBlinkTime = 0;
  uint32_t blinkEndTime = 0;

  // Auto-play mode
  bool autoPlay = false;
  uint32_t nextAutoCycle = 0;

  // Particles
  HeartParticle hearts[6];
  TearParticle tears[8];

  // Animation time
  float animTime = 0.0f;

  void init() {
    eyeLX = eyeBaseLeftX;
    eyeLY = eyeBaseY;
    eyeRX = eyeBaseRightX;
    eyeRY = eyeBaseY;

    eyeLW = targetLW = 96.0f;
    eyeLH = targetLH = 72.0f;
    eyeLR = targetLR = 36.0f;

    eyeRW = targetRW = 96.0f;
    eyeRH = targetRH = 72.0f;
    eyeRR = targetRR = 36.0f;

    eyeAngle = targetAngle = 0.0f;
    isArch = targetArch = 0.0f;

    for (int i = 0; i < 6; i++) hearts[i].active = false;
    for (int i = 0; i < 8; i++) tears[i].active = false;

    setState(STATE_REPOSO);
    nextBlinkTime = millis() + 3000;
  }

  void setState(KimoState st) {
    currentState = st;
    configureTargetsForState(currentState);
    // Smoothly return gaze to center when emotion transitions
    targetGazeX = 0.0f;
    targetGazeY = 0.0f;
    blinkScale = 1.0f;
    nextBlinkTime = millis() + 2500;
  }

  void resetGaze() {
    targetGazeX = 0.0f;
    targetGazeY = 0.0f;
  }

  void nextState() {
    int next = ((int)currentState + 1) % STATE_COUNT;
    setState((KimoState)next);
  }

  void prevState() {
    int prev = ((int)currentState - 1 + STATE_COUNT) % STATE_COUNT;
    setState((KimoState)prev);
  }

  void lookAt(int16_t touchX, int16_t touchY) {
    lastInteractionTime = millis();
    float dx = (touchX - (screenW / 2.0f)) / (screenW / 2.0f);
    float dy = (touchY - (screenH / 2.0f)) / (screenH / 2.0f);
    // Safe gentle gaze bounds: max ±10px horizontal, ±5px vertical
    targetGazeX = constrain(dx * 10.0f, -10.0f, 10.0f);
    targetGazeY = constrain(dy * 5.0f, -5.0f, 5.0f);
  }

  void configureTargetsForState(KimoState st) {
    targetAngle = 0.0f;
    targetArch = 0.0f;

    switch (st) {
      case STATE_REPOSO: // Large relaxed comfortable capsule eyes
        targetLW = 96.0f; targetLH = 68.0f; targetLR = 34.0f;
        targetRW = 96.0f; targetRH = 68.0f; targetRR = 34.0f;
        break;

      case STATE_ESCUCHA: // Attentive wide round eyes
        targetLW = 84.0f; targetLH = 98.0f; targetLR = 42.0f;
        targetRW = 84.0f; targetRH = 98.0f; targetRR = 42.0f;
        break;

      case STATE_PENSANDO: // Eyes looking up-right
        targetLW = 82.0f; targetLH = 90.0f; targetLR = 41.0f;
        targetRW = 82.0f; targetRH = 90.0f; targetRR = 41.0f;
        break;

      case STATE_EXITO: // Cheerful big bouncing eyes
        targetLW = 86.0f; targetLH = 98.0f; targetLR = 43.0f;
        targetRW = 86.0f; targetRH = 98.0f; targetRR = 43.0f;
        break;

      case STATE_CARINO: // Sweet curved happy arches ^ ^
        targetLW = 88.0f; targetLH = 54.0f; targetLR = 27.0f;
        targetRW = 88.0f; targetRH = 54.0f; targetRR = 27.0f;
        targetArch = 1.0f;
        break;

      case STATE_COOL: // Confident eyes + sunglasses
        targetLW = 90.0f; targetLH = 66.0f; targetLR = 22.0f;
        targetRW = 90.0f; targetRH = 66.0f; targetRR = 22.0f;
        break;

      case STATE_FELIZ: // Large round friendly eyes
        targetLW = 86.0f; targetLH = 100.0f; targetLR = 43.0f;
        targetRW = 86.0f; targetRH = 100.0f; targetRR = 43.0f;
        break;

      case STATE_RISA: // Happy inverted U arches ^ ^
        targetLW = 88.0f; targetLH = 54.0f; targetLR = 27.0f;
        targetRW = 88.0f; targetRH = 54.0f; targetRR = 27.0f;
        targetArch = 1.0f;
        break;

      case STATE_RISA_OJOS: // Tightly closed laughing arches
        targetLW = 88.0f; targetLH = 52.0f; targetLR = 26.0f;
        targetRW = 88.0f; targetRH = 52.0f; targetRR = 26.0f;
        targetArch = 1.0f;
        break;

      case STATE_COMPLICIDAD: // Wink (Left eye open, Right eye wink line)
        targetLW = 84.0f; targetLH = 98.0f; targetLR = 42.0f;
        targetRW = 82.0f; targetRH = 16.0f; targetRR = 8.0f;
        break;

      case STATE_SEGURO: // Confident almond eyes
        targetLW = 88.0f; targetLH = 82.0f; targetLR = 32.0f;
        targetRW = 88.0f; targetRH = 82.0f; targetRR = 32.0f;
        targetAngle = 8.0f;
        break;

      case STATE_OOPS: // Sheepish eyes looking down
        targetLW = 82.0f; targetLH = 90.0f; targetLR = 40.0f;
        targetRW = 82.0f; targetRH = 90.0f; targetRR = 40.0f;
        break;

      case STATE_VERGUENZA: // Shy downward eyes with blush
        targetLW = 82.0f; targetLH = 88.0f; targetLR = 40.0f;
        targetRW = 82.0f; targetRH = 88.0f; targetRR = 40.0f;
        break;

      case STATE_RISA_APENADA: // Nervous arched laugh
        targetLW = 88.0f; targetLH = 54.0f; targetLR = 27.0f;
        targetRW = 88.0f; targetRH = 54.0f; targetRR = 27.0f;
        targetArch = 1.0f;
        break;

      case STATE_SORPRESA: // Huge wide circular eyes
        targetLW = 90.0f; targetLH = 102.0f; targetLR = 45.0f;
        targetRW = 90.0f; targetRH = 102.0f; targetRR = 45.0f;
        break;

      case STATE_SHOCK: // Tall trembling vertical pill eyes
        targetLW = 76.0f; targetLH = 100.0f; targetLR = 38.0f;
        targetRW = 76.0f; targetRH = 100.0f; targetRR = 38.0f;
        break;

      case STATE_ENFADO: // Sharp angry inward furrowed eyes
        targetLW = 86.0f; targetLH = 68.0f; targetLR = 22.0f;
        targetRW = 86.0f; targetRH = 68.0f; targetRR = 22.0f;
        targetAngle = -16.0f;
        break;

      case STATE_TRISTEZA: // Drooping sad downward eyes
        targetLW = 86.0f; targetLH = 70.0f; targetLR = 24.0f;
        targetRW = 86.0f; targetRH = 70.0f; targetRR = 24.0f;
        targetAngle = 14.0f;
        break;
      default:
        break;
    }
  }

  void update() {
    uint32_t now = millis();
    animTime += 0.045f;

    // Auto-cycle through emotions if enabled
    if (autoPlay && now > nextAutoCycle) {
      nextState();
      nextAutoCycle = now + 4500;
    }

    // Micro-blinking (only for non-arch states to prevent flicker)
    if (isArch < 0.4f) {
      if (now > nextBlinkTime) {
        blinkEndTime = now + 130;
        nextBlinkTime = now + 2800 + (esp_random() % 3000);
      }
      float targetBlink = (now < blinkEndTime) ? 0.08f : 1.0f;
      blinkScale += (targetBlink - blinkScale) * 0.40f;
    } else {
      blinkScale = 1.0f;
    }

    // Idle gaze wandering if untouched for 3.5 seconds
    if (now - lastInteractionTime > 3500) {
      if (now > nextIdleGlance) {
        if (currentState == STATE_PENSANDO) {
          targetGazeX = 8.0f;
          targetGazeY = -4.0f; // gentle glance up-right
        } else if (currentState == STATE_OOPS) {
          targetGazeX = 5.0f;
          targetGazeY = 4.0f;
        } else if (currentState == STATE_VERGUENZA) {
          targetGazeX = 0.0f;
          targetGazeY = 3.0f;
        } else {
          // Centered gentle idle glance: max ±8 horizontal, ±3 vertical
          targetGazeX = ((esp_random() % 17) - 8);
          targetGazeY = ((esp_random() % 7) - 3);
        }
        nextIdleGlance = now + 2500 + (esp_random() % 2500);
      }
    }

    // Smooth gaze interpolation
    gazeX += (targetGazeX - gazeX) * 0.14f;
    gazeY += (targetGazeY - gazeY) * 0.14f;

    // Subtle breathing motion (±1.0px)
    float breathY = sinf(animTime * 0.8f) * 1.0f;

    // Gentle bounces and micro-jitters
    float bounceY = 0.0f;
    float jitterX = 0.0f, jitterY = 0.0f;
    if (currentState == STATE_RISA || currentState == STATE_RISA_APENADA) {
      bounceY = -fabsf(sinf(animTime * 4.5f)) * 3.0f;
    } else if (currentState == STATE_EXITO) {
      bounceY = -fabsf(sinf(animTime * 3.0f)) * 3.0f;
    } else if (currentState == STATE_SHOCK) {
      jitterX = ((esp_random() % 4) - 2) * 0.5f;
      jitterY = ((esp_random() % 4) - 2) * 0.5f;
    }

    // Interpolate eye positions
    float destLX = eyeBaseLeftX + gazeX + jitterX;
    float destLY = eyeBaseY + gazeY + breathY + bounceY + jitterY;
    float destRX = eyeBaseRightX + gazeX + jitterX;
    float destRY = eyeBaseY + gazeY + breathY + bounceY + jitterY;

    eyeLX += (destLX - eyeLX) * 0.18f;
    eyeLY += (destLY - eyeLY) * 0.18f;
    eyeRX += (destRX - eyeRX) * 0.18f;
    eyeRY += (destRY - eyeRY) * 0.18f;

    // 3D Parallax scale
    float lookNormX = gazeX / 12.0f;
    float leftScaleX = 1.0f - (lookNormX * 0.04f);
    float rightScaleX = 1.0f + (lookNormX * 0.04f);

    // Smooth eye morphing
    eyeLW += ((targetLW * leftScaleX) - eyeLW) * 0.20f;
    eyeLH += (targetLH - eyeLH) * 0.20f;
    eyeLR += (targetLR - eyeLR) * 0.20f;

    eyeRW += ((targetRW * rightScaleX) - eyeRW) * 0.20f;
    eyeRH += (targetRH - eyeRH) * 0.20f;
    eyeRR += (targetRR - eyeRR) * 0.20f;

    eyeAngle += (targetAngle - eyeAngle) * 0.20f;
    isArch += (targetArch - isArch) * 0.20f;

    // Update Particles
    updateParticles();
  }

  void updateParticles() {
    // 1. Floating Hearts for CARINO
    if (currentState == STATE_CARINO) {
      for (int i = 0; i < 6; i++) {
        if (!hearts[i].active && (esp_random() % 12 == 0)) {
          hearts[i].active = true;
          hearts[i].x = 115 + (esp_random() % 90);
          hearts[i].y = 145;
          hearts[i].speedY = 1.2f + (esp_random() % 10) / 10.0f;
          hearts[i].sway = (esp_random() % 100) / 10.0f;
          hearts[i].size = 5.0f + (esp_random() % 3);
        }
        if (hearts[i].active) {
          hearts[i].y -= hearts[i].speedY;
          hearts[i].x += sinf(animTime * 2.0f + hearts[i].sway) * 0.6f;
          if (hearts[i].y < 15) hearts[i].active = false;
        }
      }
    } else {
      for (int i = 0; i < 6; i++) hearts[i].active = false;
    }

    // 2. Cascading Tears for TRISTEZA
    if (currentState == STATE_TRISTEZA) {
      for (int i = 0; i < 8; i++) {
        if (!tears[i].active && (esp_random() % 7 == 0)) {
          tears[i].active = true;
          tears[i].isLeft = (esp_random() % 2 == 0);
          tears[i].x = tears[i].isLeft ? (eyeLX - 28 + (esp_random() % 8)) : (eyeRX + 20 + (esp_random() % 8));
          tears[i].y = eyeLY + 20;
          tears[i].speedY = 2.4f + (esp_random() % 20) / 10.0f;
          tears[i].len = 8.0f + (esp_random() % 10);
        }
        if (tears[i].active) {
          tears[i].y += tears[i].speedY;
          if (tears[i].y > screenH - 5) tears[i].active = false;
        }
      }
    } else {
      for (int i = 0; i < 8; i++) tears[i].active = false;
    }
  }

  // Safe capsule drawing: guaranteed never to cause LovyanGFX underflow or sprite corruption
  void drawSafeCapsule(LGFX_Sprite &spr, int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    if (w <= 0 || h <= 0) return;
    if (h <= 6) {
      spr.fillRect(x, y, w, h, color);
      return;
    }
    int16_t maxR = min((int16_t)((h - 2) / 2), (int16_t)((w - 2) / 2));
    if (r > maxR) r = maxR;
    if (r <= 1) {
      spr.fillRect(x, y, w, h, color);
    } else {
      spr.fillRoundRect(x, y, w, h, r, color);
    }
  }

  // Draw eye primitive
  void drawSingleEye(LGFX_Sprite &spr, float cx, float cy, float w, float h, float r, float angle, bool isLeft) {
    float curH = max(3.0f, h * blinkScale);
    float curR = min(r, curH / 2.0f);

    float halfH = curH / 2.0f;
    int16_t x0 = (int16_t)(cx - w / 2.0f);
    int16_t y0 = (int16_t)(cy - halfH);
    int16_t iw = (int16_t)w;
    int16_t ih = (int16_t)curH;
    int16_t ir = (int16_t)curR;

    // Inverted happy arch eye (^ ^) for Risa, Cariño, Risa de ojos
    if (isArch > 0.4f) {
      int16_t rad = iw / 2;
      int16_t thick = 14;
      int16_t archY = (int16_t)(cy + 8);

      // Outer semicircle
      spr.fillCircle((int16_t)cx, archY, rad, COLOR_EYES);
      // Inner cutout
      spr.fillCircle((int16_t)cx, archY, rad - thick, COLOR_BG);
      // Clear bottom half cleanly
      spr.fillRect((int16_t)(cx - rad - 2), archY, iw + 4, rad + 4, COLOR_BG);
      // Rounded end caps
      spr.fillCircle((int16_t)(cx - rad + thick / 2), archY, thick / 2, COLOR_EYES);
      spr.fillCircle((int16_t)(cx + rad - thick / 2), archY, thick / 2, COLOR_EYES);
      return;
    }

    // Angled furrow eyes (Enfado / Tristeza)
    if (fabsf(angle) > 2.0f && ih > 10) {
      drawSafeCapsule(spr, x0, y0, iw, ih, ir, COLOR_EYES);

      int16_t cutH = min((int16_t)18, (int16_t)(ih / 2));
      if (angle < 0) { // Anger (furrowed inward)
        if (isLeft) {
          spr.fillTriangle(x0 - 2, y0 - 2, x0 + iw + 2, y0 - 2, x0 + iw + 2, y0 + cutH, COLOR_BG);
        } else {
          spr.fillTriangle(x0 - 2, y0 - 2, x0 + iw + 2, y0 - 2, x0 - 2, y0 + cutH, COLOR_BG);
        }
      } else { // Sadness (drooping downward outer)
        if (isLeft) {
          spr.fillTriangle(x0 - 2, y0 - 2, x0 + iw + 2, y0 - 2, x0 - 2, y0 + cutH, COLOR_BG);
        } else {
          spr.fillTriangle(x0 - 2, y0 - 2, x0 + iw + 2, y0 - 2, x0 + iw + 2, y0 + cutH, COLOR_BG);
        }
      }
      return;
    }

    // Normal rounded capsule eye using safe renderer
    drawSafeCapsule(spr, x0, y0, iw, ih, ir, COLOR_EYES);
  }

  // Draw 4-point sparkle star
  void drawSparkle(LGFX_Sprite &spr, int16_t sx, int16_t sy, int16_t radius, uint16_t color) {
    spr.drawLine(sx - radius, sy, sx + radius, sy, color);
    spr.drawLine(sx, sy - radius, sx, sy + radius, color);
    spr.drawLine(sx - radius + 2, sy, sx + radius - 2, sy, color);
    spr.drawLine(sx, sy - radius + 2, sx, sy + radius - 2, color);
    spr.fillCircle(sx, sy, 2, COLOR_EYES);
  }

  // Draw anime anger mark (💢)
  void drawAngerMark(LGFX_Sprite &spr, int16_t cx, int16_t cy, float pulse) {
    int16_t s = (int16_t)(10 * pulse);
    int16_t t = 3;
    spr.fillRect(cx - s, cy - s, s * 2, t, COLOR_RED);
    spr.fillRect(cx - s, cy + s - t, s * 2, t, COLOR_RED);
    spr.fillRect(cx - s, cy - s, t, s * 2, COLOR_RED);
    spr.fillRect(cx + s - t, cy - s, t, s * 2, COLOR_RED);
    spr.drawLine(cx - s / 2, cy - s, cx - s / 2, cy + s, COLOR_RED);
    spr.drawLine(cx + s / 2, cy - s, cx + s / 2, cy + s, COLOR_RED);
    spr.drawLine(cx - s, cy - s / 2, cx + s, cy - s / 2, COLOR_RED);
    spr.drawLine(cx - s, cy + s / 2, cx + s, cy + s / 2, COLOR_RED);
  }

  // Draw teardrop sweatdrop
  void drawSweatdrop(LGFX_Sprite &spr, int16_t sx, int16_t sy, uint16_t col = COLOR_LIME) {
    spr.fillCircle(sx, sy + 7, 5, col);
    spr.fillTriangle(sx, sy, sx - 4, sy + 6, sx + 4, sy + 6, col);
    spr.drawPixel(sx - 1, sy + 5, COLOR_EYES);
  }

  // Draw blush lines (///)
  void drawBlush(LGFX_Sprite &spr, int16_t bx, int16_t by) {
    for (int i = -1; i <= 1; i++) {
      int16_t x = bx + (i * 9);
      spr.drawLine(x - 3, by + 5, x + 3, by - 5, COLOR_PINK);
      spr.drawLine(x - 2, by + 5, x + 4, by - 5, COLOR_PINK);
    }
  }

  // Draw sweet heart
  void drawHeart(LGFX_Sprite &spr, int16_t hx, int16_t hy, int16_t s) {
    spr.fillCircle(hx - s / 2, hy - s / 3, s / 2, COLOR_PINK);
    spr.fillCircle(hx + s / 2, hy - s / 3, s / 2, COLOR_PINK);
    spr.fillTriangle(hx - s, hy - s / 4, hx + s, hy - s / 4, hx, hy + s, COLOR_PINK);
  }

  // Main Render Frame
  void render(LGFX_Sprite &spr) {
    spr.fillScreen(COLOR_BG);

    // 1. Draw Large Expressive Eyes (NO mouth or nose dot)
    drawSingleEye(spr, eyeLX, eyeLY, eyeLW, eyeLH, eyeLR, eyeAngle, true);
    drawSingleEye(spr, eyeRX, eyeRY, eyeRW, eyeRH, eyeRR, -eyeAngle, false);

    // 2. State-Specific Overlays and Accessories

    // REPOSO: Signature Neon Lime Antenna mark
    if (currentState == STATE_REPOSO) {
      float antennaPulse = 1.0f + 0.1f * sinf(animTime * 2.0f);
      int16_t ax = (int16_t)(eyeRX + 26);
      int16_t ay = max((int16_t)24, (int16_t)(eyeRY - 44));
      spr.fillRoundRect(ax, ay, (int16_t)(26 * antennaPulse), 8, 4, COLOR_LIME);
    }

    // ESCUCHA: Animated Audio Waves on lateral edges
    if (currentState == STATE_ESCUCHA) {
      for (int i = 0; i < 3; i++) {
        float waveH = (sinf(animTime * 4.0f + i * 1.0f) * 12.0f) + 16.0f;
        int16_t lx = 16 + (i * 9);
        spr.fillRoundRect(lx, (screenH / 2) - (waveH / 2), 4, waveH, 2, COLOR_LIME);
        int16_t rx = (screenW - 20) - (i * 9);
        spr.fillRoundRect(rx, (screenH / 2) - (waveH / 2), 4, waveH, 2, COLOR_LIME);
      }
    }

    // PENSANDO: 3 Thinking Orbital Dots above gaze
    if (currentState == STATE_PENSANDO) {
      for (int i = 0; i < 3; i++) {
        float phase = animTime * 3.0f + i * 1.1f;
        int16_t dotX = (int16_t)(eyeRX + 16 + (i * 12));
        int16_t dotY = max((int16_t)24, (int16_t)(eyeRY - 36 + (sinf(phase) * 4.0f)));
        spr.fillCircle(dotX, dotY, 4, COLOR_LIME);
      }
    }

    // EXITO: Bouncing Sparkles in Lime and White
    if (currentState == STATE_EXITO) {
      float spY = sinf(animTime * 3.0f) * 4.0f;
      drawSparkle(spr, 36, 40 + spY, 10, COLOR_LIME);
      drawSparkle(spr, 284, 40 - spY, 11, COLOR_LIME);
      drawSparkle(spr, 160, 26 + spY * 0.5f, 8, COLOR_EYES);
    }

    // CARINO: Floating Hearts & Blushing Cheeks
    if (currentState == STATE_CARINO) {
      drawBlush(spr, (int16_t)(eyeLX - 6), (int16_t)(eyeLY + 30));
      drawBlush(spr, (int16_t)(eyeRX + 6), (int16_t)(eyeRY + 30));
      for (int i = 0; i < 6; i++) {
        if (hearts[i].active) {
          drawHeart(spr, (int16_t)hearts[i].x, (int16_t)hearts[i].y, (int16_t)hearts[i].size);
        }
      }
    }

    // COOL: Sleek Dark Sunglasses with diagonal specular gleam
    if (currentState == STATE_COOL) {
      int16_t gw = 90, gh = 66;
      int16_t gy = (int16_t)eyeLY;

      // Dark lenses with white border
      spr.fillRoundRect((int16_t)(eyeLX - gw / 2), gy - gh / 2, gw, gh, 18, COLOR_COOL_DARK);
      spr.fillRoundRect((int16_t)(eyeRX - gw / 2), gy - gh / 2, gw, gh, 18, COLOR_COOL_DARK);
      spr.drawRoundRect((int16_t)(eyeLX - gw / 2), gy - gh / 2, gw, gh, 18, COLOR_EYES);
      spr.drawRoundRect((int16_t)(eyeRX - gw / 2), gy - gh / 2, gw, gh, 18, COLOR_EYES);

      // Solid center bridge connecting lenses
      int16_t bX = (int16_t)(eyeLX + gw / 2 - 2);
      int16_t bW = (int16_t)((eyeRX - gw / 2) - (eyeLX + gw / 2) + 4);
      if (bW > 0) {
        spr.fillRect(bX, gy - 4, bW, 8, COLOR_EYES);
      }

      // Specular gleam restricted inside lens area
      float sh = fmodf(animTime * 28.0f, gw + 30.0f) - 15.0f;
      if (sh >= 4.0f && sh <= (gw - 16.0f)) {
        spr.drawLine((int16_t)(eyeLX - gw / 2 + sh), gy - gh / 2 + 8,
                     (int16_t)(eyeLX - gw / 2 + sh + 14), gy + gh / 2 - 8, COLOR_COOL_GLINT);
        spr.drawLine((int16_t)(eyeRX - gw / 2 + sh), gy - gh / 2 + 8,
                     (int16_t)(eyeRX - gw / 2 + sh + 14), gy + gh / 2 - 8, COLOR_COOL_GLINT);
      }
    }

    // RISA_OJOS: Center sparkle + temple sweatdrops
    if (currentState == STATE_RISA_OJOS) {
      drawSparkle(spr, 160, 26, 10, COLOR_LIME);
      drawSweatdrop(spr, (int16_t)(eyeLX - 34), max((int16_t)24, (int16_t)(eyeLY - 12)));
      drawSweatdrop(spr, (int16_t)(eyeRX + 34), max((int16_t)24, (int16_t)(eyeRY - 12)));
    }

    // SEGURO: Confident dual sparkles
    if (currentState == STATE_SEGURO) {
      drawSparkle(spr, (int16_t)(eyeLX - 34), max((int16_t)24, (int16_t)(eyeLY - 10)), 9, COLOR_LIME);
      drawSparkle(spr, (int16_t)(eyeRX + 34), max((int16_t)24, (int16_t)(eyeRY - 10)), 9, COLOR_LIME);
    }

    // OOPS: Dripping lime sweatdrop
    if (currentState == STATE_OOPS) {
      drawSweatdrop(spr, (int16_t)(eyeRX + 32), max((int16_t)24, (int16_t)(eyeRY - 8 + sinf(animTime * 2.5f) * 3.0f)));
    }

    // VERGUENZA: Blushing cheeks
    if (currentState == STATE_VERGUENZA) {
      drawBlush(spr, (int16_t)(eyeLX - 4), (int16_t)(eyeLY + 30));
      drawBlush(spr, (int16_t)(eyeRX + 4), (int16_t)(eyeRY + 30));
    }

    // RISA APENADA: Sweat drop + blush
    if (currentState == STATE_RISA_APENADA) {
      drawBlush(spr, (int16_t)(eyeLX - 4), (int16_t)(eyeLY + 28));
      drawBlush(spr, (int16_t)(eyeRX + 4), (int16_t)(eyeRY + 28));
      drawSweatdrop(spr, (int16_t)(eyeRX + 32), max((int16_t)24, (int16_t)(eyeRY - 12)));
    }

    // SHOCK: Trembling sweat drops
    if (currentState == STATE_SHOCK) {
      drawSweatdrop(spr, (int16_t)(eyeRX + 32), max((int16_t)24, (int16_t)(eyeRY - 14)));
      drawSweatdrop(spr, (int16_t)(eyeLX - 32), max((int16_t)24, (int16_t)(eyeLY - 14)));
    }

    // ENFADO: Pulsing Red Anime Anger Vein Mark
    if (currentState == STATE_ENFADO) {
      float angerPulse = 1.0f + 0.25f * sinf(animTime * 6.0f);
      drawAngerMark(spr, 276, 34, angerPulse);
    }

    // TRISTEZA: Falling Cyan Cascading Tears from outer eye edges
    if (currentState == STATE_TRISTEZA) {
      for (int i = 0; i < 8; i++) {
        if (tears[i].active) {
          spr.fillRoundRect((int16_t)tears[i].x, (int16_t)tears[i].y, 4, (int16_t)tears[i].len, 2, COLOR_CYAN);
        }
      }
    }

    // 3. Permanent Scene Number & Name Overlay
    char idStr[32];
    snprintf(idStr, sizeof(idStr), "[%02d] %s", (int)currentState + 1, KIMO_NAMES[currentState]);
    spr.setTextDatum(textdatum_t::top_left);
    int16_t txtW = spr.textWidth(idStr, &fonts::Font2);
    spr.fillRoundRect(3, 3, txtW + 10, 17, 4, COLOR_BG);
    spr.drawRoundRect(3, 3, txtW + 10, 17, 4, COLOR_LIME);
    spr.setTextColor(COLOR_LIME, COLOR_BG);
    spr.drawString(idStr, 8, 4, &fonts::Font2);
  }
};
