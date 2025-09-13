#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
#include "DHT.h"
#include <MFRC522.h>
#include <Servo.h>

Servo EnterDoor;
Servo ExitDoor;

uint8_t admin[4] = {180,253,163,44};

#define PN532_IRQ 2
#define PN532_RESET 3
Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);

#define RST_PIN 5
#define SS_PIN 53
MFRC522 mfrc522(SS_PIN, RST_PIN)

#define NEW_UID {0xDE, 0xAD, 0xBE, 0xEF}

MFRC522::MIFARE_Key key;

#define DHTPIN A0
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
float t;
float h;
int lightPin = A2;
int lightVal = 0;

bool Enter_Chel = false;

int DD = A1;
bool DD_TF = false;

int Sound = 10;

int led_r = 13;
int led_g = 12;
int led_b = 11;

bool Alarm = true;
bool Alert = false;

String cmdBuf = "";
const int INV_MAX = 32;
String invUID[INV_MAX];
int invCount[INV_MAX];

boolean array_cmp(uint8_t *a, uint8_t *b, int len_a, int len_b) {
  int n;
  if (len_a != len_b) return false;
  for (n = 0; n < len_a; n++) if (a[n] != b[n]) return false;
  return true;
}

String uidToString(uint8_t *uid, uint8_t len) {
  String s = "";
  for (uint8_t i = 0; i < len; i++) {
    if (uid[i] < 16) {
      s += "0";
    }
    s += String(uid[i], HEX);
  }
  s.toUpperCase();
  return s;
}

int invFind(String id) {
  for (int i = 0; i < INV_MAX; i++) {
    if (invUID[i].length() > 0 && invUID[i] == id) {
      return i;
    }
  }
  return -1;
}

void invAdd(String id) {
  int idx = invFind(id);
  if (idx >= 0) {
    invCount[idx] = invCount[idx] + 1;
    return;
  }
  for (int i = 0; i < INV_MAX; i++) {
    if (invUID[i].length() == 0) {
      invUID[i] = id;
      invCount[i] = 1;
      return;
    }
  }
}

void invRemove(String id) {
  int idx = invFind(id);
  if (idx >= 0) {
    if (invCount[idx] > 0) {
      invCount[idx] = invCount[idx] - 1;
    }
  }
}

void invClear() {
  for (int i = 0; i < INV_MAX; i++) {
    invUID[i] = "";
    invCount[i] = 0;
  }
}

void invList() {
  Serial.println("INVENTORY");
  for (int i = 0; i < INV_MAX; i++) {
    if (invUID[i].length() > 0) {
      Serial.print(invUID[i]);
      Serial.print(" ");
      Serial.println(invCount[i]);
    }
  }
}

void enter_open_door() {
  EnterDoor.write(10);
  delay(2000);
  EnterDoor.write(75);
}

void exit_open_door() {
  ExitDoor.write(110);
  delay(2000);
  ExitDoor.write(65);
}

void install_RFID_1() {
  while (!Serial) delay(10);
  nfc.begin();
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    while (1);
  }
  nfc.setPassiveActivationRetries(0xFF);
  nfc.SAMConfig();
}

void install_RFID_2() {
  while (!Serial);
  SPI.begin();
  mfrc522.PCD_Init();
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
}

void Check_Temp() {
  h = dht.readHumidity();
  t = dht.readTemperature();
  lightVal = analogRead(lightPin);
  if (isnan(h) || isnan(t)) {
    Serial.println("DHT_FAIL");
    return;
  }
  Serial.print("H=");
  Serial.print(h);
  Serial.print(" T=");
  Serial.print(t);
  Serial.print(" L=");
  Serial.print(lightVal);
  Serial.print(" M=");
  Serial.println(DD_TF ? 1 : 0);
}

void CheckAlert() {
  if (DD_TF) {
    Alert = true;
    return;
  }
  Alert = false;
}

void read_RFID_1() {
  boolean success;
  uint8_t uid[] = {0, 0, 0, 0};
  uint8_t uidLength;
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength, 100);
  if (success) {
    if (array_cmp(admin, uid, 4, 4) == true) {
      Alarm = false;
      enter_open_door();
      return;
    }
    if (!Alarm) {
      String id = uidToString(uid, uidLength);
      invAdd(id);
      enter_open_door();
      Serial.print("IN+ ");
      Serial.println(id);
    }
    return;
  }
}

void read_RFID_2() {
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    delay(50);
    return;
  }
  if (array_cmp(admin, mfrc522.uid.uidByte, 4, 4) == true) {
    Alarm = true;
    exit_open_door();
    mfrc522.PICC_HaltA();
    return;
  }
  if (!Alarm) {
    String id = uidToString(mfrc522.uid.uidByte, mfrc522.uid.size);
    invRemove(id);
    exit_open_door();
    Serial.print("OUT- ");
    Serial.println(id);
  }
  mfrc522.PICC_HaltA();
}

void readSerial() {
  while (Serial.available()) {
    char c = (char)Serial.read();
    if (c == '\n' || c == '\r') {
      if (cmdBuf.length() > 0) {
        if (cmdBuf == "ALARM ON") {
          Alarm = true;
        }
        else if (cmdBuf == "ALARM OFF") {
          Alarm = false;
        }
        else if (cmdBuf == "OPEN ENTER") {
          enter_open_door();
        }
        else if (cmdBuf == "OPEN EXIT") {
          exit_open_door();
        }
        else if (cmdBuf == "INV LIST") {
          invList();
        }
        else if (cmdBuf == "INV CLEAR") {
          invClear();
        }
        else if (cmdBuf == "ENV") {
          Serial.print("H=");
          Serial.print(h);
          Serial.print(" T=");
          Serial.print(t);
          Serial.print(" L=");
          Serial.print(lightVal);
          Serial.print(" M=");
          Serial.println(DD_TF ? 1 : 0);
        }
        else if (cmdBuf == "LED R ON") {
          digitalWrite(led_r, 1);
        }
        else if (cmdBuf == "LED R OFF") {
          digitalWrite(led_r, 0);
        }
        else if (cmdBuf == "LED G ON") {
          digitalWrite(led_g, 1);
        }
        else if (cmdBuf == "LED G OFF") {
          digitalWrite(led_g, 0);
        }
        else if (cmdBuf == "LED B ON") {
          digitalWrite(led_b, 1);
        }
        else if (cmdBuf == "LED B OFF") {
          digitalWrite(led_b, 0);
        }
        else if (cmdBuf == "HELP") {
          Serial.println("ALARM ON|ALARM OFF|OPEN ENTER|OPEN EXIT|INV LIST|INV CLEAR|ENV|LED R ON|LED R OFF|LED G ON|LED G OFF|LED B ON|LED B OFF");
        }
        cmdBuf = "";
      }
    }
    else {
      cmdBuf += c;
      if (cmdBuf.length() > 80) {
        cmdBuf = "";
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  EnterDoor.attach(9);
  ExitDoor.attach(7);
  install_RFID_1();
  install_RFID_2();
  dht.begin();
  pinMode(DD, INPUT);
  pinMode(led_r, OUTPUT);
  pinMode(led_g, OUTPUT);
  pinMode(led_b, OUTPUT);
  pinMode(Sound, OUTPUT);
  EnterDoor.write(75);
  ExitDoor.write(65);
}

void loop() {
  readSerial();
  DD_TF = digitalRead(DD);
  Check_Temp();
  if (Alarm) {
    digitalWrite(led_g, 0);
    CheckAlert();
    if (Alert) {
      digitalWrite(led_r, 1);
      digitalWrite(Sound, 1);
      digitalWrite(led_b, 0);
    }
    else {
      digitalWrite(led_r, 0);
      digitalWrite(Sound, 0);
      digitalWrite(led_b, 1);
    }
    read_RFID_1();
  }
  else {
    digitalWrite(led_g, 1);
    digitalWrite(led_r, 0);
    digitalWrite(led_b, 0);
    read_RFID_2();
  }
}
