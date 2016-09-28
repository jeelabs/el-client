/**
 * Simple example to demo the El-Client UDP calls
 */

#include <ELClient.h>
#include <ELClientSocket.h>

// IP address for this demo is a local IP.
// Replace it with the IP address where you have a UDP socket server running
char * const udpServer PROGMEM = "192.168.0.102"; // Send to single ip address
char * const udpServer2 PROGMEM = "192.168.0.255"; // Broadcast to given network ip mask
// Port for this demo is the port used by the UDP socket server.
// Replace it with the port that your UDP socket server is listening to
uint16_t const udpPort PROGMEM = 5000;
uint16_t const udpPort2 PROGMEM = 7000;

//###########################################################
// For ARDUINO UNO WIFI with I2C to serial chip!
//###########################################################
// Serial port to ESP8266
#include <SC16IS750.h>
SC16IS750 i2cuart = SC16IS750(SC16IS750_PROTOCOL_I2C,SC16IS750_ADDRESS_AA);

// Initialize a connection to esp-link using the I2Cuart chip of the Arduino Uno WiFi board for
// SLIP messages.
ELClient esp(&i2cuart);

//###########################################################
// For boards using the hardware serial port!
//###########################################################
// Initialize a connection to esp-link using the normal hardware serial port both for
// SLIP and for debug messages.
//ELClient esp(&Serial, &Serial);

// Initialize a UDP client on the connection to esp-link
ELClientSocket udp(&esp);
// Initialize a UDP client on the connection to esp-link
ELClientSocket udp2(&esp);

// Timer value to send out data
uint32_t wait;
// Time to wait between sending out data
uint32_t waitTime;
// Flag for wifi connection
boolean wifiConnected = false;

// Parse error codes and returns error message as char *
// Definitions from error values from espconn.h (Espressif SDK)
// #define ESPCONN_OK           0  /**< No error, everything OK.   */
// #define ESPCONN_MEM         -1  /**< Out of memory.             */
// #define ESPCONN_TIMEOUT     -3  /**< Timeout.                   */
// #define ESPCONN_RTE         -4  /**< Routing problem.           */
// #define ESPCONN_INPROGRESS  -5  /**< Operation in progress.     */
// #define ESPCONN_MAXNUM      -7  /**< Total number exceeds the maximum limitation. */

// #define ESPCONN_ABRT        -8  /**< Connection aborted.    */
// #define ESPCONN_RST         -9  /**< Connection reset.      */
// #define ESPCONN_CLSD       -10  /**< Connection closed.     */
// #define ESPCONN_CONN       -11  /**< Not connected.         */

// #define ESPCONN_ARG        -12  /**< Illegal argument.      */
// #define ESPCONN_IF         -14  /**< UDP send error.        */
// #define ESPCONN_ISCONN     -15  /**< Already connected.     */

char* const errTxt[] PROGMEM = {"No error, everything OK.","Out of memory.","Unknown code.","Timeout.","Routing problem.","Operation in progress.",
					"Unknown code.","Total number exceeds the maximum limitation.","Connection aborted.","Connection reset.","Connection closed.",
					"Not connected.","Illegal argument.","Unknown code.","UDP send error.","Already connected."};
char * getErrTxt(int16_t commError) {
	commError = commError*-1;
	if (commError <= 15) {
		return (char *) pgm_read_word (&errTxt[commError]);
	} else {
		return (char *) pgm_read_word (&errTxt[2]); // Unknown code
	}
}

// Callback for UDP socket, called if data was sent or received
// Receives socket client number, can be reused for all initialized UDP socket connections
// !!! UDP doesn't check if the data was received or if the receiver IP/socket is available !!! You need to implement your own
// error control!
void udpCb(uint8_t resp_type, uint8_t client_num, uint16_t len, char *data) {
	Serial.println("udpCb is called");
	if (len > 0) { // sending complete (no confirmation that it was received!) or we received something
		if (resp_type == USERCB_SENT) {
			Serial.println("\tSent " + String(len) + " bytes over connection #" + String(client_num));
		} else if (resp_type == USERCB_RECV) {
			char recvData[len+1]; // Prepare buffer for the received data
			memcpy(recvData, data, len); // Copy received data into the buffer
			recvData[len] = '\0'; // Terminate the buffer with 0 for proper printout!

			Serial.println("\tReceived " + String(len) + " bytes over client#" + String(client_num));
			Serial.println("\tReceived: " + String(recvData));
		} else {
			Serial.println("Received invalid response type");
		}
	} else if (len < 0) { // negative result means there was a problem
		Serial.print(F("Send error: "));
		Serial.println(getErrTxt(len));
	}
}

// Callback made from esp-link to notify of wifi status changes
// Here we print something out and set a global flag
void wifiCb(void *response) {
	ELClientResponse *res = (ELClientResponse*)response;
	if (res->argc() == 1) {
		uint8_t status;
		res->popArg(&status, 1);

		if(status == STATION_GOT_IP) {
			Serial.println(F("WIFI CONNECTED"));
			wifiConnected = true;
		} else {
			Serial.print(F("WIFI NOT READY: "));
			Serial.println(status);
			wifiConnected = false;
		}
	}
}

void setup() {
	Serial.begin(115200);
	i2cuart.begin(9600);
	Serial.println(F("EL-Client starting!"));

	// Sync-up with esp-link, this is required at the start of any sketch and initializes the
	// callbacks to the wifi status change callback. The callback gets called with the initial
	// status right after Sync() below completes.
	esp.wifiCb.attach(wifiCb); // wifi status change callback, optional (delete if not desired)
	bool ok;
	do {
		ok = esp.Sync();			// sync up with esp-link, blocks for up to 2 seconds
		if (!ok) Serial.println(F("EL-Client sync failed!"));
	} while(!ok);
	Serial.println(F("EL-Client synced!"));

	// Get immediate wifi status info for demo purposes. This is not normally used because the
	// wifi status callback registered above gets called immediately.
	esp.GetWifiStatus();
	ELClientPacket *packet;
	if ((packet=esp.WaitReturn()) != NULL) {
		Serial.print(F("Wifi status: "));
		Serial.println(packet->value);
	}

	// Set up the UDP socket client to send a short message to <udpServer> on port <>, this doesn't connect to that server,
	// it just sets-up stuff on the esp-link side
	int err = udp.begin(udpServer, udpPort, SOCKET_UDP, udpCb);
	if (err < 0) {
		Serial.print(F("UDP begin failed: "));
		Serial.println(err);
		delay(10000);
		asm volatile ("  jmp 0");
	}

	err = udp2.begin(udpServer2, udpPort2, SOCKET_UDP, udpCb);
	if (err < 0) {
		Serial.print(F("UDP2 begin failed: "));
		Serial.println(err);
		delay(10000);
		asm volatile ("  jmp 0");
	}

	Serial.println(F("EL-Client ready!"));
	wait = millis()+29000; // Start first sending in 1 second
}

void loop() {
	// process any callbacks coming from esp_link
	esp.Process();

	// if we're connected send data over UDP socket
	if(wifiConnected) {
		if (millis() - wait > 30000) { // Send some data every 30 seconds
			wait = millis();
			// Send message to the previously set-up server #1
			Serial.print(F("Sending message to "));
			Serial.println(udpServer);
			udp.send("Message from your Arduino Uno WiFi over UDP socket");

			// Send message to the previously set-up server #2
			Serial.print(F("Sending broadcast to "));
			Serial.println(udpServer2);
			udp2.send("Broadcast from your Arduino Uno WiFi over UDP socket");
		}
	} else {
		// This is just for demo, you can as well just try to reconnect
		// and setup the connection to esp-link again
		Serial.println(F("Lost connection, try to reboot"));
		delay(10000);
		asm volatile ("  jmp 0");
	}
}
