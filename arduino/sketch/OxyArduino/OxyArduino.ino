#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "MAX30100.h"
#include "MAX30100_PulseOximeter.h"

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2
#define POLL_PERIOD_US                      1E06 / 100
#define SAMPLING_RATE                       MAX30100_SAMPRATE_100HZ
#define IR_LED_CURRENT                      MAX30100_LED_CURR_50MA
#define RED_LED_CURRENT                     MAX30100_LED_CURR_27_1MA
#define PULSE_WIDTH                         MAX30100_SPC_PW_1600US_16BITS
#define HIGHRES_MODE                        true


MAX30100 sensor;
PulseOximeter pox;
uint32_t tsLastPollUs = 0;
double isum;
double rsum;
bool first = true;

long redAverage;
long iredAverage;
float ShO2v1;
float ShO2v2;
long redValue;
long iredValue;

int qual = 20;
void setup()
{

  Serial.begin(115200);
  Serial.print("Initializing MAX30100..");
  if (!sensor.begin()) {
    Serial.println("FAILED");
    for (;;);
  } else {
    Serial.println("SUCCESS");
  }
  sensor.setMode(MAX30100_MODE_SPO2);
  sensor.setLedsCurrent(IR_LED_CURRENT, RED_LED_CURRENT);
  sensor.setLedsPulseWidth(PULSE_WIDTH);
  sensor.setSamplingRate(SAMPLING_RATE);
  sensor.setHighresModeEnabled(HIGHRES_MODE);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  //display.display();
  display.clearDisplay();

}
void loop() {

  averageValues();
  sendToCom();
  sendToMon();

}

void averageValues() {
  int i = 0;
  int e = 0;
  long red;
  long ired;
  updateValues();

  while (i < qual) {
    updateValues();
    if (abs((red / i) - redValue) / (red / i) < 0.15 && abs((ired / i) - iredValue) / (ired / i) < 0.15) {
      red = red + redValue;
      ired = ired + iredValue;

      i++;
    } else {
      e++;
      if (e > i / 3) {
        e = 0;
        i = 0;
      }
    }
  }

  if (red < 100000 || ired < 100000) {
    redAverage = 0;
    iredAverage = 0;
    ShO2v1 = 0;
    ShO2v2 = 0;
  } else {
    redAverage = red / qual;
    iredAverage = ired / qual;
    ShO2v1 = (float)redAverage / iredAverage * 100;
    ShO2v2 = (float)redAverage / (redAverage + iredAverage) * 100;
  }


}

void updateValues() {
  while (true) {
    if (micros() < tsLastPollUs || micros() - tsLastPollUs > POLL_PERIOD_US) {
      sensor.update();
      tsLastPollUs = micros();
      iredValue = sensor.rawIRValue;
      redValue = sensor.rawRedValue;
      return;
    }
  }
}

void sendToCom() {

  Serial.print("RedValue::");
  Serial.print(redAverage);
  Serial.print("##");
  Serial.print("IRedValue::");
  Serial.print(iredAverage);
  Serial.print("##");
  Serial.print("ShO2v1::");
  Serial.print(ShO2v1);
  Serial.print("##");
  Serial.print("ShO2v2::");
  Serial.print(ShO2v2);
  Serial.println("##");

}

void sendToMon() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print("Red:   ");
  display.println(redAverage);
  display.print("Ired:  ");
  display.println(iredAverage);
  display.print("ShO2v1:");
  display.println(ShO2v1);
  display.print("ShO2v2:");
  display.println(ShO2v2);
  display.display();
}


