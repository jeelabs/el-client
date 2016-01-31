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
void wifiCb(void *response) {
  ELClientResponse *res = (ELClientResponse*)response;
  if (res->argc() == 1) {
    uint8_t status;
    res->popArg(&status, 1);

    if(status == STATION_GOT_IP) {
      Serial.println("WIFI CONNECTED");
      wifiConnected = true;
    } else {
      Serial.print("WIFI NOT READY: ");
      Serial.println(status);
      wifiConnected = false;
    }
  } else {
    Serial.print("WIFI CB argc=");
    Serial.println(res->argc());
  }
}

void setup() {
  Serial.begin(115200);   // the baud rate here needs to match the esp-link config
  Serial.println("EL-Client starting!");

  // Sync-up with esp-link, this is required at the start of any sketch and initializes the
  // callbacks with just a simple wifi status change callback.
  esp.wifiCb.attach(wifiCb); // wifi status change callback, optional (delete if not desired)
  bool ok;
  do {
    ok = esp.Sync();      // sync up with esp-link, blocks for up to 2 seconds
    if (!ok) Serial.println("EL-Client sync failed!");
  } while(!ok);
  Serial.println("EL-Client synced!");

  // Get immediate wifi status info for demo purposes. This is not normally used because the
  // wifi status callback registered above gets called immediately. 
  esp.GetWifiStatus();
  ELClientPacket *packet;
  while ((packet=esp.WaitReturn()) != NULL) {
    if (packet->cmd == CMD_WIFI_STATUS) {
      Serial.print("Wifi status: ");
      Serial.print(packet->value);
    }
  }

  int err = rest.begin("www.timeapi.org");
  if (err != 0) {
    Serial.print("REST begin failed: ");
    Serial.println(err);
    while(1) ;
  }
  Serial.println("EL-REST ready");
}

void loop() {
  // process any responses or input coming from esp_link
  esp.Process();

  // if we're connected make an HTTP request
  if(wifiConnected) {
    rest.get("/utc/now");

    char response[266];
    uint16_t code = rest.waitResponse(response, 266);
    if(code == HTTP_STATUS_OK){
      Serial.println("ARDUINO: GET successful:");
      Serial.println(response);
    } else {
      Serial.print("ARDUINO: GET failed: ");
      Serial.println(code);
    }
    delay(1000);
  }
}
