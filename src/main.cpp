#include "config.h"

// Wifi
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
const char* wifiSSID = CONFIG_WIFI_SSID;
const char* wifiPassword = CONFIG_WIFI_PASS;
WiFiClient espClient;

// MQTT
#include "PubSubClient.h"
const char* mqttServer = CONFIG_MQTT_HOST;
const char* mqttUsername = CONFIG_MQTT_USER;
const char* mqttPassword = CONFIG_MQTT_PASS;
const char* mqttClientId = CONFIG_MQTT_CLIENT_ID; // Must be unique on the MQTT network
PubSubClient client(espClient);
const char* temperatureTopic = CONFIG_MQTT_TOPIC_TEMP;
const char* humidityTopic = CONFIG_MQTT_TOPIC_HUMID;
const char* doorTopic = CONFIG_MQTT_TOPIC_DOOR;

// DHT
#include "DHT.h"
const int dhtPin = CONFIG_DHT_PIN;
const long sampleDhtDelay = CONFIG_DHT_SAMPLE_DELAY;
unsigned long lastDhtSampleTime = 0;
DHT dht;

// Reed
const int reedPin = CONFIG_REED_PIN;
const long sampleReedDelay = CONFIG_REED_SAMPLE_DELAY;
unsigned long lastReedSampleTime = 0;
int debouncedReedState;


void setup() {
  Serial.begin(9600 );
  pinMode(reedPin, INPUT_PULLUP);
  dht.setup(dhtPin);

  WiFiManager wifiManager;
  wifiManager.autoConnect(wifiSSID, wifiPassword);

  client.setServer(mqttServer, 1883);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(mqttClientId, mqttUsername, mqttPassword)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

bool checkBound(float newValue, float prevValue, float maxDiff) {
  return !isnan(newValue) && (newValue < prevValue - maxDiff || newValue > prevValue + maxDiff);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long currentMillis = millis();
  float hum, temp;

  if (currentMillis - lastDhtSampleTime >= sampleDhtDelay) {
    lastDhtSampleTime = currentMillis;

    float newHum = dht.getHumidity();
    float newTemp = dht.getTemperature();

    if (checkBound(newTemp, temp, 1.0)) {
      temp = newTemp;
      Serial.print("New temperature:");
      Serial.println(String(temp).c_str());
      client.publish(temperatureTopic, String(temp).c_str(), true);
    }

    if (checkBound(newHum, hum, 1.0)) {
      hum = newHum;
      Serial.print("New humidity:");
      Serial.println(String(hum).c_str());
      client.publish(humidityTopic, String(hum).c_str(), true);
    }
  }

  if (currentMillis - lastReedSampleTime >= sampleReedDelay) {
    lastReedSampleTime = currentMillis;
    int reading = digitalRead(reedPin);
    if (reading != debouncedReedState) {
      Serial.print("New port status:");
      Serial.println(reading);
      debouncedReedState = reading;
      client.publish(doorTopic, String(reading).c_str(), true);
    }
  }
}