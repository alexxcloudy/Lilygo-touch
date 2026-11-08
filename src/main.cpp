#include <Arduino.h>
#include "LGFX_Config.hpp"
#include "CST816Touch.hpp"
#include "KimoLVGL.hpp"

// Physical buttons on T-Display-S3
#define BTN_BOOT 0
#define BTN_USER 14

LGFX lcd;
CST816Touch touch;
KimoLVGL kimo;

// LVGL Draw Buffer (320 x 40 x 16-bit)
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf_1[320 * 40];
static lv_color_t buf_2[320 * 40];

uint32_t lastBtn14Press = 0;
uint32_t lastBtn0Press = 0;

// Display Flush Callback for LVGL
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  lcd.pushImageDMA(area->x1, area->y1, w, h, (lgfx::rgb565_t *)&color_p->full);
  lv_disp_flush_ready(disp);
}

// Touch Input Read Callback for LVGL
void my_touch_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data) {
  bool active = touch.update(1);
  if (active) {
    data->state = LV_INDEV_STATE_PR;
    data->point.x = touch.x;
    data->point.y = touch.y;
  } else {
    data->state = LV_INDEV_STATE_REL;
  }
}

// Screen Touch and Gesture Handler
static void screen_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);

  if (code == LV_EVENT_GESTURE) {
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
    if (dir == LV_DIR_LEFT || dir == LV_DIR_TOP) {
      kimo.nextState();
      kimo.resetGaze();
      Serial.printf("LVGL Gesture Next -> %s\n", KIMO_NAMES[kimo.currentState]);
    } else if (dir == LV_DIR_RIGHT || dir == LV_DIR_BOTTOM) {
      kimo.prevState();
      kimo.resetGaze();
      Serial.printf("LVGL Gesture Prev -> %s\n", KIMO_NAMES[kimo.currentState]);
    }
  } else if (code == LV_EVENT_PRESSING) {
    lv_point_t p;
    lv_indev_get_point(lv_indev_get_act(), &p);
    kimo.lookAt(p.x, p.y);
  } else if (code == LV_EVENT_RELEASED) {
    kimo.resetGaze();
  } else if (code == LV_EVENT_LONG_PRESSED) {
    kimo.setState(STATE_REPOSO);
    kimo.resetGaze();
    Serial.println("LVGL Long Press -> Reset to REPOSO");
  } else if (code == LV_EVENT_SHORT_CLICKED) {
    // Single tap can toggle quick wink or micro expression
  }
}

// 60 Hz Animation Timer
static void anim_timer_cb(lv_timer_t *timer) {
  kimo.update();
}

void handleButtons() {
  uint32_t now = millis();

  // Button 14 -> Next emotion
  if (digitalRead(BTN_USER) == LOW) {
    if (now - lastBtn14Press > 260) {
      lastBtn14Press = now;
      kimo.nextState();
      Serial.printf("Button 14 -> Emotion: %s\n", KIMO_NAMES[kimo.currentState]);
    }
  }

  // Button 0 -> Toggle Auto-Cycle / Showcase mode
  if (digitalRead(BTN_BOOT) == LOW) {
    if (now - lastBtn0Press > 320) {
      lastBtn0Press = now;
      kimo.autoPlay = !kimo.autoPlay;
      kimo.nextAutoCycle = now + 4000;
      Serial.printf("Boot Button -> AutoPlay: %s\n", kimo.autoPlay ? "ENABLED" : "DISABLED");
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("======================================");
  Serial.println("     KIMO ESP32-S3 COMPANION (LVGL)   ");
  Serial.println("   LilyGO T-Display-S3 Touch (1.9\")   ");
  Serial.println("======================================");

  // Initialize Hardware Buttons
  pinMode(BTN_BOOT, INPUT_PULLUP);
  pinMode(BTN_USER, INPUT_PULLUP);

  // Initialize ST7789 Parallel Display
  lcd.init();
  lcd.setRotation(1); // Landscape 320x170
  lcd.setBrightness(230);
  lcd.fillScreen(0x0000);

  // Initialize CST816 Touch Screen
  bool touchOk = touch.begin();
  Serial.printf("CST816 Touch Status: %s\n", touchOk ? "ONLINE" : "OFFLINE");

  // Initialize LVGL
  lv_init();

  // Initialize Display Driver in LVGL
  lv_disp_draw_buf_init(&draw_buf, buf_1, buf_2, 320 * 40);

  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = 320;
  disp_drv.ver_res = 170;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  // Initialize Touch Driver in LVGL
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touch_read;
  lv_indev_t *touch_indev = lv_indev_drv_register(&indev_drv);

  // Add gesture detection to active screen
  lv_obj_t *scr = lv_scr_act();
  lv_obj_add_event_cb(scr, screen_event_cb, LV_EVENT_ALL, NULL);

  // Initialize Kimo LVGL Widgets
  kimo.init();

  // Create 60 FPS animation timer
  lv_timer_create(anim_timer_cb, 16, NULL);

  Serial.println("Kimo LVGL ready! Native widgets, animations and touch gestures active.");
}

void loop() {
  handleButtons();
  lv_timer_handler();
  delay(5);
}
