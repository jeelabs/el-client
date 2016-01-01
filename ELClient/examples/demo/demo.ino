/**
 * Demo sketch that makes a rest request to api.thingspeak.com with the RTT of the previous
 * request.
 */

#include <espduino.h>
#include <rest.h>

ESP esp(&Serial, 4);

REST rest(&esp);

boolean wifiConnected = false;

void wifiCb(void* response)
{
  uint32_t status;
  RESPONSE res(response);

  if(res.getArgc() == 1) {
    res.popArgs((uint8_t*)&status, 4);
    if(status == STATION_GOT_IP) {
      Serial.println("WIFI CONNECTED");
     
      wifiConnected = true;
    } else {
      wifiConnected = false;
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(10);
  Serial.println("ARDUINO: starting");
  esp.enable();
  delay(10);
  esp.reset();
  delay(10);
  while(!esp.ready()) {
    Serial.println("ARDUINO: ESP not ready...");
    delay(1000);
  }

  Serial.println("ARDUINO: setup rest client");
  if(!rest.begin("api.thingspeak.com")) {
    Serial.println("ARDUINO: failed to setup rest client");
    while(1);
  }

  /*setup wifi*/
  Serial.println("ARDUINO: setup wifi");
  esp.wifiCb.attach(&wifiCb);

  esp.wifiConnect("","");
  Serial.println("ARDUINO: system started");
}

void printChar(char c) {
  if (c < ' ' || c >= '~') {
    Serial.print("\\x");
    uint8_t c1 = (uint8_t)c >> 4;
    Serial.print((char)(c1 >= 10 ? 'A'+c1-10 : '0' + c1));
    uint8_t c2 = (uint8_t)c & 0xf;
    Serial.print((char)(c2 >= 10 ? 'A'+c2-10 : '0' + c2));
  } else {
    Serial.print(c);
  }
}

uint32_t timer = 0;

void loop() {
  char response[266];
  esp.process();
  if(wifiConnected) {
    char buff[64];
    uint32_t t0 = millis();
    sprintf(buff, "/update?api_key=MAY03AKJDMPP4Y4I&field1=%ld", timer);
    Serial.print("ARDUINO: send get ");
    Serial.println(buff);
    rest.get((const char*)buff);

    uint16_t err = rest.getResponse(response, 266);
    if(err == HTTP_STATUS_OK){
      Serial.println("ARDUINO: GET successful");
      Serial.print("ARDUINO: got <<");
      int len = strlen(response);
      for (int i=0; i<len; i++) printChar(response[i]);
      Serial.println(">>");
    } else if (err == 0) {
      Serial.println("ARDUINO: GET timed out");
    } else {
      Serial.print("ARDUINO: GET failed: ");
      Serial.println(err);
    }
    timer = millis() - t0;
    Serial.print("ARDUINO: took ");
    Serial.print(timer);
    Serial.println("ms");
    delay(3000);
  }
  delay(100);
}
