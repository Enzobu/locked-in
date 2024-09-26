#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
WiFiClient client;
void setup() {
    Serial.begin(115200);        // Initialisation de la communication série
    SPI.begin();                 // Initialisation de la communication SPI
    rfid.PCD_Init();             // Initialisation du module RFID
    Serial.println("Placez votre carte ou porte-clé RFID.");
    // Connexion au Wi-Fi
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    Serial.print("Connexion au Wifi ...");

    while (WiFi.status() != WL_CONNECTED) {
        digitalWrite(LED_BUILTIN, LOW);   // Allume la LED
        delay(50);                // Pendant 50 ms
        digitalWrite(LED_BUILTIN, HIGH);    // Éteint la LED
        delay(450);               // Pause de 450 ms
        Serial.print(".");
    }

    Serial.println("\n");
    Serial.println("Connexion Wifi réussie");
    Serial.print("Adresse IP : ");
    Serial.println(WiFi.localIP());
}
void loop() {
}
