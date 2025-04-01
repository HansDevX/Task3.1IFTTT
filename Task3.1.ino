//library
#include <WiFiNINA.h>
#include <BH1750.h>
#include <ArduinoHttpClient.h>
#include <Wire.h>

// WiFi Password and Name
const char* ssid = "Mytest";          
const char* pass = "kavinduha";      

// WiFi client for HTTP requests and BH1750 light sensor object
WiFiClient wifiClient;                
BH1750 lightSensor;               

// IFTTT configuration
const char* host = "maker.ifttt.com";  
const char* key = "f9_5JBgMwCXab1a0M86Oi";  
const int http_Port = 80;  

// Light level threshold to detect sunlight             
const int sunlight = 700;
// Light level threshold to detect darkness
const int darkness = 300; 
// Check if sunlight was previously detected   
bool sunlightDetected = false;         

void setup() {
  Serial.begin(9600);          
  while (!Serial);            

  Wire.begin();               
  lightSensor.begin();       

  connectToWiFi();    
}  

void loop() {
  
  if (WiFi.status() != WL_CONNECTED) {
    connectToWiFi();            
  }

  // Read light level from the sensor
  int lightLevel = lightSensor.readLightLevel();
  Serial.print("Light Level: ");
  Serial.println(lightLevel);    

  // Check if sunlight is detected
  if (lightLevel > sunlight && !sunlightDetected) {
    sendToIFTTT("sunlight_detected");   
    sunlightDetected = true;             
  } 
  // Check if sunlight has stopped
  else if (lightLevel < darkness && sunlightDetected) {
    sendToIFTTT("sunlight_stopped");     
    sunlightDetected = false;            
  }

  delay(3000);  // 3 seconds delay
}

void connectToWiFi() {
  
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, pass);          
  unsigned long startTime = millis();  
  
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - startTime > 30000) {  // Wifi Timeout 30 sec
      Serial.println("WiFi connection failed.");
      return;   
    }
    Serial.print(".");  
    delay(5000);         
  }

  Serial.println("WiFi connected successfully.");
}

void sendToIFTTT(const String& eventName) {
  Serial.println("Sending event to IFTTT...");

  if (wifiClient.connect(host, http_Port)) {
    Serial.println("Connected to IFTTT server.");

    String url = "/trigger/" + eventName + "/with/key/" + key;
    
    // Send HTTP GET request to IFTTT
    wifiClient.print("GET " + url + " HTTP/1.1\r\n");
    wifiClient.print("Host: " + String(host) + "\r\n");
    wifiClient.print("Connection: close\r\n\r\n");

    unsigned long timeout = millis();  
    while (wifiClient.connected() && millis() - timeout < 5000) {
      if (wifiClient.available()) {
        String response = wifiClient.readStringUntil('\n');  
        if (response.startsWith("{\"status\":\"ok\"}")) {
          Serial.println("Event successfully sent to IFTTT.");
          break; 
        }
      }
    }
    wifiClient.stop(); 
  } else {
    Serial.println("Failed to connect to IFTTT server.");
  }
}
