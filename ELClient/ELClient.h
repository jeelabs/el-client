#ifndef _EL_CLIENT_H_
#define _EL_CLIENT_H_

#include <avr/pgmspace.h>
#include <HardwareSerial.h>
#include <Arduino.h>
#include "ELClientResponse.h"
#include "FP.h"

#define ESP_TIMEOUT 2000

// Enumeration of commands supported by esp-link, this needs to match the definition in
// esp-link!
typedef enum {
  CMD_NULL = 0,     // null, mainly to prevent 0 from doing something bad
  CMD_SYNC,         // synchronize, starts the protocol
  CMD_RESP_V,       // response with a value
  CMD_RESP_CB,      // response with a callback
  CMD_WIFI_STATUS,  // get the wifi status
  CMD_CB_ADD,       // add a custom callback
  CMD_CB_EVENTS,    // ???
  CMD_GET_TIME,     // get current time in seconds since the unix epoch
  //CMD_GET_INFO,

  CMD_MQTT_SETUP = 10,
  CMD_MQTT_PUBLISH,
  CMD_MQTT_SUBSCRIBE,
  CMD_MQTT_LWT,
  CMD_MQTT_EVENTS,

  CMD_REST_SETUP = 20,
  CMD_REST_REQUEST,
  CMD_REST_SETHEADER,
  CMD_REST_EVENTS

} CmdName;

enum WIFI_STATUS {
  STATION_IDLE = 0,
  STATION_CONNECTING,
  STATION_WRONG_PASSWORD,
  STATION_NO_AP_FOUND,
  STATION_CONNECT_FAIL,
  STATION_GOT_IP
};

typedef struct {
  uint8_t* buf;
  uint16_t bufSize;
  uint16_t dataLen;
  uint8_t isEsc;
} ELClientProtocol;

class ELClient {
  public:
    // Create an esp-link client based on a stream and with a specified debug output stream.
    ELClient(Stream* serial, Stream* debug);
    // Create an esp-link client based on a stream with no debug output
    ELClient(Stream* serial);

    Stream* _debug;

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
    FP<void, void*> wifiCb;

  //private:
    Stream* _serial;
    boolean _debugEn;
    uint16_t crc;
    ELClientProtocol _proto;
    uint8_t _protoBuf[128];

    void init();
    void DBG(const char* info);
    ELClientPacket *protoCompletedCb(void);
    void write(uint8_t data);
    void write(void* data, uint16_t len);
    uint16_t crc16Add(unsigned char b, uint16_t acc);
    uint16_t crc16Data(const unsigned char *data, uint16_t len, uint16_t acc);
};
#endif // _EL_CLIENT_H_
