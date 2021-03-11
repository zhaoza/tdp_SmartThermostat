#include <ESP8266WiFi.h>  //controller library
#include <dht11.h>  //sensor library
#include "ThingSpeak.h" //IoT library

//Device pins
#define dhtpin 4  //device pin = D2
#define RELAY 5 //device pin = D1

// WiFi details
const char WIFI_SSID[] = "###";
const char WIFI_PASS[] = "###";

//ThingSpeak parameters
unsigned long CHANNEL_ID = ###;
const char * WRITE_API_KEY = "###";
const char * READ_API_KEY = "###";

// Global variables
WiFiClient client;
dht11 DHT11; 

void setup() {
  Serial.begin(9600);
  connectWiFi();
  ThingSpeak.begin(client);
  pinMode(RELAY, OUTPUT);
}

void loop() {
  unsigned long startTime = millis(), totalTime;
  
  DHT11.read(dhtpin);
  short temp = DHT11.temperature;
  short hum = DHT11.humidity;
  
  short online_temp = ThingSpeak.readIntField(CHANNEL_ID, 1, READ_API_KEY);
  short online_hum = ThingSpeak.readIntField(CHANNEL_ID, 2, READ_API_KEY);
  bool online_relay = ThingSpeak.readIntField(CHANNEL_ID, 3, READ_API_KEY);
  bool autoON = ThingSpeak.readIntField(CHANNEL_ID, 4, READ_API_KEY);
  short autoTemp = ThingSpeak.readIntField(CHANNEL_ID, 5, READ_API_KEY);
  bool sendState = false;
  
  relayControl(autoON, temp, autoTemp, sendState, online_relay);
  readingsChange(temp, online_temp, hum, online_hum, sendState);
  readingsOutput(temp, hum , autoON, autoTemp);
  
  if (sendState) {
    ThingSpeak.writeFields(CHANNEL_ID, WRITE_API_KEY);
    Serial.print("Data sent\n");
  }

  if (WiFi.status() != WL_CONNECTED)
    connectWiFi();
    
  //sets a delay depending on the speed of the previous operations
  unsigned long endTime = millis();
  totalTime = ((endTime - startTime) > 15500) ? 15500 : (endTime - startTime);
  delay(15500 - totalTime);
}

void relayControl(bool& autoON, short& temp, short& autoTemp, bool& sendState, bool& online_relay) {
  if (autoON) {
    if (((temp < autoTemp) && (digitalRead(RELAY) != HIGH)) || ((online_relay == false) && (temp < autoTemp))) {
      ThingSpeak.setField(3, true);
      sendState = true;
      digitalWrite(RELAY, HIGH);  
    }
    else if (((temp >= autoTemp) && (digitalRead(RELAY) != LOW)) || ((online_relay == true) && (temp >= autoTemp))) {
      ThingSpeak.setField(3, false);
      sendState = true;
      digitalWrite(RELAY, LOW); 
    }  
  }
  else {
    if ((online_relay) && (digitalRead(RELAY) != HIGH))
      digitalWrite(RELAY, HIGH);
    else if ((online_relay == false) && (digitalRead(RELAY) != LOW))
      digitalWrite(RELAY, LOW);
  }
}

void readingsChange(short& temp, short& online_temp, short& hum, short& online_hum, bool& sendState) {
  if (online_temp != temp){
    ThingSpeak.setField(1, temp);
    sendState = true;
  }
  if (online_hum != hum){
    ThingSpeak.setField(2, hum);
    sendState = true;
  }
}

void readingsOutput(short& temp, short& hum, bool& autoON, short& autoTemp){
  Serial.print(temp);
  Serial.print("°C\thumidity: ");
  Serial.print(hum);
  Serial.print("\tRelay: ");
  Serial.print(digitalRead(RELAY));
  Serial.print("\tAuto-heat: ");
  Serial.print(autoON);
  Serial.print("\tAuto temp: ");
  Serial.print(autoTemp);
  Serial.print("\n");
}

// Attempt to connect to WiFi
void connectWiFi() {
  WiFi.mode(WIFI_STA);  // Set WiFi mode to station (client)
  WiFi.setSleepMode(WIFI_NONE_SLEEP); //Disables sleep mode to prevent disconnection
  WiFi.begin(WIFI_SSID, WIFI_PASS); //connection setup

  Serial.print("\nConnecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected");

  Serial.print("Local IP address: ");
  Serial.print(WiFi.localIP());
  Serial.print("\n");
}
