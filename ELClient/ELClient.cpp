/*! \file ELClient.cpp
    \brief Constructor and functions for ELClient
    \author B. Runnels
    \author T. von Eicken
    \date 2016
*/
// Copyright (c) 2016 by B. Runnels and T. von Eicken

#include "ELClient.h"

#define SLIP_END  0300        /**< Indicates end of packet */
#define SLIP_ESC  0333        /**< Indicates byte stuffing */
#define SLIP_ESC_END  0334    /**< ESC ESC_END means END data byte */
#define SLIP_ESC_ESC  0335    /**< ESC ESC_ESC means ESC data byte */

//===== Input

/*! protoCompletedCb(void *res)
@brief Process a received SLIP message
@details Callback to process a SLIP message and check CRCs
	If a user callback function was defined in the message, ELClientResponse is called to handle the callback.
@note
	This function is usually not needed for applications. The communication to the ESP8266 is handled by the cmd, rest, mqtt, tcp and udp library parts.
@return <code>ELClientPacket</code>
	Pointer to ELClientPacket structure generated or NULL if it the message is from a callback or if an error occured
@par Example
@code
	no example code yet
@endcode
*/
ELClientPacket* ELClient::protoCompletedCb(void) {
  // the packet starts with a ELClientPacket
  ELClientPacket* packet = (ELClientPacket*)_proto.buf;
  if (_debugEn) {
    _debug->print("ELC: got ");
    _debug->print(_proto.dataLen);
    _debug->print(" @");
    _debug->print((uint32_t)_proto.buf, 16);
    _debug->print(": ");
    _debug->print(packet->cmd, 16);
    _debug->print(" ");
    _debug->print(packet->value, 16);
    _debug->print(" ");
    _debug->print(packet->argc, 16);
    for (uint16_t i=8; i<_proto.dataLen; i++) {
      _debug->print(" ");
      _debug->print(*(uint8_t*)(_proto.buf+i), 16);
    }
    _debug->println();
  }

  // verify CRC
  uint16_t crc = crc16Data(_proto.buf, _proto.dataLen-2, 0);
  uint16_t resp_crc = *(uint16_t*)(_proto.buf+_proto.dataLen-2);
  if (crc != resp_crc) {
    DBG("ELC: Invalid CRC");
    return NULL;
  }

  // dispatch based on command
  if (packet->cmd == CMD_RESP_V) {
    // value response
    if (_debugEn) {
        _debug->print("RESP_V: ");
        _debug->println(packet->value);
    }
    return packet;
  } else if (packet->cmd == CMD_RESP_CB) {
    FP<void, void*> *fp;
    // callback reponse
    if (_debugEn) {
        _debug->print("RESP_CB: ");
        _debug->print(packet->value);
        _debug->print(" ");
        _debug->println(packet->argc);
    }
    fp = (FP<void, void*>*)packet->value;
    if (fp->attached()) {
      ELClientResponse resp(packet);
      (*fp)(&resp);
    }
    return NULL;
  } else {
    // command (NOT IMPLEMENTED)
    if (_debugEn) _debug->println("CMD??");
    return NULL;
  }
}

/*! Process()
@brief Handle serial input.
@details Read all characters available on the serial input and process any messages that arrive,
	but stop if a non-callback response comes in
@return <code>ELClientPacket</code>
	Pointer to ELClientResponse structure with the received response
@par Example
@code
	void loop()
	{
		ELClientPacket *packet;
		// process any callbacks coming from esp_link
		packet = esp.Process();
		if (packet != 0)
		{
			// process the received package //
		}
	}
@endcode
*/
ELClientPacket *ELClient::Process() {
  int value;
  while (_serial->available()) {
    value = _serial->read();
    if (value == SLIP_ESC) {
      _proto.isEsc = 1;
    } else if (value == SLIP_END) {
      ELClientPacket *packet = _proto.dataLen >= 8 ? protoCompletedCb() : 0;
      _proto.dataLen = 0;
      _proto.isEsc = 0;
      if (packet != NULL) return packet;
    } else {
      if (_proto.isEsc) {
        if (value == SLIP_ESC_END) value = SLIP_END;
        if (value == SLIP_ESC_ESC) value = SLIP_ESC;
        _proto.isEsc = 0;
      }
      if (_proto.dataLen < _proto.bufSize) {
        _proto.buf[_proto.dataLen++] = value;
     }
    }
  }
  return NULL;
}

//===== Output

/*! write(uint8_t data)
@brief Send a byte
@details Write a byte to the output stream and perform SLIP escaping
@note
	This function is usually not needed for applications. The communication to the ESP8266 is handled by the cmd, rest, mqtt, tcp and udp library parts.
@param data
	Byte to be sent
@par Example
@code
	no example code yet
@endcode
*/
void ELClient::write(uint8_t data) {
  switch (data) {
  case SLIP_END:
    _serial->write(SLIP_ESC);
    _serial->write(SLIP_ESC_END);
    break;
  case SLIP_ESC:
    _serial->write(SLIP_ESC);
    _serial->write(SLIP_ESC_ESC);
    break;
  default:
    _serial->write(data);
  }
}

/*! write(void* data, uint16_t len)
@brief Send several byte
@details Write some bytes to the output stream, no SLIP escaping is performed
@note
	This function is usually not needed for applications. The communication to the ESP8266 is handled by the cmd, rest, mqtt, tcp and udp library parts.
@param data
	Pointer to data buffer to be sent
@param len
	Size of data buffer
@par Example
@code
	no example code yet
@endcode
*/
void ELClient::write(void* data, uint16_t len) {
  uint8_t *d = (uint8_t*)data;
  while (len--)
    write(*d++);
}

/*! Request(uint16_t cmd, uint32_t value, uint16_t argc)
@brief Start a request
@details Start preparing a request by sending the command, number of arguments
	and the first argument (which can be a callback pointer)
@note
	This function is usually not needed for applications. The communication to the ESP8266 is handled by the cmd, rest, mqtt, tcp and udp library parts.
@param cmd
	Command for the ESP, see enum CmdName for available commands
@param value
	First argument or pointer to a callback function
@param argc
	Number of arguments in this request
@par Example
@code
	_elc->Request(CMD_MQTT_LWT, 0, 4);
	_elc->Request(topic, strlen(topic));
	_elc->Request(message, strlen(message));
	_elc->Request(&qos, 1);
	_elc->Request(&retain, 1);
	_elc->Request();
@endcode
*/
void ELClient::Request(uint16_t cmd, uint32_t value, uint16_t argc) {
  crc = 0;
  _serial->write(SLIP_END);
  write(&cmd, 2);
  crc = crc16Data((unsigned const char*)&cmd, 2, crc);

  write(&argc, 2);
  crc = crc16Data((unsigned const char*)&argc, 2, crc);

  write(&value, 4);
  crc = crc16Data((unsigned const char*)&value, 4, crc);
}

/*! Request(uint16_t cmd, uint32_t value, uint16_t argc)
@brief Append an argument to the request
@details Send additional arguments as appendment to the ESP
@note
	This function is usually not needed for applications. The communication to the ESP8266 is handled by the cmd, rest, mqtt, tcp and udp library parts.
@param data
	Pointer to the buffer with the argument
@param len
	Size of the argument buffer
@par Example
@code
	_elc->Request(CMD_MQTT_LWT, 0, 4);
	_elc->Request(topic, strlen(topic));
	_elc->Request(message, strlen(message));
	_elc->Request(&qos, 1);
	_elc->Request(&retain, 1);
	_elc->Request();
@endcode
*/
void ELClient::Request(const void* data, uint16_t len) {
  uint8_t *d = (uint8_t*)data;

  // write the length
  write(&len, 2);
  crc = crc16Data((unsigned const char*)&len, 2, crc);

  // output the data
  for (uint16_t l=len; l>0; l--) {
    write(*d);
    crc = crc16Add(*d, crc);
    d++;
  }

  // output padding
  uint16_t pad = (4-(len&3))&3;
  uint8_t temp = 0;
  while (pad--) {
    write(temp);
    crc = crc16Add(temp, crc);
  }
}

/*! Request(const __FlashStringHelper* data, uint16_t len)
@brief Append an argument to the request
@details Send additional arguments located in flash as appendment to the ESP
@note
	This function is usually not needed for applications. The communication to the ESP8266 is handled by the cmd, rest, mqtt, tcp and udp library parts.
@param data
	Pointer to the buffer with the argument
@param len
	Size of the argument buffer
@par Example
@code
	_elc->Request(CMD_MQTT_LWT, 0, 4);
	_elc->Request(topic, strlen(topic));
	_elc->Request(message, strlen(message));
	_elc->Request(&qos, 1);
	_elc->Request(&retain, 1);
	_elc->Request();
@endcode
*/
void ELClient::Request(const __FlashStringHelper* data, uint16_t len) {
  // write the length
  write(&len, 2);
  crc = crc16Data((unsigned const char*)&len, 2, crc);

  // output the data
  PGM_P p = reinterpret_cast<PGM_P>(data);
  for (uint16_t l=len; l>0; l--) {
    uint8_t c = pgm_read_byte(p++);
    write(c);
    crc = crc16Add(c, crc);
  }

  // output padding
  uint16_t pad = (4-(len&3))&3;
  uint8_t temp = 0;
  while (pad--) {
    write(temp);
    crc = crc16Add(temp, crc);
  }
}

/*! Request(void)
@brief Finish the request
@details Send final CRC and SLIP_END to the ESP to finish the request
@note
	This function is usually not needed for applications. The communication to the ESP8266 is handled by the cmd, rest, mqtt, tcp and udp library parts.
@par Example
@code
	_elc->Request(CMD_MQTT_LWT, 0, 4);
	_elc->Request(topic, strlen(topic));
	_elc->Request(message, strlen(message));
	_elc->Request(&qos, 1);
	_elc->Request(&retain, 1);
	_elc->Request();
@endcode
*/
void ELClient::Request(void) {
  write((uint8_t*)&crc, 2);
  _serial->write(SLIP_END);
}

//===== Initialization

/*! init()
@brief Initialize ELClient protocol
@details Prepare buffer for protocol
@note
	This function is usually not needed for applications. The communication to the ESP8266 is handled by the cmd, rest, mqtt, tcp and udp library parts.
@par Example
@code
	no example code yet
@endcode
*/
void ELClient::init() {
  _proto.buf = _protoBuf;
  _proto.bufSize = sizeof(_protoBuf);
  _proto.dataLen = 0;
  _proto.isEsc = 0;
}

/*! ELClient(Stream* serial)
@brief Initialize ELClient
@details Store serial stream to be used for the communication
@param serial
	Serial stream for communication with ESP
@par Example for hardware serial ports
@code
	//###########################################################
	// For boards using the hardware serial port! 
	//###########################################################
	// Initialize a connection to esp-link using the normal hardware serial port for SLIP messages.
	ELClient esp(&Serial);
@endcode
@par Example for ARDUINO UNO WIFI board with I2C to serial chip connected to the ESP8266
@code
	//###########################################################
	// For ARDUINO UNO WIFI board with I2C to serial chip connected to the ESP8266!
	//###########################################################
	// Serial port to ESP8266
	#include <SC16IS750.h>
	SC16IS750 i2cuart = SC16IS750(SC16IS750_PROTOCOL_I2C,SC16IS750_ADDRESS_AA);
	// Initialize a connection to esp-link using the I2Cuart chip of the Arduino Uno WiFi board for SLIP messages.
	ELClient esp(&i2cuart);
@endcode
*/
ELClient::ELClient(Stream* serial) :
_serial(serial) {
  _debugEn = false;
  init();
}

/*! ELClient(Stream* serial, Stream* debug)
@brief Initialize ELClient and enable debug output
@details Store serial streams to be used for the communication
@param serial
	Serial stream for communication with ESP
@param debug
	Serial stream for debug output
@par Example for hardware serial ports
@code
	//###########################################################
	// For boards using the hardware serial port! 
	//###########################################################
	// Initialize a connection to esp-link using the normal hardware serial port both for SLIP and for debug messages.
	ELClient esp(&Serial, &Serial);
@endcode
@par Example for ARDUINO UNO WIFI board with I2C to serial chip connected to the ESP8266
@code
	//###########################################################
	// For ARDUINO UNO WIFI board with I2C to serial chip connected to the ESP8266!
	//###########################################################
	// Serial port to ESP8266
	#include <SC16IS750.h>
	SC16IS750 i2cuart = SC16IS750(SC16IS750_PROTOCOL_I2C,SC16IS750_ADDRESS_AA);
	// Initialize a connection to esp-link using the I2Cuart chip of the Arduino Uno WiFi board for SLIP messages.
	ELClient esp(&i2cuart, &Serial);
@endcode
*/
ELClient::ELClient(Stream* serial, Stream* debug) :
_debug(debug), _serial(serial) {
  _debugEn = true;
  init();
}

/*! ELClient::DBG(const char* info)
@brief Send debug message over serial debug stream
@param info
	Debug message
@par Example
@code
	no example code yet
@endcode
*/
void ELClient::DBG(const char* info) {
  if (_debugEn) _debug->println(info);
}

//===== Responses

/*! WaitReturn(uint32_t timeout)
@brief Wait for a response from ESP for a given timeout
@param timeout
	Time in milliseconds to wait for a response, defaults to ESP_TIMEOUT
@return <code>ELClientPacket</code>
	Received packet or null if timeout occured
@par Example
@code
	// Wait for WiFi to be connected. 
	esp.GetWifiStatus();
	ELClientPacket *packet;
	Serial.print("Waiting for WiFi ");
	if ((packet=esp.WaitReturn()) != NULL) {
		Serial.print(".");
		Serial.println(packet->value);
	}
	Serial.println("");
@endcode
*/
ELClientPacket *ELClient::WaitReturn(uint32_t timeout) {
  uint32_t wait = millis();
  while (millis() - wait < timeout) {
    ELClientPacket *packet = Process();
    if (packet != NULL) return packet;
  }
  return NULL;
}

//===== CRC helper functions

/*! crc16Add(unsigned char b, uint16_t acc)
@brief Create CRC for a byte add it to an existing CRC checksum and return the result
@param b
	Byte which CRC will be added
@param acc
	Existing CRC checksum
@return <code>uint16_t</code>
	New CRC checksum
@par Example
@code
	no example code yet
@endcode
*/
uint16_t ELClient::crc16Add(unsigned char b, uint16_t acc)
{
  acc ^= b;
  acc = (acc >> 8) | (acc << 8);
  acc ^= (acc & 0xff00) << 4;
  acc ^= (acc >> 8) >> 4;
  acc ^= (acc & 0xff00) >> 5;
  return acc;
}

/*! crc16Data(const unsigned char *data, uint16_t len, uint16_t acc)
@brief Create/add CRC for a data buffer 
@param data
	The data buffer which will be CRCed
@param len
	Size of the data buffer
@param acc
	Existing CRC checksum
@return <code>uint16_t</code>
	New CRC checksum
@par Example
@code
	no example code yet
@endcode
*/
uint16_t ELClient::crc16Data(const unsigned char *data, uint16_t len, uint16_t acc)
{
  for (uint16_t i=0; i<len; i++)
    acc = crc16Add(*data++, acc);
  return acc;
}

//===== Basic requests built into ElClient

/*! Sync(uint32_t timeout)
@brief Synchronize the communication between the MCU and the ESP
@param timeout
	Timeout for synchronization request
@return <code>boolean</code>
	True if synchronization succeeds or False if it fails
@par Example
@code
	// Sync-up with esp-link, this is required at the start of any sketch and initializes the callbacks to the wifi status change callback. The callback gets called with the initial status right after Sync() below completes. 
	esp.wifiCb.attach(wifiCb); // wifi status change callback, optional (delete if not desired)
	bool ok;
	do
	{
		ok = esp.Sync(); // sync up with esp-link, blocks for up to 2 seconds
		if (!ok) Serial.println("EL-Client sync failed!");
	} while(!ok);
	Serial.println("EL-Client synced!");
@endcode
*/
boolean ELClient::Sync(uint32_t timeout) {
  // send sync request
  Request(CMD_SYNC, (uint32_t)&wifiCb, 0);
  Request();
  // empty the response queue hoping to find the wifiCb address
  ELClientPacket *packet;
  while ((packet = WaitReturn(timeout)) != NULL) {
    if (packet->value == (uint32_t)&wifiCb) { 
        if (_debugEn) _debug->println("SYNC!");
        return true;
    }
    if (_debugEn) {
        _debug->print("BAD: ");
        _debug->println(packet->value);
    }
  }
  // doesn't look like we got a real response
  return false;
}

/*! GetWifiStatus(void)
@brief Request WiFi status from the ESP
@par Example
@code
	// Wait for WiFi to be connected. 
	esp.GetWifiStatus();
	ELClientPacket *packet;
	Serial.print("Waiting for WiFi ");
	if ((packet=esp.WaitReturn()) != NULL) {
		Serial.print(".");
		Serial.println(packet->value);
	}
	Serial.println("");
@endcode
*/
void ELClient::GetWifiStatus(void) {
  Request(CMD_WIFI_STATUS, 0, 0);
  Request();
}
