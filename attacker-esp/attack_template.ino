#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

// ============================================
// CONFIGURATION - Filled from firmware dump
// ============================================

const char* WIFI_SSID = "POCO";           
const char* WIFI_PASSWORD = "something.";       
const char* SERVER_HOST = "10.217.244.117";  // Their actual IP from dump
const int SERVER_PORT = 3001;

const char* DEVICE_ID = "ADMIN_RESTORED";

// The key insight: RESTORED admin system needs ADMIN_RESTORED key
// NOT the impostor's fake key
const char* SECRET_KEY = "ADMIN_RESTORED_2077";  

const char* root_ca = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDLzCCAhegAwIBAgIUAzHeFsuK6RMxFRpFV6wjV+ft1akwDQYJKoZIhvcNAQEL\n" \
"BQAwGTEXMBUGA1UEAwwOMTAuMjE3LjI0NC4xMTcwHhcNMjYwMjA3MDYzNzI0WhcN\n" \
"MjcwMjA3MDYzNzI0WjAZMRcwFQYDVQQDDA4xMC4yMTcuMjQ0LjExNzCCASIwDQYJ\n" \
"KoZIhvcNAQEBBQADggEPADCCAQoCggEBANyGIkz6m/m/WBwu4wowblo+bpyM14w7\n" \
"LlFdSSqrQndhUAks2JpZuX90cG6VrGVz2BLNbXOKCAc2I1ehXpAlAsjNAAans1ka\n" \
"jcyb7n7l/ANtyzJxfFpJ1REnd14bYqXOCrN2NFZM7iJYmQvJ/MlImXMOBOiXNllq\n" \
"+PFNWAuVkLn1ORK8IwatIKgigq604mR/ZFmPKjq8fxXWTga43DuboeL9t5CZ4b1W\n" \
"8cwCws9IaEfYa9oH5SKXoWHsTfM1r+q3+7jUfD+IeSFDHjj6PPU+8txRD9FoPmB1\n" \
"P/tG19XyruBPWUMSLuWc7py7q5Y3hajdQHcGvn2C4Mbj0rnKe+ipQSUCAwEAAaNv\n" \
"MG0wHQYDVR0OBBYEFFSwb+DQIiOx1NEj3jT0FTFsxasgMB8GA1UdIwQYMBaAFFSw\n" \
"b+DQIiOx1NEj3jT0FTFsxasgMA8GA1UdEwEB/wQFMAMBAf8wGgYDVR0RBBMwEYcE\n" \
"Ctn0dYIJbG9jYWxob3N0MA0GCSqGSIb3DQEBCwUAA4IBAQBCqs9lvgwGpf8keKdQ\n" \
"WsmMHKeSFEYu73zRYgcL7u0V/iWpHsJsjbim7NPa7E2KFoY/0RVk6a1FhuvVK7fT\n" \
"QR4/yOECbp5aMZh4NTSJbPfUsa4d3oeBSqUQq3h0hLXyJZi2DNVsyZ58bmuNDubn\n" \
"JmTL2+ZK/3zlzZNzLYxlPzr6KCXmqhB3BvaVrcqXoHF6drhFflqv/8u7nQkxNGrq\n" \
"EgO2ie9HKfOIZG3czqNXWOQWearJ0uHcrJQ2+bWXId4Tw699qWo4nIWEFsc1rFdT\n" \
"G4bs4FQIi+pBmdgWDGRoaSa27CwGVHBusR1lI18/shhs2Ocovwj76z3uTb1di7jT\n" \
"YXK6\n" \
"-----END CERTIFICATE-----\n";

unsigned long lastHeartbeat = 0;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n========================================");
    Serial.println("   ADMIN SYSTEM RESTORATION");
    Serial.println("========================================\n");
    
    connectWiFi();
    Serial.println("\n✓ Ready to restore admin access\n");
}

void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        connectWiFi();
        return;
    }
    
    if (millis() - lastHeartbeat >= 10000) {
        sendHeartbeat();
        lastHeartbeat = millis();
    }
    
    delay(100);
}

void connectWiFi() {
    Serial.print("[WiFi] Connecting to: ");
    Serial.println(WIFI_SSID);
    
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n[WiFi] ✓ Connected");
        Serial.print("[WiFi] IP: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\n[WiFi] ✗ Failed");
    }
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

void sendHeartbeat() {
    String payload = buildPayload();
    
    Serial.println("┌─────────────────────────────────────");
    Serial.println("│ SENDING RESTORATION HEARTBEAT");
    Serial.println("├─────────────────────────────────────");
    Serial.print("│ Device: ");
    Serial.println(DEVICE_ID);
    Serial.println("├─────────────────────────────────────");
    Serial.print("│ Payload: ");
    Serial.println(payload);
    Serial.println("└─────────────────────────────────────");
    
    WiFiClientSecure *client = new WiFiClientSecure;
    client->setCACert(root_ca);
    
    HTTPClient http;
    String url = String("https://") + SERVER_HOST + ":" + String(SERVER_PORT) + "/admin";
    
    Serial.print("[HTTPS] Connecting to: ");
    Serial.println(url);
    
    if (!http.begin(*client, url)) {
        Serial.println("[HTTPS] ✗ Connection failed");
        delete client;
        return;
    }
    
    http.addHeader("Content-Type", "application/json");
    http.setTimeout(10000);
    
    Serial.println("[HTTPS] Sending POST request...");
    int httpCode = http.POST(payload);
    
    Serial.print("[HTTPS] Response code: ");
    Serial.println(httpCode);
    
    if (httpCode == 200) {
        String response = http.getString();
        Serial.println("[HTTPS] ✓ SUCCESS!");
        Serial.println("─────────────────────────────────────");
        Serial.println(response);
        Serial.println("─────────────────────────────────────");
        
        if (response.indexOf("nextPhase") > 0) {
            Serial.println("\n🎉 ADMIN SYSTEM RESTORED!");
            Serial.println("Check the response above for Phase 2 link!");
        }
        
    } else if (httpCode == 401) {
        String response = http.getString();
        Serial.println("[HTTPS] ✗ UNAUTHORIZED");
        Serial.print("[HTTPS] Server says: ");
        Serial.println(response);
        
    } else if (httpCode > 0) {
        String response = http.getString();
        Serial.print("[HTTPS] ✗ Error ");
        Serial.print(httpCode);
        Serial.print(": ");
        Serial.println(response);
    } else {
        Serial.print("[HTTPS] ✗ Connection error: ");
        Serial.println(http.errorToString(httpCode));
    }
    
    http.end();
    delete client;
    Serial.println();
}