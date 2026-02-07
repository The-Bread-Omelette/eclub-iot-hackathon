#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// Admin Terminal - Among Us CTF
// Students search crew records to find impostors
// Need 7 out of 10 pieces of evidence to win

#define SERVICE_UUID        "12345678-1234-1234-1234-123456789abc"

#define SCORE_UUID          "00000001-1234-1234-1234-123456789abc"
#define SUBMIT_UUID         "00000002-1234-1234-1234-123456789abc"
#define EVIDENCE1_UUID      "00000003-1234-1234-1234-123456789abc"
#define EVIDENCE2_UUID      "00000004-1234-1234-1234-123456789abc"
#define EVIDENCE3_UUID      "00000005-1234-1234-1234-123456789abc"
#define EVIDENCE4_UUID      "00000006-1234-1234-1234-123456789abc"
#define EVIDENCE5_UUID      "00000007-1234-1234-1234-123456789abc"
#define EVIDENCE6_UUID      "00000008-1234-1234-1234-123456789abc"
#define EVIDENCE7_UUID      "00000009-1234-1234-1234-123456789abc"
#define EVIDENCE8_UUID      "0000000a-1234-1234-1234-123456789abc"
#define EVIDENCE9_UUID      "0000000b-1234-1234-1234-123456789abc"
#define EVIDENCE10_UUID     "0000000c-1234-1234-1234-123456789abc"

BLEServer* pServer = NULL;
BLECharacteristic* pScoreChar = NULL;
BLECharacteristic* pSubmitChar = NULL;

// Valid evidence answers - YOU CHANGE THESE based on your questions
String validEvidence[] = {
    "EVIDENCE_1_ANSWER",
    "EVIDENCE_2_ANSWER",
    "EVIDENCE_3_ANSWER",
    "EVIDENCE_4_ANSWER",
    "EVIDENCE_5_ANSWER",
    "EVIDENCE_6_ANSWER",
    "EVIDENCE_7_ANSWER",
    "EVIDENCE_8_ANSWER",
    "EVIDENCE_9_ANSWER",
    "EVIDENCE_10_ANSWER"
};

bool evidenceFound[10] = {false};
int evidenceCount = 0;

class SubmitCallback: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        std::string value = pCharacteristic->getValue();
        
        if (value.length() > 0) {
            String submitted = String(value.c_str());
            
            Serial.println("\n========================================");
            Serial.print("Evidence submitted: ");
            Serial.println(submitted);
            
            for (int i = 0; i < 10; i++) {
                if (submitted == validEvidence[i] && !evidenceFound[i]) {
                    evidenceFound[i] = true;
                    evidenceCount++;
                    
                    Serial.print("✓ VALID EVIDENCE! Piece ");
                    Serial.print(i + 1);
                    Serial.println(" collected");
                    Serial.print("Evidence: ");
                    Serial.print(evidenceCount);
                    Serial.println("/10");
                    
                    String scoreStr = "Evidence: " + String(evidenceCount) + "/10";
                    pScoreChar->setValue(scoreStr.c_str());
                    pScoreChar->notify();
                    
                    if (evidenceCount >= 7) {
                        Serial.println("\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
                        Serial.println("! IMPOSTORS IDENTIFIED!");
                        Serial.println("! CREWMATES WIN!");
                        Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
                    }
                    
                    Serial.println("========================================\n");
                    return;
                }
            }
            
            Serial.println("✗ Invalid evidence or already collected");
            Serial.println("========================================\n");
        }
    }
};

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n========================================");
    Serial.println("    Admin Terminal - Crew Records");
    Serial.println("========================================");
    Serial.println("Find 7 pieces of evidence to identify impostors");
    Serial.println("========================================\n");
    
    BLEDevice::init("ADMIN_TERMINAL");
    
    pServer = BLEDevice::createServer();
    
    BLEService *pService = pServer->createService(SERVICE_UUID);
    
    // Score characteristic
    pScoreChar = pService->createCharacteristic(
        SCORE_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
    );
    pScoreChar->setValue("Evidence: 0/10");
    pScoreChar->addDescriptor(new BLE2902());
    
    // Submit characteristic
    pSubmitChar = pService->createCharacteristic(
        SUBMIT_UUID,
        BLECharacteristic::PROPERTY_WRITE
    );
    pSubmitChar->setCallbacks(new SubmitCallback());
    
    // Evidence characteristics
    // YOU CUSTOMIZE THESE WITH YOUR QUESTIONS/CLUES
    
    // Evidence 1 - Direct read (easy)
    BLECharacteristic* pEvidence1 = pService->createCharacteristic(
        EVIDENCE1_UUID,
        BLECharacteristic::PROPERTY_READ
    );
    pEvidence1->setValue("EVIDENCE_1_ANSWER");
    
    // Evidence 2 - Hex encoded
    BLECharacteristic* pEvidence2 = pService->createCharacteristic(
        EVIDENCE2_UUID,
        BLECharacteristic::PROPERTY_READ
    );
    pEvidence2->setValue("4556494445... (convert to hex)");
    
    // Evidence 3 - In descriptor
    BLECharacteristic* pEvidence3 = pService->createCharacteristic(
        EVIDENCE3_UUID,
        BLECharacteristic::PROPERTY_READ
    );
    pEvidence3->setValue("Check the descriptor for this crew file");
    BLEDescriptor* desc3 = new BLEDescriptor(BLEUUID((uint16_t)0x2901));
    desc3->setValue("EVIDENCE_3_ANSWER");
    pEvidence3->addDescriptor(desc3);
    
    // Evidence 4-10 - Customize these
    BLECharacteristic* pEvidence4 = pService->createCharacteristic(
        EVIDENCE4_UUID,
        BLECharacteristic::PROPERTY_READ
    );
    pEvidence4->setValue("EVIDENCE_4_ANSWER");
    
    BLECharacteristic* pEvidence5 = pService->createCharacteristic(
        EVIDENCE5_UUID,
        BLECharacteristic::PROPERTY_READ
    );
    pEvidence5->setValue("EVIDENCE_5_ANSWER");
    
    BLECharacteristic* pEvidence6 = pService->createCharacteristic(
        EVIDENCE6_UUID,
        BLECharacteristic::PROPERTY_READ
    );
    pEvidence6->setValue("EVIDENCE_6_ANSWER");
    
    BLECharacteristic* pEvidence7 = pService->createCharacteristic(
        EVIDENCE7_UUID,
        BLECharacteristic::PROPERTY_READ
    );
    pEvidence7->setValue("EVIDENCE_7_ANSWER");
    
    BLECharacteristic* pEvidence8 = pService->createCharacteristic(
        EVIDENCE8_UUID,
        BLECharacteristic::PROPERTY_READ
    );
    pEvidence8->setValue("EVIDENCE_8_ANSWER");
    
    BLECharacteristic* pEvidence9 = pService->createCharacteristic(
        EVIDENCE9_UUID,
        BLECharacteristic::PROPERTY_READ
    );
    pEvidence9->setValue("EVIDENCE_9_ANSWER");
    
    BLECharacteristic* pEvidence10 = pService->createCharacteristic(
        EVIDENCE10_UUID,
        BLECharacteristic::PROPERTY_READ
    );
    pEvidence10->setValue("EVIDENCE_10_ANSWER");
    
    pService->start();
    
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
    
    Serial.println("Admin Terminal is active");
    Serial.println("Device name: ADMIN_TERMINAL");
    Serial.println("Waiting for investigators...\n");
}

void loop() {
    delay(2000);
}