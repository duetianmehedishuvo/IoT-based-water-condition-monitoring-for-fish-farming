// firebase
#include <WiFi.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <Firebase_ESP_Client.h>

#include "time.h"

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "Md. Latifur's iPhone"
#define WIFI_PASSWORD "iPhone654123"

// Insert Firebase project API Key
#define API_KEY "AIzaSyBAmPVqs7Zrv7JJXhKixDe4j1LIRDio7Zg"

// Insert Authorized Email and Corresponding Password
#define USER_EMAIL "duetianmehedishuvo@gmail.com"
#define USER_PASSWORD "123456"

// Insert RTDB URLefine the RTDB URL
#define DATABASE_URL "kasem-sir-default-rtdb.firebaseio.com"

// Define Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;


// Variable to save USER UID
String uid;

// Database main path (to be updated in setup with the user UID)
String databasePath;
// Database child nodes
String tempPath = "/temperature";
String timePath = "/timestamp";



int timestamp;
FirebaseJson json;

const char* ntpServer = "pool.ntp.org";
// Timer variables (send new readings every three minutes)
unsigned long sendDataPrevMillis = 0;
unsigned long timerDelay = 180000;

// Initialize WiFi
void initWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  Serial.println();
}

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

String parentPath;

unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    //Serial.println("Failed to obtain time");
    return (0);
  }
  time(&now);
  return now;
}

void setup() {
  Serial.begin(9600);

  initWiFi();
  configTime(0, 0, ntpServer);
  config.api_key = API_KEY;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.database_url = DATABASE_URL;

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  config.max_token_generation_retry = 5;
  Firebase.begin(&config, &auth);
  Serial.println("Getting User UID");
  while ((auth.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  }

  sensors.begin();
}

void loop() {


  timestamp = getTime();
  parentPath = "/device1/" + String(timestamp) + "/";

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

  if (Firebase.RTDB.setInt(&fbdo, parentPath + "ph_voltage", avgValue)) {
    Serial.println("Successfully SAVE TO " + fbdo.dataPath() + " (" + fbdo.dataType() + ")");
  } else {
    Serial.println("FAILED : \n" + fbdo.errorReason());
  }

  float phValue = (float)avgValue * 5.0 / 1024 / 6; //convert the analog into millivolt
  phValue = (3.5 * phValue) / 10;                //convert the millivolt into pH value

  if (Firebase.RTDB.setFloat(&fbdo, parentPath + "ph_value", phValue)) {
    Serial.println("Successfully SAVE TO " + fbdo.dataPath() + " (" + fbdo.dataType() + ")");
  } else {
    Serial.println("FAILED : \n" + fbdo.errorReason());
  }


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

  if (Firebase.RTDB.setInt(&fbdo, parentPath + "turbidity_voltage", volt)) {
    Serial.println("Successfully SAVE TO " + fbdo.dataPath() + " (" + fbdo.dataType() + ")");
  } else {
    Serial.println("FAILED : \n" + fbdo.errorReason());
  }

  if (Firebase.RTDB.setFloat(&fbdo, parentPath + "turbidity_value", ntu)) {
    Serial.println("Successfully SAVE TO " + fbdo.dataPath() + " (" + fbdo.dataType() + ")");
  } else {
    Serial.println("FAILED : \n" + fbdo.errorReason());
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

  if (Firebase.RTDB.setFloat(&fbdo, parentPath + "temp_c", temperatureC)) {
    Serial.println("Successfully SAVE TO " + fbdo.dataPath() + " (" + fbdo.dataType() + ")");
  } else {
    Serial.println("FAILED : \n" + fbdo.errorReason());
  }

  if (Firebase.RTDB.setFloat(&fbdo, parentPath + "temp_f", temperatureF)) {
    Serial.println("Successfully SAVE TO " + fbdo.dataPath() + " (" + fbdo.dataType() + ")");
  } else {
    Serial.println("FAILED : \n" + fbdo.errorReason());
  }
}
