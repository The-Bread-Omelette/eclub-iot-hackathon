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
"MIIDEzCCAfugAwIBAgIUXEh8C5mriQirxDXHyg5NIijBz+8wDQYJKoZIhvcNAQEL\n" \
"BQAwGTEXMBUGA1UEAwwOMTAuMjE3LjI0NC4xMTcwHhcNMjYwMjA3MDgyNjM0WhcN\n" \
"MzYwMjA1MDgyNjM1WjAZMRcwFQYDVQQDDA4xMC4yMTcuMjQ0LjExNzCCASIwDQYJ\n" \
"KoZIhvcNAQEBBQADggEPADCCAQoCggEBALeQcOcdFOcftge7edqKhM8ESMNzEmbE\n" \
"vgZPf/j2mFIJJG+Ka78isd4c6heS0QRAoMZBfAK13XWRxFj5BgpOovp9eUYckTlG\n" \
"b8OC+PYZURnGSeEdQhdCztzQUAOyXm0pixvcg636CZr8JdvExwXtaGUpF3zu9qKw\n" \
"wSSlOHXnWgtzEH75mKUtzekydhvWrn6zvNVd98HL0a1F9IflZuD6WB73pM4s4uF+\n" \
"O+RTh31taFSDwspBF6TqUbBYVqyYacINLBfehPhcQWm8udUx8yXmPXB9016ob8Rb\n" \
"sVCiXSUpy4Q3SRyhrexw91CK5wcDvcCw7JKHbnobxkoBYJGcOWovvs0CAwEAAaNT\n" \
"MFEwHQYDVR0OBBYEFBNYX40bVj1g9TgtTa7+4zkquRKKMB8GA1UdIwQYMBaAFBNY\n" \
"X40bVj1g9TgtTa7+4zkquRKKMA8GA1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQEL\n" \
"BQADggEBADH+kOm5A+Cde5A0/flOgEsuG3jBqcjItjqxEd+TuXElaLVkdqncOYfi\n" \
"QLHjFjKXm5eCvkBqn0BZG5euRuhFzHmsWHbONHpkSf+CaM4a9gzNG6ym+MNbKMZl\n" \
"2Onm4EeKcR72ltNuw+2fXvL33AWXOR/ozfTJLklTC2j8ZnnaiAG6D3hVqxa6RGTI\n" \
"5bzWk+0rAk4vIk7t0Sm98br0Pa299EC5fDN8pl1oS/garDbdNoDXFg2IYRU/QrvN\n" \
"WXbeLm/40Cd/MGlYlNypMkj6JxTQEXUvlMFqJFEdEjKclYytYvHPUNLIVKSKEyti\n" \
"UnWHT4f8QQJTjIYjPGAvla6+YhgF9Mo=\n" \
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