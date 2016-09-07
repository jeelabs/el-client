// Copyright (c) 2016 by BeeGee
// adapted from ELClientRest.h Copyright (c) 2016 by B. Runnels and T. von Eicken

#ifndef _EL_CLIENT_TCP_H_
#define _EL_CLIENT_TCP_H_

#include <Arduino.h>
#include "FP.h"
#include "ELClient.h"

// Default timeout for TCP requests when waiting for a response
#define DEFAULT_TCP_TIMEOUT	5000

// Callback type definitions
#define USERCB_SENT 0
#define USERCB_RECV 1
#define USERCB_RECO 2
#define USERCB_CONN 3

// Socket mode definitions
#define SOCKET_CLIENT 0 // TCP socket client for sending only, doesn't wait for response from server
#define SOCKET_CLIENT_LISTEN 1 // TCP socket client, waits for response from server after sending
#define SOCKET_SERVER 2 // TCP socket server

// Define to enable Debug output on Serial port
//#define DEBUG_EN

// The ELClientTcp class sends data over a simple TCP socket to a remote server. Each instance
// is used to communicate with one server and multiple instances can be created to send
// to multiple servers.
// The ELClientTcp class does not support concurrent requests to the same server because
// only a single response can be recevied at a time and the responses of the two requests
// may arrive out of order.
// A major limitation of the TCP class is that it does not wait for the response data. 
class ELClientTcp {
	public:
		ELClientTcp(ELClient *e);

		// Initialize communication to a remote server, this communicates with esp-link but does not
		// open a connection to the remote server. Host may be a hostname or an IP address.
		// Port needs to be defined different from usual HTTP/HTTPS/FTP/SSH ports
		// sock_mode defines whether the TCP socket act as a client (with or without receiver) or as a server
		// Returns 0 if the set-up is
		// successful, returns a negative error code if it failed.
		// Optional a pointer to a callback function be added. The callback function will be called after data is sent out, 
		// after data was received or when an error occured. See example code port how to use it.
		int begin(const char* host, uint16_t port, uint8_t sock_mode, void (*userCb)(uint8_t resp_type, uint8_t client_num, uint16_t len, char *data)=0);

		// Send data to the remote server. The data must be null-terminated
		void send(const char* data);

		// Send data to the remote server.
		void send(const char* data, int len);

		// Retrieve the response from the remote server, returns the number of send or received bytes, 0 if no
		// response (may need to wait longer)
		uint16_t getResponse(uint8_t *resp_type, uint8_t *client_num, char* data, uint16_t maxLen);

		// Wait for the response from the remote server, returns the length of received data or 0 if no
		// response (timeout occurred)
		// Blocks the Arduino code for 5 seconds! not recommended to use. See code examples how to use the callback function instead
		uint16_t waitResponse(uint8_t *resp_type, uint8_t *client_num, char* data, uint16_t maxLen, uint32_t timeout=DEFAULT_TCP_TIMEOUT);

		int32_t remote_instance;

	private:
		ELClient *_elc;
		void tcpCallback(void* resp);
		FP<void, void*> tcpCb;

		typedef void (* _userCallback)(uint8_t resp_type, uint8_t client_num, uint16_t len, char *data);
		_userCallback _userCb;

		bool _hasUserCb = false;
		int16_t _status;
		uint16_t _len;
		char *_data;
		uint8_t _resp_type; // 0 = send, 1 = receive; 2 = reset connection, 3 = connection
		uint8_t _client_num;
};
#endif // _EL_CLIENT_TCP_H_
