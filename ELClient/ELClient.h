/*! \file ELClient.h
    \brief Definitions for ELClient
*/

#ifndef _EL_CLIENT_H_
#define _EL_CLIENT_H_

#include <avr/pgmspace.h>
#include <HardwareSerial.h>
#include <Arduino.h>
#include "ELClientResponse.h"
#include "FP.h"

#define ESP_TIMEOUT 2000 /**< Default timeout for TCP requests when waiting for a response */

// Enumeration of commands supported by esp-link, this needs to match the definition in
// esp-link!
typedef enum {
  CMD_NULL = 0,     /**< null, mainly to prevent 0 from doing something bad */
  CMD_SYNC,         /**< Synchronize, starts the protocol */
  CMD_RESP_V,       /**< Response with a value */
  CMD_RESP_CB,      /**< Response with a callback */
  CMD_WIFI_STATUS,  /**< Get the wifi status */
  CMD_CB_ADD,       /**< Add a custom callback */
  CMD_CB_EVENTS,    /**< ??? */
  CMD_GET_TIME,     /**< Get current time in seconds since the unix epoch */
  //CMD_GET_INFO,

  CMD_MQTT_SETUP = 10, /**< Register callback functions */
  CMD_MQTT_PUBLISH,    /**< Publish MQTT topic */
  CMD_MQTT_SUBSCRIBE,  /**< Subscribe to MQTT topic */
  CMD_MQTT_LWT,        /**< Define MQTT last will */

  CMD_REST_SETUP = 20, /**< Setup REST connection */
  CMD_REST_REQUEST,    /**< Make request to REST server */
  CMD_REST_SETHEADER,  /**< Define HTML header */

  CMD_TCP_SETUP = 30,  /**< Setup TCP socket connection */
  CMD_TCP_SEND,        /**< Send TCP packet */

  CMD_UDP_SETUP = 40,  /**< Setup UDP socket connection */
  CMD_UDP_SEND,        /**< Send UDP packet */
} CmdName; /**< Enumeration of commands supported by esp-link, this needs to match the definition in esp-link! */

enum WIFI_STATUS {
  STATION_IDLE = 0,        /**< Idle status */
  STATION_CONNECTING,      /**< Trying to connect */
  STATION_WRONG_PASSWORD,  /**< Connection error, wrong password */
  STATION_NO_AP_FOUND,     /**< Connection error, AP not found */
  STATION_CONNECT_FAIL,    /**< Connection error, connection failed */
  STATION_GOT_IP           /**< Connected, received IP */
}; /**< Enumeration of possible WiFi status */

typedef struct {
  uint8_t* buf;
  uint16_t bufSize;
  uint16_t dataLen;
  uint8_t isEsc;
} ELClientProtocol; /**< Protocol structure  */

class ELClient {
  public:
    // Create an esp-link client based on a stream and with a specified debug output stream.
    ELClient(Stream* serial, Stream* debug);
    // Create an esp-link client based on a stream with no debug output
    ELClient(Stream* serial);

    Stream* _debug; /**< Data stream for debug use */

    //== Requests
    // Start a request. cmd is the command to execute, value is either the address of a function
    // to call with a response or a first argument to the command if there is no CB.
    // Argc is the number of additional arguments
    void Request(uint16_t cmd, uint32_t value, uint16_t argc);
    // Add a data block as argument to a request
    void Request(const void* data, uint16_t len);
    // Add a data block from flash as argument to a request
    void Request(const __FlashStringHelper* data, uint16_t len);
    // Finish a request
    void Request(void);

    //== Responses
    // Process the input stream, call this in loop() to dispatch call-back based responses.
    // Callbacks are invoked with an ElClientResponse pointer as argument.
    // Returns the ELClientPacket if a non-callback response was received, typically this is
    // used to create an ELClientResponse. Returns NULL if no response needs to be processed.
    ELClientPacket *Process(void);
    // Busy wait for a response with a timeout in milliseconds, returns an ELClientPacket
    // if a response was recv'd and NULL otherwise. The ELClientPacket is typically used to
    // create an ELClientResponse.
    ELClientPacket *WaitReturn(uint32_t timeout=ESP_TIMEOUT);

    //== Commands built-into ELClient
    // Initialize and synchronize communication with esp-link with a timeout in milliseconds,
    // and remove all existing callbacks. Registers the wifiCb and returns true on success
    boolean Sync(uint32_t timeout=ESP_TIMEOUT);
    // Request the wifi status
    void GetWifiStatus(void);

    // Callback for wifi status changes that must be attached before calling Sync
    FP<void, void*> wifiCb; /**< Pointer to external callback function */

  //private:
    Stream* _serial; /**< Serial stream for communication with ESP */
    boolean _debugEn; /**< Flag for debug - True = enabled, False = disabled */
    uint16_t crc; /**< CRC checksum */
    ELClientProtocol _proto; /**< Protocol structure */
    uint8_t _protoBuf[128]; /**< Protocol buffer */

    void init();
    void DBG(const char* info);
    ELClientPacket *protoCompletedCb(void);
    void write(uint8_t data);
    void write(void* data, uint16_t len);
    uint16_t crc16Add(unsigned char b, uint16_t acc);
    uint16_t crc16Data(const unsigned char *data, uint16_t len, uint16_t acc);
};
#endif // _EL_CLIENT_H_
