#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_AMG88xx.h>

#define TFT_CS   14  // Pin CS ekranu TFT podłączony do pinu GPIO 14 (ESP32)
#define TFT_DC   27  // Pin DC/RD ekranu TFT podłączony do pinu GPIO 27 (ESP32)
#define TFT_RST  33  // Pin RST ekranu TFT podłączony do pinu GPIO 33 (ESP32)
#define TFT_MOSI 23  // Pin MOSI SPI ekranu TFT podłączony do pinu GPIO 23 (ESP32)
#define TFT_CLK  18  // Pin CLK SPI ekranu TFT podłączony do pinu GPIO 18 (ESP32)
#define TFT_MISO 19  // Pin MISO SPI ekranu TFT podłączony do pinu GPIO 19 (ESP32)

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240
#define AMG88xx_PIXEL_ARRAY_SIZE 64

// Create an instance of the Adafruit ILI9341 TFT library
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST, TFT_MOSI, TFT_CLK, TFT_MISO);

// Create an instance of the Adafruit AMG88xx IR sensor library
Adafruit_AMG88xx amg;

// Buffer to hold interpolated 320x240 image
uint16_t pixels[SCREEN_WIDTH * SCREEN_HEIGHT];

void setup() {
  Serial.begin(115200);
  Serial.println("Setup started");

  // Initialize TFT display
  if (!tft.begin()) {
    Serial.println("TFT initialization failed!");
    while (1);
  }
  Serial.println("TFT initialized");
  tft.setRotation(3);  // Adjust rotation if needed
  Serial.println("TFT rotation set");

  // Initialize AMG8833 sensor
  if (!amg.begin()) {
    Serial.println("Could not find a valid AMG88xx sensor, check wiring!");
    while (1);
  }
  Serial.println("AMG8833 initialized");

  // Clear the TFT screen
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  Serial.println("Screen cleared");
}

void loop() {
  // Read pixel data from AMG8833
  float pixels_raw[AMG88xx_PIXEL_ARRAY_SIZE];
  amg.readPixels(pixels_raw);
  Serial.println("Pixel data read from AMG8833");

  // Interpolate and scale the image to 320x240
  for (int y = 0; y < SCREEN_HEIGHT; y++) {
    for (int x = 0; x < SCREEN_WIDTH; x++) {
      float x_ratio = x * 8.0 / SCREEN_WIDTH;
      float y_ratio = y * 8.0 / SCREEN_HEIGHT;
      int x_int = (int)x_ratio;
      int y_int = (int)y_ratio;
      float x_frac = x_ratio - x_int;
      float y_frac = y_ratio - y_int;

      int index_tl = y_int * 8 + x_int;
      int index_tr = y_int * 8 + (x_int < 7 ? x_int + 1 : x_int);
      int index_bl = (y_int < 7 ? y_int + 1 : y_int) * 8 + x_int;
      int index_br = (y_int < 7 ? y_int + 1 : y_int) * 8 + (x_int < 7 ? x_int + 1 : x_int);

      float interp_val = pixels_raw[index_tl] * (1 - x_frac) * (1 - y_frac) +
                         pixels_raw[index_tr] * x_frac * (1 - y_frac) +
                         pixels_raw[index_bl] * (1 - x_frac) * y_frac +
                         pixels_raw[index_br] * x_frac * y_frac;

      // Map temperature value to color
      uint16_t color = temperatureToColor(interp_val);
      tft.drawPixel(x, y, color);
    }
  }
  Serial.println("Interpolation and display update complete");

  // Display the central temperature value
  float central_temp = pixels_raw[4 * 8 + 4];  // Temperature at the central point (4,4)
  tft.setCursor(10, 10);
  tft.fillRect(0, 0, 320, 30, ILI9341_BLACK);  // Clear the area where text will be displayed
  tft.print("Center Temp: ");
  tft.print(central_temp, 2);
  tft.print(" C");
  Serial.print("Central Temp: ");
  Serial.print(central_temp, 2);
  Serial.println(" C");

  delay(100);  // Adjust delay as needed for display rate
}

uint16_t temperatureToColor(float temp) {
  uint8_t r, g, b;
  if (temp < 20) {
    r = 0;
    g = 0;
    b = map(temp, 0, 20, 255, 0);
  } else if (temp < 30) {
    r = 0;
    g = map(temp, 20, 30, 0, 255);
    b = 0;
  } else {
    r = map(temp, 30, 40, 0, 255);
    g = 255 - r;
    b = 0;
  }
  return tft.color565(r, g, b);
}
