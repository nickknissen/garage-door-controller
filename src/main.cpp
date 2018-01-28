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

// ultrasonic-sensor-hc-sr04 
const int usTrigPin = CONFIG_US_TRIG_PIN;
const int usEchoPin = CONFIG_US_ECHO_PIN;
const long usSampleDelay = CONFIG_US_SAMPLE_DELAY;
unsigned long usLastSampleTime = 0;

long duration, distance;


void setup() {
  Serial.begin(9600 );

  pinMode(usTrigPin, OUTPUT);
  pinMode(usEchoPin, INPUT);

  dht.setup(dhtPin, dht.DHT22);

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

  if (currentMillis - lastDhtSampleTime >= sampleDhtDelay) {
    lastDhtSampleTime = currentMillis;

    float hum, temp;
    float newHum = dht.getHumidity();
    float newTemp = dht.getTemperature();

    if (checkBound(newTemp, temp, 1.0)) {
      temp = newTemp;
      Serial.print("New temperature:");
      Serial.println(String(temp).c_str());
      client.publish(temperatureTopic, String(temp).c_str(), true);
    } else {
      client.publish("garage/falty_temp", dht.getStatusString(), true);
    }

    if (checkBound(newHum, hum, 1.0)) {
      hum = newHum;
      Serial.print("New humidity:");
      Serial.println(String(hum).c_str());
      client.publish(humidityTopic, String(hum).c_str(), true);
    }
  }
  
  
  // ultrasonic-sensor-hc-sr04 
  if (currentMillis - usLastSampleTime >= usSampleDelay) {
    usLastSampleTime = currentMillis;

    // Clears the trigPin
    digitalWrite(usTrigPin, LOW);
    delayMicroseconds(2);

    // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(usTrigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(usTrigPin, LOW);

    // Reads the echoPin, returns the sound wave travel time in microseconds
    duration = pulseIn(usEchoPin, HIGH);

    // Calculating the distance
    distance= duration*0.034/2;
    // Prints the distance on the Serial Monitor
    Serial.print("Distance: ");
    Serial.println(distance);

  }
}