#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

/* ================= CONFIG ================= */

constexpr uint8_t XOR_KEY = 0xA9;
constexpr uint8_t REQUIRED_EVIDENCE = 7;
constexpr uint8_t TOTAL_FLAGS = 10;

/* ================= STATE ================= */

uint8_t evidenceCount = 0;
bool solved[TOTAL_FLAGS] = {0};

BLECharacteristic *counterChar;
BLECharacteristic *submitChar;

/* ================= DJB2 HASH ================= */

uint32_t djb2(const char *s) {
  uint32_t h = 5381;
  while (*s) {
    h = ((h << 5) + h) + (uint8_t)(*s);
    s++;
  }
  return h;
}

/* ================= QUESTIONS ================= */

const char *QUESTIONS[TOTAL_FLAGS] = {
  "What is the best SnT Club in IITK?",
  "Which area was John last seen in?",
  "Who sabotaged the reactor?",
  "Flag 4?",
  "Flag 5?",
  "Flag 6?",
  "Flag 7?",
  "Flag 8?",
  "Flag 9?",
  "Flag 10?"
};

/* ================= ANSWER HASHES ================= */
/* Generated OFFLINE using hash_gen.py */

uint32_t ANSWER_HASHES[TOTAL_FLAGS] = {
  0x8A3F1C22,
  0x6D3B98F1,
  0x9C4AA771,
  0x11111111,
  0x22222222,
  0x33333333,
  0x44444444,
  0x55555555,
  0x66666666,
  0x77777777
};

/* ================= QUESTION CALLBACK ================= */

class QuestionCallback : public BLECharacteristicCallbacks {
  int index;

public:
  QuestionCallback(int i) : index(i) {}

  void onRead(BLECharacteristic *c) override {
    const char *q = QUESTIONS[index];
    size_t len = strlen(q);

    uint8_t buf[128];
    for (size_t i = 0; i < len; i++) {
      buf[i] = q[i] ^ (XOR_KEY + i);
    }

    c->setValue(buf, len);
  }
};

/* ================= SUBMIT CALLBACK ================= */

class SubmitCallback : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *c) override {
    String v = c->getValue();
    uint32_t h = djb2(v.c_str());

    for (int i = 0; i < TOTAL_FLAGS; i++) {
      if (!solved[i] && h == ANSWER_HASHES[i]) {
        solved[i] = true;
        evidenceCount++;

        counterChar->setValue(&evidenceCount, 1);

        Serial.print("[+] Evidence found: ");
        Serial.println(evidenceCount);

        if (evidenceCount >= REQUIRED_EVIDENCE) {
          Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
          Serial.println("IMPOSTORS IDENTIFIED!");
          Serial.println("CREWMATES WIN!");
          Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
        }
        break;
      }
    }
  }
};

/* ================= SETUP ================= */

void setup() {
  Serial.begin(115200);
  Serial.println("ADMIN TERMINAL BOOTING");

  BLEDevice::init("ADMIN_TERMINAL");
  BLEServer *server = BLEDevice::createServer();

  BLEService *service =
      server->createService("12345678-1234-1234-1234-123456789000");

  counterChar = service->createCharacteristic(
      "12345678-1234-1234-1234-123456789001",
      BLECharacteristic::PROPERTY_READ);

  submitChar = service->createCharacteristic(
      "12345678-1234-1234-1234-123456789002",
      BLECharacteristic::PROPERTY_WRITE);

  counterChar->setValue(&evidenceCount, 1);
  submitChar->setCallbacks(new SubmitCallback());

  for (int i = 0; i < TOTAL_FLAGS; i++) {
    BLECharacteristic *q = service->createCharacteristic(
        BLEUUID((uint16_t)(0x3000 + i)),
        BLECharacteristic::PROPERTY_READ);

    q->setCallbacks(new QuestionCallback(i));
  }

  service->start();
  BLEDevice::getAdvertising()->start();

  Serial.println("ADMIN TERMINAL ONLINE");
}

/* ================= LOOP ================= */

void loop() {}
