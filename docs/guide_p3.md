# Part 3: Admin Terminal Investigation

You've gained access to the admin terminal. Now find the impostors!

---

## The Situation

The admin terminal contains crew member files.

Hidden in these files are **10 pieces of evidence** pointing to suspicious behavior.

You need **7 pieces of evidence** to positively identify the impostors.

---

## What is BLE?

BLE (Bluetooth Low Energy) is how the admin terminal stores data.

Think of it like a filing cabinet with 10 drawers (characteristics).

Each drawer might contain evidence.

---

## Tools Setup
```bash
sudo apt-get install bluetooth bluez bluez-tools
```

---

## Step 1: Flash Admin Terminal Firmware

Upload `ble_ctf.ino` to your ESP32.

**Must be ESP32** (ESP8266 doesn't have Bluetooth).

---

## Step 2: Scan for Terminal
```bash
sudo hcitool lescan
```

Look for: **ADMIN_TERMINAL**

Example output:
```
AA:BB:CC:DD:EE:FF ADMIN_TERMINAL
```

Copy the MAC address. Press Ctrl+C to stop.

---

## Step 3: Connect
```bash
gatttool -b AA:BB:CC:DD:EE:FF -I
```

At the prompt, type:
```
connect
```

Should see: `Connection successful`

---

## Step 4: List All Files
```
characteristics
```

You'll see crew file handles:
```
handle: 0x0003, uuid: 00000001-...  ← Evidence counter
handle: 0x0005, uuid: 00000002-...  ← Submit evidence here
handle: 0x0007, uuid: 00000003-...  ← Crew file 1
handle: 0x0009, uuid: 00000004-...  ← Crew file 2
...
```

---

## Step 5: Read Crew Files

Read each file:
```
char-read-hnd 0x0007
```

Output (hex):
```
4372 6577 206d 656d 6265 7220 4a6f 686e
```

---

## Step 6: Decode Evidence

Convert hex to text:
```bash
echo "4372657720..." | xxd -r -p
```

Might reveal: `Crew member John was seen in electrical`

Or use online converter: https://www.rapidtables.com/convert/number/hex-to-ascii.html

---

## Step 7: Answer Questions

Your instructor gave you a question sheet like:

**Q1: Who was last seen in electrical before the blackout?**

If you find evidence pointing to "JOHN", that's your answer.

---

## Step 8: Submit Evidence

Convert answer to hex:
```bash
echo -n "JOHN" | xxd -ps
```

Output: `4a4f484e`

Submit:
```
char-write-req 0x0005 4a4f484e
```

---

## Step 9: Check Progress
```
char-read-hnd 0x0003
```

Decode to see: `Evidence: 1/10`

---

## Investigation Strategy

1. Read ALL crew files (handles 0x0007, 0x0009, 0x000b, etc.)
2. Decode each one
3. Match evidence to questions
4. Submit answers
5. Repeat until 7/10 found

---

## Quick Commands

**Scan:**
```bash
sudo hcitool lescan
```

**Connect:**
```bash
gatttool -b MAC -I
connect
```

**List files:**
```
characteristics
```

**Read:**
```
char-read-hnd 0x0007
```

**Submit:**
```
char-write-req 0x0005 HEX_ANSWER
```

**Check score:**
```
char-read-hnd 0x0003
```

---

## Hex Conversion

**Text → Hex:**
```bash
echo -n "TEXT" | xxd -ps
```

**Hex → Text:**
```bash
echo "48656c6c6f" | xxd -r -p
```

---

## Tips

- Some evidence is plain text
- Some is hex-encoded (decode it!)
- Some might be hidden in descriptors (check!)
- Read everything carefully
- Keep notes on what you find

---

## Winning

After submitting 7 correct pieces of evidence:

ESP32 Serial Monitor shows:
```
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
IMPOSTORS IDENTIFIED!
CREWMATES WIN!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
```

Show this to your instructor - you've completed the challenge! 🎉