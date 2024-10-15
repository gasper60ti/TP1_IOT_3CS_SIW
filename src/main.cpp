#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <WebSocketsServer.h> // WebSocket library

#define LED 2 // On board LED

// SSID and Password of your WiFi router
const char* ssid = "WledHorma";
const char* password = "87654322";

ESP8266WebServer server(80);    // Server on port 80
WebSocketsServer webSocket(81); // WebSocket server on port 81

float humidity, temperature;

//===============================================================
// This routine is executed when you open its IP in the browser
//===============================================================
void handleTH() {
    String data = "{\"Temperature\":\"" + String(temperature) + "\", \"Humidity\":\"" + String(humidity) + "\"}";
    server.send(200, "text/plain", data); // Send temperature and humidity JSON to client via AJAX

    humidity += 1.0;   // Simulate humidity change
    temperature += 2.0; // Simulate temperature change

    Serial.print("Humidity: ");
    Serial.println(humidity, 1);
    Serial.print("Temperature: ");
    Serial.println(temperature, 1);
}

// WebSocket event handler
void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
    switch (type) {
        case WStype_CONNECTED: {
            Serial.printf("[%u] Connected\n", num);
            // Send the initial LED state to the client
            webSocket.sendTXT(num, digitalRead(LED) == LOW ? "ON" : "OFF");
            break;
        }
        case WStype_DISCONNECTED: {
            Serial.printf("[%u] Disconnected\n", num);
            break;
        }
        case WStype_TEXT: {
            String msg = String((char *)payload);
            Serial.printf("[%u] Message: %s\n", num, msg.c_str());

            if (msg == "LED_ON") {
                digitalWrite(LED, LOW); // Turn the LED on
                webSocket.sendTXT(num, "LED is ON");
                Serial.println("LED ON");
            } else if (msg == "LED_OFF") {
                digitalWrite(LED, HIGH); // Turn the LED off
                webSocket.sendTXT(num, "LED is OFF");
                Serial.println("LED OFF");
            }
            break;
        }
        default:
            break;
    }
}

//==============================================================
//                     SETUP
//==============================================================
void setup() {
    pinMode(LED, OUTPUT);
    digitalWrite(LED, HIGH); // Initially turn off the LED
    Serial.begin(9600);
    Serial.println();

    if (!SPIFFS.begin()) {
        Serial.println("An error occurred while mounting SPIFFS");
        return;
    }

    WiFi.begin(ssid, password); // Connect to your WiFi router
    Serial.println("");

    // Wait for connection
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    // If connection successful, show IP address in serial monitor
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP()); // IP address assigned to your ESP

    // HTTP server configuration
    server.serveStatic("/css", SPIFFS, "/css");
    server.serveStatic("/js", SPIFFS, "/js");
    server.serveStatic("/", SPIFFS, "/index.html");
    server.on("/readTH", handleTH); // This page is called by JavaScript AJAX
    server.begin();                 // Start HTTP server
    Serial.println("HTTP server started");

    // WebSocket server configuration
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    Serial.println("WebSocket server started on port 81");
}

//==============================================================
//                     LOOP
//==============================================================
void loop() {
    server.handleClient(); // Handle client requests
    webSocket.loop();      // Handle WebSocket communication
}