/*
 * This is a sample configuration file for the "mqtt_esp8266_dht22" sensor.
 *
 * Change the settings below and save the file as "config.h"
 * You can then upload the code using the Arduino IDE.
 */

// WiFi
#define CONFIG_WIFI_SSID "{WIFI-SSID}"
#define CONFIG_WIFI_PASS "{WIFI-PASSWORD}"

// MQTT
#define CONFIG_MQTT_HOST "{MQTT-SERVER}"
#define CONFIG_MQTT_USER "{MQTT-USERNAME}"
#define CONFIG_MQTT_PASS "{MQTT-PASSWORD}"

#define CONFIG_MQTT_CLIENT_ID "Garage ESP8266" // Must be unique on the MQTT network

// MQTT Topics
#define CONFIG_MQTT_TOPIC_TEMP "garage/temperature"
#define CONFIG_MQTT_TOPIC_HUMID "garage/humidity"
#define CONFIG_MQTT_TOPIC_DOOR "garage/door"

// DHT
// Define the DHT type in the code file.
#define CONFIG_DHT_PIN D1
#define CONFIG_DHT_SAMPLE_DELAY 30000 // Milliseconds between readings