#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include "DHT.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);

#define REPORTING_PERIOD_MS     1000
 
PulseOximeter pox;
uint32_t tsLastReport = 0;

int heartRate = 0;
int spo2 = 0;

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

#define DHTPIN 4

#define DHTTYPE DHT11  

DHT dht(DHTPIN, DHTTYPE);

#define ONE_WIRE_BUS 32

// Setup a oneWire instance to communicate with any OneWire device
OneWire oneWire(ONE_WIRE_BUS);  

// Pass oneWire reference to DallasTemperature library
DallasTemperature sensors(&oneWire);


const char* ssid = "REPLACE_WITH_YOUR_SSID";
const char* password = "REPLACE_WITH_YOUR_PASSWORD";

//Your Domain name with URL path or IP address with path
const char* serverName = "https://healthmonitoring.nafzasystem.online/api/updatedata";

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 5 seconds (5000)
unsigned long timerDelay = 5000;

int StartState = 0;

int Var = 1;

void onBeatDetected()
{
    Serial.println("Beat!");
    StartState = StartState + 1;
    Serial.println(StartState);
}

void setup() {
  Serial.begin(115200);
  // initialize the LCD
  lcd.begin(); //kalu error kat sini tukar begin jd init exp: lcd.init();
  lcd.backlight();
  dht.begin();

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

  if (!pox.begin()) {
        Serial.println("FAILED");
        for(;;);
    } else {
        Serial.println("SUCCESS");
    }
  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
 
  // Register a callback for the beat detection
  pox.setOnBeatDetectedCallback(onBeatDetected);

  lcd.setCursor(3, 0);
  lcd.print("HEALTH MONITOR");
  lcd.setCursor(0, 1);
  lcd.print("HR = ");
  lcd.setCursor(10, 1);
  lcd.print("SPO2 = ");
  lcd.setCursor(0, 2);
  lcd.print("RT = ");
  lcd.setCursor(10, 2);
  lcd.print("BT = ");
  lcd.setCursor(0, 3);
  lcd.print("RH = ");
}

void loop() {

        switch (Var) {
          case 1:

          pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
           
          pox.update();
      
          // Asynchronously dump heart rate and oxidation levels to the serial
          // For both, a value of 0 means "invalid"
          if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
              Serial.print("Heart rate:");
              Serial.print(pox.getHeartRate());
              Serial.print("bpm / SpO2:");
              Serial.print(pox.getSpO2());
              Serial.println("%");
      
              tsLastReport = millis();
          }

          if (StartState > 10){
            Var = 2;
            }
            
            break;
          case 2:
                sensors.requestTemperatures();
          
                valueHeartRate = heartRate;
                valueSpo2 = spo2;
                valueRoomTemp = dht.readTemperature();
                valueRoomHum = dht.readHumidity();
                valueBodyTemp = sensors.getTempCByIndex(0);
                 
                ValueRoomTemp = String(valueRoomTemp);
                ValueBodyTemp = String(valueBodyTemp);
                ValueRoomHum = String(valueRoomHum);
                ValueHeartRate = String(valueHeartRate);
                ValueSpo2 = String(valueSpo2);
                  
                lcd.setCursor(5, 1);
                lcd.print("    ");
                lcd.setCursor(5, 1);
                lcd.print(ValueHeartRate);
                  
                lcd.setCursor(17, 1);
                lcd.print("   ");
                lcd.setCursor(17, 1);
                lcd.print(ValueSpo2);
                  
                lcd.setCursor(5, 2);
                lcd.print("    ");
                lcd.setCursor(5, 2);
                lcd.print(ValueRoomTemp);
                  
                lcd.setCursor(15, 2);
                lcd.print("    ");
                lcd.setCursor(15, 2);
                lcd.print(ValueBodyTemp);
                  
                lcd.setCursor(5, 3);
                lcd.print("    ");
                lcd.setCursor(5, 3);
                lcd.print(ValueRoomHum);
                  
                PostData();
                StartState = 0;
                Var = 1;
            break;
        }
  
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
