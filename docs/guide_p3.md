# Part 3: BLE Admin Terminal Investigation

In this exercise, you will interact with an ESP32-based Admin Terminal using Bluetooth Low Energy (BLE).

The terminal exposes multiple BLE characteristics. Some of these contain information related to crew activity. Some of this information counts as valid evidence.

Your task is to extract evidence and submit correct answers.

You must submit **at least 7 correct pieces of evidence** to complete this part.

---

## Required Tools (Linux)

Install the required Bluetooth utilities:

```bash
sudo apt update
sudo apt install bluetooth bluez bluez-tools
```

---

## Step 1: Flash the Firmware

You are provided with a compiled firmware binary file (ble.ino.merged.bin). [LINK](https://github.com/The-Bread-Omelette/uploads.git)

Flash the firmware onto the ESP32 using:

```bash
py -m esptool --chip esp32 --port COM16 --baud 460800 write_flash -z 0x0 ble.ino.merged.bin

```

After flashing completes, reset the ESP32.

---

## Step 2: Scan for BLE Devices

Scan for nearby BLE devices:

```bash
sudo hcitool lescan
```

Look for a device named:

```
ADMIN_TERMINAL
```

Note the MAC address and stop scanning using `Ctrl+C`.

---

## Step 3: Connect to the Device

Start an interactive BLE session:

```bash
gatttool -b <MAC_ADDRESS> -I
```

At the prompt, type:

```
connect
```

You should see:

```
Connection successful
```

---

## Step 4: List Available Characteristics

List all BLE characteristics exposed by the device:

```text
characteristics
```

You will see multiple handles, including:
- an evidence counter
- an answer submission characteristic
- several crew file characteristics

Each crew file may contain relevant information.

---

## Step 5: Read Crew Files

Read a crew file using its handle:

```text
char-read-hnd 0x0007
```

The output will be in hexadecimal format.

Example:

```text
43726577206d656d626572204a6f686e
```

---

## Step 6: Decode Hexadecimal Output

Convert hexadecimal output to readable text:

```bash
echo "43726577206d656d626572204a6f686e" | xxd -r -p
```

---

## Step 7: Answer the Questions

A separate question sheet will be provided by the instructor.

Match the decoded evidence to the questions and determine the correct answers.

---

## Step 8: Submit an Answer

Convert your answer to hexadecimal:

```bash
echo -n "ANSWER_TEXT" | xxd -ps
```

Submit the answer:

```text
char-write-req 0x0005 HEX_VALUE
```

---

## Step 9: Check Progress

Read the evidence counter:

```text
char-read-hnd 0x0003
```

Decode the value to check how many correct submissions you have made.

---

## Completion Condition

Once at least **7 correct pieces of evidence** have been submitted, the terminal will indicate successful completion on the serial output.

---

## Notes

- Firmware dumping is allowed but will not reveal answers
- Not all crew files contain valid evidence
- Carefully read and decode all accessible data
- The intended solution path is BLE interaction
