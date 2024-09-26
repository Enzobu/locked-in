#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
// Pin de la carte RFID
#define SS_PIN 4    // SDA (D2)
#define RST_PIN 5   // RST (D1)

// Pin des leds et transistor
#define TRANSISTOR_PIN D0
#define GREEN_LED_PIN D3
#define LED_BUILTIN D4
#define RED_LED_PIN D8
void lectureRFID();
// Instance de l'objet RFID
MFRC522 rfid(SS_PIN, RST_PIN);

// Instances des objet http et client
HTTPClient http;
WiFiClient client;
void setup() {
    Serial.begin(115200);        // Initialisation de la communication série
    SPI.begin();                 // Initialisation de la communication SPI
    rfid.PCD_Init();             // Initialisation du module RFID
    Serial.println("Placez votre carte ou porte-clé RFID.");

    pinMode(TRANSISTOR_PIN, OUTPUT);      // Transistor serrure
    pinMode(RED_LED_PIN, OUTPUT);         // LED pour carte non reconnue
    pinMode(GREEN_LED_PIN, OUTPUT);       // LED pour carte reconnue
    pinMode(LED_BUILTIN, OUTPUT);         // LED pour clignoter lors de la recherche du Wi-Fi
    digitalWrite(LED_BUILTIN, HIGH);      // Eteidre la LED

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
    // Lecture de la carte RFID
    lectureRFID();
}

}
// Lecture de la carte RFID et gestion de l'ouverture du casier
void lectureRFID() {
    // Vérifie si une nouvelle carte est présente
    if (rfid.PICC_IsNewCardPresent()) {
        // Essaie de lire la carte RFID
        if (rfid.PICC_ReadCardSerial()) {
            // Affiche l'UID de la carte
            Serial.print("UID de la carte : ");
            String idRfid = ""; // Initialise une chaîne vide pour stocker l'UID
            for (byte i = 0; i < rfid.uid.size; i++) {
                Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
                Serial.print(rfid.uid.uidByte[i], HEX);
                // Construit la chaîne d'UID
                idRfid += (rfid.uid.uidByte[i] < 0x10 ? " 0" : " ") + String(rfid.uid.uidByte[i], HEX);
            }
            Serial.print("id rfid : " + idRfid);
            Serial.println(); // Nouvelle ligne après l'UID

            // Arrête la communication avec la carte
            rfid.PICC_HaltA();


        }
    }
}