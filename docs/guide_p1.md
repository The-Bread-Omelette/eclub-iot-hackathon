# Part 1: Dumping ESP Firmware

## What is firmware?

Firmware is the code that runs on the ESP chip. It's like the brain of the device.

When someone writes code for an ESP and uploads it, the code gets stored in the ESP's flash memory.

We can **dump** (copy) this memory to see what's inside.

---

## Why dump firmware?

Sometimes developers hardcode secrets in their code:
- WiFi passwords
- API keys
- Server addresses
- Device IDs

If we can read the firmware, we can find these secrets.

---

## Step 1: Connect the ESP

1. Plug the ESP into your laptop via USB
2. Find which port it's connected to:

**Linux/Mac:**
```bash
ls /dev/tty* | grep -E "USB|ACM"
```

**Windows:**
Check Device Manager → Ports (COM & LPT)

You'll see something like `/dev/ttyUSB0` or `COM3`

---

## Step 2: Dump the Flash Memory

Run this command (replace `/dev/ttyUSB0` with your port):
```bash
esptool.py --port /dev/ttyUSB0 --baud 115200 read_flash 0x00000 0x400000 firmware_dump.bin
```

**What this does:**
- `--port /dev/ttyUSB0` - which port to use
- `read_flash` - command to read memory
- `0x00000` - start address (beginning of memory)
- `0x400000` - how much to read (4MB, typical ESP size)
- `firmware_dump.bin` - output file name


---

## Step 3: Extract Readable Strings

The dump file is binary (looks like gibberish).

But some parts are text (strings) - like WiFi passwords, URLs, etc.

Extract them:
```bash
strings firmware_dump.bin > strings.txt
```

Open `strings.txt` in a text editor.

---

## Step 4: Look for Secrets

Search for:
- WiFi SSID names
- Passwords
- IP addresses (format: `192.168.x.x`)
- URLs (starting with `http://` or `https://`)
- Keywords like "key", "secret", "password"

**Tip:** Use Ctrl+F (Find) in your text editor to search.

## Step 5: Open Wireshark and monitor network

We know the addr ip and port 
- we will filter using 

```
ip.addr ==<IP> && tcp.port == 3001
```

We will notice a periodic request every 5 mins
