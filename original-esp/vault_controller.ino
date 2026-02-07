#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

// SABOTAGED Admin System
// Students will dump this firmware and find the problems

const char* WIFI_SSID = "POCO";
const char* WIFI_PASSWORD = "something.";

// SABOTAGE: Wrong server address
const char* SERVER_HOST = "10.217.244.117";  // Change to YOUR actual IP
const int SERVER_PORT = 3001;
const char* DEVICE_ID = "ADMIN_ESP_01";

// SABOTAGE: Impostor's fake key
const char* SECRET_KEY = "IMPOSTOR_FAKE_KEY";

unsigned long lastHeartbeat = 0;

const char* root_ca =  \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDCTCCAfGgAwIBAgIUOUQJey2lFTMkpzv5yYD9FZR+h64wDQYJKoZIhvcNAQEL\n" \
"BQAwFDESMBAGA1UEAwwJbG9jYWxob3N0MB4XDTI2MDEyMjA5MTcxNloXDTI3MDEy\n" \
"MjA5MTcxNlowFDESMBAGA1UEAwwJbG9jYWxob3N0MIIBIjANBgkqhkiG9w0BAQEF\n" \
"AAOCAQ8AMIIBCgKCAQEAvryM2lD0B3HZ+GPQaHIpUwQWuAA1CqSwCnTH3bU71z7l\n" \
"M9ts0k9VNz50v7+tv1JgK+QU3YsT2TWIFsXryuLcyrwmBzk+n1QNCjrOUR3DZzdl\n" \
"dKqmvC4qay6kPqz6oiaqnyV/dMhgjBi4usAPDqHLjiiXi4trlgOCydmie5+ya5Sa\n" \
"EvT7qmeZ1YFNrNZgBlOJ3hh6CBA6mXDq3kqS4d/9eerKmZ89ZtC6F4JDDd8gRDXk\n" \
"vr/g+fGswklOpSjlhxyFOjHATKVXyMwAswJKuRSdP+abowo3yezVFRjszElJMMvW\n" \
"VLe47Iixpb7TNSU8MqksSFGdGOtu8gGWMyL9Qlyb5wIDAQABo1MwUTAdBgNVHQ4E\n" \
"FgQUJLNHoe05KPwnEpMeqS3S3NG8cnswHwYDVR0jBBgwFoAUJLNHoe05KPwnEpMe\n" \
"qS3S3NG8cnswDwYDVR0TAQH/BAUwAwEB/zANBgkqhkiG9w0BAQsFAAOCAQEAVPiw\n" \
"uF9VQreM54cxyhkJEAhTV1sJXRxtV7lV/iyYX6tkVdiczH6yPpvb+QF6JMmsc9K/\n" \
"Q3rEaCT2LFfRHnef3rqD0232p3xw0LVerTyPNvH9iYSjkzwJlBn47De0N2gI3m4U\n" \
"b9Aqij3bELN+rcx08T9YS8HMIhWx3uHE2tEqLbzQdBctWChmqZNA7DUMOCxGuTNb\n" \
"ETOoM6SfmdHXYHvCgubkRDzuKbQZAn3dCm80aB7GH5Thnhb+hyFahjWYK+ZiyyHG\n" \
"4NMBSdNGuSzhAKobNwOmR9vG5KzCrRgZD4YbzrKFT3jOJtViL48U/y/NXnuBRbFW\n" \
"kcIuuOF2a4WMABezkw==\n" \
"-----END CERTIFICATE-----\n";


void WiFiEvent(arduino_event_id_t event) {
    switch (event) {

        case IP_EVENT_STA_GOT_IP:
            Serial.print("[WiFi] ✓ Connected, IP: ");
            Serial.println(WiFi.localIP());
            break;

        case WIFI_EVENT_STA_DISCONNECTED:
            Serial.println("[WiFi] ✗ Disconnected, retrying...");
            WiFi.reconnect();
            break;

        default:
            break;
    }
}



void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("BOOTING...");

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    Serial.println("\n[WiFi] Scanning networks...");
    int n = WiFi.scanNetworks();
    
    Serial.println("\nAvailable networks:");
    for (int i = 0; i < n; i++) {
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.print(WiFi.SSID(i));
        Serial.print(" (");
        Serial.print(WiFi.RSSI(i));
        Serial.print(" dBm) ");
        Serial.println(WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "Open" : "Secured");
    }
    Serial.println();

    // Now try connecting
    Serial.println("[WiFi] Attempting connection to: " + String(WIFI_SSID));
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n[WiFi] Connected!");
        Serial.print("[WiFi] IP: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\n[WiFi] FAILED");
        Serial.print("[WiFi] Status: ");
        Serial.println(WiFi.status());
    }
    delay(2000);
    sendHeartbeat();
}


void loop() {
    if (WiFi.status() == WL_CONNECTED) {
        if (millis() - lastHeartbeat >= 5000*60) {
            sendHeartbeat();
            lastHeartbeat = millis();
        }
    }
    delay(10);
}

String buildPayload() {
    String payload = "{";
    
    payload += "\"deviceId\":\"";
    payload += DEVICE_ID;
    payload += "\",";
    
    payload += "\"status\":\"online\",";
    
    payload += "\"secretKey\":\"";
    payload += SECRET_KEY;
    payload += "\",";
    
    payload += "\"timestamp\":";
    payload += String(millis());
    
    payload += "}";
    
    return payload;
}
// Add at the top of your file (after includes):
void sendHeartbeat() {
    String payload = buildPayload();
    
    Serial.println("\n┌─────────────────────────────────────");
    Serial.println("│ SENDING HEARTBEAT");
    Serial.println("├─────────────────────────────────────");
    Serial.print("│ Device: ");
    Serial.println(DEVICE_ID);
    Serial.print("│ Secret: ");
    Serial.println(SECRET_KEY);
    Serial.println("├─────────────────────────────────────");
    Serial.print("│ Payload: ");
    Serial.println(payload);
    Serial.println("└─────────────────────────────────────");
    
    WiFiClientSecure *client = new WiFiClientSecure;
    client->setInsecure();  // Use certificate instead of setInsecure()
    
    HTTPClient http;
    
    String url = String("https://") + SERVER_HOST + ":" + String(SERVER_PORT) + "/admin";
    
    Serial.print("[HTTP] Connecting to: ");
    Serial.println(url);
    
    if (!http.begin(*client, url)) {
        Serial.println("[HTTP] ✗ begin() failed");
        delete client;
        return;
    }
    
    http.addHeader("Content-Type", "application/json");
    http.setTimeout(10000);
    
    Serial.println("[HTTP] Sending POST...");
    int httpCode = http.POST(payload);
    
    Serial.print("[HTTP] Response code: ");
    Serial.println(httpCode);
    
    if (httpCode == 200) {
        String response = http.getString();
        Serial.print("[HTTP] ✓ Body: ");
        Serial.println(response);
    } else if (httpCode > 0) {
        String response = http.getString();
        Serial.print("[HTTP] ✗ Error ");
        Serial.print(httpCode);
        Serial.print(" Body: ");
        Serial.println(response);
    } else {
        Serial.print("[HTTP] ✗ Connection failed, error: ");
        Serial.println(http.errorToString(httpCode));
    }
    
    http.end();
    delete client;
    Serial.println();
}