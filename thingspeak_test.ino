/*
 * 
 * @refer: https://diyi0t.com/rain-sensor-tutorial-for-arduino-and-esp8266/
 * 
 */

#include <WiFi.h>
#include "ThingSpeak.h"
#include "DHT.h"
#include "secrets.h"

const char* ssid = SECRET_SSID;
const char* password = SECRET_PASS;

WiFiClient  client;

unsigned long myChannelNumber = SECRET_CHNL;
const char * myWriteAPIKey = SECRET_APIK;

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 5000;

#define ldrPin 34
#define echoPin 26
#define trigPin 27
#define soilMoisture 35
#define rainSensor 33
#define dhtPin 4

long duration;
long distance;
float light;
float moisture;
float rainValue;
float t;
float h;
float hic;

DHT dht(dhtPin, DHT11);

void setup() {
  Serial.begin(115200);  //Initialize serial

  pinMode(ldrPin, INPUT);
  pinMode(trigPin, OUTPUT); 
  pinMode(echoPin, INPUT);

  dht.begin(); // //Initialize DHT11

  WiFi.mode(WIFI_STA);   
  ThingSpeak.begin(client);  // Initialize ThingSpeak
}

void loop() {
  if ((millis() - lastTime) > timerDelay) {
    
    if(WiFi.status() != WL_CONNECTED){
      Serial.print("Attempting to connect...");
      while(WiFi.status() != WL_CONNECTED){
        WiFi.begin(ssid, password); 
        Serial.print("...");
        delay(5000);     
      } 
      Serial.println("\nConnected!!!");
    }

    light = analogRead(ldrPin);
    
    Serial.print("Light Intensity: ");
    Serial.print(light);

    if (light < 40) {
      Serial.println(", Dark");
    } else if (light < 100) {
      Serial.println(", Dim");
    } else if (light < 1000) {
      Serial.println(", Light");
    } else if (light < 2000) {
      Serial.println(", Bright");
    } else {
      Serial.println(", Very bright");
    }

    
    digitalWrite(trigPin, LOW); // Clears the trigPin condition
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH); // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    duration = pulseIn(echoPin, HIGH); // Reads the echoPin, returns the sound wave travel time in microseconds
    distance = duration * 0.034 / 2; // Calculating the distance: Speed of sound wave divided by 2 (go and back)
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");

    moisture = analogRead(soilMoisture);

    Serial.print("Soil Moisture: ");
    Serial.print(moisture);

    if (moisture < 1500) {
      Serial.println(", In water");
    } else if (moisture < 2500) {
      Serial.println(", Humid soil");
    } else {
      Serial.println(", Dry soil");
    }

    rainValue = analogRead(rainSensor);

    Serial.print("Raining Intensity: ");
    Serial.println(rainValue);

    if (rainValue < 1500) {
      Serial.println(", Heavy Rain");
    } else if (rainValue < 2500) {
      Serial.println(", Light Rain");
    } else {
      Serial.println(", No Rain");
    }

    h = dht.readHumidity();
    t = dht.readTemperature();
    float f = dht.readTemperature(true); // Read temperature as Fahrenheit
    hic = dht.computeHeatIndex(t, h, false); // Heat Index: what the temperature feels like to the human body when relative humidity is combined with the air temperature

    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.println("%");
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.println("C");
    Serial.print("Heat index: ");
    Serial.print(hic);
    Serial.println("C");

    if (isnan(h) || isnan(t) || isnan(f) || isnan(hic)) {
      Serial.println(F("Failed to read from DHT sensor!"));
      return;
    }
 
    // set the fields with the values
    ThingSpeak.setField(1, 321);
    ThingSpeak.setField(2, 321);
    ThingSpeak.setField(3, 321);
    ThingSpeak.setField(4, 321);
    ThingSpeak.setField(5, t);
    ThingSpeak.setField(6, h);
    ThingSpeak.setField(7, hic);

    int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

    if(x == 200){
      Serial.println("Channel update successful.");
    }
    else{
      Serial.println("Problem updating channel. HTTP error code " + String(x));
    }

    Serial.println("\n\n");
    
    lastTime = millis();
  }
}
