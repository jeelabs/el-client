/**
 * Simple example for LED flashing using web-server
 * 
 * Test board:
 *    ESP8266 RX    <--level shifter-->    Arduino TX
 *    ESP8266 TX    <--level shifter-->    Arduino RX
 *    ESP8266 GPIO0 <--level shifter-->    Arduino RESET   (optional)
 * 
 *    Arduino LED is on PIN 13
 * 
 * Video:
 *    https://www.youtube.com/watch?v=vBESCO0UhYI
 */

#include <ELClient.h>
#include <ELClientWebServer.h>

// flash LED on PIN 13
#define LED_PIN 13

// Initialize a connection to esp-link using the normal hardware serial port
//
// DEBUG is disasbled as
// - packet logging is slow and UART receive buffer can overrun (HTML form submission)
ELClient esp(&Serial);

// Initialize the Web-Server client
ELClientWebServer webServer(&esp);

void ledPageLoadAndRefreshCb(char * url)
{
  if( digitalRead(LED_PIN) )
    webServer.setArgString(F("text"), F("LED is on"));
  else
    webServer.setArgString(F("text"), F("LED is off"));
}

void ledButtonPressCb(char * btnId)
{
  String id = btnId;
  if( id == F("btn_on") )
    digitalWrite(LED_PIN, true);
  else if( id == F("btn_off") )
    digitalWrite(LED_PIN, false);
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
  
  webServer.setup();
}

void setup()
{
  Serial.begin(115200);
  
  URLHandler *ledHandler = webServer.createURLHandler(F("/SimpleLED.html.json"));
  ledHandler->loadCb.attach(&ledPageLoadAndRefreshCb);
  ledHandler->refreshCb.attach(&ledPageLoadAndRefreshCb);
  ledHandler->buttonCb.attach(&ledButtonPressCb);

  esp.resetCb = resetCb;
  resetCb();
}

void loop()
{
  esp.Process();
}
