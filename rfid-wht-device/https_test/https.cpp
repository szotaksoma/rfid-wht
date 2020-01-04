#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h> 
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
 
#define SSID ""
#define PSK ""
#define HTTPS_TIMEOUT 15000
#define HTTPS_MAX_RETRIES 10

#define TARGET_HOST "postman-echo.com"
#define TARGET_URL "/post"
#define HTTPS_PORT 443

// SHA1 finger print of certificate
const char fingerprint[] PROGMEM = "B5 32 1E 55 8E F7 29 81 22 A4 DA 78 97 EA 8A 27 82 A8 F5 1C";
 
void setup() {
  Serial.begin(9600);
  
  WiFi.mode(WIFI_OFF);
  delay(1000);
  WiFi.mode(WIFI_STA);  
  WiFi.begin(SSID, PSK);
  Serial.printf("Connecting to AP '%s'\n", SSID);

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
  }
 
  Serial.printf("Connected to '%s'\n", SSID);
  Serial.println(WiFi.localIP());

}

void send_request() {
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
    Serial.printf("Connection failed\n");
    return;
  }
  
  Serial.printf("Sending POST request\n");

  httpsClient.print(String("POST ") + TARGET_URL + " HTTP/1.1\r\n" +
               "Host: " + TARGET_HOST + "\r\n" +
               "Content-Type: application/x-www-form-urlencoded"+ "\r\n" +
               "Content-Length: 18" + "\r\n\r\n" +
               "say=Hello&to=World" + "\r\n" +
               "Connection: close\r\n\r\n");
 
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
  }

  Serial.print("\n");

}
 
void loop() {
  send_request();
  delay(10000);
}
