/**
 * Simple example to demo the El-Client REST calls
 *
 */

#include <ElClient.h>
#include <ElClientRest.h>

ElClient esp(&Serial, -1);
ElClientRest rest(&esp);

boolean wifiConnected = false;

void wifiCb(void* response)
{
  uint32_t status;
  RESPONSE res(response);

  if(res.getArgc() == 1) {
    res.popArgs((uint8_t*)&status, 4);
    if(status == STATION_GOT_IP) {
      debugPort.println("WIFI CONNECTED");
     
      wifiConnected = true;
    } else {
      wifiConnected = false;
    }
    
  }
}

void setup() {
  Serial.begin(115200);
  esp.init();
  while(!esp.ready());

  Serial.println("EL-Client ready");
  if(!rest.begin("api.ipify.org")) {
    Serial.println("failed to setup rest client");
    while(1);
  }
}

void loop() {
  esp.process();
  if(wifiConnected) {
    char response[266];
    rest.get("/");
    if(rest.getResponse(response, 266) == HTTP_STATUS_OK){
      debugPort.println("ARDUINO: GET successful");
      debugPort.println(response);
    }
    delay(1000);
  }
}
