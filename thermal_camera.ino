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

  // Initialize TFT display
  tft.begin();
  tft.setRotation(3);  // Adjust rotation if needed

  // Initialize AMG8833 sensor
  if (!amg.begin()) {
    Serial.println("Could not find a valid AMG88xx sensor, check wiring!");
    while (1);
  }

  // Clear the TFT screen
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
}

void loop() {
  // Read pixel data from AMG8833
  float pixels_raw[AMG88xx_PIXEL_ARRAY_SIZE];
  amg.readPixels(pixels_raw);

  // Interpolate and scale the image to 320x240
  for (int y = 0; y < SCREEN_HEIGHT; y++) {
    for (int x = 0; x < SCREEN_WIDTH; x++) {
      float x_ratio = x * 8.0 / 320.0;
      float y_ratio = y * 8.0 / 240.0;
      int x_int = (int)x_ratio;
      int y_int = (int)y_ratio;
      float x_frac = x_ratio - x_int;
      float y_frac = y_ratio - y_int;

      int index_tl = y_int * 8 + x_int;
      int index_tr = y_int * 8 + (x_int + 1);
      int index_bl = (y_int + 1) * 8 + x_int;
      int index_br = (y_int + 1) * 8 + (x_int + 1);

      float interp_val = pixels_raw[index_tl] * (1 - x_frac) * (1 - y_frac) +
                         pixels_raw[index_tr] * x_frac * (1 - y_frac) +
                         pixels_raw[index_bl] * (1 - x_frac) * y_frac +
                         pixels_raw[index_br] * x_frac * y_frac;

      // Map temperature value to color (assuming grayscale for simplicity)
      uint16_t color = tft.color565(interp_val, interp_val, interp_val);

      tft.drawPixel(x, y, color);
    }
  }

  // Display the central temperature value
  float central_temp = pixels_raw[4 * 8 + 4];  // Temperature at the central point (4,4)
  tft.setCursor(10, 10);
  tft.fillRect(0, 0, 320, 30, ILI9341_BLACK);  // Clear the area where text will be displayed
  tft.print("Center Temp: ");
  tft.print(central_temp, 2);
  tft.print(" C");

  delay(100);  // Adjust delay as needed for display rate
}
