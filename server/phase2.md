# Phase 2: Bluetooth CTF Challenge

Congratulations! You've successfully sabotaged the vault's network connection.

The vault's security system has a backup - a **Bluetooth terminal** that only activates when network attacks are detected.

Your mission: Hack the Bluetooth terminal to gain full admin access.

---

## What You Need

1. **ESP32 board** (with Bluetooth)
2. **Bluetooth tools** on your laptop
3. **The firmware code below**

---

## Step 1: Flash BLE Firmware

Upload this code to your ESP32:

[Download BLE CTF Firmware](http://localhost:3000/ble_firmware.ino)

Or copy from the instructor.

---

## Step 2: Scan for the Device

Open a terminal and run:
```bash
sudo hcitool lescan
```

Look for a device named: **IMPOSTOR_TERMINAL**

Note its MAC address (looks like `AA:BB:CC:DD:EE:FF`)

---

## Step 3: Connect to It
```bash
gatttool -b AA:BB:CC:DD:EE:FF -I
```

Then type:
```
connect
```

---

## Step 4: Find the Flags

The terminal has 10 hidden flags (answers to questions).

Find **7 out of 10** to unlock admin access.

### List all characteristics:
```
characteristics
```

You'll see handles like `0x0003`, `0x0005`, etc.

### Read a handle:
```
char-read-hnd 0x0003
```

This shows hex values. Convert to text to find flags.

### Submit a flag:
```
char-write-req 0x0005 <your_answer_in_hex>
```

---

## Helpful Commands

**Check your score:**
```
char-read-hnd 0x0003
```

**Convert text to hex (for submitting):**
```bash
echo -n "ANSWER" | xxd -ps
```

**Convert hex to text (for reading flags):**
```bash
echo "48656c6c6f" | xxd -r -p
```

---

## Questions

Ask your instructor for the question sheet.

Each question has a specific answer (the flag).

Find the flags hidden in different Bluetooth characteristics and submit them.

**You need 7 correct answers to win!**

Good luck, Impostor! 🕵️