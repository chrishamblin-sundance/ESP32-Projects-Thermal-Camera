//inspred / stolen from https://thesolaruniverse.wordpress.com/2022/08/02/a-true-infrared-video-camera-amg8833-sensor-esp32-microcontroller-and-3-2-inch-320240-tft-display/

// For the Pixel Panel
#include <Adafruit_NeoPixel.h>
#define NEO_PIXEL_PIN      21
#define NEO_NUMPIXELS      64
#define NEO_BRIGHTNESS     20  // Keep low to prevent power brownouts
Adafruit_NeoPixel Neo_pixels(NEO_NUMPIXELS, NEO_PIXEL_PIN, NEO_GRB + NEO_KHZ800);
uint32_t currentHue = 0;    // Tracks the color for the whole block
// end of pixel panel

#include <SPI.h>
#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();

#include <XPT2046_Touchscreen.h>
// Touchscreen pins
#define XPT2046_IRQ 9   // T_IRQ 
#define XPT2046_MOSI 3  // T_DIN
#define XPT2046_MISO 46  // T_OUT
#define XPT2046_CLK 18   // T_CLK
#define XPT2046_CS 8    // T_CS
SPIClass touchscreenSPI = SPIClass(SPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 320

   
#include <SparkFun_GridEYE_Arduino_Library.h>
GridEYE grideye;


// org color lookup table
   const uint16_t camColors[] = {0x480F,                                                       // colors usedin 256 col 16 bit
   0x400F,0x400F,0x400F,0x4010,0x3810,0x3810,0x3810,0x3810,0x3010,0x3010,
   0x3010,0x2810,0x2810,0x2810,0x2810,0x2010,0x2010,0x2010,0x1810,0x1810,
   0x1811,0x1811,0x1011,0x1011,0x1011,0x0811,0x0811,0x0811,0x0011,0x0011,
   0x0011,0x0011,0x0011,0x0031,0x0031,0x0051,0x0072,0x0072,0x0092,0x00B2,
   0x00B2,0x00D2,0x00F2,0x00F2,0x0112,0x0132,0x0152,0x0152,0x0172,0x0192,
   0x0192,0x01B2,0x01D2,0x01F3,0x01F3,0x0213,0x0233,0x0253,0x0253,0x0273,
   0x0293,0x02B3,0x02D3,0x02D3,0x02F3,0x0313,0x0333,0x0333,0x0353,0x0373,
   0x0394,0x03B4,0x03D4,0x03D4,0x03F4,0x0414,0x0434,0x0454,0x0474,0x0474,
   0x0494,0x04B4,0x04D4,0x04F4,0x0514,0x0534,0x0534,0x0554,0x0554,0x0574,
   0x0574,0x0573,0x0573,0x0573,0x0572,0x0572,0x0572,0x0571,0x0591,0x0591,
   0x0590,0x0590,0x058F,0x058F,0x058F,0x058E,0x05AE,0x05AE,0x05AD,0x05AD,
   0x05AD,0x05AC,0x05AC,0x05AB,0x05CB,0x05CB,0x05CA,0x05CA,0x05CA,0x05C9,
   0x05C9,0x05C8,0x05E8,0x05E8,0x05E7,0x05E7,0x05E6,0x05E6,0x05E6,0x05E5,
   0x05E5,0x0604,0x0604,0x0604,0x0603,0x0603,0x0602,0x0602,0x0601,0x0621,
   0x0621,0x0620,0x0620,0x0620,0x0620,0x0E20,0x0E20,0x0E40,0x1640,0x1640,
   0x1E40,0x1E40,0x2640,0x2640,0x2E40,0x2E60,0x3660,0x3660,0x3E60,0x3E60,
   0x3E60,0x4660,0x4660,0x4E60,0x4E80,0x5680,0x5680,0x5E80,0x5E80,0x6680,
   0x6680,0x6E80,0x6EA0,0x76A0,0x76A0,0x7EA0,0x7EA0,0x86A0,0x86A0,0x8EA0,
   0x8EC0,0x96C0,0x96C0,0x9EC0,0x9EC0,0xA6C0,0xAEC0,0xAEC0,0xB6E0,0xB6E0,
   0xBEE0,0xBEE0,0xC6E0,0xC6E0,0xCEE0,0xCEE0,0xD6E0,0xD700,0xDF00,0xDEE0,
   0xDEC0,0xDEA0,0xDE80,0xDE80,0xE660,0xE640,0xE620,0xE600,0xE5E0,0xE5C0,
   0xE5A0,0xE580,0xE560,0xE540,0xE520,0xE500,0xE4E0,0xE4C0,0xE4A0,0xE480,
   0xE460,0xEC40,0xEC20,0xEC00,0xEBE0,0xEBC0,0xEBA0,0xEB80,0xEB60,0xEB40,
   0xEB20,0xEB00,0xEAE0,0xEAC0,0xEAA0,0xEA80,0xEA60,0xEA40,0xF220,0xF200,
   0xF1E0,0xF1C0,0xF1A0,0xF180,0xF160,0xF140,0xF100,0xF0E0,0xF0C0,0xF0A0,
   0xF080,0xF060,0xF040,0xF020,0xF800,};

   #define BLACK   0x0000
   #define BLUE    0x001F
   #define RED     0xF800
   #define GREEN   0x07E0
   #define CYAN    0x07FF
   #define MAGENTA 0xF81F
   #define YELLOW  0xFFE0
   #define WHITE   0xFFFF

   float pixels[64];
   byte pixelIndex = 0;
   float temp_display;
   float temp_01;
   float temp_01_old;
   float fps;
   int iteration;

   
   float Max_temp = 0;
   float Min_temp = 100;
   float Max_temp_Screen = 0;
   float Min_temp_Screen = 100;

//   #define MINTEMP 22                                                                    // low range of the sensor (this will be blue on the screen)
//   #define MAXTEMP 28                                                                    // high range of the sensor (this will be red on the screen)
int MINTEMP = 22;
int MAXTEMP = 65;
   int i,j;

   #define AMG_COLS 8
   #define AMG_ROWS 8
   #define INTERPOLATED_COLS 24
   #define INTERPOLATED_ROWS 24

   float get_point(float *p, uint8_t rows, uint8_t cols, int8_t x, int8_t y);
   void set_point (float *p, uint8_t rows, uint8_t cols, int8_t x, int8_t y, float f);
   void get_adjacents_1d (float *src, float *dest, uint8_t rows, uint8_t cols, int8_t x, int8_t y);
   void get_adjacents_2d (float *src, float *dest, uint8_t rows, uint8_t cols, int8_t x, int8_t y);
   float cubicInterpolate (float p[], float x);
   float bicubicInterpolate (float p[], float x, float y);
   void interpolate_image (float *src, uint8_t src_rows, uint8_t src_cols, float *dest, uint8_t dest_rows, uint8_t dest_cols);
   float dest_2d[576];

   float iterationTime;
   long  iterationTotal;
   unsigned long previousMillis;
   unsigned long currentMillis;
   unsigned long t;

// #define I2C_SDA (32)
// #define I2C_SCL (33)
#define I2C_SDA (2)
#define I2C_SCL (1)


//sliders
int sliderHue = 0;
int sliderBright = NEO_BRIGHTNESS;

void setup() {

   Serial.begin (115200);
   Serial.println ();

  
  Neo_pixels.begin();
  Neo_pixels.setBrightness(NEO_BRIGHTNESS);
  Neo_pixels.clear();
  Neo_pixels.show();
  Serial.println("Rainbow Block Cycle Started");

   tft.begin ();                             
   tft.setSwapBytes (true);                                                              // swap the byte order for pushImage() - corrects endianness
   tft.setRotation (2);
   tft.fillScreen (BLACK);
   tft.setTextSize (1);  
   tft.setCursor (10,10); 
   tft.print ("AMG8833 GridEYE infrared sensor video");        
   tft.drawRoundRect ( 140, 280,  100,  40, 4, GREEN);    
   tft.fillRect ( 165, 275, 50,  10, BLACK); 
   tft.setCursor (170, 275);  
   tft.print ("cycles"); 
   tft.setCursor (220, 300); 
   tft.print ("Hz");      
   
   Serial.println ();
   Serial.println ("ESP32 and Sparkfun GridEye and AMG8833 thermal 8*8");
   delay (500);
   Wire.begin(I2C_SDA, I2C_SCL);
   delay (250); 
   grideye.begin();
   delay (250); 


//sliders
  tft.begin();
  tft.setRotation(1); // Set to 1 for Landscape (480x320)
  tft.fillScreen(BLACK);

  // Initialize Touch
  touchscreen.begin();
  touchscreen.setRotation(1);

  // ... rest of your setup ...
  drawInterface(); // Initial UI draw
}

void loop() {

// 1. Handle Touch Input
  if (touchscreen.touched()) {
    TS_Point p = touchscreen.getPoint();
    // Map raw touch to screen coordinates (Adjust 200/3700 if tracking is off)
    int tx = map(p.x, 200, 3700, 0, 480);
    int ty = map(p.y, 200, 3700, 0, 320);

    // Check Hue Slider (X: 400-430, Y: 50-250)
    if (tx >= 400 && tx <= 430 && ty >= 50 && ty <= 250) {
      sliderHue = map(ty, 250, 50, 0, 65535); // Vertical slider
      updateNeoPixels();
      drawHueSlider();
    }
    // Check Brightness Slider (X: 440-470, Y: 50-250)
    if (tx >= 440 && tx <= 470 && ty >= 50 && ty <= 250) {
      sliderBright = map(ty, 250, 50, 0, 255);
      updateNeoPixels();
      drawBrightSlider();
    }

   t = millis();
   for (i=0; i < 64; i++) 
      {
      temp_01 = grideye.getPixelTemperature(i);  
      pixels[i] = temp_01;  
      } 
                
   interpolate_image(pixels, 8, 8, dest_2d, 24, 24);                                     // sensor rows-columns - color tile dimensions
   drawpixels(dest_2d, 24, 24, 10, 10, false);

   iteration++;

   if (iteration > 100)
     { 
      iterationTotal=iterationTotal+iteration-1;
      currentMillis = t;
      iterationTime = currentMillis-previousMillis;
      fps = (iteration / (iterationTime/1000));
      previousMillis = millis ();

  
      Serial.println ("going out of while loop");
      Serial.print ("after cycle nr. "); 
      Serial.println (iterationTotal);
      Serial.print ("time for 100 cycles: ");
      Serial.print (iterationTime);
      Serial.println (" ms");
      Serial.print ("display refresh rate: ");
      Serial.print (fps,1);
      Serial.println (" Hz");
      Serial.println ("");
      Serial.print ("Min & Max Temps: ");
      Serial.print (Min_temp);
      Serial.print (" - ");
      Serial.println (Max_temp);
      

      tft.fillRect (160,295,55,14,BLACK);
      tft.setTextSize (2);
      tft.setCursor (165,295);   
      tft.print (fps,1);
      
      tft.setTextSize (2);
      tft.setCursor (10,325);  
      tft.print ("Min Temp: "); 
      tft.print (Min_temp,1);
      MINTEMP = Min_temp;
 
      tft.print (" C");
      tft.setCursor (10,345);
      tft.print ("Max Temp: ");
      tft.print (Max_temp,1); 
      MAXTEMP = Max_temp; 
      
      Max_temp = 0;
      Min_temp = 100;
  
      tft.print (" C");
      
      iteration = 0;
      iterationTime = 0;  
     }   

}

void drawpixels(float *p, uint8_t rows, uint8_t cols, uint8_t boxWidth, uint8_t boxHeight, boolean showVal) {
  int colorTemp;
  for (int y=0; y<rows; y++) {
    for (int x=0; x<cols; x++) {
      float val = get_point(p, rows, cols, x, y);
      if(val >= MAXTEMP) colorTemp = MAXTEMP;
      else if(val <= MINTEMP) colorTemp = MINTEMP;
      else colorTemp = val;
      
      uint8_t colorIndex = map(colorTemp, MINTEMP, MAXTEMP, 0, 255);
      colorIndex = (uint8_t)constrain((int16_t)colorIndex, (int16_t)0, (int16_t)255);
      //draw the pixels!
      //uint16_t color = val * 2;
      tft.fillRect(40+boxWidth * x, 25+boxHeight * y, boxWidth, boxHeight, camColors[colorIndex]);


    if(val >= Max_temp){
      Max_temp = val;
      MAXTEMP = Max_temp;
    }
    if(val <= Min_temp){
      Min_temp = val;
      MINTEMP = Min_temp;
    }
        
    } 
  }
} 
void updateNeoPixels() {
  uint32_t rgbColor = Neo_pixels.ColorHSV(sliderHue, 255, 255);
  Neo_pixels.setBrightness(sliderBright);
  for (int i = 0; i < NEO_NUMPIXELS; i++) {
    Neo_pixels.setPixelColor(i, rgbColor);
  }
  Neo_pixels.show();
}

void drawInterface() {
  tft.setTextColor(WHITE);
  tft.drawString("COLOR", 395, 30, 2);
  tft.drawString("BRIGHT", 435, 30, 2);
  drawHueSlider();
  drawBrightSlider();
}

void drawHueSlider() {
  int yPos = map(sliderHue, 0, 65535, 250, 50);
  tft.fillRect(400, 50, 30, 200, BLACK); // Clear
  tft.drawRect(400, 50, 30, 200, WHITE); // Border
  tft.fillRect(402, yPos, 26, 10, GREEN); // Thumb
}

void drawBrightSlider() {
  int yPos = map(sliderBright, 0, 255, 250, 50);
  tft.fillRect(440, 50, 30, 200, BLACK); // Clear
  tft.drawRect(440, 50, 30, 200, WHITE); // Border
  tft.fillRect(442, yPos, 26, 10, WHITE); // Thumb
}
