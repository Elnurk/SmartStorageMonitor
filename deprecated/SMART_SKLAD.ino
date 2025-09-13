#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h> 
#include "DHT.h"
#include <MFRC522.h>
#include <Servo.h>

Servo EnterDoor; 
Servo ExitDoor;

uint8_t admin[4] = {180,253,163,44};
String kazirgi = "";

#define PN532_IRQ   (2) // 1_RFID_IRQ
#define PN532_RESET (3) // 1_RFID_RSTQ
Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET); // 1_RFID

#define RST_PIN   5     // Configurable, see typical pin layout above
#define SS_PIN    53    // Configurable, see typical pin layout above
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance

#define NEW_UID {0xDE, 0xAD, 0xBE, 0xEF}

MFRC522::MIFARE_Key key;

#define DHTPIN A0
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
float t;
float h;

bool Enter_Chel = false;

int DD = A1;
bool DD_TF = false;

int Sound = 10;

int led_r = 13;
int led_g = 12;
int led_b = 11;

bool Alarm = true;
bool Alert = false;

void setup() {
  Serial.begin(115200);
  EntryDoor.attach(9);  
  ExitDoor.attach(7);
  
  install_RFID_1();
  install_RFID_2();

  dht.begin();

  pinMode(DD, INPUT);

  pinMode(led_r, OUTPUT);
  pinMode(led_g, OUTPUT);
  pinMode(Sound, OUTPUT);

  EntryDoor.write(75);
  ExitDoor.write(65);  
}

void loop() {
  Check_Temp();
  DD_TF = digitalRead(DD);
  
  if(Alarm){
    digitalWrite(led_g, 0);
    CheckAlert();
    if(Alert){
      digitalWrite(led_r, 1);
      digitalWrite(Sound, 1);
      digitalWrite(led_b, 0);
    }
    else{
      digitalWrite(led_r, 0);
      digitalWrite(Sound, 0);
      digitalWrite(led_b, 1);
    }
    
    read_RFID_1();
  }
  else{
    digitalWrite(led_g, 1);
    digitalWrite(led_r, 0);
    digitalWrite(led_b, 0);
    
    read_RFID_2();
  }
}

void Check_Temp(){
  // Reading temperature or humidity takes about 250 milliseconds!
  h = dht.readHumidity();
  t = dht.readTemperature();
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  Serial.print(F("%  Humidity: "));
  Serial.println(h);
  Serial.print(F("%  Temperature: "));
  Serial.println(t);
  Serial.println(DD_TF);
}

void enter_open_door(){
  EntryServo.write(10);   
  delay(2000);    
  EntryServo.write(75); 
}

void exit_open_door(){
  ExitServo.write(110); 
  delay(2000);    
  ExitServo.write(65); 
}

void CheckAlert(){
  if(DD_TF || (t>60) || (h>70)){
    Alert = true;
    Serial.println("Alarm");
  }
  else{
    Alert = false;
    Serial.println("Waiting");
  }
}

void read_RFID_1(){
  boolean success;
  uint8_t uid[] = { 0, 0, 0, 0,};  // Buffer to store the returned UID
  uint8_t uidLength;        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
  
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength, 100);
  
  if (success) {
    Serial.println("Found a card!");
    Serial.print("UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
    Serial.print("UID Value: ");
    if ( array_cmp(admin, uid, 4, 4) == true ){
      Alarm = false;
      Serial.println("Entered");
      enter_open_door();
    }
    for (uint8_t i=0; i < uidLength; i++) 
    {
      Serial.print(" 0x");Serial.print(uid[i]);
    }
    Serial.println("");
  }
  else
  {
    Serial.println("Timed out waiting for a card");
  }
}

void read_RFID_2(){
  if ( ! mfrc522.PICC_IsNewCardPresent() || ! mfrc522.PICC_ReadCardSerial() ) {
    delay(50);
    return;
  }
  
  // Now a card is selected. The UID and SAK is in mfrc522.uid.
  
  // Dump UID
  if ( array_cmp(admin, mfrc522.uid.uidByte, 4, 4) == true ){
      Alarm = true;
      Serial.println("Left");
      exit_open_door();
    }
    
  Serial.print(F("Card UID:"));
  
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    Alarm = true;
  } 
  Serial.println();

  // Set new UID
  byte newUid[] = NEW_UID;
  if ( mfrc522.MIFARE_SetUid(newUid, (byte)4, true) ) {
    Serial.println(F("Wrote new UID to card."));
  }
  
  // Halt PICC and re-select it so DumpToSerial doesn't get confused
  mfrc522.PICC_HaltA();
  if ( ! mfrc522.PICC_IsNewCardPresent() || ! mfrc522.PICC_ReadCardSerial() ) {
    return;
  }
  
  // Dump the new memory contents
  Serial.println(F("New UID and contents:"));
  mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
}

void install_RFID_1(){
  while (!Serial) delay(10);

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
  
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
  
  nfc.setPassiveActivationRetries(0xFF);
  
  nfc.SAMConfig();
}

void install_RFID_2(){
  while (!Serial);     // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
  SPI.begin();         // Init SPI bus
  mfrc522.PCD_Init();  // Init MFRC522 card
  Serial.println(F("Warning: this example overwrites the UID of your UID changeable card, use with care!"));
  
  // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
}

boolean array_cmp(uint8_t *a, uint8_t *b, int len_a, int len_b){
      int n;
      // if their lengths are different, return false
      if (len_a != len_b) return false;
      // test each element to be the same. if not, return false
      for (n=0;n<len_a;n++) if (a[n]!=b[n]) return false;
      //ok, if we have not returned yet, they are equal :)
      return true;
}
