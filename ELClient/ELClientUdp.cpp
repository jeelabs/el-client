/*! \file ELClientUdp.cpp
	\brief Constructor and functions for ELClientUdp
	\author BeeGee
	\version 1.0
	\date 2016
	\copyright GNU Public License.
	\warning Needs ESP-LINK V2.4
*/

#include "ELClientUdp.h"

/*! ELClientUdp(ELClient *e)
@brief Class to send/receive data
@details The ELClientUdp class sends data over a UDP socket to a remote server.
	Each instance is used to communicate with one server and multiple instances can be created to send to multiple servers.
	The ELClientUdp class does not support concurrent requests to the same server because only a single response can be recevied at a time and the responses of the two requests may arrive out of order.
@param e
	Pointer to ELClient. Check ELClient API documentation.
@par Example
@code
	ELClientUdp udp(&esp);
	@endcode
*/
ELClientUdp::ELClientUdp(ELClient *e) 
{
	_elc = e;
	remote_instance = -1;
}

/*! udpCallback(void *res)
@brief Function called by esp-link when data is sent or received.
@details The function is called by esp-link when data is sent or received from the remote server.
	If a user callback function (userCb) was defined it is called and the response is sent as an argument.
@note Internal library function
@param res
	Pointer to ELClientResponse structure
@warning The content of the response structure is overwritten when the next package arrives!
*/
void ELClientUdp::udpCallback(void *res) 
{
	if (!res) return;

	ELClientResponse *resp = (ELClientResponse *)res;

	#ifdef DEBUG_EN
		int argNum = resp->argc();
		Serial.println("Number of arguments: "+String(argNum));
		uint16_t _cmd = resp->cmd();
		Serial.println("Command: "+String(_cmd));
		uint16_t _value = resp->value();
		Serial.println("Value: "+String(_value));
	#endif

	resp->popArg(&_resp_type, 1);
	resp->popArg(&_client_num, 1);
	resp->popArg(&_len, 2);
	#ifdef DEBUG_EN
		Serial.print("Type: ");
		Serial.print(_resp_type);
		Serial.print(" client: ");
		Serial.print(_client_num);
		Serial.print(" size: "+String(_len));
	#endif
	if (_resp_type == 1) 
	{
		#ifdef DEBUG_EN
			int argLen = resp->argLen();
			Serial.print(" data length: "+String(argLen));
		#endif
		resp->popArgPtr(&_data);
		#ifdef DEBUG_EN
			_data[_len] = '\0';
			Serial.print(" data: "+String(_data));
		#endif
	}
	#ifdef DEBUG_EN
		Serial.println("");
	#endif
	_status = 1;
	if (_hasUserCb) 
	{
		_userCb(_resp_type, _client_num, _len, _data);
	}
}

/*! begin(const char* host, uint16_t port, void (*userCb)(uint8_t resp_type, uint8_t client_num, uint16_t len, char *data))
@brief Initialize communication to a remote server
@details Initialize communication to a remote server,
	this communicates with esp-link but does not open a connection to the remote server.
@param host
	Host to be connected. Can be a URL or an IP address in the format of xxx.xxx.xxx.xxx . To make a UDP broadcast (send UDP packet to more than one IP) use the host argument as the netmask. Ex.: "192.168.0.255" sends to UDP packet to all IPs from 192.168.0.0 to 192.168.0.254
@param port
	Port to be used to send/receive packets. Port MUST NOT be 80, 23 or 2323, as these ports are already used by EL-CLIENT on the ESP8266
@param (*userCb)(uint8_t resp_type, uint8_t client_num, uint16_t len, char *data)
	(optional) Pointer to callback function that is called if data after data has been sent, received or if an error occured
@warning Port MUST NOT be 80, 23 or 2323, as these ports are already used by EL-CLIENT on the ESP8266.
	Max 4 connections are supported!
@par Example
@code
	int err = udp.begin(udpServer, udpPort, udpCb);
	if (err != 0)
	{
		Serial.print("UDP begin failed: ");
		Serial.println(err);
	}
	else
	{
		Serial.print("UDP setup successful");
	}
@endcode
*/
int ELClientUdp::begin(const char* host, uint16_t port, void (*userCb)(uint8_t resp_type, uint8_t client_num, uint16_t len, char *data)) 
{
	if (userCb != 0) 
	{
		_userCb = userCb;
		_hasUserCb = true;
	}

	uint8_t sec = false;
	udpCb.attach(this, &ELClientUdp::udpCallback);

	_elc->Request(CMD_UDP_SETUP, (uint32_t)&udpCb, 3);
	_elc->Request(host, strlen(host));
	_elc->Request(&port, 2);
	_elc->Request(&sec, 1);
	_elc->Request();

	ELClientPacket *pkt = _elc->WaitReturn();
	if (pkt && (int32_t)pkt->value >= 0) 
	{
		remote_instance = pkt->value;
		return 0;
	}
	return (int)pkt->value;
}

/*! send(const char* data, int len)
@brief Send data to the remote server.
@param data
	Pointer to UDP packet
@param len
	Length of UDP packet
@par Example
@code
	char udpPacket = "{"device":"spm","s":622.02,"c":-165.86}"
	udp.send(udpPacket, 39);
@endcode
*/
void ELClientUdp::send(const char* data, int len) 
{
	_status = 0;
	if (remote_instance < 0) return;
	_elc->Request(CMD_UDP_SEND, remote_instance, 2);
	_elc->Request(data, strlen(data));
	if (data != NULL && len > 0) 
	{
		_elc->Request(data, len);
	}

	_elc->Request();
}

/*! send(const char* data)
@brief Send null-terminated data to the remote server.
@param data
	Pointer to UDP packet, must be null-terminated
@par Example
@code
	udp.send("Message from your Arduino Uno WiFi over UDP socket");
@endcode
*/
void ELClientUdp::send(const char* data) 
{
	send(data, strlen(data));
}

/*! getResponse(uint8_t *resp_type, uint8_t *client_num, char* data, uint16_t maxLen)
@brief Retrieve response.
@details Checks if a response from the remote server has arrived,
	returns the number of send or received bytes,
	0 if no response (may need to wait longer)
@param resp_type
	Pointer to response type. Is USERCB_SENT if packet was sent or USERCB_RECV if a packet was received.
@param client_num
	Pointer to connection number. Can be used to distinguish between different UDP clients.
@param data
	Pointer to buffer for received packet
@param maxLen
	Size of buffer for received packet. If the received packet is larger than the buffer, the received packet will be truncated.
@return <code>uint16_t</code>
	Size of received packet or number of sent bytes or 0 if no response
@warning The received packet might not be null-terminated!
@par Example
@code
	#define BUFLEN 266
	char response[BUFLEN];
	bool haveRemoteResponse = true;
	void loop() 
	{
		// process any callbacks coming from esp_link
		esp.Process();
		if (haveRemoteResponse) // If last packet was sent, send a new one
		{
			Serial.println("Sending JSON array to UDP server");
			char udpPacket = "{"device":"spm","s":622.02,"c":-165.86}"
			udp.send(udpPacket, 39);
			haveRemoteResponse = false;
		}
		// Check if we received a packet or if the last send request has finished
		memset(response, 0, BUFLEN);
		uint8_t resp_type;
		uint8_t client_num;
		uint16_t len = udp.getResponse(&resp_type, &client_num, response, BUFLEN);
		if (len != 0)
		{
			if (resp_type == USERCB_SENT)
			{
				Serial.println("Sent "+String(len)+" bytes");
			}
			else
			{
				Serial.print("Received packet: ");
				for (int i=0; i<len; i++)
				{
					Serial.print(response[i]);
				}
				Serial.println("");
			}
		}
	}
@endcode
*/
uint16_t ELClientUdp::getResponse(uint8_t *resp_type, uint8_t *client_num, char* data, uint16_t maxLen) 
{
	if (_status == 0) return 0;
	memcpy(data, _data, _len>maxLen?maxLen:_len);
	*resp_type = _resp_type;
	*client_num = _client_num;
	_status = 0;
	return _len;
}

/*! waitResponse(uint8_t *resp_type, uint8_t *client_num, char* data, uint16_t maxLen)
@brief Wait for the response
@details Checks if a response from the remote server has arrived for <code>timeout</code> seconds,
	returns the number of send or received bytes,
	0 if no response (may need to wait longer)
@warning Blocks the Arduino code for 5 seconds! not recommended to use. Use callback function instead!
	Received packet is NOT null-terminated
	!!! UDP doesn’t check if the data was received or if the receiver IP/socket is available !!!
@param resp_type
	Pointer to response type. Is USERCB_SENT if packet was sent or USERCB_RECV if a packet was received.
@param client_num
	Pointer to connection number. Can be used to distinguish between different UDP clients.
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
	char response[BUFLEN];
	bool haveRemoteResponse = true;
	void loop() 
	{
		// process any callbacks coming from esp_link
		esp.Process();
		if (haveRemoteResponse) // If last packet was sent, send a new one
		{
			Serial.println("Sending JSON array to UDP server");
			char udpPacket = "{"device":"spm","s":622.02,"c":-165.86}"
			udp.send(udpPacket, 39);
			haveRemoteResponse = false;
		}
		// Check if we received a packet or if the last send request has finished
		memset(response, 0, BUFLEN);
		uint8_t resp_type;
		uint8_t client_num;
		uint16_t len = udp.waitResponse(&resp_type, &client_num, response, BUFLEN);
		if (len != 0)
		{
			if (resp_type == USERCB_SENT)
			{
				Serial.println("Sent "+String(len)+" bytes");
			}
			else
			{
				Serial.print("Received packet: ");
				for (int i=0; i<len; i++)
				{
					Serial.print(response[i]);
				}
				Serial.println("");
				haveRemoteResponse = true;
			}
		}
	}
@endcode
*/
uint16_t ELClientUdp::waitResponse(uint8_t *resp_type, uint8_t *client_num, char* data, uint16_t maxLen, uint32_t timeout) 
{
	uint32_t wait = millis();
	while (_status == 0) 
	{
		if ( millis() - wait < timeout) 
		{
			_elc->Process();
		}
		else
		{
			return -3;
		}
	}
	return getResponse(resp_type, client_num, data, maxLen);
}
