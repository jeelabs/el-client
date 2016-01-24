/**
 * Simple example to demo the El-Client REST calls
 *
 */

#include <ELClient.h>
#include <ELClientRest.h>

// Initialize a connection to esp-link using the normal hardware serial port both for
// SLIP and for debug messages.
ELClient esp(&Serial, &Serial);
// Initialize a REST client on the connection to esp-link
ELClientRest rest(&esp);

boolean wifiConnected = false;

// Callback made from esp-link to respond to ...
void wifiCb(void* response)
{
  uint32_t status;
  ELClientResponse res(response); // fetch the response

  if(res.getArgc() == 1) {
    // we expect one argument, which is that wifi status code
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
  Serial.begin(115200);   // the baud rate here needs to match the esp-link config
  Serial.println("EL-Client hello world!");
  uint16_t crc = esp.crc16Data("\x01\x00\x00\x00\x00\x00\x1C\x01\x00\x00\x01\x00\x04\x00\x00\x00i\x00",18,0);
  Serial.print("CRC: ");
  Serial.print(crc,16);
  Serial.println();

  esp.wifiCb.attach(wifiCb);
  bool ok = esp.Sync();   // sync up with esp-link, also removes all previous callbacks
  Serial.println("EL-Client synced!");
  ok = ok && rest.begin("http://www.timeapi.org/"); // free API to get the time
  if (!ok) {
    Serial.println("setup failed");
    while(1) ;
  }
  Serial.println("EL-Client ready");
}

void loop() {
  // process any responses or input coming from esp_link
  esp.Process();

  // if we're connected make an HTTP request
  if(wifiConnected) {
    rest.get("/");

    char response[266];
    if(rest.getResponse(response, 266) == HTTP_STATUS_OK){
      Serial.println("ARDUINO: GET successful");
      Serial.println(response);
    }
    delay(1000);
  }
}
