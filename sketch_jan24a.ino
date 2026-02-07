#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>

// ==================== CONFIGURATION ====================
const char* WIFI_SSID = "POCO";
const char* WIFI_PASSWORD = "something.";
const char* SERVER_HOST = "10.132.21.117";
const int SERVER_PORT = 3001;
String deviceId = "HHHHHHHHH";

// Pin definitions
#define LED_PIN D4        // Built-in LED (NodeMCU: D4 = GPIO2)
#define BUTTON_PIN D3     // Button to toggle lock/unlock

// ==================== GLOBAL STATE ====================
bool isLocked = true;
unsigned long lastHeartbeat = 0;
unsigned long lastButtonCheck = 0;
bool lastButtonState = HIGH;

// ==================== SETUP ====================
void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n========================================");
    Serial.println("    Vault Device Starting");
    Serial.println("========================================");
    
    // Setup pins
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    digitalWrite(LED_PIN, LOW); // Start locked (LED OFF)
    
    Serial.print("Device ID: ");
    Serial.println(deviceId);
    
    // Connect to WiFi
    connectWiFi();
    
    Serial.println("========================================");
    Serial.println("Ready - Press button to lock/unlock");
    Serial.println("========================================\n");
}

// ==================== MAIN LOOP ====================
void loop() {
    // Reconnect WiFi if disconnected
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[WiFi] Connection lost - reconnecting...");
        connectWiFi();
        return;
    }
    
    // Check button every 50ms (debounce)
    if (millis() - lastButtonCheck >= 50) {
        bool buttonState = digitalRead(BUTTON_PIN);
        
        // Button pressed (goes LOW when pressed)
        if (buttonState == LOW && lastButtonState == HIGH) {
            // Toggle lock state
            isLocked = !isLocked;
            digitalWrite(LED_PIN, isLocked ? LOW : HIGH); // Inverted for ESP8266
            
            Serial.println("----------------------------------------");
            Serial.print("[USER ACTION] Button pressed - Vault is now: ");
            Serial.println(isLocked ? "LOCKED 🔒" : "UNLOCKED 🔓");
            Serial.println("----------------------------------------");
            
            // Send immediate update
            sendHeartbeat();
        }
        
        lastButtonState = buttonState;
        lastButtonCheck = millis();
    }
    
    // Send heartbeat every 3 seconds
    if (millis() - lastHeartbeat >= 3000) {
        sendHeartbeat();
        lastHeartbeat = millis();
    }
    
    delay(10);
}


// ==================== CONNECT TO WIFI ====================
void connectWiFi() {
    Serial.print("[WiFi] Connecting to: ");
    Serial.println(WIFI_SSID);
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    Serial.println();
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("[WiFi] ✓ Connected!");
        Serial.print("[WiFi] IP Address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("[WiFi] ✗ Connection failed");
    }
}

// ==================== BUILD JSON PAYLOAD ====================
String buildPayload() {
    // Manually build JSON (no library needed)
    String payload = "{";
    
    // Add deviceId
    payload += "\"deviceId\":\"";
    payload += deviceId;
    payload += "\",";
    
    // Add vaultState
    payload += "\"vaultState\":\"";
    payload += (isLocked ? "LOCKED" : "UNLOCKED");
    payload += "\",";
    
    // Add timestamp
    payload += "\"timestamp\":";
    payload += String(millis());
    
    payload += "}";
    
    return payload;
}

// ==================== SEND HEARTBEAT TO SERVER ====================
void sendHeartbeat() {
    // Build payload
    String payload = buildPayload();
    
    Serial.println("\n┌─────────────────────────────────────");
    Serial.println("│ SENDING HEARTBEAT");
    Serial.println("├─────────────────────────────────────");
    Serial.print("│ Device ID:    ");
    Serial.println(deviceId);
    Serial.print("│ Vault State:  ");
    Serial.println(isLocked ? "LOCKED" : "UNLOCKED");
    Serial.print("│ Timestamp:    ");
    Serial.println(millis());
    Serial.println("├─────────────────────────────────────");
    Serial.print("│ JSON Payload: ");
    Serial.println(payload);
    Serial.println("└─────────────────────────────────────");
    
    // Create HTTPS client
    WiFiClientSecure client;
    client.setInsecure(); // Skip certificate validation (workshop only)
    
    HTTPClient http;
    
    // Build URL
    String url = String("https://") + SERVER_HOST + ":" + String(SERVER_PORT) + "/heartbeat";
    
    Serial.print("[HTTP] Connecting to: ");
    Serial.println(url);
    
    if (!http.begin(client, url)) {
        Serial.println("[HTTP] ✗ Connection failed");
        return;
    }
    
    // Set headers
    http.addHeader("Content-Type", "application/json");
    
    // Send POST request
    int httpCode = http.POST(payload);
    
    // Handle response
    Serial.print("[HTTP] Response code: ");
    Serial.println(httpCode);
    
    if (httpCode == 200) {
        String response = http.getString();
        Serial.print("[HTTP] ✓ SUCCESS - Server response: ");
        Serial.println(response);
    } else if (httpCode > 0) {
        Serial.print("[HTTP] ✗ ERROR - Server returned: ");
        Serial.println(httpCode);
        String response = http.getString();
        if (response.length() > 0) {
            Serial.print("[HTTP] Error details: ");
            Serial.println(response);
        }
    } else {
        Serial.print("[HTTP] ✗ FAILED - Error: ");
        Serial.println(http.errorToString(httpCode));
    }
    
    http.end();
    Serial.println();
}