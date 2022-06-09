#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <AHT10.h>
#include <OneWire.h>
#include <DallasTemperature.h>

float valueRoomTemp = 0;
float valueBodyTemp = 0;
int valueRoomHum = 0;
int valueHeartRate = 0;
int valueSpo2 = 0;

String ValueRoomTemp;
String ValueBodyTemp;
String ValueRoomHum;
String ValueHeartRate;
String ValueSpo2;

#define ONE_WIRE_BUS 4

// Setup a oneWire instance to communicate with any OneWire device
OneWire oneWire(ONE_WIRE_BUS);  

// Pass oneWire reference to DallasTemperature library
DallasTemperature sensors(&oneWire);

AHT10             myAHT10(AHT10_ADDRESS_0X38);

#define REPORTING_PERIOD_MS     1000
 
PulseOximeter pox;
uint32_t tsLastReport = 0;

const char* ssid = "REPLACE_WITH_YOUR_SSID";
const char* password = "REPLACE_WITH_YOUR_PASSWORD";

//Your Domain name with URL path or IP address with path
const char* serverName = "http://your-ip-address/api/updatedata";

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 5 seconds (5000)
unsigned long timerDelay = 5000;

void onBeatDetected()
{
    Serial.println("Beat!");
}

void setup() {
  Serial.begin(115200);

  if (!pox.begin()) {
        Serial.println("FAILED");
        for(;;);
    } else {
        Serial.println("SUCCESS");
    }
   pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
 
    // Register a callback for the beat detection
   pox.setOnBeatDetectedCallback(onBeatDetected);

  /* AHT10 connection check */
  while (myAHT10.begin() != true)
  {
    Serial.println(F("AHT10 not connected or fail to load calibration coefficient"));
    delay(5000);
  }

  sensors.begin();  // Start up the library

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
 
  Serial.println("Timer set to 5 seconds (timerDelay variable), it will take 5 seconds before publishing the first reading.");
}

void loop() {
  sensors.requestTemperatures(); 
  pox.update();
    if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
        Serial.print("Heart rate:");
        Serial.print(pox.getHeartRate());
        Serial.print("bpm / SpO2:");
        Serial.print(pox.getSpO2());
        Serial.println("%");
 
        tsLastReport = millis();
    }
    
  valueHeartRate = pox.getHeartRate();
  valueSpo2 = pox.getSpO2();
  valueRoomTemp = myAHT10.readTemperature(AHT10_FORCE_READ_DATA);
  valueRoomHum = myAHT10.readHumidity(AHT10_USE_READ_DATA);
  valueBodyTemp = sensors.getTempCByIndex(0);

  ValueRoomTemp = String(valueRoomTemp);
  ValueBodyTemp = String(valueBodyTemp);
  ValueRoomHum = String(valueRoomHum);
  ValueHeartRate = String(valueHeartRate);
  ValueSpo2 = String(valueSpo2);

  PostData();
}


void PostData(){
  //Send an HTTP POST request every 10 minutes
  if ((millis() - lastTime) > timerDelay) {
    //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      WiFiClient client;
      HTTPClient http;
    
      // Your Domain name with URL path or IP address with path
      http.begin(client, serverName);

      
      // If you need an HTTP request with a content type: application/json, use the following:
      http.addHeader("Content-Type", "application/json");
      int httpResponseCode = http.POST("{\"HeartRate\":\""+ ValueHeartRate +"\",\"Spo2\":\""+ ValueSpo2 +"\",\"BodyTemperature\":\""+ ValueBodyTemp +"\",\"RoomTemperature\":\""+ ValueRoomTemp +"\",\"RoomHumidity\":\""+ ValueRoomHum +"\"}");

      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
        
      // Free resources
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
  }
