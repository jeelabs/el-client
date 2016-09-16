/*! \file ELClientSocket.h
	\brief Definitions for ELClientSocket
	\author BeeGee
	\version 1.0
	\date 2016
	\copyright GNU Public License.
	\warning Needs ESP-LINK V2.4
*/

#ifndef _EL_CLIENT_SOCKET_H_
#define _EL_CLIENT_SOCKET_H_

#include <Arduino.h>
#include "FP.h"
#include "ELClient.h"

#define DEFAULT_SOCKET_TIMEOUT	5000 /**< Default timeout for SOCKET requests when waiting for a response */

#define USERCB_SENT 0 /**< Type of callback from ELClient. SOCKET packet has been sent */
#define USERCB_RECV 1 /**< Type of callback from ELClient. SOCKET packet has been received */
#define USERCB_RECO 2 /**< Type of callback from ELClient. SOCKET connection error */
#define USERCB_CONN 3 /**< Type of callback from ELClient. SOCKET socket connected or disconnected */

// Socket mode definitions
#define SOCKET_TCP_CLIENT 0 /**< TCP socket client for sending only, doesn't wait for response from server */
#define SOCKET_TCP_CLIENT_LISTEN 1 /**< TCP socket client, waits for response from server after sending */
#define SOCKET_TCP_SERVER 2 /**< TCP socket server */
#define SOCKET_UDP 3 /**< UDP socket for sending and receiving UDP packets */
	
// Enable/disable debug output. If defined enables the debug output on Serial port
//#define DEBUG_EN /**< Enable/disable debug output */

// The ELClientSocket class sends data over a simple Socket connection to a remote server. Each instance
// is used to communicate with one server and multiple instances can be created to send
// to multiple servers.
// The ELClientSocket class does not support concurrent requests to the same server because
// only a single response can be recevied at a time and the responses of the two requests
// may arrive out of order.
// A major limitation of the Socket class is that it does not wait for the response data. 
class ELClientSocket {
	public:
		ELClientSocket(ELClient *e);

		// Initialize communication to a remote server, this communicates with esp-link but does not
		// open a connection to the remote server. Host may be a hostname or an IP address.
		// Port needs to be defined different from usual HTTP/HTTPS/FTP/SSH ports
		// sock_mode defines whether the socket act as a client (with or without receiver) or as a server
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
		// !!! UDP doesn't check if the data was received or if the receiver IP/socket is available !!! You need to implement your own
		// error control!
		uint16_t getResponse(uint8_t *resp_type, uint8_t *client_num, char* data, uint16_t maxLen);

		// Wait for the response from the remote server, returns the length of received data or 0 if no
		// response (timeout occurred)
		// Blocks the Arduino code for 5 seconds! not recommended to use. See code examples how to use the callback function instead
		uint16_t waitResponse(uint8_t *resp_type, uint8_t *client_num, char* data, uint16_t maxLen, uint32_t timeout=DEFAULT_SOCKET_TIMEOUT);

		int32_t remote_instance; /**< Connection number, value can be 0 to 3 */

	private:
		ELClient *_elc; /**< ELClient instance */
		void socketCallback(void* resp);
		FP<void, void*> socketCb; /**< Pointer to external callback function */

		/*! void (* _userCallback)(uint8_t resp_type, uint8_t client_num, uint16_t len, char *data)
		@brief Callback function when data is sent or received
		@details This function is called by ELClient library when a packet was sent, a packet was received or an error occured
			The function is user specific and therefor included in the program code, not in the library
			This function does not block the Arduino code execution
		@param resp_type
			Response type. Is USERCB_SENT if packet was sent, USERCB_RECV if a packet was received, USERCB_RECO if a connection error occured or USERCB_CONN on a connect or disconnect event
		@param client_num
			Connection number. Can be used to distinguish between different UDP clients.
		@param len
			Size of received packet or number of sent bytes.
		@param data
			Buffer with the received packet.
		@note
			The function is user specific and therefor included in the program code, not in the library
		@par Example
		@code
			void socketCb(uint8_t resp_type, uint8_t client_num, uint16_t len, char *data)
			{
				Serial.println("socketCb connection #"+String(client_num));
				if (resp_type == USERCB_SENT)
				{
					Serial.println("\tSent " + String(len) + " bytes over client#" + String(client_num));
				}
				else if (resp_type == USERCB_RECV)
				{
					char recvData[len+1]; // Prepare buffer for the received data
					memcpy(recvData, data, len); // Copy received data into the buffer
					recvData[len] = '\0'; // Terminate the buffer with 0 for proper printout!
					Serial.println("\tReceived " + String(len) + " bytes over the server on connection #" + String(client_num));
					Serial.println("\tReceived: " + String(recvData));
					char respData[len+11]; // Prepare buffer for the response data
					char *respHdr = "Received: ";
					memcpy (respData, respHdr, 10);
					memcpy(&respData[10], recvData, len); // Copy received data into the buffer
					respData[len+10] = '\0';
					Serial.println("\tSend response: " + String(respData));
					socket.send(respData);
				}
				else if (resp_type == USERCB_RECO)
				{
					Serial.println("Connection problem: "+String(len));
				}
				else if (resp_type == USERCB_CONN)
				{
					if (len == 0)
					{
						Serial.println("\tDisconnected");
					}
					else
					{
						Serial.println("\tConnected");
					}
				}
				else
				{
					Serial.println("Received invalid response type");
				}
			}
		@endcode
		*/
		typedef void (* _userCallback)(uint8_t resp_type, uint8_t client_num, uint16_t len, char *data);
		_userCallback _userCb; /**< Pointer to internal callback function */

		bool _hasUserCb = false; /**< Flag for user callback, true if user callback has been set */
		int16_t _status; /**< Connection status */
		uint16_t _len; /**< Number of sent/received bytes */
		char *_data; /**< Buffer for received data */
		uint8_t _resp_type; /**< Response type: 0 = send, 1 = receive; 2 = reset connection, 3 = connection */
		uint8_t _client_num; /**< Connection number, value can be 0 to 3 */
};
#endif // _EL_CLIENT_SOCKET_H_
