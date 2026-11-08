#pragma once

#include <Arduino.h>
#include <lvgl.h>

// Color definitions (RGB565 / Hex)
#define KIMO_HEX_BG         0x000000
#define KIMO_HEX_EYES       0xFFFFFF
#define KIMO_HEX_LIME       0x84FF00
#define KIMO_HEX_PINK       0xFF4E78
#define KIMO_HEX_CYAN       0x00D2FF
#define KIMO_HEX_RED        0xFF2A38
#define KIMO_HEX_COOL_DARK  0x1A1E24
#define KIMO_HEX_COOL_GLINT 0xFFFFFF

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

struct LVGLHeart {
  lv_obj_t* obj;
  float x, y;
  float speedY;
  float sway;
  bool active;
};

struct LVGLTear {
  lv_obj_t* obj;
  float x, y;
  float speedY;
  bool active;
};

class KimoLVGL {
public:
  KimoState currentState = STATE_REPOSO;

  // Screen metrics (Landscape 320x170)
  const int16_t screenW = 320;
  const int16_t screenH = 170;
  const int16_t eyeBaseLeftX = 100;
  const int16_t eyeBaseRightX = 220;
  const int16_t eyeBaseY = 85;

  // LVGL Widgets
  lv_obj_t* eyeL = nullptr;
  lv_obj_t* eyeR = nullptr;
  lv_obj_t* archL = nullptr;
  lv_obj_t* archR = nullptr;

  // Overlays
  lv_obj_t* badgeContainer = nullptr;
  lv_obj_t* badgeLabel = nullptr;
  lv_obj_t* antenna = nullptr;

  // Audio waves for ESCUCHA
  lv_obj_t* wavesL[3];
  lv_obj_t* wavesR[3];

  // Thinking dots for PENSANDO
  lv_obj_t* thinkingDots[3];

  // Sunglasses for COOL
  lv_obj_t* sunglassL = nullptr;
  lv_obj_t* sunglassR = nullptr;
  lv_obj_t* sunglassBridge = nullptr;

  // Sparkles
  lv_obj_t* sparkle1 = nullptr;
  lv_obj_t* sparkle2 = nullptr;

  // Sweatdrops
  lv_obj_t* sweatL = nullptr;
  lv_obj_t* sweatR = nullptr;

  // Anger mark for ENFADO
  lv_obj_t* angerMark = nullptr;

  // Blush for CARINO / VERGUENZA
  lv_obj_t* blushL = nullptr;
  lv_obj_t* blushR = nullptr;

  // Particles
  LVGLHeart hearts[4];
  LVGLTear tears[4];

  // Dynamics
  float eyeLX, eyeLY;
  float eyeRX, eyeRY;
  float eyeLW, eyeLH, eyeLR;
  float eyeRW, eyeRH, eyeRR;
  float eyeAngle;
  float isArch;
  float blinkScale = 1.0f;

  float targetLW, targetLH, targetLR;
  float targetRW, targetRH, targetRR;
  float targetAngle;
  float targetArch;

  float gazeX = 0, gazeY = 0;
  float targetGazeX = 0, targetGazeY = 0;
  uint32_t lastInteractionTime = 0;
  uint32_t nextIdleGlance = 0;
  uint32_t nextBlinkTime = 0;
  uint32_t blinkEndTime = 0;

  bool autoPlay = false;
  uint32_t nextAutoCycle = 0;
  float animTime = 0.0f;

  void init() {
    lv_obj_t* scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_hex(KIMO_HEX_BG), 0);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

    // 1. Create Eyes (Capsules)
    eyeL = lv_obj_create(scr);
    eyeR = lv_obj_create(scr);
    setupEyeStyle(eyeL);
    setupEyeStyle(eyeR);

    // 2. Create Arched Eyes (for Risa, Cariño) using styled arcs
    archL = lv_arc_create(scr);
    archR = lv_arc_create(scr);
    setupArchStyle(archL);
    setupArchStyle(archR);

    // 3. Status Badge
    badgeContainer = lv_obj_create(scr);
    lv_obj_clear_flag(badgeContainer, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(badgeContainer, lv_color_hex(KIMO_HEX_BG), 0);
    lv_obj_set_style_border_color(badgeContainer, lv_color_hex(KIMO_HEX_LIME), 0);
    lv_obj_set_style_border_width(badgeContainer, 1, 0);
    lv_obj_set_style_radius(badgeContainer, 4, 0);
    lv_obj_set_style_pad_all(badgeContainer, 2, 0);
    lv_obj_set_pos(badgeContainer, 4, 4);

    badgeLabel = lv_label_create(badgeContainer);
    lv_obj_set_style_text_color(badgeLabel, lv_color_hex(KIMO_HEX_LIME), 0);
    lv_obj_set_style_text_font(badgeLabel, &lv_font_montserrat_12, 0);
    lv_label_set_text(badgeLabel, "[01] REPOSO");

    // 4. Antenna (REPOSO)
    antenna = lv_obj_create(scr);
    lv_obj_clear_flag(antenna, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(antenna, lv_color_hex(KIMO_HEX_LIME), 0);
    lv_obj_set_style_border_width(antenna, 0, 0);
    lv_obj_set_style_radius(antenna, 4, 0);

    // 5. Audio Waves (ESCUCHA)
    for (int i = 0; i < 3; i++) {
      wavesL[i] = lv_obj_create(scr);
      wavesR[i] = lv_obj_create(scr);
      setupPillStyle(wavesL[i], KIMO_HEX_LIME);
      setupPillStyle(wavesR[i], KIMO_HEX_LIME);
    }

    // 6. Thinking Dots (PENSANDO)
    for (int i = 0; i < 3; i++) {
      thinkingDots[i] = lv_obj_create(scr);
      setupPillStyle(thinkingDots[i], KIMO_HEX_LIME);
      lv_obj_set_style_radius(thinkingDots[i], LV_RADIUS_CIRCLE, 0);
      lv_obj_set_size(thinkingDots[i], 8, 8);
    }

    // 7. Sunglasses (COOL)
    sunglassL = lv_obj_create(scr);
    sunglassR = lv_obj_create(scr);
    sunglassBridge = lv_obj_create(scr);
    setupSunglassStyle(sunglassL);
    setupSunglassStyle(sunglassR);
    setupPillStyle(sunglassBridge, KIMO_HEX_EYES);

    // 8. Sparkles (EXITO / SEGURO / RISA_OJOS)
    sparkle1 = lv_label_create(scr);
    sparkle2 = lv_label_create(scr);
    lv_obj_set_style_text_color(sparkle1, lv_color_hex(KIMO_HEX_LIME), 0);
    lv_obj_set_style_text_color(sparkle2, lv_color_hex(KIMO_HEX_LIME), 0);
    lv_label_set_text(sparkle1, "*");
    lv_label_set_text(sparkle2, "*");

    // 9. Sweatdrops
    sweatL = lv_obj_create(scr);
    sweatR = lv_obj_create(scr);
    setupPillStyle(sweatL, KIMO_HEX_LIME);
    setupPillStyle(sweatR, KIMO_HEX_LIME);
    lv_obj_set_size(sweatL, 8, 12);
    lv_obj_set_size(sweatR, 8, 12);

    // 10. Blush Cheeks
    blushL = lv_label_create(scr);
    blushR = lv_label_create(scr);
    lv_obj_set_style_text_color(blushL, lv_color_hex(KIMO_HEX_PINK), 0);
    lv_obj_set_style_text_color(blushR, lv_color_hex(KIMO_HEX_PINK), 0);
    lv_label_set_text(blushL, "///");
    lv_label_set_text(blushR, "///");

    // 11. Anger Mark
    angerMark = lv_label_create(scr);
    lv_obj_set_style_text_color(angerMark, lv_color_hex(KIMO_HEX_RED), 0);
    lv_label_set_text(angerMark, "##");

    // 12. Particles (Hearts & Tears)
    for (int i = 0; i < 4; i++) {
      hearts[i].obj = lv_label_create(scr);
      lv_obj_set_style_text_color(hearts[i].obj, lv_color_hex(KIMO_HEX_PINK), 0);
      lv_label_set_text(hearts[i].obj, "+");
      hearts[i].active = false;
      lv_obj_add_flag(hearts[i].obj, LV_OBJ_FLAG_HIDDEN);

      tears[i].obj = lv_obj_create(scr);
      setupPillStyle(tears[i].obj, KIMO_HEX_CYAN);
      lv_obj_set_size(tears[i].obj, 4, 14);
      tears[i].active = false;
      lv_obj_add_flag(tears[i].obj, LV_OBJ_FLAG_HIDDEN);
    }

    // Initialize Variables
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

    setState(STATE_REPOSO);
    nextBlinkTime = millis() + 3000;
  }

  void setupEyeStyle(lv_obj_t* obj) {
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(obj, lv_color_hex(KIMO_HEX_EYES), 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(obj, 0, 0);
    lv_obj_set_style_pad_all(obj, 0, 0);
  }

  void setupArchStyle(lv_obj_t* arc) {
    lv_obj_clear_flag(arc, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE);
    lv_arc_set_bg_angles(arc, 0, 0);
    lv_arc_set_angles(arc, 200, 340);
    lv_obj_set_style_arc_color(arc, lv_color_hex(KIMO_HEX_EYES), LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(arc, 14, LV_PART_INDICATOR);
    lv_obj_set_style_arc_rounded(arc, true, LV_PART_INDICATOR);
    lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
    lv_obj_add_flag(arc, LV_OBJ_FLAG_HIDDEN);
  }

  void setupPillStyle(lv_obj_t* obj, uint32_t color) {
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(obj, lv_color_hex(color), 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(obj, 0, 0);
    lv_obj_set_style_radius(obj, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_pad_all(obj, 0, 0);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
  }

  void setupSunglassStyle(lv_obj_t* obj) {
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(obj, lv_color_hex(KIMO_HEX_COOL_DARK), 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(obj, lv_color_hex(KIMO_HEX_EYES), 0);
    lv_obj_set_style_border_width(obj, 2, 0);
    lv_obj_set_style_radius(obj, 18, 0);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
  }

  void setState(KimoState st) {
    currentState = st;
    configureTargetsForState(currentState);

    char str[32];
    snprintf(str, sizeof(str), "[%02d] %s", (int)currentState + 1, KIMO_NAMES[currentState]);
    lv_label_set_text(badgeLabel, str);

    targetGazeX = 0.0f;
    targetGazeY = 0.0f;
    blinkScale = 1.0f;
    nextBlinkTime = millis() + 2500;
  }

  void nextState() {
    int next = ((int)currentState + 1) % STATE_COUNT;
    setState((KimoState)next);
  }

  void prevState() {
    int prev = ((int)currentState - 1 + STATE_COUNT) % STATE_COUNT;
    setState((KimoState)prev);
  }

  void resetGaze() {
    targetGazeX = 0.0f;
    targetGazeY = 0.0f;
  }

  void lookAt(int16_t touchX, int16_t touchY) {
    lastInteractionTime = millis();
    float dx = (touchX - (screenW / 2.0f)) / (screenW / 2.0f);
    float dy = (touchY - (screenH / 2.0f)) / (screenH / 2.0f);
    targetGazeX = constrain(dx * 10.0f, -10.0f, 10.0f);
    targetGazeY = constrain(dy * 5.0f, -5.0f, 5.0f);
  }

  void configureTargetsForState(KimoState st) {
    targetAngle = 0.0f;
    targetArch = 0.0f;

    switch (st) {
      case STATE_REPOSO:
        targetLW = 96.0f; targetLH = 68.0f; targetLR = 34.0f;
        targetRW = 96.0f; targetRH = 68.0f; targetRR = 34.0f;
        break;
      case STATE_ESCUCHA:
        targetLW = 84.0f; targetLH = 98.0f; targetLR = 42.0f;
        targetRW = 84.0f; targetRH = 98.0f; targetRR = 42.0f;
        break;
      case STATE_PENSANDO:
        targetLW = 82.0f; targetLH = 90.0f; targetLR = 41.0f;
        targetRW = 82.0f; targetRH = 90.0f; targetRR = 41.0f;
        break;
      case STATE_EXITO:
        targetLW = 86.0f; targetLH = 98.0f; targetLR = 43.0f;
        targetRW = 86.0f; targetRH = 98.0f; targetRR = 43.0f;
        break;
      case STATE_CARINO:
        targetLW = 88.0f; targetLH = 54.0f; targetLR = 27.0f;
        targetRW = 88.0f; targetRH = 54.0f; targetRR = 27.0f;
        targetArch = 1.0f;
        break;
      case STATE_COOL:
        targetLW = 90.0f; targetLH = 66.0f; targetLR = 22.0f;
        targetRW = 90.0f; targetRH = 66.0f; targetRR = 22.0f;
        break;
      case STATE_FELIZ:
        targetLW = 86.0f; targetLH = 100.0f; targetLR = 43.0f;
        targetRW = 86.0f; targetRH = 100.0f; targetRR = 43.0f;
        break;
      case STATE_RISA:
      case STATE_RISA_OJOS:
      case STATE_RISA_APENADA:
        targetLW = 88.0f; targetLH = 54.0f; targetLR = 27.0f;
        targetRW = 88.0f; targetRH = 54.0f; targetRR = 27.0f;
        targetArch = 1.0f;
        break;
      case STATE_COMPLICIDAD:
        targetLW = 84.0f; targetLH = 98.0f; targetLR = 42.0f;
        targetRW = 82.0f; targetRH = 16.0f; targetRR = 8.0f;
        break;
      case STATE_SEGURO:
        targetLW = 88.0f; targetLH = 82.0f; targetLR = 32.0f;
        targetRW = 88.0f; targetRH = 82.0f; targetRR = 32.0f;
        targetAngle = 8.0f;
        break;
      case STATE_OOPS:
      case STATE_VERGUENZA:
        targetLW = 82.0f; targetLH = 88.0f; targetLR = 40.0f;
        targetRW = 82.0f; targetRH = 88.0f; targetRR = 40.0f;
        break;
      case STATE_SORPRESA:
        targetLW = 90.0f; targetLH = 102.0f; targetLR = 45.0f;
        targetRW = 90.0f; targetRH = 102.0f; targetRR = 45.0f;
        break;
      case STATE_SHOCK:
        targetLW = 76.0f; targetLH = 100.0f; targetLR = 38.0f;
        targetRW = 76.0f; targetRH = 100.0f; targetRR = 38.0f;
        break;
      case STATE_ENFADO:
        targetLW = 86.0f; targetLH = 68.0f; targetLR = 22.0f;
        targetRW = 86.0f; targetRH = 68.0f; targetRR = 22.0f;
        targetAngle = -16.0f;
        break;
      case STATE_TRISTEZA:
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

    // Auto-cycle through emotions
    if (autoPlay && now > nextAutoCycle) {
      nextState();
      nextAutoCycle = now + 4500;
    }

    // Micro-blinking
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

    // Idle gaze wandering
    if (now - lastInteractionTime > 3500) {
      if (now > nextIdleGlance) {
        if (currentState == STATE_PENSANDO) {
          targetGazeX = 8.0f;
          targetGazeY = -4.0f;
        } else if (currentState == STATE_OOPS) {
          targetGazeX = 5.0f;
          targetGazeY = 4.0f;
        } else if (currentState == STATE_VERGUENZA) {
          targetGazeX = 0.0f;
          targetGazeY = 3.0f;
        } else {
          targetGazeX = ((esp_random() % 17) - 8);
          targetGazeY = ((esp_random() % 7) - 3);
        }
        nextIdleGlance = now + 2500 + (esp_random() % 2500);
      }
    }

    gazeX += (targetGazeX - gazeX) * 0.14f;
    gazeY += (targetGazeY - gazeY) * 0.14f;

    float breathY = sinf(animTime * 0.8f) * 1.0f;
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

    float destLX = eyeBaseLeftX + gazeX + jitterX;
    float destLY = eyeBaseY + gazeY + breathY + bounceY + jitterY;
    float destRX = eyeBaseRightX + gazeX + jitterX;
    float destRY = eyeBaseY + gazeY + breathY + bounceY + jitterY;

    eyeLX += (destLX - eyeLX) * 0.18f;
    eyeLY += (destLY - eyeLY) * 0.18f;
    eyeRX += (destRX - eyeRX) * 0.18f;
    eyeRY += (destRY - eyeRY) * 0.18f;

    eyeLW += (targetLW - eyeLW) * 0.20f;
    eyeLH += (targetLH - eyeLH) * 0.20f;
    eyeLR += (targetLR - eyeLR) * 0.20f;

    eyeRW += (targetRW - eyeRW) * 0.20f;
    eyeRH += (targetRH - eyeRH) * 0.20f;
    eyeRR += (targetRR - eyeRR) * 0.20f;

    eyeAngle += (targetAngle - eyeAngle) * 0.20f;
    isArch += (targetArch - isArch) * 0.20f;

    // Apply values to LVGL Widgets
    renderLVGL();
  }

  void renderLVGL() {
    float curLH = max(4.0f, eyeLH * blinkScale);
    float curRH = max(4.0f, eyeRH * blinkScale);

    if (isArch > 0.5f) {
      // Show Arch, hide capsule
      lv_obj_add_flag(eyeL, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(eyeR, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(archL, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(archR, LV_OBJ_FLAG_HIDDEN);

      lv_obj_set_size(archL, (int16_t)eyeLW, (int16_t)eyeLW);
      lv_obj_set_size(archR, (int16_t)eyeRW, (int16_t)eyeRW);
      lv_obj_set_pos(archL, (int16_t)(eyeLX - eyeLW / 2), (int16_t)(eyeLY - eyeLW / 4));
      lv_obj_set_pos(archR, (int16_t)(eyeRX - eyeRW / 2), (int16_t)(eyeRY - eyeRW / 4));
    } else {
      // Show Capsule, hide Arch
      lv_obj_clear_flag(eyeL, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(eyeR, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(archL, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(archR, LV_OBJ_FLAG_HIDDEN);

      lv_obj_set_size(eyeL, (int16_t)eyeLW, (int16_t)curLH);
      lv_obj_set_size(eyeR, (int16_t)eyeRW, (int16_t)curRH);
      lv_obj_set_pos(eyeL, (int16_t)(eyeLX - eyeLW / 2), (int16_t)(eyeLY - curLH / 2));
      lv_obj_set_pos(eyeR, (int16_t)(eyeRX - eyeRW / 2), (int16_t)(eyeRY - curRH / 2));

      int16_t radL = min((int16_t)eyeLR, (int16_t)(curLH / 2));
      int16_t radR = min((int16_t)eyeRR, (int16_t)(curRH / 2));
      lv_obj_set_style_radius(eyeL, radL, 0);
      lv_obj_set_style_radius(eyeR, radR, 0);

      // Angled furrows for Enfado / Tristeza using LVGL transform
      lv_obj_set_style_transform_angle(eyeL, (int16_t)(eyeAngle * 10), 0);
      lv_obj_set_style_transform_angle(eyeR, (int16_t)(-eyeAngle * 10), 0);
    }

    // Overlays Management
    // 1. REPOSO Antenna
    if (currentState == STATE_REPOSO) {
      lv_obj_clear_flag(antenna, LV_OBJ_FLAG_HIDDEN);
      float pulse = 1.0f + 0.1f * sinf(animTime * 2.0f);
      lv_obj_set_size(antenna, (int16_t)(26 * pulse), 8);
      lv_obj_set_pos(antenna, (int16_t)(eyeRX + 26), (int16_t)(eyeRY - 44));
    } else {
      lv_obj_add_flag(antenna, LV_OBJ_FLAG_HIDDEN);
    }

    // 2. ESCUCHA Audio waves
    if (currentState == STATE_ESCUCHA) {
      for (int i = 0; i < 3; i++) {
        lv_obj_clear_flag(wavesL[i], LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(wavesR[i], LV_OBJ_FLAG_HIDDEN);
        int16_t waveH = (int16_t)((sinf(animTime * 4.0f + i * 1.0f) * 12.0f) + 16.0f);
        lv_obj_set_size(wavesL[i], 4, waveH);
        lv_obj_set_size(wavesR[i], 4, waveH);
        lv_obj_set_pos(wavesL[i], 16 + (i * 9), (screenH / 2) - (waveH / 2));
        lv_obj_set_pos(wavesR[i], (screenW - 20) - (i * 9), (screenH / 2) - (waveH / 2));
      }
    } else {
      for (int i = 0; i < 3; i++) {
        lv_obj_add_flag(wavesL[i], LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(wavesR[i], LV_OBJ_FLAG_HIDDEN);
      }
    }

    // 3. PENSANDO Thinking Dots
    if (currentState == STATE_PENSANDO) {
      for (int i = 0; i < 3; i++) {
        lv_obj_clear_flag(thinkingDots[i], LV_OBJ_FLAG_HIDDEN);
        float phase = animTime * 3.0f + i * 1.1f;
        int16_t dotX = (int16_t)(eyeRX + 16 + (i * 12));
        int16_t dotY = (int16_t)(eyeRY - 36 + (sinf(phase) * 4.0f));
        lv_obj_set_pos(thinkingDots[i], dotX, dotY);
      }
    } else {
      for (int i = 0; i < 3; i++) lv_obj_add_flag(thinkingDots[i], LV_OBJ_FLAG_HIDDEN);
    }

    // 4. COOL Sunglasses
    if (currentState == STATE_COOL) {
      lv_obj_clear_flag(sunglassL, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(sunglassR, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(sunglassBridge, LV_OBJ_FLAG_HIDDEN);

      int16_t gw = 90, gh = 66;
      lv_obj_set_size(sunglassL, gw, gh);
      lv_obj_set_size(sunglassR, gw, gh);
      lv_obj_set_pos(sunglassL, (int16_t)(eyeLX - gw / 2), (int16_t)(eyeLY - gh / 2));
      lv_obj_set_pos(sunglassR, (int16_t)(eyeRX - gw / 2), (int16_t)(eyeRY - gh / 2));

      int16_t bX = (int16_t)(eyeLX + gw / 2 - 2);
      int16_t bW = (int16_t)((eyeRX - gw / 2) - (eyeLX + gw / 2) + 4);
      if (bW > 0) {
        lv_obj_set_size(sunglassBridge, bW, 8);
        lv_obj_set_pos(sunglassBridge, bX, (int16_t)(eyeLY - 4));
      }
    } else {
      lv_obj_add_flag(sunglassL, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(sunglassR, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(sunglassBridge, LV_OBJ_FLAG_HIDDEN);
    }

    // 5. Sparkles (EXITO / SEGURO / RISA_OJOS)
    if (currentState == STATE_EXITO || currentState == STATE_SEGURO || currentState == STATE_RISA_OJOS) {
      lv_obj_clear_flag(sparkle1, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(sparkle2, LV_OBJ_FLAG_HIDDEN);
      float spY = sinf(animTime * 3.0f) * 4.0f;
      lv_obj_set_pos(sparkle1, 36, (int16_t)(40 + spY));
      lv_obj_set_pos(sparkle2, 280, (int16_t)(40 - spY));
    } else {
      lv_obj_add_flag(sparkle1, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(sparkle2, LV_OBJ_FLAG_HIDDEN);
    }

    // 6. Sweatdrops (OOPS, RISA_APENADA, SHOCK)
    if (currentState == STATE_OOPS || currentState == STATE_RISA_APENADA || currentState == STATE_SHOCK) {
      lv_obj_clear_flag(sweatR, LV_OBJ_FLAG_HIDDEN);
      lv_obj_set_pos(sweatR, (int16_t)(eyeRX + 34), (int16_t)(eyeRY - 12));
      if (currentState == STATE_SHOCK) {
        lv_obj_clear_flag(sweatL, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(sweatL, (int16_t)(eyeLX - 38), (int16_t)(eyeLY - 12));
      } else {
        lv_obj_add_flag(sweatL, LV_OBJ_FLAG_HIDDEN);
      }
    } else {
      lv_obj_add_flag(sweatL, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(sweatR, LV_OBJ_FLAG_HIDDEN);
    }

    // 7. Blush (CARINO, VERGUENZA, RISA_APENADA)
    if (currentState == STATE_CARINO || currentState == STATE_VERGUENZA || currentState == STATE_RISA_APENADA) {
      lv_obj_clear_flag(blushL, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(blushR, LV_OBJ_FLAG_HIDDEN);
      lv_obj_set_pos(blushL, (int16_t)(eyeLX - 8), (int16_t)(eyeLY + 30));
      lv_obj_set_pos(blushR, (int16_t)(eyeRX + 6), (int16_t)(eyeRY + 30));
    } else {
      lv_obj_add_flag(blushL, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(blushR, LV_OBJ_FLAG_HIDDEN);
    }

    // 8. Anger Mark (ENFADO)
    if (currentState == STATE_ENFADO) {
      lv_obj_clear_flag(angerMark, LV_OBJ_FLAG_HIDDEN);
      lv_obj_set_pos(angerMark, 276, 26);
    } else {
      lv_obj_add_flag(angerMark, LV_OBJ_FLAG_HIDDEN);
    }

    // 9. Floating Hearts for CARINO
    if (currentState == STATE_CARINO) {
      for (int i = 0; i < 4; i++) {
        if (!hearts[i].active && (esp_random() % 14 == 0)) {
          hearts[i].active = true;
          hearts[i].x = 120 + (esp_random() % 80);
          hearts[i].y = 145;
          hearts[i].speedY = 1.2f + (esp_random() % 10) / 10.0f;
          hearts[i].sway = (esp_random() % 100) / 10.0f;
          lv_obj_clear_flag(hearts[i].obj, LV_OBJ_FLAG_HIDDEN);
        }
        if (hearts[i].active) {
          hearts[i].y -= hearts[i].speedY;
          hearts[i].x += sinf(animTime * 2.0f + hearts[i].sway) * 0.6f;
          lv_obj_set_pos(hearts[i].obj, (int16_t)hearts[i].x, (int16_t)hearts[i].y);
          if (hearts[i].y < 15) {
            hearts[i].active = false;
            lv_obj_add_flag(hearts[i].obj, LV_OBJ_FLAG_HIDDEN);
          }
        }
      }
    } else {
      for (int i = 0; i < 4; i++) {
        hearts[i].active = false;
        lv_obj_add_flag(hearts[i].obj, LV_OBJ_FLAG_HIDDEN);
      }
    }

    // 10. Cascading Tears for TRISTEZA
    if (currentState == STATE_TRISTEZA) {
      for (int i = 0; i < 4; i++) {
        if (!tears[i].active && (esp_random() % 10 == 0)) {
          tears[i].active = true;
          bool isLeft = (i % 2 == 0);
          tears[i].x = isLeft ? (eyeLX - 24) : (eyeRX + 20);
          tears[i].y = eyeLY + 20;
          tears[i].speedY = 2.4f + (esp_random() % 20) / 10.0f;
          lv_obj_clear_flag(tears[i].obj, LV_OBJ_FLAG_HIDDEN);
        }
        if (tears[i].active) {
          tears[i].y += tears[i].speedY;
          lv_obj_set_pos(tears[i].obj, (int16_t)tears[i].x, (int16_t)tears[i].y);
          if (tears[i].y > screenH - 10) {
            tears[i].active = false;
            lv_obj_add_flag(tears[i].obj, LV_OBJ_FLAG_HIDDEN);
          }
        }
      }
    } else {
      for (int i = 0; i < 4; i++) {
        tears[i].active = false;
        lv_obj_add_flag(tears[i].obj, LV_OBJ_FLAG_HIDDEN);
      }
    }
  }
};
