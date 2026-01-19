/*
 Pocket ESP32 Tool v1.5 FINAL REAL
 Autor: Tutui425
 Licença: MIT
*/

#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Update.h>
#include <Bounce2.h>
#include <EEPROM.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <RTClib.h>

/* ===== CONFIG ===== */
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C
#define EEPROM_SIZE 64

/* ===== PINOS ===== */
#define BTN_UP     32
#define BTN_DOWN   33
#define BTN_LEFT   25
#define BTN_RIGHT  26
#define BTN_OK     27
#define BTN_BACK   14

#define LED_STATUS 2
#define LED1       4
#define LED2       5
#define BUZZER_PIN 13
#define BAT_PIN    34

/* ===== BATERIA ===== */
#define ADC_MAX     4095.0
#define BAT_DIVIDER 2.0
#define BAT_MIN     3.0
#define BAT_MAX     4.2
#define BAT_CAP_MAH 1200.0

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
RTC_DS3231 rtc;
WebServer server(80);

/* ===== BOTÕES ===== */
Bounce bUp, bDown, bLeft, bRight, bOk, bBack;

/* ===== SETTINGS ===== */
struct Settings {
  uint8_t brightness;
  bool sound;
} settings;

/* ===== ESTADO ===== */
bool screenOn = true;
bool powerSave = false;
unsigned long lastInteraction = 0;

/* ===== APPS ===== */
enum App {
  APP_MENU,
  APP_CLOCK,
  APP_CALENDAR,
  APP_TEMP,
  APP_STOPWATCH,
  APP_TIMER,
  APP_BATTERY,
  APP_SYSTEM,
  APP_SNAKE,
  APP_SETTINGS
};
App currentApp = APP_MENU;

/* ===== MENU ===== */
const char* menuItems[] = {
  "Relogio","Calendario","Temperatura","Cronometro","Timer",
  "Bateria","Sistema","Snake","Config"
};
const uint8_t MENU_COUNT = 9;
uint8_t menuIndex = 0;

/* ===== UTIL ===== */
void beep(uint16_t d = 80) {
  if (settings.sound) tone(BUZZER_PIN, 3000, d);
}

void wakeScreen() {
  screenOn = true;
  powerSave = false;
  display.ssd1306_command(SSD1306_DISPLAYON);
  lastInteraction = millis();
}

/* ===== BATERIA ===== */
float readBattery() {
  int raw = analogRead(BAT_PIN);
  return (raw / ADC_MAX) * 3.3 * BAT_DIVIDER;
}

int batteryPercent() {
  float v = readBattery();
  if (v <= BAT_MIN) return 0;
  if (v >= BAT_MAX) return 100;
  return (int)((v - BAT_MIN) * 100.0 / (BAT_MAX - BAT_MIN));
}

float batteryHours() {
  float pct = batteryPercent() / 100.0;
  float mah = pct * BAT_CAP_MAH;
  float current = powerSave ? 25.0 : 70.0;
  return mah / current;
}

/* ===== EEPROM ===== */
void loadSettings() {
  EEPROM.get(0, settings);
  if (settings.brightness < 10 || settings.brightness > 255) {
    settings.brightness = 180;
    settings.sound = true;
  }
}

void saveSettings() {
  EEPROM.put(0, settings);
  EEPROM.commit();
}

/* ===== OTA ===== */
void setupOTA() {
  WiFi.softAP("PocketESP", "12345678");

  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html",
      "<form method='POST' action='/update' enctype='multipart/form-data'>"
      "<input type='file' name='update'>"
      "<input type='submit' value='Update'></form>");
  });

  server.on("/update", HTTP_POST,
    []() {
      server.send(200, "text/plain", Update.hasError() ? "FAIL" : "OK");
      ESP.restart();
    },
    []() {
      HTTPUpload& u = server.upload();
      if (u.status == UPLOAD_FILE_START) Update.begin();
      else if (u.status == UPLOAD_FILE_WRITE) Update.write(u.buf, u.currentSize);
      else if (u.status == UPLOAD_FILE_END) Update.end(true);
    }
  );

  server.begin();
}

/* ===== APPS ===== */

void drawMenu() {
  if (bUp.fell())   menuIndex = (menuIndex + MENU_COUNT - 1) % MENU_COUNT;
  if (bDown.fell()) menuIndex = (menuIndex + 1) % MENU_COUNT;
  if (bOk.fell()) { currentApp = (App)(menuIndex + 1); beep(); }

  for (int i = 0; i < MENU_COUNT; i++) {
    if (i == menuIndex) {
      display.fillRect(0, i * 8, 128, 8, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
    } else display.setTextColor(SSD1306_WHITE);
    display.setCursor(2, i * 8);
    display.print(menuItems[i]);
  }
}

void drawClock() {
  DateTime n = rtc.now();
  display.setTextSize(2);
  display.setCursor(0, 18);
  display.printf("%02d:%02d", n.hour(), n.minute());
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.printf("%02d/%02d/%04d", n.day(), n.month(), n.year());
  if (bBack.fell()) currentApp = APP_MENU;
}

void drawCalendar() {
  DateTime n = rtc.now();
  display.setCursor(0,0);
  display.printf("%02d/%04d", n.month(), n.year());
  int d = n.day(), c = 0, r = 0;
  for (int i = 1; i <= 31; i++) {
    int x = c * 18, y = 16 + r * 8;
    if (i == d) {
      display.fillRect(x, y, 16, 8, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
    } else display.setTextColor(SSD1306_WHITE);
    display.setCursor(x, y);
    display.printf("%2d", i);
    c++; if (c > 6) { c = 0; r++; }
  }
  if (bBack.fell()) currentApp = APP_MENU;
}

void drawTemp() {
  display.setTextSize(2);
  display.setCursor(0, 18);
  display.printf("%.1fC", rtc.getTemperature());
  if (bBack.fell()) currentApp = APP_MENU;
}

/* ===== STOPWATCH ===== */
bool swRun = false;
unsigned long swStart = 0, swAcc = 0;

void drawStopwatch() {
  if (bOk.fell()) {
    if (!swRun) swStart = millis();
    else swAcc += millis() - swStart;
    swRun = !swRun;
    beep();
  }
  if (bBack.fell()) {
    swRun = false; swAcc = 0; currentApp = APP_MENU;
  }
  unsigned long t = swAcc + (swRun ? millis() - swStart : 0);
  display.setTextSize(2);
  display.setCursor(0, 18);
  display.printf("%02lu:%02lu", t / 60000, (t / 1000) % 60);
}

/* ===== TIMER ===== */
int tMin = 1;
bool tRun = false;
unsigned long tStart = 0;

void drawTimer() {
  if (!tRun) {
    if (bUp.fell()) tMin++;
    if (bDown.fell() && tMin > 1) tMin--;
  }
  if (bOk.fell()) {
    tRun = !tRun;
    if (tRun) tStart = millis();
    beep();
  }
  if (bBack.fell()) { tRun = false; currentApp = APP_MENU; }

  unsigned long total = tMin * 60000UL;
  unsigned long rem = tRun ? (millis() - tStart >= total ? 0 : total - (millis() - tStart)) : total;

  if (rem == 0 && tRun) {
    tRun = false;
    beep(300);
  }

  display.setTextSize(2);
  display.setCursor(0, 18);
  display.printf("%02lu:%02lu", rem / 60000, (rem / 1000) % 60);
}

void drawBattery() {
  display.setTextSize(2);
  display.setCursor(0, 18);
  display.printf("%d%%", batteryPercent());
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.printf("%.2fV %.1fh", readBattery(), batteryHours());
  if (bBack.fell()) currentApp = APP_MENU;
}

void drawSystem() {
  display.setCursor(0,0);
  display.printf("Heap:%u", ESP.getFreeHeap());
  display.setCursor(0,10);
  display.printf("CPU:%dMHz", getCpuFrequencyMhz());
  display.setCursor(0,20);
  display.printf("WiFi:%s", WiFi.softAPgetStationNum() ? "ON" : "OFF");
  if (bBack.fell()) currentApp = APP_MENU;
}

/* ===== SNAKE ===== */
struct P { int x,y; };
P snake[32], food;
int sLen = 5, dx = 1, dy = 0;
unsigned long sT = 0;

void resetSnake() {
  sLen = 5; dx = 1; dy = 0;
  for (int i = 0; i < sLen; i++) snake[i] = {64 - i * 4, 32};
  food = {random(0,32)*4, random(0,16)*4};
}

void drawSnake() {
  if (bBack.fell()) { currentApp = APP_MENU; return; }
  if (bUp.fell())    { dx = 0; dy = -1; }
  if (bDown.fell())  { dx = 0; dy = 1; }
  if (bLeft.fell())  { dx = -1; dy = 0; }
  if (bRight.fell()) { dx = 1; dy = 0; }

  if (millis() - sT > 200) {
    sT = millis();
    for (int i = sLen - 1; i > 0; i--) snake[i] = snake[i - 1];
    snake[0].x += dx * 4;
    snake[0].y += dy * 4;

    if (snake[0].x < 0 || snake[0].x > 124 || snake[0].y < 0 || snake[0].y > 60)
      resetSnake();

    for (int i = 1; i < sLen; i++)
      if (snake[0].x == snake[i].x && snake[0].y == snake[i].y)
        resetSnake();

    if (snake[0].x == food.x && snake[0].y == food.y && sLen < 31) {
      sLen++;
      food = {random(0,32)*4, random(0,16)*4};
      beep();
    }
  }

  for (int i = 0; i < sLen; i++)
    display.fillRect(snake[i].x, snake[i].y, 4, 4, SSD1306_WHITE);
  display.fillRect(food.x, food.y, 4, 4, SSD1306_WHITE);
}

void drawSettings() {
  static uint8_t idx = 0;
  if (bUp.fell() || bDown.fell()) idx = !idx;
  if (bLeft.fell() || bRight.fell()) {
    if (idx == 0)
      settings.brightness = constrain(settings.brightness + (bRight.fell()?10:-10), 10, 255);
    else settings.sound = !settings.sound;
    display.ssd1306_command(SSD1306_SETCONTRAST);
    display.ssd1306_command(settings.brightness);
    saveSettings();
    beep();
  }
  display.setCursor(0,0);
  display.printf("%cBrilho:%d", idx==0?'>':' ', settings.brightness);
  display.setCursor(0,12);
  display.printf("%cSom:%s", idx==1?'>':' ', settings.sound?"ON":"OFF");
  if (bBack.fell()) currentApp = APP_MENU;
}

/* ===== SETUP ===== */
void setup() {
  EEPROM.begin(EEPROM_SIZE);
  loadSettings();

  pinMode(LED_STATUS, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  Wire.begin();
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.clearDisplay();
  display.ssd1306_command(SSD1306_SETCONTRAST);
  display.ssd1306_command(settings.brightness);

  rtc.begin();

  bUp.attach(BTN_UP, INPUT_PULLUP);     bUp.interval(10);
  bDown.attach(BTN_DOWN, INPUT_PULLUP); bDown.interval(10);
  bLeft.attach(BTN_LEFT, INPUT_PULLUP); bLeft.interval(10);
  bRight.attach(BTN_RIGHT, INPUT_PULLUP);bRight.interval(10);
  bOk.attach(BTN_OK, INPUT_PULLUP);     bOk.interval(10);
  bBack.attach(BTN_BACK, INPUT_PULLUP); bBack.interval(10);

  setupOTA();
  resetSnake();
  wakeScreen();
}

/* ===== LOOP ===== */
void loop() {
  bUp.update(); bDown.update(); bLeft.update();
  bRight.update(); bOk.update(); bBack.update();

  digitalWrite(LED_STATUS, millis()/500 % 2);
  digitalWrite(LED1, powerSave);
  digitalWrite(LED2, batteryPercent() < 20);

  if (millis() - lastInteraction > 15000 && screenOn) {
    powerSave = true;
    screenOn = false;
    display.ssd1306_command(SSD1306_DISPLAYOFF);
  }

  if (!screenOn) {
    if (bOk.read() == LOW) {
      static unsigned long t = 0;
      if (!t) t = millis();
      if (millis() - t > 3000) { wakeScreen(); t = 0; }
    } else { static unsigned long t = 0; t = 0; }
    delay(50);
    return;
  }

  lastInteraction = millis();
  server.handleClient();
  display.clearDisplay();

  switch (currentApp) {
    case APP_MENU: drawMenu(); break;
    case APP_CLOCK: drawClock(); break;
    case APP_CALENDAR: drawCalendar(); break;
    case APP_TEMP: drawTemp(); break;
    case APP_STOPWATCH: drawStopwatch(); break;
    case APP_TIMER: drawTimer(); break;
    case APP_BATTERY: drawBattery(); break;
    case APP_SYSTEM: drawSystem(); break;
    case APP_SNAKE: drawSnake(); break;
    case APP_SETTINGS: drawSettings(); break;
    default: currentApp = APP_MENU;
  }

  display.display();
  delay(powerSave ? 20 : 10); // até ~50 FPS
}
