/*! \file ELClientUdp.h
		\brief Definitions for ELClientUdp
		\author BeeGee
		\version 1.0
		\date 2016
		\copyright GNU Public License.
		\warning Needs ESP-LINK V2.4
*/

#ifndef _EL_CLIENT_UDP_H_
#define _EL_CLIENT_UDP_H_

#include <Arduino.h>
#include "FP.h"
#include "ELClient.h"

#define DEFAULT_UDP_TIMEOUT	5000 /**< Default timeout for UCP requests when waiting for a response */

#define USERCB_SENT 0 /**< Type of callback from ELClient. UDP packet has been sent */
#define USERCB_RECV 1 /**< Type of callback from ELClient. UDP packet has been received */

// Enable/disable debug output. If defined enables the debug output on Serial port
//#define DEBUG_EN /**< Enable/disable debug output */

// The ELClientUdp class sends data over a simple UDP socket to a remote server. Each instance
// is used to communicate with one server and multiple instances can be created to send
// to multiple servers.
// The ELClientUdp class does not support concurrent requests to the same server because
// only a single response can be recevied at a time and the responses of the two requests
// may arrive out of order.
// A major limitation of the UDP class is that it does not wait for the response data.
class ELClientUdp {
	public:
		ELClientUdp(ELClient *e);

		// Initialize communication to a remote server, this communicates with esp-link but does not
		// open a connection to the remote server. Host may be a hostname or an IP address.
		// Port needs to be defined different from usual HTTP/HTTPS/FTP/SSH ports
		// Returns 0 if the set-up is
		// successful, returns a negative error code if it failed.
		// Optional a pointer to a callback function be added. The callback function will be called after data is sent out,
		// after data was received or when an error occured. See udp.ino example port how to use it.
		int begin(const char* host, uint16_t port, void (*userCb)(uint8_t resp_type, uint8_t client_num, uint16_t len, char *data)=0);

		// Send data to the remote server. The data must be null-terminated
		void send(const char* data);

		// Send data to the remote server.
		void send(const char* data, int len);

		// Retrieve the response from the remote server, returns the number of send or received bytes, 0 if no
		// response (may need to wait longer)
		// !!! UDP doesn't check if the data was received or if the receiver IP/socket is available !!! You need to implement your own
		// error control!
		uint16_t getResponse(uint8_t *resp_type, uint8_t *client_num, char* data, uint16_t maxLen);

		// Wait for the response from the remote server, returns the number of send or received bytes, 0 if no
		// response (timeout occurred)
		// Blocks the Arduino code for 5 seconds! not recommended to use. See udp.ino example how to use the callback function instead
		// !!! UDP doesn't check if the data was received or if the receiver IP/socket is available !!! You need to implement your own
		// error control!
		uint16_t waitResponse(uint8_t *resp_type, uint8_t *client_num, char* data, uint16_t maxLen, uint32_t timeout=DEFAULT_UDP_TIMEOUT);

	private:
		int32_t remote_instance; /**< Connection number, value can be 0 to 3 */
		ELClient *_elc; /**< ELClient instance */
		void udpCallback(void* resp);
		FP<void, void*> udpCb; /**< Pointer to external callback function */

		/*! void (* _userCallback)(uint8_t resp_type, uint8_t client_num, uint16_t len, char *data)
		@brief Callback function when data is sent or received
		@details This function is called by ELClient library when a packet was sent, a packet was received or an error occured
			The function is user specific and therefor included in the program code, not in the library
			This function does not block the Arduino code execution
		@param resp_type
			Response type. Is USERCB_SENT if packet was sent or USERCB_RECV if a packet was received.
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
			void udpCb(uint8_t resp_type, uint8_t client_num, uint16_t len, char *data)
			{
				Serial.println("udpCb is called");
				if (len > 0)	// sending complete (no confirmation that it was received!) or we received something
				{
					if (resp_type == USERCB_SENT)
					{
						Serial.println("\tSent " + String(len) + " bytes over connection #" + String(client_num));
					}
					else if (resp_type == USERCB_RECV)
					{
						char recvData[len+1]; // Prepare buffer for the received data
						memcpy(recvData, data, len); // Copy received data into the buffer
						recvData[len] = '\0'; // Terminate the buffer with 0 for proper printout!
						Serial.println("\tReceived " + String(len) + " bytes over client#" + String(client_num));
						Serial.println("\tReceived: " + String(recvData));
					}
					else
					{
						Serial.println("Received invalid response type");
					}
				}
				else if (len < 0) // negative result means there was a problem
				{
					Serial.print("Send error: ");
					Serial.println(getErrTxt(len));
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
		uint8_t _resp_type; /**< Response type: 0 = send, 1 = receive */
		uint8_t _client_num; /**< Connection number, value can be 0 to 3 */
};
#endif // _EL_CLIENT_UDP_H_
