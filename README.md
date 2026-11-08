# Kimo ESP32-S3 - LilyGO T-Display-S3 Touch (1.9")

Avatar emocional interactivo y compañero robótico diseñado para la placa **LilyGO T-Display-S3 Touch** (ESP32-S3 con pantalla ST7789 paralela de 8 bits y panel táctil capacitivo CST816).

![Kimo Display](https://img.shields.io/badge/PlatformIO-ESP32--S3-orange.svg)
![License](https://img.shields.io/badge/License-MIT-blue.svg)
![FPS](https://img.shields.io/badge/FPS-60-brightgreen.svg)

---

## ✨ Características

- **18 Expresiones Emocionales Completas**:
  1. `[01] REPOSO`: Presencia tranquila, micro-parpadeo y antena neón superior.
  2. `[02] ESCUCHA`: Ojos atentos con ondas de audio laterales animadas.
  3. `[03] PENSANDO`: Mirada dirigida hacia arriba con puntos de procesamiento orbitales.
  4. `[04] ÉXITO`: Ojos alegres con destellos en estrella rebotantes.
  5. `[05] CARIÑO`: Arcos felices `(^ ^)` con corazones flotantes y mejillas sonrojadas.
  6. `[06] COOL`: Gafas de sol oscuras con reflejo especular en movimiento.
  7. `[07] FELIZ`: Ojos grandes y redondos amigables.
  8. `[08] RISA`: Arcos en `(^ ^)` con rebote alegre.
  9. `[09] RISA DE OJOS`: Arcos cerrados con destello central y gotas de risa.
  10. `[10] COMPLICIDAD`: Guiño cómplice (ojo izquierdo abierto, derecho cerrado en línea).
  11. `[11] SEGURO`: Ojos almendrados confiados con destellos dobles.
  12. `[12] OOPS`: Mirada hacia abajo con gota de sudor nerviosa.
  13. `[13] VERGÜENZA`: Mirada tímida con líneas de sonrojo `(///)`.
  14. `[14] RISA APENADA`: Risa arqueada nerviosa con gota y sonrojo.
  15. `[15] SORPRESA`: Ojos circulares gigantes bien abiertos `(O O)`.
  16. `[16] SHOCK`: Ojos alargados con micro-temblor y gotas dobles.
  17. `[17] ENFADO`: Ceño fruncido angular con símbolo de vena enfadada anime `(💢)`.
  18. `[18] TRISTEZA`: Ojos caídos hacia abajo con cascada de lágrimas de neón.

- **Rendimiento a 60 FPS sin tirones**:
  - Motor gráfico de doble búfer (*LGFX_Sprite*) con renderizado procedural vectorial directo.
  - Cero parpadeos (*flicker-free*) y sin desincronizaciones en el bus paralelo.
  - Micro-parpadeos fisiológicos (*blinking*) periódicos automáticos.

- **Control Táctil Capacitivo (CST816)**:
  - **Seguimiento de mirada**: Los ojos siguen con suavidad la posición del dedo en pantalla y regresan al centro al soltar.
  - **Deslizar (Swipe)**: Deslizar a la izquierda o derecha (o arriba/abajo) cambia a la emoción siguiente/anterior con protección antirebote de disparo único.
  - **Doble toque**: Activa / desactiva el modo carrusel automático (*AutoPlay*).
  - **Pulsación larga**: Regresa directamente al estado de `REPOSO`.

- **Botones Físicos**:
  - **Botón USER (GPIO 14)**: Siguiente emoción.
  - **Botón BOOT (GPIO 0)**: Alternar modo automático (*AutoPlay*).

- **Simulador Web Incluido**:
  - Archivo `kimo_simulator.html` para previsualizar y probar las 18 expresiones en cualquier navegador web con seguimiento de ratón.

---

## 🛠️ Hardware y Pinout

- **Placa**: LilyGO T-Display-S3 Touch (1.9" IPS ST7789, 320x170 apaisado / horizontal).
- **SoC**: ESP32-S3 Dual-Core Xtensa LX7, 16MB Flash, 8MB PSRAM.
- **Bus LCD Paralelo 8 bits (i8080)**:
  - `WR`: GPIO 8
  - `RD`: GPIO 9
  - `RS / DC`: GPIO 7
  - `CS`: GPIO 6
  - `RST`: GPIO 5
  - `PWR`: GPIO 15
  - `BL (Backlight)`: GPIO 38
  - `D0-D7`: GPIO 39, 40, 41, 42, 45, 46, 47, 48
- **Táctil Capacitivo I2C (CST816)**:
  - `SDA`: GPIO 18
  - `SCL`: GPIO 17
  - `RST`: GPIO 21
  - `INT`: GPIO 16

---

## 🚀 Instalación y Compilación con PlatformIO

1. Clona el repositorio:
   ```bash
   git clone https://github.com/alexxcloudy/Lilygo-touch.git
   cd Lilygo-touch
   ```

2. Abre la carpeta en **VS Code** con la extensión **PlatformIO IDE** (o usa PlatformIO CLI).

3. Conecta la placa LilyGO por USB-C y sube el firmware:
   ```bash
   pio run -t upload
   ```

---

## 📄 Licencia

Código bajo licencia MIT.
