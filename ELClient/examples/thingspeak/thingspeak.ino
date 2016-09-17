/**
 * \file
 *			 ESP8266 RESTful Thingspeak example
 * \author
 *			 BeeGee
 */

#include <ELClient.h>
#include <ELClientRest.h>

//###########################################################
// For ARDUINO UNO WIFI with I2C to serial chip!
//###########################################################
// Serial port to ESP8266
// #include <SC16IS750.h>
// SC16IS750 i2cuart = SC16IS750(SC16IS750_PROTOCOL_I2C,SC16IS750_ADDRESS_AA);

// Initialize a connection to esp-link using the I2Cuart chip of the Arduino Uno WiFi board for
// SLIP messages.
// ELClient esp(&i2cuart);

//###########################################################
// For boards using the hardware serial port!
//###########################################################
// Initialize a connection to esp-link using the normal hardware serial port both for
// SLIP and for debug messages.
ELClient esp(&Serial, &Serial);

// Initialize a REST client on the connection to esp-link
ELClientRest rest(&esp);

boolean wifiConnected = false;

// Callback made from esp-link to notify of wifi status changes
// Here we print something out and set a global flag
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
	}
}

void setup() {
	Serial.begin(9600);
//###########################################################
// For ARDUINO UNO WIFI with I2C to serial chip!
//###########################################################
	// i2cuart.begin(9600);
	
	Serial.println("");
	Serial.println("EL-Client starting!");

	// Sync-up with esp-link, this is required at the start of any sketch and initializes the
	// callbacks to the wifi status change callback. The callback gets called with the initial
	// status right after Sync() below completes.
	esp.wifiCb.attach(wifiCb); // wifi status change callback, optional (delete if not desired)
	bool ok;
	do {
		ok = esp.Sync();			// sync up with esp-link, blocks for up to 2 seconds
		if (!ok) {
			Serial.print("\nEL-Client sync failed! err: ");
			Serial.println(ok);
		}
	} while(!ok);
	Serial.println("EL-Client synced!");

	// Wait for WiFi to be connected. 
Serial.println("esp.GetWifiStatus()");
	esp.GetWifiStatus();
	ELClientPacket *packet;
	Serial.println("Waiting for WiFi ");
	if ((packet=esp.WaitReturn()) != NULL) {
		Serial.print(".");
		Serial.println(packet->value);
	}
	Serial.println("");

	// Set up the REST client to talk to api.thingspeak.com, this doesn't connect to that server,
	// it just sets-up stuff on the esp-link side
	// int err = rest.begin("api.thingspeak.com");
	int err = rest.begin("184.106.153.149");
	if (err != 0) {
		Serial.print("REST begin failed: ");
		Serial.println(err);
		while(1) ;
	}
	Serial.println("EL-REST ready");
}

float solarValue = 99.5;
// Change to your own Thingspeak API key
char *api_key = "K9LDRXS7BXSN8X1J";
// expand buffer size to your needs
#define BUFLEN 266

void loop() {
	// process any callbacks coming from esp_link
	esp.Process();

	// if we're connected make an REST request
	if(wifiConnected) {
		
		// Generate a fake value starting from 100 going up to 300
		solarValue = solarValue + 0.5;
		if (solarValue == 300) {
			solarValue = 100;
		}
		String solarValString = String(solarValue);
		const char *solarValChar = solarValString.c_str();
		
		// Reserve a buffer for sending the data
		char path_data[BUFLEN];
		// Copy the path and API key into the buffer
		sprintf(path_data, "%s", "/update?api_key=");
		sprintf(path_data + strlen(path_data), "%s", api_key);
		
		// Copy the field number and value into the buffer
		// If you have more than one field to update,
		// repeat and change field1 to field2, field3, ...
		sprintf(path_data + strlen(path_data), "%s", "&field1=");
		sprintf(path_data + strlen(path_data), "%s", solarValChar);
		
		// Send PUT request to thingspeak.com
		rest.post(path_data,"");	

		// Reserve a buffer for the response from Thingspeak
		char response[BUFLEN];
		// Clear the buffer
		memset(response, 0, BUFLEN);
		// Wait for response from Thingspeak
		uint16_t code = rest.waitResponse(response, BUFLEN-1);
		// Check the response from Thingspeak
		if(code == HTTP_STATUS_OK){
			Serial.println("Thingspeak: POST successful:");
			Serial.print("Response: ");
			Serial.println(response);
		} else {
			Serial.print("Thingspeak: POST failed with error ");
			Serial.println(code);
			Serial.print("Response: ");
			Serial.println(response);
		}
		// Send next data in 20 seconds
		delay(20000);
	}
}