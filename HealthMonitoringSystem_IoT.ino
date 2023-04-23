#include <PubSubClient.h>
#include <WiFi.h>
#include <stdio.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include "DHT.h" 
#define DHTTYPE DHT11 
#define DHTPIN 32
#define DS18B20 4
#define REPORTING_PERIOD_MS     1000
#define TOKEN "BBFF-0GS87GgW4KJF2Z1pb3EBULU4mlnrJ4" // Put your Ubidots' TOKEN
#define DEVICE_LABEL "health-monitoring-system" // Put the device label
#define VARIABLE_LABEL_1 "heartrate" 
#define VARIABLE_LABEL_2 "SPo2" 
#define VARIABLE_LABEL_3 "humidity" 
#define VARIABLE_LABEL_4 "temperature" 
#define VARIABLE_LABEL_5 "tempC" 
#define MQTT_CLIENT_NAME "EI_OXMO" // MQTT client Name, put a Random ASCII

float bodytemperature, h, t, tempC;
const char* ssid = "RS Room";  // Enter SSID here
const char* password = "8080911447";  //Enter Password here
uint32_t tsLastReport = 0;
char mqttBroker[] = "industrial.api.ubidots.com";
char payload[700];
char topic[150];
// Space to store values to send
char str_val_1[6];
char str_val_2[6];
char str_val_3[6];
char str_val_4[6];
char str_val_5[6];
int flag = 0;
int count = 0;
WiFiClient ubidots;
PubSubClient client(ubidots);

PulseOximeter pox;
DHT dht(DHTPIN, DHTTYPE);; 
OneWire oneWire(DS18B20);
DallasTemperature sensors(&oneWire);

void onBeatDetected(){;}

void callback(char* topic, byte* payload, unsigned int length) {;}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(MQTT_CLIENT_NAME, TOKEN, "")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      // Wait 2 seconds before retrying
      delay(2000);
    }
  }
}

void connectWiFi() {
  //connect to your local wi-fi network
  WiFi.begin(ssid, password);

  //check wi-fi is connected to wi-fi network
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");
  Serial.println(WiFi.localIP());
}

void connectMQTT() {
  if (flag == 0)
  {
    client.connect(MQTT_CLIENT_NAME, TOKEN, "");
    Serial.println("MQTT connected again");
    flag = 1;
  }
  if (!client.connected()) {
    Serial.print("Reconnecting ... ");
    reconnect();
  }
}

void publishData() {
  // sensors.requestTemperatures();
  dtostrf(pox.getHeartRate(), 4, 2, str_val_1);
  dtostrf(pox.getSpO2(), 4, 2, str_val_2);
  dtostrf(dht.readTemperature(), 4, 2, str_val_3);
  dtostrf(dht.readHumidity(), 4, 2, str_val_4);
  // sensors.requestTemperatures();
  dtostrf(tempC, 4, 2, str_val_5);
  // dtostrf(sensors.getTempCByIndex(0), 4, 2, str_val_5);
  sprintf(topic, "%s", ""); // Cleans the topic content
  sprintf(topic, "%s%s", "/v1.6/devices/", DEVICE_LABEL);
  sprintf(payload, "%s", ""); // Cleans the payload content
  sprintf(payload, "{\"%s\":", VARIABLE_LABEL_1); // Adds the variable label
  sprintf(payload, "%s {\"value\": %s}", payload, str_val_1); // Adds the value
  sprintf(payload, "%s, \"%s\":", payload, VARIABLE_LABEL_2); // Adds the variable label
  sprintf(payload, "%s {\"value\": %s}", payload, str_val_2); // Adds the value
  sprintf(payload, "%s, \"%s\":", payload, VARIABLE_LABEL_3); // Adds the variable label
  sprintf(payload, "%s {\"value\": %s}", payload, str_val_3); // Adds the value
  sprintf(payload, "%s, \"%s\":", payload, VARIABLE_LABEL_4); // Adds the variable label
  sprintf(payload, "%s {\"value\": %s}", payload, str_val_4); // Adds the value
  sprintf(payload, "%s, \"%s\":", payload, VARIABLE_LABEL_5); // Adds the variable label
  sprintf(payload, "%s {\"value\": %s}", payload, str_val_5); // Adds the value
  sprintf(payload, "%s}", payload); // Closes the dictionary brackets
  Serial.println(payload);
  // Serial.println(topic);
  client.publish(topic, payload);
  client.loop();
}

void publishData1() {
  // sensors.requestTemperatures();
  float values[] = {pox.getHeartRate(), pox.getSpO2(), dht.readHumidity(), dht.readTemperature(), sensors.getTempCByIndex(0)};
  const char* labels[] = {VARIABLE_LABEL_1, VARIABLE_LABEL_2, VARIABLE_LABEL_3, VARIABLE_LABEL_4, VARIABLE_LABEL_5};
  sprintf(topic, "/v1.6/devices/%s", DEVICE_LABEL);
  sprintf(payload, "{");
  for (int i = 0; i < 5; i++) {
    char str_val[10];
    dtostrf(values[i], 4, 2, str_val);
    sprintf(payload, "%s \"%s\":{\"value\":%s}", payload, labels[i], str_val);
    if (i < 4) { sprintf(payload, "%s,", payload); }
  }
  sprintf(payload, "%s }", payload);
  Serial.println(payload);
  client.publish(topic, payload);
  client.loop();
}

void setup(){
  Serial.begin(115200);
  connectWiFi(); //connect to your local wi-fi network
  client.setServer(mqttBroker, 1883);
  client.setCallback(callback);
  connectMQTT();
  sensors.begin(); // Start up the library
  dht.begin();
  if (!pox.begin()) { Serial.println("FAILED");for (;;);} 
  else {Serial.println("SUCCESS");}
  pox.setIRLedCurrent(MAX30100_LED_CURR_46_8MA);
  pox.setOnBeatDetectedCallback(onBeatDetected); // Register a callback for the beat detection
}

void loop(){
  pox.update(); 
  // publishData();  
  // delay(1000);
  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
    publishData1();
    tsLastReport = millis();
    // tempC = sensors.getTempCByIndex(0);  
  }
  sensors.requestTemperatures();
}
