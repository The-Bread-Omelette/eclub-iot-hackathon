# Part 2: Restoring the Admin System

You've successfully dumped the sabotaged firmware. Now it's time to analyze what you found and write clean code to restore admin access.

---

## What We're Doing

The **admin system is compromised**. When you dumped the firmware, you found it was sending heartbeats to the server, but the server keeps rejecting them:
```
401 - Admin system still sabotaged
```

**Your mission:**
1. Analyze the dumped firmware
2. Identify what the impostor changed
3. Write proper restoration code
4. Gain access to Phase 2

---

## What You Found in the Firmware Dump

From your strings analysis, you extracted:
```cpp
const char* WIFI_SSID = "...";
const char* WIFI_PASSWORD = "...";
const char* SERVER_HOST = "...";
const int SERVER_PORT = 3001;
const char* DEVICE_ID = "";
const char* SECRET_KEY = "";
```

The sabotaged ESP32 sends heartbeats, but the server rejects them. Something in that code is wrong.

---

## Your Task: Write Clean Code

The impostor sabotaged the admin system. Your job is to write **clean, legitimate admin code** that the server will accept.

**Think about:**
- What would a real admin system send?
- What did the impostor add or change?
- What's the correct secret key for a restored system?

---

## Code Template

Create `admin_restore.ino`:
```cpp
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

// ============================================
// CONFIGURATION - Fill from firmware dump
// ============================================

const char* WIFI_SSID = "";           
const char* WIFI_PASSWORD = "";       
const char* SERVER_HOST = "";         
const int SERVER_PORT = 3001;

const char* DEVICE_ID = "";  // Your device ID

// TODO: What's the correct secret key for a RESTORED admin system?
const char* SECRET_KEY = "";  

const char* root_ca = \
"-----BEGIN CERTIFICATE-----\n" \
// ... copy from cert ...
"-----END CERTIFICATE-----\n";

unsigned long lastHeartbeat = 0;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n=== ADMIN SYSTEM RESTORATION ===\n");
    
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
    // Build JSON payload for admin heartbeat
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
    
    Serial.println("┌─────────────────────────────");
    Serial.println("│ SENDING HEARTBEAT");
    Serial.println("├─────────────────────────────");
    Serial.print("│ ");
    Serial.println(payload);
    Serial.println("└─────────────────────────────");
    
    WiFiClientSecure *client = new WiFiClientSecure;
    client->setCACert(root_ca);
    
    HTTPClient http;
    String url = String("https://") + SERVER_HOST + ":" + String(SERVER_PORT) + "/admin";
    
    if (!http.begin(*client, url)) {
        Serial.println("[HTTPS] ✗ Connection failed");
        delete client;
        return;
    }
    
    http.addHeader("Content-Type", "application/json");
    http.setTimeout(10000);
    
    int httpCode = http.POST(payload);
    
    Serial.print("[HTTPS] Response: ");
    Serial.println(httpCode);
    
    if (httpCode == 200) {
        String response = http.getString();
        Serial.println("[HTTPS] ✓ SUCCESS!");
        Serial.println("─────────────────────────────");
        Serial.println(response);
        Serial.println("─────────────────────────────");
        
        if (response.indexOf("nextPhase") > 0) {
            Serial.println("\n🎉 ADMIN ACCESS RESTORED!");
            Serial.println("Check response for Phase 2 link");
        }
        
    } else if (httpCode == 401) {
        String response = http.getString();
        Serial.println("[HTTPS] ✗ UNAUTHORIZED");
        Serial.println(response);
        
    } else if (httpCode > 0) {
        String response = http.getString();
        Serial.print("[HTTPS] ✗ Error: ");
        Serial.println(response);
    } else {
        Serial.println("[HTTPS] ✗ Connection error");
    }
    
    http.end();
    delete client;
    Serial.println();
}
```

---

## How to Solve This

1. **Fill in WiFi credentials** from your firmware dump
2. **Think about the secret key**: 
   - The impostor used `"IMPOSTOR_FAKE_KEY"`
   - What would a **restored** admin system use?
   - Hint: Think about what "restored" means
3. **Upload and test**

---

## Testing

Upload to your ESP32 and open Serial Monitor (115200 baud).

**Still unauthorized?**
```
[HTTPS] Response: 401
[HTTPS] ✗ UNAUTHORIZED
```
→ Check your secret key

**Success looks like:**
```
[HTTPS] Response: 200
[HTTPS] ✓ SUCCESS!
{
  "status": "restored",
  "message": "Admin system restored! Access granted.",
  "nextPhase": "http://..."
}

🎉 ADMIN ACCESS RESTORED!
```

---

## Common Issues

**"Response code: 401"**
- Wrong secret key
- Think: what should a RESTORED system identify itself with?

**"Response code: -1"**
- Server not reachable
- Check `SERVER_HOST` IP
- Verify server is running

**"WiFi connection failed"**
- Double-check SSID/password from dump

---

## The Challenge

You found the impostor's fake key. Now you need to figure out what the **correct** key should be for a properly restored admin system.

The answer is in thinking about what you're trying to achieve: **restoring** the admin system.

Good luck! 