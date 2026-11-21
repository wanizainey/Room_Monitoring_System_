
#include <Arduino.h>
#include <Wire.h>
#include <DHT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ===== PIN CONFIGURATION =====
#define DHTPIN 4
#define DHTTYPE DHT11

#define PIR_PIN 26
#define TOUCH_PIN 27
#define TOUCH_THRESHOLD 700

#define SDA_PIN 21
#define SCL_PIN 22

// ===== OLED CONFIGURATION =====
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

DHT dht(DHTPIN, DHTTYPE);

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  OLED_RESET
);

// ===== SENSOR VARIABLES =====
float temperature = NAN;
float humidity = NAN;
bool motionDetected = false;

// 0 = DHT11 Page
// 1 = PIR Page
int currentPage = 0;

// ===== TIMING =====
unsigned long lastDHTRead = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long lastTouchTime = 0;

const unsigned long DHT_INTERVAL = 2000;
const unsigned long DISPLAY_INTERVAL = 200;
const unsigned long TOUCH_DEBOUNCE = 500;

// Touch state
bool touchWasPressed = false;

// ===== SETUP =====
void setup() {
  Serial.begin(115200);

  dht.begin();
  pinMode(PIR_PIN, INPUT);

  Wire.begin(SDA_PIN, SCL_PIN);

  if (!display.begin(
      SSD1306_SWITCHCAPVCC,
      SCREEN_ADDRESS
  )) {
    Serial.println("OLED initialization failed!");
    while (true) {
      delay(1000);
    }
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println("SMART ROOM SYSTEM");
  display.println("-----------------");
  display.println("Initializing...");
  display.display();

  Serial.println("SMART ROOM MONITOR");
  Serial.println("System Started!");

  delay(1000);
}

// ===== READ SENSORS =====
void readSensors() {

  // PIR continuously checked
  motionDetected = digitalRead(PIR_PIN) == HIGH;

  // DHT11 every 2 seconds
  if (millis() - lastDHTRead >= DHT_INTERVAL) {

    lastDHTRead = millis();

    float newTemperature = dht.readTemperature();
    float newHumidity = dht.readHumidity();

    if (!isnan(newTemperature) &&
        !isnan(newHumidity)) {

      temperature = newTemperature;
      humidity = newHumidity;

      Serial.print("Temperature: ");
      Serial.print(temperature, 1);
      Serial.println(" C");

      Serial.print("Humidity: ");
      Serial.print(humidity, 1);
      Serial.println(" %");

    } else {
      Serial.println("DHT11 reading failed!");
    }

    Serial.print("Motion: ");
    Serial.println(
      motionDetected ? "Detected" : "No Motion"
    );

    Serial.println("--------------------");
  }
}

// ===== TOUCH DETECTION =====
void checkTouch() {

  int touchValue = touchRead(TOUCH_PIN);

  bool isTouched = touchValue < TOUCH_THRESHOLD;

  // Switch only once per touch
  if (isTouched && !touchWasPressed &&
      millis() - lastTouchTime >= TOUCH_DEBOUNCE) {

    currentPage = (currentPage + 1) % 2;

    lastTouchTime = millis();

    Serial.print("Touch Detected! Page: ");
    Serial.println(currentPage + 1);
  }

  touchWasPressed = isTouched;
}

// ===== OLED PAGE 1 =====
void showDHTPage() {

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("ROOM MONITOR - P1");

  display.drawLine(
    0, 12, 127, 12, SSD1306_WHITE
  );

  display.setCursor(0, 18);
  display.println("Temperature:");

  display.setTextSize(2);
  display.setCursor(0, 29);

  if (isnan(temperature)) {
    display.println("--");
  } else {
    display.print(temperature, 1);
    display.write(247);
    display.println("C");
  }

  display.setTextSize(1);
  display.setCursor(0, 52);

  display.print("Humidity: ");

  if (isnan(humidity)) {
    display.println("--");
  } else {
    display.print(humidity, 1);
    display.println("%");
  }
}

// ===== OLED PAGE 2 =====
void showPIRPage() {

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("MOTION MONITOR - P2");

  display.drawLine(
    0, 12, 127, 12, SSD1306_WHITE
  );

  display.setCursor(0, 22);
  display.println("PIR Sensor Status:");

  display.setTextSize(2);
  display.setCursor(0, 37);

  if (motionDetected) {
    display.println("MOTION!");
  } else {
    display.println("NO MOTION");
  }
}

// ===== UPDATE OLED =====
void updateDisplay() {

  if (millis() - lastDisplayUpdate <
      DISPLAY_INTERVAL) {
    return;
  }

  lastDisplayUpdate = millis();

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  if (currentPage == 0) {
    showDHTPage();
  } else {
    showPIRPage();
  }

  display.display();
}

// ===== MAIN LOOP =====
void loop() {

  readSensors();

  checkTouch();

  updateDisplay();
}
