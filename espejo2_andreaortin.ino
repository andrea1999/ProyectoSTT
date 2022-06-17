#include <Arduino.h>

#include <Adafruit_GFX.h>
#include <Adafruit_NeoPixel.h>

#include <ESPWiFi.h>
#include <ESPHTTPClient.h>
#include <JsonListener.h>

// time
#include <time.h>                       // time() ctime()
#include <sys/time.h>                   // struct timeval
#include <coredecls.h>                  // settimeofday_cb()

#include "SSD1306Wire.h"
#include "OLEDDisplayUi.h"
#include "Wire.h"

#include "OpenWeatherMapCurrent.h"
#include "OpenWeatherMapForecast.h"
#include "WeatherStationFonts.h"
#include "WeatherStationImages.h"

#include "Astronomy.h"
#include "MoonImages.h"


// Neopixels
#define PIXEL_PIN D6
#define NUMPIXELS 2

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIXEL_PIN, NEO_GRB + NEO_KHZ800);


// WIFI
const char* WIFI_SSID = "iPhone de Andrea";
const char* WIFI_PWD = "12345678";


#define TZ              -2       // (utc+) TZ in hours
#define DST_MN           0      // use 60mn for summer time in some countries


// Setup
const int UPDATE_INTERVAL_SECS = 20 * 60; // Update every 20 minutes


// Ajustes display
const int I2C_DISPLAY_ADDRESS = 0x3c;
#if defined(ESP8266)
const int SDA_PIN = D3;
const int SDC_PIN = D4;
#else
const int SDA_PIN = 5; //D3;
const int SDC_PIN = 4; //D4;
#endif


// Ajustes OpenWeatherMap
String OPEN_WEATHER_MAP_APP_ID = "fb30d750157453f423abf982b7afa6b6";


// ID ciudad
String OPEN_WEATHER_MAP_LOCATION_ID = "3104324";

// Idioma
// Arabic - ar, Bulgarian - bg, Catalan - ca, Czech - cz, German - de, Greek - el,
// English - en, Persian (Farsi) - fa, Finnish - fi, French - fr, Galician - gl,
// Croatian - hr, Hungarian - hu, Italian - it, Japanese - ja, Korean - kr,
// Latvian - la, Lithuanian - lt, Macedonian - mk, Dutch - nl, Polish - pl,
// Portuguese - pt, Romanian - ro, Russian - ru, Swedish - se, Slovak - sk,
// Slovenian - sl, Spanish - es, Turkish - tr, Ukrainian - ua, Vietnamese - vi,
// Chinese Simplified - zh_cn, Chinese Traditional - zh_tw.
String OPEN_WEATHER_MAP_LANGUAGE = "es";
const uint8_t MAX_FORECASTS = 4;

const boolean IS_METRIC = true;

// Dias de la semana y meses
const String WDAY_NAMES[] = {"DOM", "LUN", "MAR", "MIE", "JUE", "VIE", "SAB"};
const String MONTH_NAMES[] = {"ENE", "FEB", "MAR", "ABR", "MAY", "JUN", "JUL", "AGO", "SEP", "OCT", "NOV", "DIC"};


// Inicializamos la pantalla oled: 0x3c
SSD1306Wire     display(I2C_DISPLAY_ADDRESS, SDA_PIN, SDC_PIN);
OLEDDisplayUi   ui( &display );

OpenWeatherMapCurrentData currentWeather;
OpenWeatherMapCurrent currentWeatherClient;

OpenWeatherMapForecastData forecasts[MAX_FORECASTS];
OpenWeatherMapForecast forecastClient;

#define TZ_MN           ((TZ)*60)
#define TZ_SEC          ((TZ)*3600)
#define DST_SEC         ((DST_MN)*60)
time_t now;

bool readyForWeatherUpdate = false;

String lastUpdate = "--";

long timeSinceLastWUpdate = 0;

// declaramos prototipos
void drawProgress(OLEDDisplay *display, int percentage, String label);
void updateData(OLEDDisplay *display);
void drawDateTime(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawCurrentWeather(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawForecast(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawForecastDetails(OLEDDisplay *display, int x, int y, int dayIndex);
void drawHeaderOverlay(OLEDDisplay *display, OLEDDisplayUiState* state);
void setReadyForWeatherUpdate();


// Añadimos frames
// este array muestra el orden de los frames de izquierda a derecha
FrameCallback frames[] = { drawDateTime, drawCurrentWeather, drawForecast };
int numberOfFrames = 3;

OverlayCallback overlays[] = { drawHeaderOverlay };
int numberOfOverlays = 1;

Astronomy astronomy;

const int sensorPin = D1;
int estado = 0;

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();

  pinMode(sensorPin, INPUT);

  // inicializamos display
  display.init();
  display.clear();
  display.display();

  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setContrast(255);

  // inicializamos el wifi
  WiFi.begin(WIFI_SSID, WIFI_PWD);

  int counter = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    display.clear();
    display.drawString(64, 10, "Connecting to WiFi");
    display.drawXbm(46, 30, 8, 8, counter % 3 == 0 ? activeSymbole : inactiveSymbole);
    display.drawXbm(60, 30, 8, 8, counter % 3 == 1 ? activeSymbole : inactiveSymbole);
    display.drawXbm(74, 30, 8, 8, counter % 3 == 2 ? activeSymbole : inactiveSymbole);
    display.display();

    counter++;
  }

  // Obtenemos la hora desde internet
  configTime(TZ_SEC, DST_SEC, "pool.ntp.org");

  ui.setTargetFPS(30);

  ui.setActiveSymbol(activeSymbole);
  ui.setInactiveSymbol(inactiveSymbole);

  // Posicion del indicador de el frame en pantalla
  ui.setIndicatorPosition(BOTTOM);
  ui.setIndicatorDirection(LEFT_RIGHT);

  // Transicion que usa el indicador
  // SLIDE_LEFT, SLIDE_RIGHT, SLIDE_TOP, SLIDE_DOWN
  ui.setFrameAnimation(SLIDE_LEFT);

  ui.setFrames(frames, numberOfFrames);

  ui.setOverlays(overlays, numberOfOverlays);

  // Inicia UI que inicia el display.
  ui.init();

  Serial.println("");

  updateData(&display);

  pixels.begin();
  pixels.setBrightness(25);
  pixels.clear();

}

void loop() {

  /*if (millis() - timeSinceLastWUpdate > (1000L * UPDATE_INTERVAL_SECS)) {
    setReadyForWeatherUpdate();
    timeSinceLastWUpdate = millis();
  }*/

  if (readyForWeatherUpdate && ui.getUiState()->frameState == FIXED) {
    updateData(&display);
  }

  int remainingTimeBudget = ui.update();

  if (remainingTimeBudget > 0) {
    delay(remainingTimeBudget);
  }

  luces();
  estado = digitalRead(sensorPin);


  //mandar mensaje a puerto serie en función del valor leido
  while (estado == HIGH) {
    display.clear();
    drawMoon();
    delay(3500);
    estado = digitalRead(sensorPin);
  }

}

void drawProgress(OLEDDisplay *display, int percentage, String label) {
  display->clear();
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  display->drawString(64, 10, label);
  display->drawProgressBar(2, 28, 124, 10, percentage);
  display->display();
}

void updateData(OLEDDisplay *display) {
  drawProgress(display, 10, "Updating time...");
  drawProgress(display, 30, "Updating weather...");
  currentWeatherClient.setMetric(IS_METRIC);
  currentWeatherClient.setLanguage(OPEN_WEATHER_MAP_LANGUAGE);
  currentWeatherClient.updateCurrentById(&currentWeather, OPEN_WEATHER_MAP_APP_ID, OPEN_WEATHER_MAP_LOCATION_ID);
  drawProgress(display, 50, "Updating forecasts...");
  forecastClient.setMetric(IS_METRIC);
  forecastClient.setLanguage(OPEN_WEATHER_MAP_LANGUAGE);
  uint8_t allowedHours[] = {12};
  forecastClient.setAllowedHours(allowedHours, sizeof(allowedHours));
  forecastClient.updateForecastsById(forecasts, OPEN_WEATHER_MAP_APP_ID, OPEN_WEATHER_MAP_LOCATION_ID, MAX_FORECASTS);

  readyForWeatherUpdate = false;
  drawProgress(display, 100, "Done...");
  delay(1000);
}

void drawDateTime(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  now = time(nullptr);
  struct tm* timeInfo;
  timeInfo = localtime(&now);
  char buff[16];

  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  String date = WDAY_NAMES[timeInfo->tm_wday];

  sprintf_P(buff, PSTR("%s, %02d/%02d/%04d"), WDAY_NAMES[timeInfo->tm_wday].c_str(), timeInfo->tm_mday, timeInfo->tm_mon + 1, timeInfo->tm_year + 1900);
  display->drawString(64 + x, 5 + y, String(buff));
  display->setFont(ArialMT_Plain_24);

  sprintf_P(buff, PSTR("%02d:%02d:%02d"), timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec);
  display->drawString(64 + x, 15 + y, String(buff));
  display->setTextAlignment(TEXT_ALIGN_LEFT);
}

void drawCurrentWeather(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  display->setFont(ArialMT_Plain_10);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(64 + x, 38 + y, currentWeather.description);

  display->setFont(ArialMT_Plain_24);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  String temp = String(currentWeather.temp, 1) + (IS_METRIC ? "°C" : "°F");
  display->drawString(60 + x, 5 + y, temp);

  display->setFont(Meteocons_Plain_36);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(32 + x, 0 + y, currentWeather.iconMeteoCon);
}


void drawForecast(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  drawForecastDetails(display, x, y, 0);
  drawForecastDetails(display, x + 44, y, 1);
  drawForecastDetails(display, x + 88, y, 2);
}

void drawForecastDetails(OLEDDisplay *display, int x, int y, int dayIndex) {
  time_t observationTimestamp = forecasts[dayIndex].observationTime;
  struct tm* timeInfo;
  timeInfo = localtime(&observationTimestamp);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  display->drawString(x + 20, y, WDAY_NAMES[timeInfo->tm_wday]);

  display->setFont(Meteocons_Plain_21);
  display->drawString(x + 20, y + 12, forecasts[dayIndex].iconMeteoCon);
  String temp = String(forecasts[dayIndex].temp, 0) + (IS_METRIC ? "°C" : "°F");
  display->setFont(ArialMT_Plain_10);
  display->drawString(x + 20, y + 34, temp);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
}

void drawHeaderOverlay(OLEDDisplay *display, OLEDDisplayUiState* state) {
  now = time(nullptr);
  struct tm* timeInfo;
  timeInfo = localtime(&now);
  char buff[14];
  sprintf_P(buff, PSTR("%02d:%02d"), timeInfo->tm_hour, timeInfo->tm_min);

  display->setColor(WHITE);
  display->setFont(ArialMT_Plain_10);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(0, 54, String(buff));
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  String temp = String(currentWeather.temp, 1) + (IS_METRIC ? "°C" : "°F");
  display->drawString(128, 54, temp);
  display->drawHorizontalLine(0, 52, 128);
}

void setReadyForWeatherUpdate() {
  Serial.println("Setting readyForUpdate to true");
  readyForWeatherUpdate = true;
}

// activa los neopixeles
void luces() {
  pixels.clear();
  temperatura();
  cielo();
}

// establece el neopixel 0 segun la temperatura actual
void temperatura() {
  if (currentWeather.temp < 0) {
    //azul turquesa
    pixels.setPixelColor(1, 0, 255, 255);
    pixels.show();
  } else if (currentWeather.temp >= 0 && currentWeather.temp < 10) {
    //verde
    pixels.setPixelColor(1, 0, 255, 0);
    pixels.show();
  } else if (currentWeather.temp >= 10 && currentWeather.temp < 20) {
    //amarillo
    pixels.setPixelColor(1, 255, 255, 0);
    pixels.show();
  } else if (currentWeather.temp >= 20 && currentWeather.temp < 30) {
    //naranja
    pixels.setPixelColor(1, 255, 128, 0);
    pixels.show();
  } else if (currentWeather.temp >= 30 && currentWeather.temp < 40) {
    //rojo
    pixels.setPixelColor(1, 255, 0, 0);
    pixels.show();
  } else if (currentWeather.temp >= 40) {
    //magenta
    pixels.setPixelColor(1, 255, 0, 128);
    pixels.show();
  } else {
    pixels.setPixelColor(1, 0, 0, 0);
    pixels.show();
  }
}

// establece el neopixel 1 segun el estado del cielo actual
void cielo() {
  if (currentWeather.weatherId == 800) {
    //group 800: clear - azul turquesa
    pixels.setPixelColor(0, 0, 255, 255);
    pixels.show();
  } else if (currentWeather.weatherId == 801) {
    // 801: few clouds - azul grisaceo
    pixels.setPixelColor(0, 0, 128, 255);
    pixels.show();
  } else if (currentWeather.weatherId == 802) {
    //802: scattered clouds - azul pocho
    pixels.setPixelColor(0, 200, 200, 255);
    pixels.show();
  } else if (currentWeather.weatherId == 803 || currentWeather.weatherId == 804) {
    //803, 804: broken clouds, overcast clouds - morado
    pixels.setPixelColor(0, 128, 0, 255);
    pixels.show();
  } else if (currentWeather.weatherId >= 300 && currentWeather.weatherId <= 321) {
    //group 3xx: drizzle - azul turquesa parpadea
    pixels.setPixelColor(0, 0, 255, 255);
    pixels.show();
    delay(500);
    pixels.setPixelColor(0, 0, 0, 0);
    pixels.show();
    delay(500);
  } else if (currentWeather.weatherId >= 500 && currentWeather.weatherId <= 531) {
    //group 5xx: rain - azul oscuro parpadea
    pixels.setPixelColor(0, 0, 0, 255);
    pixels.show();
    delay(500);
    pixels.setPixelColor(0, 0, 0, 0);
    pixels.show();
    delay(500);
  } else if (currentWeather.weatherId >= 200 && currentWeather.weatherId <= 232) {
    //group 2xx: thunderstorm - morado y amarillo
    pixels.setPixelColor(0, 128, 0, 255);
    pixels.show();
    delay(500);
    pixels.setPixelColor(0, 0, 0, 0);
    pixels.show();
    delay(500);
    pixels.setPixelColor(0, 255, 255, 0);
    pixels.show();
    delay(500);
    pixels.setPixelColor(0, 0, 0, 0);
    pixels.show();
    delay(500);
  } else if (currentWeather.weatherId >= 600 && currentWeather.weatherId <= 622) {
    //group 6xx: snow - blanco parpadeante
    pixels.setPixelColor(0, 255, 255, 255);
    pixels.show();
    delay(500);
    pixels.setPixelColor(0, 0, 0, 0);
    pixels.show();
    delay(500);
  } else if (currentWeather.weatherId == 701 || currentWeather.weatherId == 711 || currentWeather.weatherId == 721 || currentWeather.weatherId == 741 || currentWeather.weatherId == 771 || currentWeather.weatherId == 781) {
    //701, 711, 721, 741, 771, 781: mist, smoke, haze, fog, squalls, tornado- blanco
    pixels.setPixelColor(0, 255, 255, 255);
    pixels.show();
  } else if (currentWeather.weatherId == 731 || currentWeather.weatherId == 751 || currentWeather.weatherId == 761 || currentWeather.weatherId == 762) {
    //sand/ dust whirls, sand, dust, volcanic ash - naranja
    pixels.setPixelColor(0, 255, 128, 0);
    pixels.show();
  } else {
    pixels.setPixelColor(0, 0, 0, 0);
    pixels.show();
  }
}

void drawMoon() {
  OverlayCallback overlays[] = { drawHeaderOverlay };
  int numberOfOverlays = 1;

  // prepare the input values
  time_t now = time(nullptr);
  struct tm * timeinfo = localtime (&now);

  // now calculate the moon phase by the timestamp
  uint8_t phase = astronomy.calculateMoonPhase(now);


  display.setTextAlignment(TEXT_ALIGN_CENTER);
  
  if (phase == 0) {
    display.drawString(60, 0, "Luna Nueva");
    display.drawXbm(45, 20, 30, 30, new_moon_bits);
    display.display();
  } else if (phase == 1) {
    display.drawString(60, 0, "Creciente");
    display.display();
    display.drawXbm(45, 20, 30, 30, waxing_crescent_bits);
    display.display();
  } else if (phase == 2) {
    display.drawString(60, 0, "Cuarto creciente");
    display.display();
    display.drawXbm(45, 20, 30, 30, first_quarter_bits);
    display.display();
  } else if (phase == 3) {
    display.drawString(60, 0, "Creciente Gibosa");
    display.display();
    display.drawXbm(45, 20, 30, 30, waxing_gibbous_bits);
    display.display();
  } else if (phase == 4) {
    display.drawString(60, 0, "Luna Llena");
    display.display();
    display.drawXbm(45, 20, 30, 30, full_moon_bits);
    display.display();
  } else if (phase == 5) {
    display.drawString(60, 0, "Menguante Gibosa");
    display.display();
    display.drawXbm(45, 20, 30, 30, waning_gibbous_bits);
    display.display();
  } else if (phase == 6) {
    display.drawString(60, 0, "Cuarto Menguante");
    display.display();
    display.drawXbm(45, 20, 30, 30, third_quarter_bits);
    display.display();
  } else if (phase == 7) {
    display.drawString(60, 0, "Menguante");
    display.display();
    display.drawXbm(45, 20, 30, 30, waning_crescent_bits);
    display.display();
  }
}
