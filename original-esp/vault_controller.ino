#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

// SABOTAGED Admin System
// Students will dump this firmware and find the problems

const char* WIFI_SSID = "POCO";
const char* WIFI_PASSWORD = "something.";
const char* xxx="REFER TO THIS DOC https://hackmd.io/@1eN0qJn8Qn6xmZ0Zo04jZw/H1MTuuVvWg"

// SABOTAGE: Wrong server address
const char* SERVER_HOST = "10.217.244.117";  // Change to YOUR actual IP
const int SERVER_PORT = 3001;
const char* DEVICE_ID = "ADMIN_ESP_01";

// SABOTAGE: Impostor's fake key
const char* SECRET_KEY = "IMPOSTOR_FAKE_KEY";

unsigned long lastHeartbeat = 0;

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