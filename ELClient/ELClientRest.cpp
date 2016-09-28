/*! \file ELClientRest.cpp
    \brief Constructor and functions for ELClientRest
    \author B. Runnels
    \author T. von Eicken
    \date 2016
*/
// Copyright (c) 2016 by B. Runnels and T. von Eicken

#include "ELClientRest.h"

typedef enum {
  HEADER_GENERIC = 0,    /**< Header is generic */
  HEADER_CONTENT_TYPE,   /**< Header is content type */
  HEADER_USER_AGENT      /**< Header is user agent */
} HEADER_TYPE; /**< Enum of header types */

/*! ELClientRest(ELClient *e)
@brief Constructor for ELClientRest
@param e
	Pointer to ELClient structure
@par Example
@code
	ELClientRest(ELClient *e);
@endcode
*/
ELClientRest::ELClientRest(ELClient *e)
{
  _elc = e;
  remote_instance = -1;
}

/*! restCallback(void *res)
@brief Function called by esp-link when data is sent, received or an error occured.
@details The function is called by esp-link when data is sent or received from the remote server.
@note Internal library function
@param res
	Pointer to ELClientResponse structure
@warning The content of the response structure is overwritten when the next package arrives!
*/
void ELClientRest::restCallback(void *res)
{
  if (!res) return;

  ELClientResponse *resp = (ELClientResponse *)res;

  resp->popArg(&_status, sizeof(_status));
  if (_elc->_debugEn) {
    _elc->_debug->print("REST code ");
    _elc->_debug->println(_status);
  }

  _len = resp->popArgPtr(&_data);
}

/*! begin(const char* host, uint16_t port, boolean security)
@brief Initialize communication to a REST server
@details Initialize communication to a remote server,
	this communicates with esp-link but does not open a connection to the remote server.
@param host
	Host to be connected. Can be a URL or an IP address in the format of xxx.xxx.xxx.xxx .
@param port
	Port to be used to send/receive packets. Port MUST NOT be 80, 23 or 2323, as these ports are already used by EL-CLIENT on the ESP8266
@param security
	Flag if secure connection should be established
@warning Port MUST NOT be 80, 23 or 2323, as these ports are already used by EL-CLIENT on the ESP8266.
	Max 4 connections are supported!
@par Example
@code
	int err = rest.begin("www.timeapi.org");
	if (err != 0) 
	{
		Serial.print("REST begin failed: ");
		Serial.println(err);
		while(1) ;
	}
@endcode
*/
int ELClientRest::begin(const char* host, uint16_t port, boolean security)
{
  uint8_t sec = !!security;
  restCb.attach(this, &ELClientRest::restCallback);

  _elc->Request(CMD_REST_SETUP, (uint32_t)&restCb, 3);
  _elc->Request(host, strlen(host));
  _elc->Request(&port, 2);
  _elc->Request(&sec, 1);
  _elc->Request();

  ELClientPacket *pkt = _elc->WaitReturn();
  if (pkt && (int32_t)pkt->value >= 0) {
    remote_instance = pkt->value;
    return 0;
  }
  return (int)pkt->value;
}

/*! request(const char* path, const char* method, const char* data, int len)
@brief Send request to REST server.
@param path
	Path that extends the URL of the REST request (command or data for the REST server)
@param method
	REST method, allowed values are "GET", "POST", "PUT" or "DELETE"
@param data
	Pointer to data buffer
@param len
	Size of data buffer
@par Example
@code
	no example code yet
@endcode
*/
void ELClientRest::request(const char* path, const char* method, const char* data, int len)
{
  _status = 0;
  if (remote_instance < 0) return;
  if (data != 0 && len > 0) _elc->Request(CMD_REST_REQUEST, remote_instance, 3);
  else                      _elc->Request(CMD_REST_REQUEST, remote_instance, 2);
  _elc->Request(method, strlen(method));
  _elc->Request(path, strlen(path));
  if (data != NULL && len > 0) {
    _elc->Request(data, len);
  }

  _elc->Request();
}

/*! request(const char* path, const char* method, const char* data)
@brief Send request to REST server.
@details The data must be null-terminated.
@param path
	Path that extends the URL of the REST request (command or data for the REST server)
@param method
	REST method, allowed values are "GET", "POST", "PUT" or "DELETE"
@param data
	Pointer to data buffer
@par Example
@code
	no example code yet
@endcode
*/
void ELClientRest::request(const char* path, const char* method, const char* data)
{
  request(path, method, data, strlen(data));
}

/*! get(const char* path, const char* data)
@brief Send GET request to REST server
@warning The received data might not be null-terminated.
@param path
	Path that extends the URL of the REST request (command or data for the REST server)
@param data
	Pointer to data buffer 
@par Example
@code
	// Request /utc/now from the previously set-up server
	rest.get("/utc/now");
@endcode
*/
void ELClientRest::get(const char* path, const char* data) { request(path, "GET", data); }

/*! post(const char* path, const char* data)
@brief Send POST request to REST server
@warning The received data must be null-terminated.
@param path
	Path that extends the URL of the REST request (command or data for the REST server)
@param data
	Pointer to data buffer 
@par Example
@code
	// Generate a fake value starting from 100 going up to 300
	solarValue = solarValue + 0.5;
	if (solarValue == 300) 
	{
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
@endcode
*/
void ELClientRest::post(const char* path, const char* data) { request(path, "POST", data); }

/*! put(const char* path, const char* data)
@brief Send PUT request to REST server
@warning The received data must be null-terminated.
@param path
	Path that extends the URL of the REST request (command or data for the REST server)
@param data
	Pointer to data buffer 
@par Example
@code
	no example code yet
@endcode
*/
void ELClientRest::put(const char* path, const char* data) { request(path, "PUT", data); }

/*! del(const char* path)
@brief Send DELETE request to REST server
@param path
	Path that extends the URL of the REST request (command or data for the REST server)
@par Example
@code
	no example code yet
@endcode
*/
void ELClientRest::del(const char* path) { request(path, "DELETE", 0); }

/*! setHeader(const char* value)
@brief Set generic header content
@details If no generic header is set, it defaults to an empty string
@param value
	Header content
@par Example
@code
	no example code yet
@endcode
*/
void ELClientRest::setHeader(const char* value)
{
  uint8_t header_index = HEADER_GENERIC;
  _elc->Request(CMD_REST_SETHEADER, remote_instance, 2);
  _elc->Request(&header_index, 1);
  _elc->Request(value, strlen(value));
  _elc->Request();
}

/*! setContentType(const char* value)
@brief Set content type of header
@details If no content type is set, it defaults to "x-www-form-urlencoded"
@param value
	Content type
@par Example
@code
	no example code yet
@endcode
*/
void ELClientRest::setContentType(const char* value)
{
  uint8_t header_index = HEADER_CONTENT_TYPE;
  _elc->Request(CMD_REST_SETHEADER, remote_instance, 2);
  _elc->Request(&header_index, 1);
  _elc->Request(value, strlen(value));
  _elc->Request();
}

/*! setUserAgent(const char* value)
@brief Set user agent of header
@details If no user agent is set, it defaults to "esp-link"
@param value
	User agent
@par Example
@code
	no example code yet
@endcode
*/
void ELClientRest::setUserAgent(const char* value)
{
  uint8_t header_index = HEADER_USER_AGENT;
  _elc->Request(CMD_REST_SETHEADER, remote_instance, 2);
  _elc->Request(&header_index, 1);
  _elc->Request(value, strlen(value));
  _elc->Request();
}

/*! getResponse(char* data, uint16_t maxLen)
@brief Retrieve response.
@details Checks if a response from the remote server was received,
	returns the HTTP status code or 0 if no response (may need to wait longer)
@param data
	Pointer to buffer for received packet
@param maxLen
	Size of buffer for received packet. If the received packet is larger than the buffer, the received packet will be truncated.
@return <code>uint16_t</code>
	Size of received packet or number of sent bytes or 0 if no response
@par Example
@code
	#define BUFLEN 266
	void loop()
	{
		// process any callbacks coming from esp_link
		esp.Process();
		void loop()
		{
			// process any callbacks coming from esp_link
			esp.Process();
			// if we're connected make an HTTP request
			if(wifiConnected)
			{
				// Request /utc/now from the previously set-up server
				rest.get("/utc/now");
				char response[BUFLEN];
				memset(response, 0, BUFLEN);
				uint16_t code = rest.waitResponse(response, BUFLEN);
				if(code == HTTP_STATUS_OK)
				{
					Serial.println("ARDUINO: GET successful:");
					Serial.println(response);
				}
				else
				{
					Serial.print("ARDUINO: GET failed: ");
					Serial.println(code);
				}
				delay(1000);
			}
		}
	}
@endcode
*/
uint16_t ELClientRest::getResponse(char* data, uint16_t maxLen)
{
  if (_status == 0) return 0;
  memcpy(data, _data, _len>maxLen?maxLen:_len);
  int16_t s = _status;
  _status = 0;
  return s;
}

/*! waitResponse(char* data, uint16_t maxLen, uint32_t timeout)
@brief Wait for the response
@details Wait for the response from the remote server for <code>time_out</code>,
	returns the HTTP status code, 0 if no response (may need to wait longer)
@warning Blocks the Arduino code for 5 seconds! not recommended to use.
	Received packet is NOT null-terminated
@param data
	Pointer to buffer for received packet
@param maxLen
	Size of buffer for received packet. If the received packet is larger than the buffer, the received packet will be truncated.
@param timeout
	(optional) Timout in milli seconds to wait for a response, defaults to 5000ms
@return <code>uint16_t</code>
	Size of received packet or number of sent bytes or 0 if no response
@par Example
@code
	#define BUFLEN 266
	void loop() 
	{
		// process any callbacks coming from esp_link
		esp.Process();
		// if we're connected make an HTTP request
		if(wifiConnected) 
		{
			// Request /utc/now from the previously set-up server
			rest.get("/utc/now");
			char response[BUFLEN];
			memset(response, 0, BUFLEN);
			uint16_t code = rest.waitResponse(response, BUFLEN);
			if(code == HTTP_STATUS_OK)
			{
				Serial.println("ARDUINO: GET successful:");
				Serial.println(response);
			}
			else
			{
				Serial.print("ARDUINO: GET failed: ");
				Serial.println(code);
			}
			delay(1000);
		}
	}
@endcode
*/
uint16_t ELClientRest::waitResponse(char* data, uint16_t maxLen, uint32_t timeout)
{
  uint32_t wait = millis();
  while (_status == 0 && (millis() - wait < timeout)) {
    _elc->Process();
  }
  return getResponse(data, maxLen);
}
