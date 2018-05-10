
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <MAX30100.h>

#define IR_LED_CURRENT        MAX30100_LED_CURR_20_8MA
#define RED_LED_CURRENT_ON    MAX30100_LED_CURR_20_8MA
#define RED_LED_CURRENT_OFF   MAX30100_LED_CURR_0MA
#define OLED_RESET            4
#define SCANNING              "Scanning..."
Adafruit_SSD1306 display(OLED_RESET);
MAX30100 sensor;
boolean isEnable;
void setup() {
  Serial.begin(115200);
  Serial.print("Initializing MAX30100..");

  if (!sensor.begin()) {
    Serial.println("FAILED");
  } else {
    Serial.println("SUCCESS");
  }
  sensor.setMode(MAX30100_MODE_SPO2_HR);
  sensor.setLedsCurrent(IR_LED_CURRENT, RED_LED_CURRENT_OFF);
  isEnable = false;
  sensor.setHighresModeEnabled(true);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
}

void loop() {

  uint16_t ir, red;
  sensor.update();
  sensor.getRawValues(&ir, &red);
  isEnable = enableRedLed(ir, isEnable);
  if (isEnable) {
    printToMonitorVal(ir, red);
    printToSerialVal(ir, red);
  } else {
    printToMonitorScan();
  }
}
boolean enableRedLed(uint16_t ir, boolean isEnable) {
  if (isEnable && ir < 10000) {
    sensor.setLedsCurrent(IR_LED_CURRENT, RED_LED_CURRENT_OFF);
    return false;
  } else if (!isEnable && ir > 10000) {
    sensor.setLedsCurrent(IR_LED_CURRENT, RED_LED_CURRENT_ON);
    return true;
  }
}
void printToSerialVal(uint16_t ir, uint16_t red) {
  Serial.println(ir);
  Serial.println(red);
  Serial.println();
}

void printToMonitorVal(uint16_t ir, uint16_t red) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print("Red: ");
  display.println(red);
  display.print("Ired:");
  display.println(ir);
  display.display();
}

void printToMonitorScan() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 5);
  display.print("Put finger");
  display.display();
  
}


