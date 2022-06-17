#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <JsonListener.h>
#include <time.h>
#include "OpenWeatherMapCurrent.h"

// initiate the client
OpenWeatherMapCurrent client;
OpenWeatherMapCurrentData data;

String OPEN_WEATHER_MAP_APP_ID = "fb30d750157453f423abf982b7afa6b6";

String OPEN_WEATHER_MAP_LOCATION_ID = "3104324";

/*
  Arabic - ar, Bulgarian - bg, Catalan - ca, Czech - cz, German - de, Greek - el,
  English - en, Persian (Farsi) - fa, Finnish - fi, French - fr, Galician - gl,
  Croatian - hr, Hungarian - hu, Italian - it, Japanese - ja, Korean - kr,
  Latvian - la, Lithuanian - lt, Macedonian - mk, Dutch - nl, Polish - pl,
  Portuguese - pt, Romanian - ro, Russian - ru, Swedish - se, Slovak - sk,
  Slovenian - sl, Spanish - es, Turkish - tr, Ukrainian - ua, Vietnamese - vi,
  Chinese Simplified - zh_cn, Chinese Traditional - zh_tw.
*/
String OPEN_WEATHER_MAP_LANGUAGE = "en";
boolean IS_METRIC = true;

/**
   WiFi Settings
*/
const char* ESP_HOST_NAME = "esp-" + ESP.getFlashChipId();
const char* WIFI_SSID     = "iPhone de Andrea";
const char* WIFI_PASSWORD = "12345678";

// initiate the WifiClient
WiFiClient wifiClient;


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for SSD1306 display connected using software SPI (default case):
#define OLED_D1   D7
#define OLED_D0   D5
#define OLED_DC    D2
#define OLED_CS    D8
#define OLED_RESET D1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_D1, OLED_D0, OLED_DC, OLED_RESET, OLED_CS);

#define PIXEL_PIN D6
#define NUMPIXELS 20

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIXEL_PIN, NEO_GRB + NEO_KHZ800);


const int Y_pin = A0; // analog pin connected to Y output
String modos[] = {"Apagado", "Modo 1", "Modo 2", "Modo 3", "Modo 4", "Modo 5", "Modo 6", "Modo 7", "Modo 8", "Modo 9", "Modo 10", "Modo 11", "Modo 12", "Modo 13", "Modo 14", "Modo 15", "Modo 16", "Tiempo"};
int modo = 0;

void setup() {
  Serial.begin(9600);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
  Serial.println(F("Initialized!"));

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();
  // Refresh (apply command)
  display.display();

  // Set color of the text
  display.setTextColor(SSD1306_WHITE);


  display.setCursor(5, 0);
  display.setTextSize(2);
  display.print("Conectando al WiFi");
  display.display();
  delay(500);
  connectWifi();
  display.clearDisplay();


  Serial.println();
  Serial.println("\n\nNext Loop-Step: " + String(millis()) + ":");

  client.setLanguage(OPEN_WEATHER_MAP_LANGUAGE);
  client.setMetric(IS_METRIC);
  client.updateCurrentById(&data, OPEN_WEATHER_MAP_APP_ID, OPEN_WEATHER_MAP_LOCATION_ID);


  pixels.begin();
  pixels.setBrightness(100);
  pixels.clear();

}

void loop() {
  luces();
}


/**
   Helping funtions
*/
void connectWifi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected!");
  Serial.println(WiFi.localIP());
  Serial.println();
}



void menu() {
  if (analogRead(Y_pin) >= 1000) {
    if (modo == 17) {
      modo = 0;
    } else {
      modo = modo + 1;
    }
  } else if (analogRead(Y_pin) <= 100) {
    if (modo == 0) {
      modo = 17;
    } else {
      modo = modo - 1;
    }
  }
  if (modo == 17) {
    display.clearDisplay();
    display.setCursor(5, 0);
    display.setTextSize(2);
    display.print(modos[modo]);
    display.setCursor(5, 20);
    display.setTextSize(2);
    display.print(modos[0]);
    display.display();
    Serial.println(modos[modo] + " " + modos[0]);
  } else {
    display.clearDisplay();
    display.setCursor(5, 0);
    display.setTextSize(2);
    display.print(modos[modo]);
    display.setCursor(5, 20);
    display.setTextSize(2);
    display.print(modos[modo + 1]);
    display.display();
    Serial.println(modos[modo] + " " + modos[modo + 1]);
  }
}

void luces() {
  switch (modo) {
    case 0: {
        menu();
        pixels.clear();
        for (int i = 0; i < NUMPIXELS; i++) {
          pixels.setPixelColor(i, 0, 0, 0); // Brillo moderado en rojo
          pixels.show();   // Mostramos y actualizamos el color del pixel de nuestra cinta led RGB
        }
        break;
      }
    case 1: {
        //azules y verdes
        menu();
        pixels.clear();
        pixels.setPixelColor(0, 0, 255, 115);
        pixels.setPixelColor(1, 24, 0, 255);
        pixels.setPixelColor(2, 0, 255, 111);
        pixels.setPixelColor(3, 0, 156, 255);
        pixels.setPixelColor(4, 0, 255, 255);
        pixels.setPixelColor(5, 0, 255, 115);
        pixels.setPixelColor(6, 24, 0, 255);
        pixels.setPixelColor(7, 0, 255, 111);
        pixels.setPixelColor(8, 0, 156, 255);
        pixels.setPixelColor(9, 0, 255, 255);
        pixels.setPixelColor(10, 0, 255, 115);
        pixels.setPixelColor(11, 24, 0, 255);
        pixels.setPixelColor(12, 0, 255, 111);
        pixels.setPixelColor(13, 0, 156, 255);
        pixels.setPixelColor(14, 0, 255, 255);
        pixels.setPixelColor(15, 0, 255, 115);
        pixels.setPixelColor(16, 24, 0, 255);
        pixels.setPixelColor(17, 0, 255, 111);
        pixels.setPixelColor(18, 0, 156, 255);
        pixels.setPixelColor(19, 0, 255, 255);
        pixels.show();
        break;
      }
    case 2: {
        //naranja, azul, frambuesa, verde
        menu();
        pixels.clear();
        pixels.setPixelColor(0, 255, 32, 0);
        pixels.setPixelColor(1, 88, 173, 255);
        pixels.setPixelColor(2, 200, 4, 29);
        pixels.setPixelColor(3, 106, 255, 56);
        pixels.setPixelColor(4, 255, 32, 0);
        pixels.setPixelColor(5, 88, 173, 255);
        pixels.setPixelColor(6, 200, 4, 29);
        pixels.setPixelColor(7, 106, 255, 56);
        pixels.setPixelColor(8, 255, 32, 0);
        pixels.setPixelColor(9, 88, 173, 255);
        pixels.setPixelColor(10, 200, 4, 29);
        pixels.setPixelColor(11, 106, 255, 56);
        pixels.setPixelColor(12, 255, 32, 0);
        pixels.setPixelColor(13, 88, 173, 255);
        pixels.setPixelColor(14, 200, 4, 29);
        pixels.setPixelColor(15, 106, 255, 56);
        pixels.setPixelColor(16, 255, 32, 0);
        pixels.setPixelColor(17, 88, 173, 255);
        pixels.setPixelColor(18, 200, 4, 29);
        pixels.setPixelColor(19, 106, 255, 56);
        pixels.show();   // Mostramos y actualizamos el color del pixel de nuestra cinta led RGB
        break;
      }
    case 3: {
        //amarillo, morado, naranja
        menu();
        pixels.clear();
        pixels.setPixelColor(0, 190, 100, 1);
        pixels.setPixelColor(1, 211, 8, 255);
        pixels.setPixelColor(2, 239, 65, 11);
        pixels.setPixelColor(3, 190, 100, 1);
        pixels.setPixelColor(4, 211, 8, 255);
        pixels.setPixelColor(5, 239, 65, 11);
        pixels.setPixelColor(6, 190, 100, 1);
        pixels.setPixelColor(7, 211, 8, 255);
        pixels.setPixelColor(8, 239, 65, 11);
        pixels.setPixelColor(9, 190, 100, 1);
        pixels.setPixelColor(10, 211, 8, 255);
        pixels.setPixelColor(11, 239, 65, 11);
        pixels.setPixelColor(12, 190, 100, 1);
        pixels.setPixelColor(13, 211, 8, 255);
        pixels.setPixelColor(14, 239, 65, 11);
        pixels.setPixelColor(15, 190, 100, 1);
        pixels.setPixelColor(16, 211, 8, 255);
        pixels.setPixelColor(17, 239, 65, 11);
        pixels.setPixelColor(18, 190, 100, 1);
        pixels.setPixelColor(19, 211, 8, 255);
        pixels.show();
        break;
      }
    case 4: {
        //verde, morado, azul, rosa
        menu();
        pixels.clear();
        pixels.setPixelColor(0, 56, 255, 0);
        pixels.setPixelColor(1, 140, 0, 255);
        pixels.setPixelColor(2, 0, 111, 255);
        pixels.setPixelColor(3, 255, 0, 112);
        pixels.setPixelColor(4, 56, 255, 0);
        pixels.setPixelColor(5, 140, 0, 255);
        pixels.setPixelColor(6, 0, 111, 255);
        pixels.setPixelColor(7, 255, 0, 112);
        pixels.setPixelColor(8, 56, 255, 0);
        pixels.setPixelColor(9, 140, 0, 255);
        pixels.setPixelColor(10, 0, 111, 255);
        pixels.setPixelColor(11, 255, 0, 112);
        pixels.setPixelColor(12, 56, 255, 0);
        pixels.setPixelColor(13, 140, 0, 255);
        pixels.setPixelColor(14, 0, 111, 255);
        pixels.setPixelColor(15, 255, 0, 112);
        pixels.setPixelColor(16, 56, 255, 0);
        pixels.setPixelColor(17, 140, 0, 255);
        pixels.setPixelColor(18, 0, 111, 255);
        pixels.setPixelColor(19, 255, 0, 112);
        pixels.show();
        break;
      }
    case 5: {
        //rojo, naranja, amarillo
        menu();
        pixels.clear();
        pixels.setPixelColor(0, 255, 106, 0);
        pixels.setPixelColor(1, 255, 53, 0);
        pixels.setPixelColor(2, 255, 0, 0);
        pixels.setPixelColor(3, 255, 106, 0);
        pixels.setPixelColor(4, 255, 53, 0);
        pixels.setPixelColor(5, 255, 0, 0);
        pixels.setPixelColor(6, 255, 106, 0);
        pixels.setPixelColor(7, 255, 53, 0);
        pixels.setPixelColor(8, 255, 0, 0);
        pixels.setPixelColor(9, 255, 106, 0);
        pixels.setPixelColor(10, 255, 53, 0);
        pixels.setPixelColor(11, 255, 0, 0);
        pixels.setPixelColor(12, 255, 106, 0);
        pixels.setPixelColor(13, 255, 53, 0);
        pixels.setPixelColor(14, 255, 0, 0);
        pixels.setPixelColor(15, 255, 106, 0);
        pixels.setPixelColor(16, 255, 53, 0);
        pixels.setPixelColor(17, 255, 0, 0);
        pixels.setPixelColor(18, 255, 106, 0);
        pixels.setPixelColor(19, 255, 53, 0);
        pixels.show();
        break;
      }
    case 6: {
        //morado, rosa
        menu();
        pixels.clear();
        pixels.setPixelColor(0, 255, 36, 251);
        pixels.setPixelColor(1, 255, 24, 96);
        pixels.setPixelColor(2, 255, 36, 251);
        pixels.setPixelColor(3, 255, 24, 96);
        pixels.setPixelColor(4, 255, 36, 251);
        pixels.setPixelColor(5, 255, 24, 96);
        pixels.setPixelColor(6, 255, 36, 251);
        pixels.setPixelColor(7, 255, 24, 96);
        pixels.setPixelColor(8, 255, 36, 251);
        pixels.setPixelColor(9, 255, 24, 96);
        pixels.setPixelColor(10, 255, 36, 251);
        pixels.setPixelColor(11, 255, 24, 96);
        pixels.setPixelColor(12, 255, 36, 251);
        pixels.setPixelColor(13, 255, 24, 96);
        pixels.setPixelColor(14, 255, 36, 251);
        pixels.setPixelColor(15, 255, 24, 96);
        pixels.setPixelColor(16, 255, 36, 251);
        pixels.setPixelColor(17, 255, 24, 96);
        pixels.setPixelColor(18, 255, 36, 251);
        pixels.setPixelColor(19, 255, 24, 96);
        pixels.show();
        break;
      }
    case 7: {
        //turquesa
        menu();
        pixels.clear();
        for (int i = 0; i < NUMPIXELS; i++) {
          pixels.setPixelColor(i, 28, 255, 188);
        }
        pixels.show();
        break;
      }
    case 8: {
        //rosa
        menu();
        pixels.clear();
        for (int i = 0; i < NUMPIXELS; i++) {
          pixels.setPixelColor(i, 144, 6, 12);
        }
        pixels.show();
        break;
      }
    case 9: {
        //morado
        menu();
        pixels.clear();
        for (int i = 0; i < NUMPIXELS; i++) {
          pixels.setPixelColor(i, 197, 0, 255);
        }
        pixels.show();
        break;
      }
    case 10: {
        //azul verdoso
        menu();
        pixels.clear();
        for (int i = 0; i < NUMPIXELS; i++) {
          pixels.setPixelColor(i, 64, 255, 67);
        }
        pixels.show();
        break;
      }
    case 11: {
        //rojo
        menu();
        pixels.clear();
        for (int i = 0; i < NUMPIXELS; i++) {
          pixels.setPixelColor(i, 255, 0, 0);
        }
        pixels.show();
        break;
      }
    case 12: {
        //melocotón
        menu();
        pixels.clear();
        for (int i = 0; i < NUMPIXELS; i++) {
          pixels.setPixelColor(i, 255, 71, 31);
        }
        pixels.show();
        break;
      }
    case 13: {
        //blanco
        menu();
        pixels.clear();
        for (int i = 0; i < NUMPIXELS; i++) {
          pixels.setPixelColor(i, 255, 255, 255);
        }
        pixels.show();
        break;
      }
    case 14: {
        //blanco, verde, morado
        menu();
        pixels.clear();
        pixels.setPixelColor(0, 255, 255, 255);
        pixels.setPixelColor(1, 0, 255, 25);
        pixels.setPixelColor(2, 128, 0, 255);
        pixels.setPixelColor(3, 255, 255, 255);
        pixels.setPixelColor(4, 0, 255, 25);
        pixels.setPixelColor(5, 128, 0, 255);
        pixels.setPixelColor(6, 255, 255, 255);
        pixels.setPixelColor(7, 0, 255, 25);
        pixels.setPixelColor(8, 128, 0, 255);
        pixels.setPixelColor(9, 255, 255, 255);
        pixels.setPixelColor(10, 0, 255, 25);
        pixels.setPixelColor(11, 128, 0, 255);
        pixels.setPixelColor(12, 255, 255, 255);
        pixels.setPixelColor(13, 0, 255, 25);
        pixels.setPixelColor(14, 128, 0, 255);
        pixels.setPixelColor(15, 255, 255, 255);
        pixels.setPixelColor(16, 0, 255, 25);
        pixels.setPixelColor(17, 128, 0, 255);
        pixels.setPixelColor(18, 255, 255, 255);
        pixels.setPixelColor(19, 0, 255, 25);
        pixels.show();
        break;
      }
    case 15: {
        //rosa, azul, blanco
        menu();
        pixels.clear();
        pixels.setPixelColor(0, 255, 48, 121);
        pixels.setPixelColor(1, 123, 104, 255);
        pixels.setPixelColor(2, 255, 255, 255);
        pixels.setPixelColor(3, 255, 48, 121);
        pixels.setPixelColor(4, 123, 104, 255);
        pixels.setPixelColor(5, 255, 255, 255);
        pixels.setPixelColor(6, 255, 48, 121);
        pixels.setPixelColor(7, 123, 104, 255);
        pixels.setPixelColor(8, 255, 255, 255);
        pixels.setPixelColor(9, 255, 48, 121);
        pixels.setPixelColor(10, 123, 104, 255);
        pixels.setPixelColor(11, 255, 255, 255);
        pixels.setPixelColor(12, 255, 48, 121);
        pixels.setPixelColor(13, 123, 104, 255);
        pixels.setPixelColor(14, 255, 255, 255);
        pixels.setPixelColor(15, 255, 48, 121);
        pixels.setPixelColor(16, 123, 104, 255);
        pixels.setPixelColor(17, 255, 255, 255);
        pixels.setPixelColor(18, 255, 48, 121);
        pixels.setPixelColor(19, 123, 104, 255);
        pixels.show();
        break;
      }
    case 16: {
        //amarillo, blanco, azul
        menu();
        pixels.clear();
        pixels.setPixelColor(0, 255, 255, 0);
        pixels.setPixelColor(1, 255, 255, 255);
        pixels.setPixelColor(2, 100, 128, 255);
        pixels.setPixelColor(3, 255, 255, 0);
        pixels.setPixelColor(4, 255, 255, 255);
        pixels.setPixelColor(5, 100, 128, 255);
        pixels.setPixelColor(6, 255, 255, 0);
        pixels.setPixelColor(7, 255, 255, 255);
        pixels.setPixelColor(8, 100, 128, 255);
        pixels.setPixelColor(9, 255, 255, 0);
        pixels.setPixelColor(10, 255, 255, 255);
        pixels.setPixelColor(11, 100, 128, 255);
        pixels.setPixelColor(12, 255, 255, 0);
        pixels.setPixelColor(13, 255, 255, 255);
        pixels.setPixelColor(14, 100, 128, 255);
        pixels.setPixelColor(15, 255, 255, 0);
        pixels.setPixelColor(16, 255, 255, 255);
        pixels.setPixelColor(17, 100, 128, 255);
        pixels.setPixelColor(18, 255, 255, 0);
        pixels.setPixelColor(19, 255, 255, 255);
        pixels.show();
        break;
      }
    case 17: {
        //tiempo
        menu();
        pixels.clear();
        temperatura();
        cielo();
      }
  }
  delay(250);
}

//establece los neopixel impares segun la temperatura actual
void temperatura() {
  if (data.temp < 0) {
    //azul turquesa
    pixels.setPixelColor(1, 0, 255, 255);
    pixels.setPixelColor(3, 0, 255, 255);
    pixels.setPixelColor(5, 0, 255, 255);
    pixels.setPixelColor(7, 0, 255, 255);
    pixels.setPixelColor(9, 0, 255, 255);
    pixels.setPixelColor(11, 0, 255, 255);
    pixels.setPixelColor(13, 0, 255, 255);
    pixels.setPixelColor(15, 0, 255, 255);
    pixels.setPixelColor(17, 0, 255, 255);
    pixels.setPixelColor(19, 0, 255, 255);
    pixels.show();
  } else if (data.temp >= 0 && data.temp < 10) {
    //verde
    pixels.setPixelColor(1, 0, 255, 0);
    pixels.setPixelColor(3, 0, 255, 0);
    pixels.setPixelColor(5, 0, 255, 0);
    pixels.setPixelColor(7, 0, 255, 0);
    pixels.setPixelColor(9, 0, 255, 0);
    pixels.setPixelColor(11, 0, 255, 0);
    pixels.setPixelColor(13, 0, 255, 0);
    pixels.setPixelColor(15, 0, 255, 0);
    pixels.setPixelColor(17, 0, 255, 0);
    pixels.setPixelColor(19, 0, 255, 0);
    pixels.show();
  } else if (data.temp >= 10 && data.temp < 20) {
    //amarillo
    pixels.setPixelColor(1, 255, 255, 0);
    pixels.setPixelColor(3, 255, 255, 0);
    pixels.setPixelColor(5, 255, 255, 0);
    pixels.setPixelColor(7, 255, 255, 0);
    pixels.setPixelColor(9, 255, 255, 0);
    pixels.setPixelColor(11, 255, 255, 0);
    pixels.setPixelColor(13, 255, 255, 0);
    pixels.setPixelColor(15, 255, 255, 0);
    pixels.setPixelColor(17, 255, 255, 0);
    pixels.setPixelColor(19, 255, 255, 0);
    pixels.show();
  } else if (data.temp >= 20 && data.temp < 30) {
    //naranja
    pixels.setPixelColor(1, 255, 128, 0);
    pixels.setPixelColor(3, 255, 128, 0);
    pixels.setPixelColor(5, 255, 128, 0);
    pixels.setPixelColor(7, 255, 128, 0);
    pixels.setPixelColor(9, 255, 128, 0);
    pixels.setPixelColor(11, 255, 128, 0);
    pixels.setPixelColor(13, 255, 128, 0);
    pixels.setPixelColor(15, 255, 128, 0);
    pixels.setPixelColor(17, 255, 128, 0);
    pixels.setPixelColor(19, 255, 128, 0);
    pixels.show();
  } else if (data.temp >= 30 && data.temp < 40) {
    //rojo
    pixels.setPixelColor(1, 255, 0, 0);
    pixels.setPixelColor(3, 255, 0, 0);
    pixels.setPixelColor(5, 255, 0, 0);
    pixels.setPixelColor(7, 255, 0, 0);
    pixels.setPixelColor(9, 255, 0, 0);
    pixels.setPixelColor(11, 255, 0, 0);
    pixels.setPixelColor(13, 255, 0, 0);
    pixels.setPixelColor(15, 255, 0, 0);
    pixels.setPixelColor(17, 255, 0, 0);
    pixels.setPixelColor(19, 255, 0, 0);
    pixels.show();
  } else if (data.temp >= 40) {
    //magenta
    pixels.setPixelColor(1, 255, 0, 128);
    pixels.setPixelColor(3, 255, 0, 128);
    pixels.setPixelColor(5, 255, 0, 128);
    pixels.setPixelColor(7, 255, 0, 128);
    pixels.setPixelColor(9, 255, 0, 128);
    pixels.setPixelColor(11, 255, 0, 128);
    pixels.setPixelColor(13, 255, 0, 128);
    pixels.setPixelColor(15, 255, 0, 128);
    pixels.setPixelColor(17, 255, 0, 128);
    pixels.setPixelColor(19, 255, 0, 128);
    pixels.show();
  } else {
    pixels.setPixelColor(1, 0, 0, 0);
    pixels.setPixelColor(3, 0, 0, 0);
    pixels.setPixelColor(5, 0, 0, 0);
    pixels.setPixelColor(7, 0, 0, 0);
    pixels.setPixelColor(9, 0, 0, 0);
    pixels.setPixelColor(11, 0, 0, 0);
    pixels.setPixelColor(13, 0, 0, 0);
    pixels.setPixelColor(15, 0, 0, 0);
    pixels.setPixelColor(17, 0, 0, 0);
    pixels.setPixelColor(19, 0, 0, 0);
    pixels.show();
  }
}

void cielo() {
  if (data.weatherId == 800) {
    //group 800: clear - azul turquesa
    pixels.setPixelColor(0, 0, 255, 255);
    pixels.setPixelColor(2, 0, 255, 255);
    pixels.setPixelColor(4, 0, 255, 255);
    pixels.setPixelColor(6, 0, 255, 255);
    pixels.setPixelColor(8, 0, 255, 255);
    pixels.setPixelColor(10, 0, 255, 255);
    pixels.setPixelColor(12, 0, 255, 255);
    pixels.setPixelColor(14, 0, 255, 255);
    pixels.setPixelColor(16, 0, 255, 255);
    pixels.setPixelColor(18, 0, 255, 255);
    pixels.show();
  } else if (data.weatherId == 801) {
    // 801: few clouds - azul grisaceo
    pixels.setPixelColor(0, 0, 128, 255);
    pixels.setPixelColor(2, 0, 128, 255);
    pixels.setPixelColor(4, 0, 128, 255);
    pixels.setPixelColor(6, 0, 128, 255);
    pixels.setPixelColor(8, 0, 128, 255);
    pixels.setPixelColor(10, 0, 128, 255);
    pixels.setPixelColor(12, 0, 128, 255);
    pixels.setPixelColor(14, 0, 128, 255);
    pixels.setPixelColor(16, 0, 128, 255);
    pixels.setPixelColor(18, 0, 128, 255);
    pixels.show();
  } else if (data.weatherId == 802) {
    //802: scattered clouds - azul pocho
    pixels.setPixelColor(0, 200, 200, 255);
    pixels.setPixelColor(2, 200, 200, 255);
    pixels.setPixelColor(4, 200, 200, 255);
    pixels.setPixelColor(6, 200, 200, 255);
    pixels.setPixelColor(8, 200, 200, 255);
    pixels.setPixelColor(10, 200, 200, 255);
    pixels.setPixelColor(12, 200, 200, 255);
    pixels.setPixelColor(14, 200, 200, 255);
    pixels.setPixelColor(16, 200, 200, 255);
    pixels.setPixelColor(18, 200, 200, 255);
    pixels.show();
  } else if (data.weatherId == 803 || data.weatherId == 804) {
    //803, 804: broken clouds, overcast clouds - morado
    pixels.setPixelColor(0, 128, 0, 255);
    pixels.setPixelColor(2, 128, 0, 255);
    pixels.setPixelColor(4, 128, 0, 255);
    pixels.setPixelColor(6, 128, 0, 255);
    pixels.setPixelColor(8, 128, 0, 255);
    pixels.setPixelColor(10, 128, 0, 255);
    pixels.setPixelColor(12, 128, 0, 255);
    pixels.setPixelColor(14, 128, 0, 255);
    pixels.setPixelColor(16, 128, 0, 255);
    pixels.setPixelColor(18, 128, 0, 255);
    pixels.show();
  } else if (data.weatherId >= 300 && data.weatherId <= 321) {
    //group 3xx: drizzle - azul turquesa parpadea
    pixels.setPixelColor(0, 0, 255, 255);
    pixels.setPixelColor(2, 0, 255, 255);
    pixels.setPixelColor(4, 0, 255, 255);
    pixels.setPixelColor(6, 0, 255, 255);
    pixels.setPixelColor(8, 0, 255, 255);
    pixels.setPixelColor(10, 0, 255, 255);
    pixels.setPixelColor(12, 0, 255, 255);
    pixels.setPixelColor(14, 0, 255, 255);
    pixels.setPixelColor(16, 0, 255, 255);
    pixels.setPixelColor(18, 0, 255, 255);
    pixels.show();
    delay(500);
    pixels.setPixelColor(0, 0, 0, 0);
    pixels.setPixelColor(2, 0, 0, 0);
    pixels.setPixelColor(4, 0, 0, 0);
    pixels.setPixelColor(6, 0, 0, 0);
    pixels.setPixelColor(8, 0, 0, 0);
    pixels.setPixelColor(10, 0, 0, 0);
    pixels.setPixelColor(12, 0, 0, 0);
    pixels.setPixelColor(14, 0, 0, 0);
    pixels.setPixelColor(16, 0, 0, 0);
    pixels.setPixelColor(18, 0, 0, 0);
    pixels.show();
    delay(500);
  } else if (data.weatherId >= 500 && data.weatherId <= 531) {
    //group 5xx: rain - azul oscuro parpadea
    pixels.setPixelColor(0, 0, 0, 255);
    pixels.setPixelColor(2, 0, 0, 255);
    pixels.setPixelColor(4, 0, 0, 255);
    pixels.setPixelColor(6, 0, 0, 255);
    pixels.setPixelColor(8, 0, 0, 255);
    pixels.setPixelColor(10, 0, 0, 255);
    pixels.setPixelColor(12, 0, 0, 255);
    pixels.setPixelColor(14, 0, 0, 255);
    pixels.setPixelColor(16, 0, 0, 255);
    pixels.setPixelColor(18, 0, 0, 255);
    pixels.show();
    delay(500);
    pixels.setPixelColor(0, 0, 0, 0);
    pixels.setPixelColor(2, 0, 0, 0);
    pixels.setPixelColor(4, 0, 0, 0);
    pixels.setPixelColor(6, 0, 0, 0);
    pixels.setPixelColor(8, 0, 0, 0);
    pixels.setPixelColor(10, 0, 0, 0);
    pixels.setPixelColor(12, 0, 0, 0);
    pixels.setPixelColor(14, 0, 0, 0);
    pixels.setPixelColor(16, 0, 0, 0);
    pixels.setPixelColor(18, 0, 0, 0);
    pixels.show();
    delay(500);
  } else if (data.weatherId >= 200 && data.weatherId <= 232) {
    //group 2xx: thunderstorm - morado y amarillo
    pixels.setPixelColor(0, 128, 0, 255);
    pixels.setPixelColor(2, 128, 0, 255);
    pixels.setPixelColor(4, 128, 0, 255);
    pixels.setPixelColor(6, 128, 0, 255);
    pixels.setPixelColor(8, 128, 0, 255);
    pixels.setPixelColor(10, 128, 0, 255);
    pixels.setPixelColor(12, 128, 0, 255);
    pixels.setPixelColor(14, 128, 0, 255);
    pixels.setPixelColor(16, 128, 0, 255);
    pixels.setPixelColor(18, 128, 0, 255);
    pixels.show();
    delay(500);
    pixels.setPixelColor(0, 0, 0, 0);
    pixels.setPixelColor(2, 0, 0, 0);
    pixels.setPixelColor(4, 0, 0, 0);
    pixels.setPixelColor(6, 0, 0, 0);
    pixels.setPixelColor(8, 0, 0, 0);
    pixels.setPixelColor(10, 0, 0, 0);
    pixels.setPixelColor(12, 0, 0, 0);
    pixels.setPixelColor(14, 0, 0, 0);
    pixels.setPixelColor(16, 0, 0, 0);
    pixels.setPixelColor(18, 0, 0, 0);
    pixels.show();
    delay(500);
    pixels.setPixelColor(0, 255, 255, 0);
    pixels.setPixelColor(2, 255, 255, 0);
    pixels.setPixelColor(4, 255, 255, 0);
    pixels.setPixelColor(6, 255, 255, 0);
    pixels.setPixelColor(8, 255, 255, 0);
    pixels.setPixelColor(10, 255, 255, 0);
    pixels.setPixelColor(12, 255, 255, 0);
    pixels.setPixelColor(14, 255, 255, 0);
    pixels.setPixelColor(16, 255, 255, 0);
    pixels.setPixelColor(18, 255, 255, 0);
    pixels.show();
    delay(500);
    pixels.setPixelColor(0, 0, 0, 0);
    pixels.setPixelColor(2, 0, 0, 0);
    pixels.setPixelColor(4, 0, 0, 0);
    pixels.setPixelColor(6, 0, 0, 0);
    pixels.setPixelColor(8, 0, 0, 0);
    pixels.setPixelColor(10, 0, 0, 0);
    pixels.setPixelColor(12, 0, 0, 0);
    pixels.setPixelColor(14, 0, 0, 0);
    pixels.setPixelColor(16, 0, 0, 0);
    pixels.setPixelColor(18, 0, 0, 0);
    pixels.show();
    delay(500);
  } else if (data.weatherId >= 600 && data.weatherId <= 622) {
    //group 6xx: snow - blanco parpadeante
    pixels.setPixelColor(0, 255, 255, 255);
    pixels.setPixelColor(2, 255, 255, 255);
    pixels.setPixelColor(4, 255, 255, 255);
    pixels.setPixelColor(6, 255, 255, 255);
    pixels.setPixelColor(8, 255, 255, 255);
    pixels.setPixelColor(10, 255, 255, 255);
    pixels.setPixelColor(12, 255, 255, 255);
    pixels.setPixelColor(14, 255, 255, 255);
    pixels.setPixelColor(16, 255, 255, 255);
    pixels.setPixelColor(18, 255, 255, 255);
    pixels.show();
    delay(500);
    pixels.setPixelColor(0, 0, 0, 0);
    pixels.setPixelColor(2, 0, 0, 0);
    pixels.setPixelColor(4, 0, 0, 0);
    pixels.setPixelColor(6, 0, 0, 0);
    pixels.setPixelColor(8, 0, 0, 0);
    pixels.setPixelColor(10, 0, 0, 0);
    pixels.setPixelColor(12, 0, 0, 0);
    pixels.setPixelColor(14, 0, 0, 0);
    pixels.setPixelColor(16, 0, 0, 0);
    pixels.setPixelColor(18, 0, 0, 0);
    pixels.show();
    delay(500);
  } else if (data.weatherId == 701 || data.weatherId == 711 || data.weatherId == 721 || data.weatherId == 741 || data.weatherId == 771 || data.weatherId == 781) {
    //701, 711, 721, 741, 771, 781: mist, smoke, haze, fog, squalls, tornado- blanco
    pixels.setPixelColor(0, 255, 255, 255);
    pixels.setPixelColor(2, 255, 255, 255);
    pixels.setPixelColor(4, 255, 255, 255);
    pixels.setPixelColor(6, 255, 255, 255);
    pixels.setPixelColor(8, 255, 255, 255);
    pixels.setPixelColor(10, 255, 255, 255);
    pixels.setPixelColor(12, 255, 255, 255);
    pixels.setPixelColor(14, 255, 255, 255);
    pixels.setPixelColor(16, 255, 255, 255);
    pixels.setPixelColor(18, 255, 255, 255);
    pixels.show();
  } else if (data.weatherId == 731 || data.weatherId == 751 || data.weatherId == 761 || data.weatherId == 762) {
    //sand/ dust whirls, sand, dust, volcanic ash - naranja
    pixels.setPixelColor(0, 255, 128, 0);
    pixels.setPixelColor(2, 255, 128, 0);
    pixels.setPixelColor(4, 255, 128, 0);
    pixels.setPixelColor(6, 255, 128, 0);
    pixels.setPixelColor(8, 255, 128, 0);
    pixels.setPixelColor(10, 255, 128, 0);
    pixels.setPixelColor(12, 255, 128, 0);
    pixels.setPixelColor(14, 255, 128, 0);
    pixels.setPixelColor(16, 255, 128, 0);
    pixels.setPixelColor(18, 255, 128, 0);
    pixels.show();
  } else {
    pixels.setPixelColor(0, 0, 0, 0);
    pixels.setPixelColor(2, 0, 0, 0);
    pixels.setPixelColor(4, 0, 0, 0);
    pixels.setPixelColor(6, 0, 0, 0);
    pixels.setPixelColor(8, 0, 0, 0);
    pixels.setPixelColor(10, 0, 0, 0);
    pixels.setPixelColor(12, 0, 0, 0);
    pixels.setPixelColor(14, 0, 0, 0);
    pixels.setPixelColor(16, 0, 0, 0);
    pixels.setPixelColor(18, 0, 0, 0);
    pixels.show();
  }
}
