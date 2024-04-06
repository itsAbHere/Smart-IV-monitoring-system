#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_Sensor.h>
#include <Arduino.h>
#include "HX711.h"
#include "soc/rtc.h"
#include <PubSubClient.h> // Include the MQTT library

// MQTT Broker details
const char* mqtt_server = "172.20.10.3";
const int mqtt_port = 1883;
const char* mqtt_username = "sd";
const char* mqtt_password = "a";

// MQTT topics
const char* weight_topic = "intravenous_bag/weight";

// Buzzer pin
#define BUZZER_PIN 18 // Change this to the actual pin number you're using for the buzzer

// LED pin
#define LED_PIN 19 // Change this to the actual pin number you're using for the LED

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 16;
const int LOADCELL_SCK_PIN = 4;

HX711 scale;
WiFiClient espClient;
PubSubClient client(espClient);

// Replace with your network credentials
const char* ssid = "Ganesh’s iPhone";
const char* password = "ganesh19496";

void setup() {
  Serial.begin(115200);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT); // Set LED pin as output

  // Connect to Wi-Fi
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  
  // Connect to MQTT Broker
  client.setServer(mqtt_server, mqtt_port);
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client", mqtt_username, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }

  // Initialize HX711 scale
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(-694.168);
  scale.tare();
}

void loop() {
  // Check MQTT connection
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Read weight
  float rawReading = abs(scale.get_units());

  // Calculate percentage
  float percentage;
  if (rawReading >= 600) {
    percentage = 100;
  } else if (rawReading >= 500) {
    percentage = map(rawReading, 500, 600, 75, 100); 
  } else if (rawReading >= 300) {
    percentage = map(rawReading, 300, 500, 50, 75); 
  } else if (rawReading >= 200) {
    percentage = map(rawReading, 200, 300, 25, 50); 
  } else if (rawReading >= 50) {
    percentage = map(rawReading, 50, 200, 5, 25);   
  } else {
    percentage = 0;
  }
  Serial.print("Raw Reading: ");
  Serial.print(rawReading);
  Serial.print(", Percentage: ");
  Serial.print(percentage);
  Serial.println("%");
  // Publish weight to MQTT
  char message[50];
  sprintf(message, "%.2f", rawReading);
  client.publish(weight_topic, message);

  // Trigger buzzer and LED based on weight percentage
  if (percentage < 25) {
    digitalWrite(BUZZER_PIN, HIGH); // Turn on the buzzer
    digitalWrite(LED_PIN, HIGH); // Turn on the LED
    delay(2000); // Wait for 2 seconds
    digitalWrite(BUZZER_PIN, LOW); // Turn off the buzzer
    digitalWrite(LED_PIN, LOW); // Turn off the LED
    delay(2000); // Wait for 2 seconds
  }  
  else if ( percentage > 25){
    
     digitalWrite(BUZZER_PIN, HIGH); 
 digitalWrite(LED_PIN, HIGH);
  }
  scale.power_down();             // put the ADC in sleep mode
  delay(5000);
  scale.power_up();
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client", mqtt_username, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}
