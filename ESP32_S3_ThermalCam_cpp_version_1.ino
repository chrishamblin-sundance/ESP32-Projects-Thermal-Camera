#include <Adafruit_NeoPixel.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <Wire.h>
#include <SparkFun_GridEYE_Arduino_Library.h>

/* ======================================================================
   COMPILE-TIME SETTINGS (Modern C++ approach instead of #define)
   ====================================================================== */
constexpr uint8_t NEO_PIXEL_PIN     = 21;
constexpr uint8_t NEO_NUMPIXELS     = 64;
constexpr uint8_t DEFAULT_BRIGHT    = 128;  // Startup & Max LED Brightness
constexpr unsigned long FRAME_DELAY = 30;   // Target ms between frames
constexpr uint8_t INTERP_SIZE       = 32;   // Upscaled grid resolution

// --- Compile-Time UI Centering Math ---
constexpr uint8_t BOX_W    = 312 / INTERP_SIZE;
constexpr uint8_t BOX_H    = 240 / INTERP_SIZE;
constexpr int OFFSET_X     = (320 - (BOX_W * INTERP_SIZE)) / 2;
constexpr int OFFSET_Y     = (248 - (BOX_H * INTERP_SIZE)) / 2;

/* --- Touchscreen Pins --- */
constexpr uint8_t XPT2046_IRQ       = 9;
constexpr uint8_t XPT2046_MOSI      = 3;
constexpr uint8_t XPT2046_MISO      = 46;
constexpr uint8_t XPT2046_CLK       = 18;
constexpr uint8_t XPT2046_CS        = 8;

/* --- Color Definitions --- */
constexpr uint16_t COLOR_BLACK = 0x0000;
constexpr uint16_t COLOR_WHITE = 0xFFFF;
constexpr uint16_t COLOR_RED   = 0xF800;
constexpr uint16_t COLOR_GREEN = 0x07E0;
constexpr uint16_t COLOR_BLUE  = 0x001F;

// ======================================================================
// SUNDANCE COMPANY LOGO BITMAP
// ======================================================================
#include "logo.h"

// 256-color Thermal Palette
const uint16_t camColors[] = {
  0x480F, 0x400F, 0x400F, 0x400F, 0x4010, 0x3810, 0x3810, 0x3810, 0x3810, 0x3010, 0x3010,
  0x3010, 0x2810, 0x2810, 0x2810, 0x2810, 0x2010, 0x2010, 0x2010, 0x1810, 0x1810,
  0x1811, 0x1811, 0x1011, 0x1011, 0x1011, 0x0811, 0x0811, 0x0811, 0x0011, 0x0011,
  0x0011, 0x0011, 0x0011, 0x0031, 0x0031, 0x0051, 0x0072, 0x0072, 0x0092, 0x00B2,
  0x00B2, 0x00D2, 0x00F2, 0x00F2, 0x0112, 0x0132, 0x0152, 0x0152, 0x0172, 0x0192,
  0x0192, 0x01B2, 0x01D2, 0x01F3, 0x01F3, 0x0213, 0x0233, 0x0253, 0x0253, 0x0273,
  0x0293, 0x02B3, 0x02D3, 0x02D3, 0x02F3, 0x0313, 0x0333, 0x0333, 0x0353, 0x0373,
  0x0394, 0x03B4, 0x03D4, 0x03D4, 0x03F4, 0x0414, 0x0434, 0x0454, 0x0474, 0x0474,
  0x0494, 0x04B4, 0x04D4, 0x04F4, 0x0514, 0x0534, 0x0534, 0x0554, 0x0554, 0x0574,
  0x0574, 0x0573, 0x0573, 0x0573, 0x0572, 0x0572, 0x0572, 0x0571, 0x0591, 0x0591,
  0x0590, 0x0590, 0x058F, 0x058F, 0x058F, 0x058E, 0x05AE, 0x05AE, 0x05AD, 0x05AD,
  0x05AD, 0x05AC, 0x05AC, 0x05AB, 0x05CB, 0x05CB, 0x05CA, 0x05CA, 0x05CA, 0x05C9,
  0x05C9, 0x05C8, 0x05E8, 0x05E8, 0x05E7, 0x05E7, 0x05E6, 0x05E6, 0x05E6, 0x05E5,
  0x05E5, 0x0604, 0x0604, 0x0604, 0x0603, 0x0603, 0x0602, 0x0602, 0x0601, 0x0621,
  0x0621, 0x0620, 0x0620, 0x0620, 0x0620, 0x0E20, 0x0E20, 0x0E40, 0x1640, 0x1640,
  0x1E40, 0x1E40, 0x2640, 0x2640, 0x2E40, 0x2E60, 0x3660, 0x3660, 0x3E60, 0x3E60,
  0x3E60, 0x4660, 0x4660, 0x4E60, 0x4E80, 0x5680, 0x5680, 0x5E80, 0x5E80, 0x6680,
  0x6680, 0x6E80, 0x6EA0, 0x76A0, 0x76A0, 0x7EA0, 0x7EA0, 0x86A0, 0x86A0, 0x8EA0,
  0x8EC0, 0x96C0, 0x96C0, 0x9EC0, 0x9EC0, 0xA6C0, 0xAEC0, 0xAEC0, 0xB6E0, 0xB6E0,
  0xBEE0, 0xBEE0, 0xC6E0, 0xC6E0, 0xCEE0, 0xCEE0, 0xD6E0, 0xD700, 0xDF00, 0xDEE0,
  0xDEC0, 0xDEA0, 0xDE80, 0xDE80, 0xE660, 0xE640, 0xE620, 0xE600, 0xE5E0, 0xE5C0,
  0xE5A0, 0xE580, 0xE560, 0xE540, 0xE520, 0xE500, 0xE4E0, 0xE4C0, 0xE4A0, 0xE480,
  0xE460, 0xEC40, 0xEC20, 0xEC00, 0xEBE0, 0xEBC0, 0xEBA0, 0xEB80, 0xEB60, 0xEB40,
  0xEB20, 0xEB00, 0xEAE0, 0xEAC0, 0xEAA0, 0xEA80, 0xEA60, 0xEA40, 0xF220, 0xF200,
  0xF1E0, 0xF1C0, 0xF1A0, 0xF180, 0xF160, 0xF140, 0xF000, 0xF0E0, 0xF0C0, 0xF0A0,
  0xF080, 0xF060, 0xF040, 0xF020, 0xF800,
};

/* ======================================================================
   STATE ENCAPSULATION
   ====================================================================== */
struct SystemState {
  int sliderHue = 0;
  int sliderBright = DEFAULT_BRIGHT;
  int lastSliderHue = -1;
  int lastSliderBright = -1;

  float minTemp = 18.0f;
  float maxTemp = 30.0f;
  float frameMin = 100.0f;
  float frameMax = 0.0f;

  bool isPaused = false;
  bool isFrozen = false;
  bool showSliders = true;
  
  int iteration = 0;
  unsigned long previousMillis = 0;
  unsigned long lastThermalUpdate = 0;
  bool updateFlag = true;
};

SystemState state;

/* ======================================================================
   GLOBAL OBJECTS & BUFFERS
   ====================================================================== */
Adafruit_NeoPixel Neo_pixels(NEO_NUMPIXELS, NEO_PIXEL_PIN, NEO_GRB + NEO_KHZ800);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();
GridEYE grideye;

float pixels[64];
float dest_2d[INTERP_SIZE * INTERP_SIZE]; 
uint8_t sCurveLUT[256]; // The math optimization table

unsigned long lastHotspotUpdate = 0; // Tracks the 1-second timer
int hotX = 0;                        // Hotspot X coordinate
int hotY = 0;                        // Hotspot Y coordinate
float hotTemp = 0.0f;                // Hottest temperature found

/* ======================================================================
   SETUP
   ====================================================================== */
void setup() {
  Serial.begin(115200);

  // 1. Generate the Lookup Table (LUT) to eliminate floating point math in the loop
  for (int i = 0; i < 256; i++) {
    float norm = i / 255.0f;
    float slanted = norm * norm * (3.0f - 2.0f * norm); // S-Curve formula
    sCurveLUT[i] = (uint8_t)(slanted * 255.0f);
  }

  // 2. Init Hardware
  Neo_pixels.begin();
  updateNeoPixels(); 

  SPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(); 
  touchscreen.setRotation(0); 

  Wire.begin(2, 1); 
  Wire.setTimeOut(100);
  grideye.begin();

  tft.begin();
  tft.setSwapBytes(true);
  tft.setRotation(2); 
  tft.fillScreen(COLOR_WHITE); 

  // Boot Logo
  tft.pushImage(40, 120, 240, 240, sundance_logo);
  tft.setTextSize(1);
  tft.setTextColor(COLOR_BLACK, COLOR_WHITE);
  tft.drawCentreString("Initializing Hardware System...", 160, 300, 2);
  
  delay(3000);
  tft.fillScreen(COLOR_BLACK);
  drawStaticUI();
}

/* ======================================================================
   MAIN LOOP
   ====================================================================== */
void loop(){
  handleTouch();
  if (millis() - state.lastThermalUpdate >= FRAME_DELAY && !state.isFrozen) {
    state.lastThermalUpdate = millis();

    if (state.updateFlag) {
      for (int i = 0; i < 64; i++) pixels[i] = grideye.getPixelTemperature(i);
      interpolate_image(pixels, 8, 8, dest_2d, INTERP_SIZE, INTERP_SIZE);
      
      if (millis() - lastHotspotUpdate >= 1000) {
        updateHotspotLocation();
        lastHotspotUpdate = millis();
      }
    }
    
    // Call drawThermalDisplay ONCE. 
    // Ensure drawCrosshair is called INSIDE drawThermalDisplay at the very end.
    drawThermalDisplay(dest_2d, 32, 32, BOX_W, BOX_H);
    
    updateStats();
    state.updateFlag = !state.updateFlag;
  }
}

/* ======================================================================
   UI & DRAWING FUNCTIONS
   ====================================================================== */
void handleTouch() {
  if (touchscreen.touched()) {
    static unsigned long lastTouchUpdate = 0;
    if (millis() - lastTouchUpdate < 30) return; 
    lastTouchUpdate = millis();
    TS_Point p = touchscreen.getPoint();

    int tx = constrain(map(p.x, 200, 3700, 0, 320), 0, 320);
    int ty = constrain(map(p.y, 240, 3800, 0, 480), 0, 480);

    // Menu Buttons
    if (tx > 110 && tx < 190) {
      if (ty > 280 && ty < 320) { 
        state.showSliders = !state.showSliders;
        tft.fillRect(0, 260, 320, 220, COLOR_BLACK); 
        drawStaticUI();
        delay(250); 
        return;
      }
      if (ty > 330 && ty < 370) { 
        state.isPaused = !state.isPaused;
        drawPauseButton();
        delay(250); 
        return;
      }
      if (ty >= 390 && ty < 440) { 
        state.isFrozen = !state.isFrozen;
        drawFreezeButton();
        delay(250); 
        return;
      }
    }

    // Sliders
    if (state.showSliders) {
      if (tx < 150 && ty > 280) {
        state.sliderHue = constrain(map(ty, 450, 300, 0, 65535), 0, 65535);
        updateNeoPixels();
        drawHueSlider(false);
      }
      if (tx >= 150 && ty > 280) {
        state.sliderBright = constrain(map(ty, 450, 300, 0, DEFAULT_BRIGHT), 0, DEFAULT_BRIGHT);
        updateNeoPixels();
        drawBrightSlider(false);
      }
    }
  }
}

void drawMenuButton() {
  int bx = 115, by = 280;
  tft.setTextSize(2);
  if (state.showSliders) {
    tft.fillRect(bx, by, 70, 40, COLOR_BLUE);
    tft.drawCentreString("HIDE", 150, 285, 2);
  } else {
    tft.fillRect(bx, by, 70, 40, 0x39E7);
    tft.drawCentreString("MENU", 150, 285, 2);
  }
}

void drawPauseButton() {
  int bx = 115, by = 330;
  tft.fillRect(bx, by, 70, 40, state.isPaused ? COLOR_RED : COLOR_GREEN);
  tft.setTextColor(state.isPaused ? COLOR_WHITE : COLOR_BLACK);
  tft.drawCentreString(state.isPaused ? "HOLD" : "AUTO", 150, 342, 2);
}

void drawFreezeButton() {
  int bx = 115, by = 390;
  tft.drawRect(bx, by, 70, 48, COLOR_WHITE);
  tft.fillRect(bx + 1, by + 1, 68, 46, COLOR_BLACK);
  if (state.isFrozen) tft.fillTriangle(bx + 25, by + 10, bx + 25, by + 38, bx + 50, by + 24, COLOR_GREEN);
  else {
    tft.fillRect(bx + 22, by + 10, 8, 28, COLOR_WHITE);
    tft.fillRect(bx + 40, by + 10, 8, 28, COLOR_WHITE);
  }
}

void drawStaticUI() {
  tft.setTextColor(COLOR_WHITE, COLOR_BLACK);
  for (int i = 0; i < 256; i++) {
    tft.drawFastVLine(32 + i, 248, 8, camColors[i]);
  }
  tft.drawRect(31, 247, 258, 10, COLOR_WHITE); 

  if (state.showSliders) {
    tft.fillRect(0, 260, 320, 20, COLOR_BLACK);
    tft.setTextSize(1); 
    tft.drawCentreString("Colour", 75, 280, 1); 
    tft.drawCentreString("Bright", 225, 280, 1);
    
    drawHueSlider(true);
    drawBrightSlider(true);
  }

  drawMenuButton();
  drawPauseButton();
  drawFreezeButton();
}

void updateNeoPixels() {
  uint32_t rgbColor = Neo_pixels.gamma32(Neo_pixels.ColorHSV(state.sliderHue, 255, state.sliderBright));
  for (int i = 0; i < NEO_NUMPIXELS; i++) {
    Neo_pixels.setPixelColor(i, rgbColor);
  }
  Neo_pixels.show();
}

void drawHueSlider(bool force) {
  if (!state.showSliders) return; 
  if (!force && abs(state.sliderHue - state.lastSliderHue) < 500) return;
  
  for (int i = 0; i < 148; i++) {
    uint8_t colorIdx = map(i, 0, 148, 0, 255);
    tft.fillRect(40, 301 + i, 15, 1, camColors[colorIdx]);
  }
  int yPos = map(state.sliderHue, 0, 65535, 442, 300);
  tft.drawRect(65, 300, 30, 150, COLOR_WHITE);
  tft.fillRect(66, 301, 28, 148, COLOR_BLACK);
  tft.fillRect(66, yPos, 28, 8, COLOR_GREEN); 
  
  state.lastSliderHue = state.sliderHue;
}

void drawBrightSlider(bool force) {
  if (!state.showSliders) return; 
  if (!force && abs(state.sliderBright - state.lastSliderBright) < 2) return;
  
  int yPos = map(state.sliderBright, 0, DEFAULT_BRIGHT, 442, 300); 
  tft.drawRect(205, 300, 30, 150, COLOR_WHITE);
  tft.fillRect(206, 301, 28, 148, COLOR_BLACK);
  tft.fillRect(206, yPos, 28, 8, COLOR_WHITE);
  
  state.lastSliderBright = state.sliderBright;
}

void drawThermalDisplay(float *p, uint8_t rows, uint8_t cols, uint8_t boxW, uint8_t boxH) {
  float range = state.maxTemp - state.minTemp;
  if (range < 1.0f) range = 1.0f; 

  for (int y = 0; y < INTERP_SIZE; y++) {
    for (int x = 0; x < INTERP_SIZE; x++) {
      float val = get_point(dest_2d, INTERP_SIZE, INTERP_SIZE, x, y);
      uint8_t colorIndex = sCurveLUT[(uint8_t)(constrain((val - state.minTemp) / range, 0.0f, 1.0f) * 255.0f)];
      tft.fillRect(OFFSET_X + (BOX_W * x), OFFSET_Y + (BOX_H * y), BOX_W, BOX_H, camColors[colorIndex]);
      
      if (val > state.frameMax) state.frameMax = val;
      if (val < state.frameMin) state.frameMin = val;
    }
  }
  // Draw the crosshair ONLY once per thermal frame refresh
  drawCrosshair(hotX, hotY, hotTemp);
}

void drawCrosshair(int x, int y, float temp) {
  int centerX = OFFSET_X + (BOX_W * x) + (BOX_W / 2);
  int centerY = OFFSET_Y + (BOX_H * y) + (BOX_H / 2);

  // Define the clipping area (the thermal image window)
  tft.setViewport(OFFSET_X, OFFSET_Y, BOX_W * INTERP_SIZE, BOX_H * INTERP_SIZE);

  // Draw lines
  tft.drawLine(centerX - 6, centerY, centerX + 6, centerY, COLOR_WHITE);
  tft.drawLine(centerX, centerY - 6, centerX, centerY + 6, COLOR_WHITE);

  // Set text background to BLACK so it auto-erases
  tft.setTextColor(COLOR_WHITE, COLOR_BLACK);
  tft.setTextSize(1);
  
  // Logic to keep text within the window
  int textX = (centerX + 8 > OFFSET_X + (BOX_W * INTERP_SIZE) - 40) ? centerX - 48 : centerX + 8;
  tft.setCursor(textX, centerY - 4);
  tft.print(temp, 1);

  // RESET the viewport so the rest of the UI (sliders/buttons) works normally!
  tft.resetViewport();
}

void updateStats() {
  state.iteration++;
  if (state.iteration >= 20) {
    unsigned long now = millis();
    float fps = state.iteration / ((now - state.previousMillis) / 1000.0f);
    state.previousMillis = now;
    
    if (!state.isPaused) {
      state.minTemp = (state.minTemp * 0.8f) + (state.frameMin * 0.2f);
      state.maxTemp = (state.maxTemp * 0.8f) + (state.frameMax * 0.2f);
    }

    tft.fillRect(0, 248, 30, 10, COLOR_BLACK);
    tft.fillRect(292, 248, 28, 10, COLOR_BLACK); 
    tft.fillRect(0, 265, 150, 12, COLOR_BLACK);  

    tft.setTextSize(1);
    tft.setTextColor(COLOR_WHITE, COLOR_BLACK);
    tft.setCursor(5, 248); tft.print((int)state.minTemp); 
    tft.setCursor(294, 248); tft.print((int)state.maxTemp);
    tft.setTextColor(COLOR_GREEN, COLOR_BLACK);
    tft.setCursor(10, 265); tft.print("FPS: "); tft.print(fps, 1);

    state.frameMax = 0.0f;
    state.frameMin = 100.0f;
    state.iteration = 0;
  }
}

/* ======================================================================
   MATH / INTERPOLATION
   ====================================================================== */
float get_point(float *p, uint8_t rows, uint8_t cols, int8_t x, int8_t y) {
  if (x < 0) x = 0;
  if (y < 0) y = 0;
  if (x >= cols) x = cols - 1;
  if (y >= rows) y = rows - 1;
  return p[y * cols + x];
}

float cubicInterpolate(float p[], float x) {
  return p[1] + 0.5f * x * (p[2] - p[0] + x * (2.0f * p[0] - 5.0f * p[1] + 4.0f * p[2] - p[3] + x * (3.0f * (p[1] - p[2]) + p[3] - p[0])));
}

float bicubicInterpolate(float p[], float x, float y) {
  float arr[4];
  arr[0] = cubicInterpolate(p, x);
  arr[1] = cubicInterpolate(p + 4, x);
  arr[2] = cubicInterpolate(p + 8, x);
  arr[3] = cubicInterpolate(p + 12, x);
  return cubicInterpolate(arr, y);
}

void interpolate_image(float *src, uint8_t src_rows, uint8_t src_cols, float *dest, uint8_t dest_rows, uint8_t dest_cols) {
  float mu_x = (float)src_cols / dest_cols;
  float mu_y = (float)src_rows / dest_rows;
  float adj_2d[16];
  
  for (uint8_t y = 0; y < dest_rows; y++) {
    float src_y = (y + 0.5f) * mu_y - 0.5f;
    int8_t y_int = (int8_t)floor(src_y);
    float weight_y = src_y - y_int;

    for (uint8_t x = 0; x < dest_cols; x++) {
      float src_x = (x + 0.5f) * mu_x - 0.5f;
      int8_t x_int = (int8_t)floor(src_x);
      
      for (int8_t i = -1; i < 3; i++) {
        for (int8_t j = -1; j < 3; j++) {
          adj_2d[(i + 1) * 4 + (j + 1)] = get_point(src, src_rows, src_cols, x_int + j, y_int + i);
        }
      }
      dest[y * dest_cols + x] = bicubicInterpolate(adj_2d, src_x - x_int, weight_y);
    }
  }
}

void updateHotspotLocation() {
  float maxVal = -100.0f;
  // Use INTERP_SIZE (which is 32) as defined in your code 
  for (int y = 0; y < 32; y++) {
    for (int x = 0; x < 32; x++) {
      float val = dest_2d[y * 32 + x];
      if (val > maxVal) {
        maxVal = val;
        hotX = x;
        hotY = y;
        hotTemp = val;
      }
    }
  }
}

