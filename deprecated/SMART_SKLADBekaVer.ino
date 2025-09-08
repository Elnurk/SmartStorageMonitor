#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h> 
#include "DHT.h"
#include <MFRC522.h>
#include <Servo.h>

#define FIREBASE_HOST "smart-storage-72e72-default-rtdb.europe-west1.firebasedatabase.app"
#define FIREBASE_AUTH "FSnockjffp226KnT2ToXzmt9k3XiFkvdD29kD2L6"
#define WIFI_SSID "your WIFI SSID"
#define WIFI_PASSWORD "your WIF PASSWORD"

Servo myservo_vhod; 
Servo myservo_vyhod;

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


///--------------------------------
#define DHTPIN A0
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
float h;
float t;

bool Enter_Chel = false;

int DD = A1;
bool DD_TF = false;

int ZVUK = 10;

int led_r = 13;
int led_g = 12;
int led_b = 11;

int servo_inp = 9;
int servo_out = 7;

bool Alarm = true;
bool Trivoga = false;

void setup(void) {
  Serial.begin(115200);

  myservo_vhod.attach(9);  
  myservo_vyhod.attach(7);
  
  install_RFID_1();
  install_RFID_2();

  Serial.println(F("DHTxx test!"));
  dht.begin();

  pinMode(DD, INPUT);

  pinMode(led_r, OUTPUT);
  pinMode(led_g, OUTPUT);
  pinMode(ZVUK, OUTPUT);

  myservo_vhod.write(75);
  myservo_vyhod.write(65);  
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());
  
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

void loop(void) {
  Check_Temp();
  Check_DD();
  
  if(Alarm){
    digitalWrite(led_g, 0);
//    chek_trivoga();
    if(Trivoga){
      digitalWrite(led_r, 1);
      digitalWrite(ZVUK, 1);
      digitalWrite(led_b, 0);
    }
    else{
      digitalWrite(led_r, 0);
      digitalWrite(ZVUK, 0);
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
  
  SendFirebase();
}

void Check_Temp(){
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  h = dht.readHumidity();
  t = dht.readTemperature();
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.println(t);
  Serial.println(DD_TF);
}

void Check_DD(){
  DD_TF = digitalRead(DD);
  Serial.println(DD_TF);
}


// RFID 1 -------------------------------------------------------------------------------
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
      Serial.println("KIRDI");
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
//--------------------------------------------------------------------------------------------------

// RFID 2 -------------------------------------------------------------------------------
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

void read_RFID_2(){
  if ( ! mfrc522.PICC_IsNewCardPresent() || ! mfrc522.PICC_ReadCardSerial() ) {
    delay(50);
    return;
  }
  
  // Now a card is selected. The UID and SAK is in mfrc522.uid.
  
  // Dump UID
  if ( array_cmp(admin, mfrc522.uid.uidByte, 4, 4) == true ){
      Alarm = true;
      Serial.println("Shuktu");
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
//--------------------------------------------------------------------------------------------------

void enter_open_door(){
  myservo_vhod.write(10);   
  delay(2000);    
  myservo_vhod.write(75); 
}

void exit_open_door(){
  myservo_vyhod.write(110); 
  delay(2000);    
  myservo_vyhod.write(65); 
}

void chek_trivoga(){
  if(DD_TF || (t>60) || (h>70)){
    Trivoga = true;
    
    Serial.println("triviga");
  }
  else{
    Trivoga = false;
    Serial.println("ahudania");
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

void SendFirebase() {
  Firebase.setFloat("humidity", h);
  if (Firebase.failed()) {
      Serial.print("setting humidity failed:");
      Serial.println(Firebase.error());  
      return; 
  }
  Firebase.setFloat("temperature", t);
  if (Firebase.failed()) {
      Serial.print("setting temperature failed:");
      Serial.println(Firebase.error());  
      return; 
  } 
}
