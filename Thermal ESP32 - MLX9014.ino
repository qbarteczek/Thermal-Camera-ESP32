#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_MLX90614.h>
#include <Wire.h>

// Definicje pinów dla wyświetlacza TFT
#define TFT_CS   4  
#define TFT_DC   17  
#define TFT_RST  16  
#define TFT_MOSI 23  
#define TFT_SCK  18  
#define TFT_MISO 19  

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

// Tablice do przechowywania wartości temperatur
float ambientTemp;
float objectTemp;
const int displayWidth = 320;
const int displayHeight = 240;

// Funkcja do mapowania temperatur na kolory (kolory termowizyjne)
uint16_t mapTemperatureToColor(float temperature) {
    // Zdefiniuj zakres temperatur dla kolorów (możesz dostosować do swoich potrzeb)
    float minTemp = 20.0;
    float maxTemp = 40.0;
    
    if (temperature < minTemp) temperature = minTemp;
    if (temperature > maxTemp) temperature = maxTemp;
    
    float range = maxTemp - minTemp;
    float normalized = (temperature - minTemp) / range;

    // Kolory termowizyjne: od ciemnoniebieskiego przez jasnoniebieski, żółty do czerwonego
    uint8_t red, green, blue;
    
    if (normalized < 0.25) {
        red = 0;
        green = 0;
        blue = 128 + 127 * (normalized / 0.25);
    } else if (normalized < 0.5) {
        red = 0;
        green = 255 * ((normalized - 0.25) / 0.25);
        blue = 255;
    } else if (normalized < 0.75) {
        red = 255 * ((normalized - 0.5) / 0.25);
        green = 255;
        blue = 255 - 255 * ((normalized - 0.5) / 0.25);
    } else {
        red = 255;
        green = 255 - 255 * ((normalized - 0.75) / 0.25);
        blue = 0;
    }

    return tft.color565(red, green, blue);
}

void setup() {
    Serial.begin(115200);
    tft.begin();
    tft.setRotation(1); // Ustawienie orientacji poziomej
    tft.fillScreen(ILI9341_BLACK);
    
    if (!mlx.begin()) {
        Serial.println("Nie można znaleźć czujnika MLX90614");
        while (1);
    }
    delay(100);
}

void loop() {
    ambientTemp = mlx.readAmbientTempC();
    objectTemp = mlx.readObjectTempC();
    
    // Mapowanie temperatury na kolor
    uint16_t color = mapTemperatureToColor(objectTemp);
    
    // Wyświetlanie danych na ekranie
    tft.fillScreen(color);
    
    // Wyświetlanie temperatury
    tft.setTextColor(ILI9341_WHITE);
    tft.setCursor(10, displayHeight - 30);
    tft.setTextSize(2);
    tft.print("Ambient: ");
    tft.print(ambientTemp, 1);
    tft.print("C");
    
    tft.setCursor(10, displayHeight - 60);
    tft.print("Object: ");
    tft.print(objectTemp, 1);
    tft.print("C");
    
    delay(500); // Czekanie na kolejny odczyt (czas można dostosować)
}
