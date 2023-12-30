// for PH

#include <WiFi.h>
#include <ArduinoJson.h>
#define SensorPin 34          // the pH meter Analog output is connected with the Arduino’s Analog
unsigned long int avgValue;  //Store the average value of the sensor feedback
float b;
int buf[10], temp;
long previousMillis = 0;
int interval = 5000; // 5 seconds

// for Turbitary
int sensorPin = 12;
float volt;
float ntu;

// for temperature

#include <OneWire.h>
#include <DallasTemperature.h>
const int oneWireBus = 13;
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(9600);
  sensors.begin();
}

void loop() {
  phFunction();
  turbitaryFunction();
  temperatureFunction();
  delay(500);
}


void phFunction()
{
  for (int i = 0; i < 10; i++) //Get 10 sample value from the sensor for smooth the value
  {
    buf[i] = analogRead(SensorPin);
    delay(10);
  }
  for (int i = 0; i < 9; i++) //sort the analog from small to large
  {
    for (int j = i + 1; j < 10; j++)
    {
      if (buf[i] > buf[j])
      {
        temp = buf[i];
        buf[i] = buf[j];
        buf[j] = temp;
      }
    }
  }
  avgValue = 0;
  for (int i = 2; i < 8; i++)               //take the average value of 6 center sample
    avgValue += buf[i];
  float phValue = (float)avgValue * 5.0 / 1024 / 6; //convert the analog into millivolt
  phValue = (3.5 * phValue) / 10;                //convert the millivolt into pH value
  Serial.print("    pH:");
  Serial.print(phValue, 2);
  Serial.println(" ");
}



void turbitaryFunction() {
  volt = 0;
  for (int i = 0; i < 800; i++) {
    volt += ((float)analogRead(sensorPin) / 4096) * 3.3; // Conversion for 3.3V, as ESP has a 12-bit ADC
  }
  volt = volt / 800;
  volt = round_to_dp(volt, 2);

  // Transformation of voltage values
  volt = mapf(volt, 0.0, 3.3, 1.83, 3.68); // Mapping actual voltage to the required range

  ntu = 493.17 * pow(volt, 2) - 3749.1 * volt + 7152.3; // Calculating NTU using the provided quadratic equation

  // Limiting the NTU value between 0 and 2000
  if (ntu < 0) {
    ntu = 0;
  } else if (ntu > 2000) {
    ntu = 2000;
  }

  Serial.print(volt);
  Serial.print(" V, ");
  Serial.print(ntu);
  Serial.println(" NTU");
  
}

float round_to_dp(float in_value, int decimal_place) {
  float multiplier = powf(10.0f, decimal_place);
  in_value = roundf(in_value * multiplier) / multiplier;
  return in_value;
}

float mapf(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min; // Mapping function for float values
}

void temperatureFunction() {
  sensors.requestTemperatures();
  float temperatureC = sensors.getTempCByIndex(0);
  float temperatureF = sensors.getTempFByIndex(0);
  Serial.print(temperatureC);
  Serial.println("ºC");
  Serial.print(temperatureF);
  Serial.println("ºF");
  
}
