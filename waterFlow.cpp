#include <SD.h>
#include <SPI.h>
// #include <FlowMeter.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1305.h>
#include <HX711.h>


#define OLED_RESET 9

#define OLED_ADDR  0x3C


Adafruit_SSD1305 display(128, 64, &Wire, OLED_RESET);


const int flowSensorPin =  2;
const int pressureSensor = A0;
const int LOADCELL_DOUT_PIN = 6;
const int LOADCELL_SCK_PIN =  7;
const int chipSelect = 53;

const int Key = 4;
int keyVal;
int Count = 0;
int counter;
String status;

HX711 myScale;
File dataFile;

volatile int flowPulseCount = 0;

float flowRate;
float flowRate_m3s;
float pressure;
float pressureVal;
float pressureVoltage;
float weight;
bool isLogging = false;
int buttonState;

unsigned long oldTime = 0;
// unsigned long lastTime = 0;
// unsigned long updateInterval = 1000;

// const float flowCalibrationFactor = 5.0;
// const float pressureCalibrationFactor = 0.05;

// const unsigned long period = 100;


// void Meter1ISR() {
//   Meter1.count();
// }
void flowPulseCounter() {
  flowPulseCount++;
}

void setup() {
  Serial.begin(9600);


  pinMode(flowSensorPin, INPUT_PULLUP);
  pinMode(Key, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(flowSensorPin), flowPulseCounter, FALLING);
  

  if ( ! display.begin(0x3C)){
    Serial.println("Unable to initialize OLED");
    while (1) yield();
  }

  // attachInterrupt(INT0, Meter1ISR, RISING);

  // Meter1.reset();

  display.clearDisplay();   // clears the screen and buffer
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(18, 10);
  display.println("Jet Vane");
  display.setCursor(14, 40);
  display.println("Apparatus");
  display.display();
  delay(5000);

  myScale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  myScale.set_scale(2280.f);
  myScale.tare();

  if (!SD.begin(chipSelect)) {
    Serial.println("SD Card initialization failed.");
    delay(2000);
    // return;
  }
  Serial.println("SD Card initialized");
  
}

void logData() {
  File dataFile = SD.open("datalog.txt", FILE_WRITE);
  if (dataFile) {
    dataFile.print(millis());
    dataFile.print(" , ");
    dataFile.print(flowRate);
    dataFile.println(" m³/s");

    dataFile.print(" , ");
    dataFile.print(pressure);
    dataFile.println(" Pa");

    dataFile.print(" , ");
    dataFile.print(weight);
    dataFile.println(" Kg");

    dataFile.close();
  } else {
    Serial.println("Error opening datalog.txt");
  }

  delay(1000);
}


void loop() {
  keyVal = digitalRead(Key);
  if (keyVal == LOW) {
    isLogging = !isLogging;
    delay(2000);
  }

  if (keyVal == HIGH) {
    counter = Count++;
    delay(50);
  }

  Serial.println(digitalRead(Key));

  if (Count > 1) {
    Count = 0;
  }

  if (counter == 0) {
    status = "on";
  }

  if (counter == 1) {
    status = "off";
  }

 
  if (millis() - oldTime > 1000){
    detachInterrupt(digitalPinToInterrupt(flowSensorPin));
    flowRate = ((1000.0 / (millis() - oldTime)) * flowPulseCount) / 7.5;
    flowRate_m3s = flowRate * 2.77778e-10;
    flowPulseCount = 0;

    oldTime = millis();

    pressureVal = analogRead(pressureSensor);
    pressureVoltage = (pressureVal * 5.0) / 1023.0;
    pressure = pressureVoltage * 10.0;

    weight = myScale.get_units(5);

    updateDisplay();
    if (isLogging) {
      logData();
    }
  attachInterrupt(digitalPinToInterrupt(flowSensorPin), flowPulseCounter, FALLING);
  }
}

void updateDisplay() {

  String data = "Data " + status;

  display.clearDisplay();

  display.setCursor(1,1);
  display.setTextSize(1);
  display.println(F("J.V"));

  display.setCursor(78,1);
  display.println(data);

  display.setCursor(3, 20);
  display.print(F("Weight(Kg): "));
  display.print(weight);
  
  display.setCursor(3, 30);
  display.print(F("Flow Rate: "));
  display.print(flowRate_m3s);
  display.println(F(" m3/s"));

  display.setCursor(3,40);
  display.print(F("Pressure: "));
  display.print(pressure);
  display.println(F(" KPa"));

  
  
  display.display();
}
