# Kimo ESP32-S3 - LilyGO T-Display-S3 Touch (Rama `lvgl`)

Esta rama (`lvgl`) contiene la migración completa de **Kimo** al framework **[LVGL v8](https://lvgl.io/)** (Light and Versatile Graphics Library), ejecutándose sobre la placa **LilyGO T-Display-S3 Touch** (ESP32-S3 con pantalla ST7789 paralela de 8 bits y táctil CST816).

![LVGL](https://img.shields.io/badge/UI-LVGL%208.4.0-3792cb.svg)
![PlatformIO](https://img.shields.io/badge/PlatformIO-ESP32--S3-orange.svg)
![License](https://img.shields.io/badge/License-MIT-blue.svg)

---

## 🏗️ Arquitectura de la Migración a LVGL

1. **Gestor de Pantalla (Display Driver)**:
   - LovyanGFX actúa como capa física DMA ultrarrápida para el bus paralelo i8080 de 8 bits.
   - El callback `my_disp_flush` transfiere los búferes de dibujo de LVGL (`lv_disp_draw_buf_t`, 320x40 doble búfer) directamente al controlador ST7789 por DMA.

2. **Gestor de Entrada Táctil (Input Device Driver)**:
   - El chip CST816 está registrado como dispositivo de entrada tipo puntero (`LV_INDEV_TYPE_POINTER`).
   - Los gestos de deslizamiento (*swipes*), pulsación mantenida (*long press*) y arrastre de mirada se gestionan mediante eventos nativos de LVGL (`LV_EVENT_GESTURE`, `LV_EVENT_PRESSING`, `LV_EVENT_RELEASED`, `LV_EVENT_LONG_PRESSED`).

3. **Renderizado de Ojos y Emociones como Widgets Nativos**:
   - Ojos representados como objetos LVGL (`lv_obj_t`) con estilos dinámicos de radio (`lv_obj_set_style_radius`), escala y transformaciones angulares (`lv_obj_set_style_transform_angle`) para ceños fruncidos (Enfado/Tristeza).
   - Ojos arqueados felices creados con arcos vectoriales nativos (`lv_arc_t`).
   - Todos los accesorios (gafas de sol oscuras, antena neón, ondas de audio, destellos, gotas de sudor, lágrimas y corazones flotantes) son objetos y etiquetas LVGL manejados por un temporizador interno de 60 Hz (`lv_timer_create`).

4. **Configuración de memoria (`lv_conf.h`)**:
   - `LV_COLOR_DEPTH 16` (RGB565).
   - Heap dedicado para LVGL configurado en 48 KB.
   - `LV_TICK_CUSTOM 1` vinculado directamente a `millis()`.

---

## 🚀 Compilación y Flasheo

```bash
# Cambiar a la rama lvgl
git checkout lvgl

# Compilar y subir por USB
pio run -t upload
```
