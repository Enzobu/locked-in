// ====================================================================================================
// ===================================== Import des bibliothèques =====================================
// ====================================================================================================

#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

// ====================================================================================================
// ===================================== Définition des variables =====================================
// ====================================================================================================

// Pin de la carte RFID
#define SS_PIN 4    // SDA (D2)
#define RST_PIN 5   // RST (D1)


// Lien de l'API
String _serveur = "api.workshop-b3.enzo-palermo.com:3000";
String _api = "/locker/islockerbooked/";
String _rfidId;

// Pin des leds et transistor
#define TRANSISTOR_PIN D0
#define GREEN_LED_PIN D3
#define LED_BUILTIN D4
#define RED_LED_PIN D8

// ====================================================================================================
// ===================================== Définition des fonctions =====================================
// ====================================================================================================

int getLockerId(String _serveur, String _api, String _rfidId);
String removeSpaces(const String &input);
String getContentType(String path);
void lectureRFID();
int getLockerIdFromJson(String jsonString);


// ====================================================================================================
// ====================================== Définition des objets =======================================
// ====================================================================================================

// Instance de l'objet RFID
MFRC522 rfid(SS_PIN, RST_PIN);

// Instances des objet http et client
HTTPClient http;
WiFiClient client;


// ====================================================================================================
// ============================================== SETUP ===============================================
// ====================================================================================================

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

// ====================================================================================================
// =============================================== LOOP ===============================================
// ====================================================================================================

void loop() {
    // Lecture de la carte RFID
    lectureRFID();
}


// ====================================================================================================
// =============================================== RFID ===============================================
// ====================================================================================================

// fonction qui tape l'API
int getLockerId(String _serveur, String _api, String _rfidId) {
    // Envoie de la requete
    String url = "http://" + _serveur + _api + _rfidId;
    http.begin(client, url);
    int httpCode = http.GET();

    // Ecrire la réponse
    Serial.println("url : " + url);
    Serial.println(http.getString());

    // Ecrire l'état de la requete (pour le debug)
    Serial.println("[HTTP] GET... : " + http.errorToString(httpCode));

    // Extraction de json et récupération du numéro du casier puis, affichage
    int id = getLockerIdFromJson(http.getString());
    Serial.println(id);

    // Déconnexion
    http.end();
    return id;
}

// Fonction qui retire les espace pour passer le numéro de la carte à l'API
String removeSpaces(const String &input) {
    String result;
    for (char c : input) {
        if (c != ' ') {
            result += c;
        }
    }
    return result;
}

// Transforme en Json le retour de l'API et retourne le numéro du casier
int getLockerIdFromJson(String jsonString) {
    // Créer un document JSON
    StaticJsonDocument<200> doc;

    // Parser la chaîne JSON
    DeserializationError error = deserializeJson(doc, jsonString);

    // Vérifier s'il y a une erreur dans le parsing
    if (error) {
        Serial.print("Erreur lors de la désérialisation: ");
        Serial.println(error.f_str());
        return -1;
    }

    // Extraire la valeur associée à la clé "lockerId"
    int lockerId = doc["lockerId"];

    return lockerId;
}


// ====================================================================================================
// =============================================== RFID ===============================================
// ====================================================================================================

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

            int lockerId = getLockerId(_serveur, _api, removeSpaces(idRfid));
            Serial.println("Résultat de la fct getLockerId" + lockerId);

            if (lockerId > 0) {
                digitalWrite(TRANSISTOR_PIN, HIGH);
                digitalWrite(GREEN_LED_PIN, HIGH);
                delay(1000);
                digitalWrite(TRANSISTOR_PIN, LOW);
                digitalWrite(GREEN_LED_PIN, LOW);
            } else {
                digitalWrite(RED_LED_PIN, HIGH);
                delay(1000);
                digitalWrite(RED_LED_PIN, LOW);
            }
        }
    }
}