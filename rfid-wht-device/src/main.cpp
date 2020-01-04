#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h> 
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <MFRC522.h>
#include <Hash.h>
#include <SPI.h>
 
#define SSID ""
#define PSK ""
#define HTTPS_TIMEOUT 15000
#define HTTPS_MAX_RETRIES 10
#define WIFI_CONNECT_MAX_RETRIES 100

#define TARGET_HOST "wht.datapp.tech"
#define TARGET_URL "/rfid_wht.php"
#define API_KEY ""
#define CONTENT_TEMPLATE_LENGTH 8
#define HASH_HEX_LENGTH 40
#define HTTPS_PORT 443

#define RFID_SDA 2
#define RFID_SCK 14
#define RFID_MOSI 13
#define RFID_MISO 12
#define RFID_RST 0
#define CARD_CHECK_INTERVAL 100

#define BUZZER 16
#define BUZZER_FREQ 2400
#define BUZZER_DURATION 80

// #define BEEP
#define ONLINE

MFRC522 rfid(RFID_SDA, RFID_RST);

// SHA1 finger print of certificate
const char fingerprint[] PROGMEM = "53 CF D4 E8 AE D7 2C 39 7D 89 30 83 89 6B 1D D4 C6 78 F3 8A";

// Helping array for converting bytes to HEX strings
const char hexmap[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'C', 'D', 'E', 'F'};

void(* reset) (void) = 0;

void beep(int beeps) {
  #ifdef BEEP
    for(int i = 0; i < beeps; i++) {
      tone(BUZZER, BUZZER_FREQ, BUZZER_DURATION);
      delay(2 * BUZZER_DURATION);
    }
  #else
    Serial.printf("BEEP (%d)\n", beeps);
  #endif
}
 
void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();

  #ifdef ONLINE
    WiFi.mode(WIFI_OFF);
    delay(1000);
    WiFi.mode(WIFI_STA);  
    WiFi.begin(SSID, PSK);
    Serial.printf("Connecting to AP '%s'\n", SSID);
    int i = 0;
    while (WiFi.status() != WL_CONNECTED && i < WIFI_CONNECT_MAX_RETRIES) {
      delay(100);
      i++;
    }

    if (i == WIFI_CONNECT_MAX_RETRIES)
    {
      Serial.printf("Couldn't connect to network, resetting\n");
      beep(5);
      reset();
    }
    
  
    Serial.printf("Connected to '%s'\n", SSID);
    Serial.println(WiFi.localIP());
  #endif

}

int send_request(String uid) {
  Serial.print("Sending hash: ");
  Serial.println(uid);
  WiFiClientSecure httpsClient;
  Serial.printf("Setting up connection to '%s'\n", TARGET_HOST);
  Serial.printf("Using fingerprint '%s'\n", fingerprint);
  httpsClient.setFingerprint(fingerprint);
  httpsClient.setTimeout(HTTPS_TIMEOUT);
  
  Serial.printf("Estalishing secure connection\n");
  int retry_count = 0;

  while((!httpsClient.connect(TARGET_HOST, HTTPS_PORT)) && (retry_count < HTTPS_MAX_RETRIES)){
      delay(100);
      retry_count++;
  }

  if(retry_count == HTTPS_MAX_RETRIES) {
    Serial.printf("Connection failed, resetting\n");
    return -1;
  }
  
  Serial.printf("Sending POST request\n");

  httpsClient.print(String("POST ") + TARGET_URL + " HTTP/1.1\r\n" +
               "Host: " + TARGET_HOST + "\r\n" +
               "Content-Type: application/x-www-form-urlencoded"+ "\r\n" +
               "Content-Length: " + String(HASH_HEX_LENGTH + CONTENT_TEMPLATE_LENGTH) + "\r\n" +
               "X-Api-Key: " + API_KEY + "\r\n\r\n"
               "wht-uid=" + uid + "\r\n" +
               "Connection: close\r\n\r\n"
  );
 
  Serial.printf("\nResponse headers:\n\n");
  while (httpsClient.connected()) {
    String line = httpsClient.readStringUntil('\n');
    if (line == "\r") {
      break;
    } else {
      Serial.println(line);
    }
  }
 
  Serial.printf("\nResponse body:\n\n");
  String line;
  while(httpsClient.available()){        
    line = httpsClient.readStringUntil('\n');
    Serial.println(line);
    if (line.indexOf('{') != -1) {
      int stateIndex;
      
      stateIndex = line.indexOf("state");
      if (stateIndex != -1) {
        stateIndex += 8;
      } else {
        return -1;
      }
      
      if (line.charAt(stateIndex) == '1') {
        return 1;
      }
      return 0;
    }
  }
  Serial.print("\n");
  return -1;
}

char* get_uid() {
  char* uid = (char*)malloc(sizeof(char) * (2 * rfid.uid.size + 1));
  for (int i = 0; i < rfid.uid.size; i++) {
    uid[2 * i] = hexmap[(rfid.uid.uidByte[i] & 0xF0) >> 4];
    uid[2 * i + 1] = hexmap[rfid.uid.uidByte[i] & 0x0F];
  }
  uid[2 * rfid.uid.size] = '\0';
  Serial.printf("Card UID: %s\n", uid);
  beep(1);
  return uid;
}

void check_card() {
  int state;
  if(!rfid.PICC_IsNewCardPresent()) {
    return;
  }
  if(!rfid.PICC_ReadCardSerial()) {
    return;
  }
  char* uid = get_uid();
  #ifdef ONLINE
    state = send_request(sha1(String(uid)));
  #endif
  if (state == -1) {
    beep(5);
    reset();
  } else {
    beep(state ? 1 : 2);
  }
  free((void*)uid);
}

void loop() {
  check_card();
  delay(CARD_CHECK_INTERVAL);
}
