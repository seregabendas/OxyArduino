#include <ArduinoJson.h>


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
    for (;;);
  } else {
    Serial.println("SUCCESS");
  }
  sensor.setMode(MAX30100_MODE_SPO2_HR);
  sensor.setLedsCurrent(IR_LED_CURRENT, RED_LED_CURRENT_OFF);
  sensor.setHighresModeEnabled(true);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
}

void loop() {
  unsigned long start = millis();
  enableRedLed(checkIrLed());
  if (checkIrLed()) {
    int ir, red;
    while (!getAvarageValues(&ir, &red, 300)) {
      if (!checkIrLed()) {
        return;
      }
    }
    int ShO2 = calculateShO2(ir, red);
    JsonObject& json = getJsonT(ir, red, ShO2);
    json.printTo(Serial);
    Serial.println();
    printToMonitorVal(ir, red);
  } else {
    JsonObject& json = getJsonF();
    json.printTo(Serial);
    Serial.println();
    printToMonitorText("Put finger");
  }
}

JsonObject& getJsonF() {
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["device"] = "Tyrol_v2";
  root["status"] = false;
  return root;
}

JsonObject& getJsonT(int ir, int red, int ShO2) {
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  JsonObject& values = jsonBuffer.createObject();

  root["device"] = "Tyrol_v2";
  root["status"] = true;
  values["ir"] = ir;
  values["red"] = red;
  values["ShO2"] = ShO2;
  root["values"] = values;
  return root;
}
int calculateShO2(long ir, long red) {
  return (float)red / (ir + red) * 100;
}
boolean getAvarageValues(int *irResult, int *redResult, int num) {
  long irRes = 0;
  long redRes = 0;
  for (int i = 0; i < num; i++) {
    uint16_t ir, red;
    sensor.update();
    sensor.getRawValues(&ir, &red);

    irRes = irRes + ir;
    redRes = redRes + red;
  }
  *irResult = irRes / num;
  *redResult = redRes / num;
  return true;
}
boolean checkIrLed() {
  uint16_t ir, red;
  sensor.update();
  sensor.getRawValues(&ir, &red);
  return ir > 10000;
}
void enableRedLed(boolean isEnable) {
  if (isEnable) {
    sensor.setLedsCurrent(IR_LED_CURRENT, RED_LED_CURRENT_ON);
  } else if (!isEnable) {
    sensor.setLedsCurrent(IR_LED_CURRENT, RED_LED_CURRENT_OFF);
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

void printToMonitorText(String text) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 5);
  display.print(text);
  display.display();

}


