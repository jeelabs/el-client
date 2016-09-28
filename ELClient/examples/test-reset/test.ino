/**
 * Simple example to test resetting either arduino or esp-link while doing MQTT
 */

#include <ELClient.h>
#include <ELClientCmd.h>

// Initialize a connection to esp-link using the normal hardware serial port both for
// SLIP and for debug messages. Esp-link will show the debug messages in the uC console
// because they are not SLIP framed.
ELClient esp(&Serial, &Serial);

// Initialize CMD client (for GetTime)
ELClientCmd cmd(&esp);

// Callback made from esp-link to notify of wifi status changes
// Here we just print something out for grins
void wifiCb(void* response) {
  ELClientResponse *res = (ELClientResponse*)response;
  if (res->argc() == 1) {
    uint8_t status;
    res->popArg(&status, 1);

    if(status == STATION_GOT_IP) {
      Serial.println("WIFI CONNECTED");
    } else {
      Serial.print("WIFI NOT READY: ");
      Serial.println(status);
    }
  }
}

// Callback made form esp-link to notify that it has just come out of a reset. This means we
// need to initialize it!
void resetCb(void) {
  Serial.println("EL-Client (re-)starting!");
  bool ok = false;
  do {
    ok = esp.Sync();      // sync up with esp-link, blocks for up to 2 seconds
    if (!ok) Serial.println("EL-Client sync failed!");
  } while(!ok);
  Serial.println("EL-Client synced!");
}

void setup() {
  Serial.begin(115200);

  // Sync-up with esp-link, this is required at the start of any sketch and initializes the
  // callbacks to the wifi status change callback. The callback gets called with the initial
  // status right after Sync() below completes.
  esp.wifiCb.attach(wifiCb); // wifi status change callback, optional (delete if not desired)
  esp.resetCb = resetCb;
  resetCb();
}

static uint32_t last;

void loop() {
  esp.Process();

  if ((millis()-last) > 4000) {
    Serial.println("requesting time");
    uint32_t t = cmd.GetTime();
    Serial.print("Time: "); Serial.println(t);

    last = millis();
  }
}
