#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_AMG88xx.h>

// Definicje pinów dla wyświetlacza TFT
#define TFT_CS   4  
#define TFT_DC   17  
#define TFT_RST  16  
#define TFT_MOSI 23  
#define TFT_SCK  18  
#define TFT_MISO 19  

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
Adafruit_AMG88xx amg;

// Tablice do przechowywania wartości temperatur
float pixels[64]; // 8x8 matryca
float interpolatedPixels[40][30]; // 40x30 matryca po interpolacji
uint16_t colors[40][30]; // Tablica kolorów do wyświetlenia

const int gridWidth = 8; // Szerokość matrycy czujnika
const int gridHeight = 8; // Wysokość matrycy czujnika
const int displayWidth = 320;
const int displayHeight = 240;
const int cellWidth = displayWidth / 40; // Szerokość komórki po skalowaniu (320 / 40)
const int cellHeight = displayHeight / 30; // Wysokość komórki po skalowaniu (240 / 30)

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

// Funkcja do interpolacji danych 8x8 do 40x30
void interpolatePixels() {
    for (int y = 0; y < 30; y++) {
        for (int x = 0; x < 40; x++) {
            float gx = (float)x / 40 * 7;
            float gy = (float)y / 30 * 7;
            int gxi = (int)gx;
            int gyi = (int)gy;
            float c00 = pixels[gyi * 8 + gxi];
            float c10 = pixels[gyi * 8 + (gxi + 1) % 8];
            float c01 = pixels[(gyi + 1) % 8 * 8 + gxi];
            float c11 = pixels[(gyi + 1) % 8 * 8 + (gxi + 1) % 8];
            float rm = gx - gxi;
            float cm = gy - gyi;
            interpolatedPixels[x][y] = c00 * (1 - rm) * (1 - cm) + c10 * rm * (1 - cm) + c01 * (1 - rm) * cm + c11 * rm * cm;
        }
    }
}

void setup() {
    Serial.begin(115200);
    tft.begin();
    tft.setRotation(1); // Ustawienie orientacji poziomej
    tft.fillScreen(ILI9341_BLACK);
    
    if (!amg.begin()) {
        Serial.println("Nie można znaleźć czujnika AMG8830");
        while (1);
    }
    delay(100);
}

void loop() {
    amg.readPixels(pixels); // Odczyt danych z czujnika
    
    // Interpolacja danych
    interpolatePixels();
    
    // Mapowanie temperatur na kolory
    for (int y = 0; y < 30; y++) {
        for (int x = 0; x < 40; x++) {
            colors[x][y] = mapTemperatureToColor(interpolatedPixels[x][y]);
        }
    }
    
    // Wyświetlanie danych na ekranie
    for (int y = 0; y < 30; y++) {
        for (int x = 0; x < 40; x++) {
            tft.fillRect(x * cellWidth, y * cellHeight, cellWidth, cellHeight, colors[x][y]);
        }
    }
    
    // Wyświetlanie skali temperatur
    int scaleHeight = 20;
    for (int i = 0; i < displayWidth; i++) {
        float temp = 20.0 + (i * (40.0 - 20.0) / displayWidth);
        uint16_t color = mapTemperatureToColor(temp);
        tft.drawFastVLine(i, displayHeight - scaleHeight, scaleHeight, color);
    }
    tft.setTextColor(ILI9341_WHITE);
    tft.setCursor(10, displayHeight - scaleHeight - 10); // Pozycja dla 20°C
    tft.print("20C");
    tft.setCursor(displayWidth - 30, displayHeight - scaleHeight - 10); // Pozycja dla 40°C
    tft.print("40C");
    
    // Wyświetlanie temperatury centralnego punktu
    float centralTemp = pixels[27]; // Centralny punkt matrycy 8x8
    tft.setCursor(displayWidth / 2 - 20, displayHeight / 2 - 10); // Pozycja centralna
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    tft.setTextSize(2);
    tft.print(centralTemp, 1);
    tft.print("C");
    
    delay(500); // Czekanie na kolejny odczyt (czas można dostosować)
}
