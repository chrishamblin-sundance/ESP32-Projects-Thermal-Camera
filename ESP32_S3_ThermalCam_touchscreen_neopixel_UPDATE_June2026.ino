#include <Adafruit_NeoPixel.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <Wire.h>
#include <SparkFun_GridEYE_Arduino_Library.h>

/* ======================================================================
   USER ADJUSTABLE SETTINGS (Tweak these for performance/look)
   ====================================================================== */
#define NEO_PIXEL_PIN     21
#define NEO_NUMPIXELS     64
#define DEFAULT_BRIGHT    128   // Startup LED Brightness (0-255)
#define FRAME_DELAY       30    // Time between frames in ms (Lower = Faster FPS)
#define INTERP_SIZE       24    // Current interpolation is 24x24 pixels

/* --- Touchscreen Pins --- */
#define XPT2046_IRQ 9   
#define XPT2046_MOSI 3  
#define XPT2046_MISO 46 
#define XPT2046_CLK 18  
#define XPT2046_CS 8    

/* --- Color Definitions --- */
#define BLACK   0x0000
#define WHITE   0xFFFF
#define RED     0xF800
#define GREEN   0x07E0
#define BLUE    0x001F

// ======================================================================
// SUNDANCE COMPANY LOGO BITMAP
// 150x150 pixels, 16-bit RGB565 format
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
   GLOBAL OBJECTS & VARIABLES
   ====================================================================== */
Adafruit_NeoPixel Neo_pixels(NEO_NUMPIXELS, NEO_PIXEL_PIN, NEO_GRB + NEO_KHZ800);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();
GridEYE grideye;

int sliderHue = 0;
int sliderBright = DEFAULT_BRIGHT;
int lastSliderHue = -1;
int lastSliderBright = -1;

float pixels[64];              // Raw data from sensor
float dest_2d[576];            // Interpolated data (24x24)
float MINTEMP = 18;            // Dynamic scale bottom
float MAXTEMP = 30;            // Dynamic scale top
float Max_temp = 0;            // Frame tracker
float Min_temp = 100;          // Frame tracker

int iteration = 0;
unsigned long previousMillis = 0;
bool isPaused = false;         // Logic for "HOLD" (Scale Lock)
bool isFrozen = false;         // Logic for "FREEZE" (Screen Lock)
bool showSliders = true;       // UI toggle tracking for Neopixel adjustments

/* ======================================================================
   SETUP
   ====================================================================== */
void setup() {
  Serial.begin(115200);

  Neo_pixels.begin();
  updateNeoPixels(); 



  // Initialize SPI for Touch
  SPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(); 
  touchscreen.setRotation(0); 

  // Initialize Sensor
  Wire.begin(2, 1); 
  Wire.setTimeOut(100); 
  grideye.begin();


  tft.begin();
  tft.setSwapBytes(true);
  tft.setRotation(2); 
  tft.fillScreen(WHITE); // White background to seamlessly match the logo background

  // --------------------------------------------------------------------
  // DRAW SUN DANCE LOGO SPLASH SCREEN
  // Centered on a 320px wide screen: X_pos = (320 - 150) / 2 = 85
  // Y-positioned beautifully near the middle vertical space
  // --------------------------------------------------------------------
  tft.pushImage(40, 120, 240, 240, sundance_logo);
  
  tft.setTextSize(1);
  tft.setTextColor(BLACK, WHITE);
  tft.drawCentreString("Initializing Hardware System...", 160, 300, 2);
  
  delay(3000);           // Allow 3 seconds for boot logo & sensor warmup
  tft.fillScreen(BLACK);  // Clean layout background for regular UI operation
  // --------------------------------------------------------------------

  
  drawStaticUI();
}

/* ======================================================================
   MAIN LOOP
   ====================================================================== */
void loop() {
  handleTouch(); // Check for user input

  static unsigned long lastThermalUpdate = 0;
  // Only update thermal if not "Frozen"
  if (millis() - lastThermalUpdate >= FRAME_DELAY && !isFrozen) {
    lastThermalUpdate = millis();

    // 1. Get raw sensor data
    for (int i = 0; i < 64; i++) {
      pixels[i] = grideye.getPixelTemperature(i);
    }

    // 2. Interpolate 8x8 -> 24x24
    interpolate_image(pixels, 8, 8, dest_2d, 24, 24);
    // 3. Draw the result
    drawThermalDisplay(dest_2d, 24, 24, 13, 10);
    // 4. Update Scale Math and Labels
    updateStats();
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
    int tx = map(p.x, 200, 3700, 0, 320); 
    int ty = map(p.y, 240, 3800, 0, 480);
    tx = constrain(tx, 0, 320);
    ty = constrain(ty, 0, 480);

    // 1. BUTTON HITBOXES (Center Column)
    if (tx > 110 && tx < 190) {
      if (ty > 280 && ty < 320) { // MENU Toggle
        showSliders = !showSliders;
        tft.fillRect(0, 260, 320, 220, BLACK); // Clear below the scale
        drawStaticUI();
        delay(250); 
        return;
      }
      if (ty > 330 && ty < 370) { // HOLD/AUTO
        isPaused = !isPaused;
        drawPauseButton();
        delay(250); 
        return;
      }
      if (ty >= 390 && ty < 440) { // FREEZE
        isFrozen = !isFrozen;
        drawFreezeButton();
        delay(250); 
        return;
      }
    }

    // --- SLIDER HITBOXES (Only processing tracking loop if menu active) ---
    if (showSliders) {
      // Left side: Hue
      if (tx < 150 && ty > 280) {
        sliderHue = map(ty, 450, 300, 0, 65535);
        sliderHue = constrain(sliderHue, 0, 65535);
        updateNeoPixels();
        drawHueSlider(false);
      }
      // Right side: Brightness
      if (tx >= 150 && ty > 280) {
        sliderBright = map(ty, 450, 300, 0, 255);
        sliderBright = constrain(sliderBright, 0, 255);
        updateNeoPixels();
        drawBrightSlider(false);
      }
    }
  }
}

void drawMenuButton() {
  int bx = 115, by = 280;
  tft.setTextSize(2);
  if (showSliders) {
    tft.fillRect(bx, by, 70, 40, BLUE);
    tft.drawCentreString("HIDE", 150, 285, 2);
  } else {
    tft.fillRect(bx, by, 70, 40, 0x39E7);
    tft.drawCentreString("MENU", 150, 285, 2);
  }
}

void drawPauseButton() {
  int bx = 115, by = 330;
  tft.fillRect(bx, by, 70, 40, isPaused ? RED : GREEN);
  tft.setTextColor(isPaused ? WHITE : BLACK);
  tft.drawCentreString(isPaused ? "HOLD" : "AUTO", 150, 342, 2);
}

void drawFreezeButton() {
  int bx = 115, by = 390;
  tft.drawRect(bx, by, 70, 48, WHITE);
  tft.fillRect(bx + 1, by + 1, 68, 46, BLACK);
  if (isFrozen) tft.fillTriangle(bx + 25, by + 10, bx + 25, by + 38, bx + 50, by + 24, GREEN);
  else {
    tft.fillRect(bx + 22, by + 10, 8, 28, WHITE);
    tft.fillRect(bx + 40, by + 10, 8, 28, WHITE);
  }
}

void drawStaticUI() {
  tft.setTextColor(WHITE, BLACK);
  
  // Always draw the scale [cite: 47, 48]
  for (int i = 0; i < 256; i++) {
    tft.drawFastVLine(32 + i, 248, 8, camColors[i]);
  }
  tft.drawRect(31, 247, 258, 10, WHITE); 

  if (showSliders) {
    // FORCE CLEAR the label area first to prevent "ghosting" or size distortion
    tft.fillRect(0, 260, 320, 20, BLACK); 
    
    // Explicitly set text parameters before drawing
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
  uint32_t rgbColor = Neo_pixels.gamma32(Neo_pixels.ColorHSV(sliderHue, 255, sliderBright));
  for (int i = 0; i < NEO_NUMPIXELS; i++) {
    Neo_pixels.setPixelColor(i, rgbColor);
  }
  Neo_pixels.show();
}

void drawHueSlider(bool force) {
  if (!showSliders) return; 
  if (!force && abs(sliderHue - lastSliderHue) < 500) return;
  
  // 1. Draw a wider Reference Scale (15px wide)
  // Instead of drawFastVLine, we draw a small filled rectangle for each slice.
  // This ensures it is visually solid.
  for (int i = 0; i < 148; i++) {
    uint8_t colorIdx = map(i, 0, 148, 0, 255);
    // Draw a 15px wide horizontal bar for each vertical position
    tft.fillRect(40, 301 + i, 15, 1, camColors[colorIdx]); 
  }

  // 2. Draw the slider (now shifted slightly to accommodate the wider scale)
  int yPos = map(sliderHue, 0, 65535, 442, 300);
  tft.drawRect(65, 300, 30, 150, WHITE);
  tft.fillRect(66, 301, 28, 148, BLACK);
  tft.fillRect(66, yPos, 28, 8, GREEN); 
  
  lastSliderHue = sliderHue;
}

void drawBrightSlider(bool force) {
  if (!showSliders) return; 
  if (!force && abs(sliderBright - lastSliderBright) < 2) return;
  int yPos = map(sliderBright, 0, 255, 442, 300);
  tft.drawRect(205, 300, 30, 150, WHITE);
  tft.fillRect(206, 301, 28, 148, BLACK);
  tft.fillRect(206, yPos, 28, 8, WHITE);
  lastSliderBright = sliderBright;
}

void drawThermalDisplay(float *p, uint8_t rows, uint8_t cols, uint8_t boxW, uint8_t boxH) {
  for (int y = 0; y < rows; y++) {
    for (int x = 0; x < cols; x++) {
      float val = get_point(p, rows, cols, x, y);
      // Contrast Logic
      float range = MAXTEMP - MINTEMP;
      if (range < 1.0) range = 1.0; 
      float norm = (val - MINTEMP) / range;
      norm = constrain(norm, 0.0, 1.0);
      // S-Curve Enhancement
      float slanted = norm * norm * (3.0 - 2.0 * norm);
      uint8_t colorIndex = (uint8_t)(slanted * 255.0);
      
      tft.fillRect(4 + (boxW * x), 5 + (boxH * y), boxW, boxH, camColors[colorIndex]);
      if (val > Max_temp) Max_temp = val;
      if (val < Min_temp) Min_temp = val;
    }
  }
}

void updateStats() {
  iteration++;
  if (iteration >= 20) {
    unsigned long now = millis();
    float fps = iteration / ((now - previousMillis) / 1000.0);
    previousMillis = now;
    // Scale Logic: Blend current range with historical range
    if (!isPaused) {
      MINTEMP = (MINTEMP * 0.8) + (Min_temp * 0.2);
      MAXTEMP = (MAXTEMP * 0.8) + (Max_temp * 0.2);
    }

    // Clear and Redraw text labels
    tft.fillRect(0, 248, 30, 10, BLACK);
    tft.fillRect(292, 248, 28, 10, BLACK); 
    tft.fillRect(0, 265, 150, 12, BLACK);  

    tft.setTextSize(1);
    tft.setTextColor(WHITE, BLACK);
    tft.setCursor(5, 248); tft.print((int)MINTEMP); 
    tft.setCursor(294, 248); tft.print((int)MAXTEMP);
    tft.setTextColor(GREEN, BLACK);
    tft.setCursor(10, 265); tft.print("FPS: "); tft.print(fps, 1);

    // Reset frame trackers
    Max_temp = 0;
    Min_temp = 100;
    iteration = 0;
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
  return p[1] + 0.5 * x * (p[2] - p[0] + x * (2.0 * p[0] - 5.0 * p[1] + 4.0 * p[2] - p[3] + x * (3.0 * (p[1] - p[2]) + p[3] - p[0])));
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
    for (uint8_t x = 0; x < dest_cols; x++) {
      float src_x = (x + 0.5) * mu_x - 0.5;
      float src_y = (y + 0.5) * mu_y - 0.5;
      int8_t x_int = (int8_t)floor(src_x);
      int8_t y_int = (int8_t)floor(src_y);
      for (int8_t i = -1; i < 3; i++) {
        for (int8_t j = -1; j < 3; j++) {
          adj_2d[(i + 1) * 4 + (j + 1)] = get_point(src, src_rows, src_cols, x_int + j, y_int + i);
        }
      }
      dest[y * dest_cols + x] = bicubicInterpolate(adj_2d, src_x - x_int, src_y - y_int);
    }
  }
}