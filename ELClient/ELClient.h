#ifndef _EL_CLIENT_H_
#define _EL_CLIENT_H_

#include <avr/pgmspace.h>
#include <HardwareSerial.h>
#include <Arduino.h>
#include "ELClientResponse.h"
#include "FP.h"

#define ESP_TIMEOUT 2000

#define SLIP_END  0300        // indicates end of packet
#define SLIP_ESC  0333        // indicates byte stuffing
#define SLIP_ESC_END  0334    // ESC ESC_END means END data byte
#define SLIP_ESC_ESC  0335    // ESC ESC_ESC means ESC data byte

// Enumeration of commands supported by esp-link, this needs to match the definition in
// esp-link!
typedef enum {
  CMD_NULL = 0,
  CMD_SYNC,
  CMD_IS_READY,
  CMD_CLEAR_CBS,
  CMD_CB_ADD,
  CMD_CB_EVENTS,
  CMD_WIFI_INIT,
  CMD_GET_TIME,
  CMD_GET_INFO,

  CMD_MQTT_INIT,
  CMD_MQTT_PUBLISH,
  CMD_MQTT_SUBSCRIBE,
  CMD_MQTT_LWT,
  CMD_MQTT_EVENTS,

  CMD_REST_SETUP,
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
    ELClient(Stream* serial);
    Stream* _debug;

    uint32_t return_value; // 32-bit value returned in response
    uint16_t return_cmd;   // ???
    boolean is_return;     // flag indicating that response has been received

    //== Requests
    // Start a request
    uint16_t Request(uint16_t cmd, uint32_t callback, uint32_t _return, uint16_t argc);
    // Add a data block to a request
    uint16_t Request(uint16_t crc_in, const void* data, uint16_t len);
    // Add a data block from flash to a request
    uint16_t Request(uint16_t crc_in, const __FlashStringHelper* data, uint16_t len);
    // Finish a request
    void Request(uint16_t crc);

    //== Responses
    // Initialize and synchronize communication with esp-link with a timeout in milliseconds,
    // and remove all existing callbacks. Registers the wifiCb and returns true on success
    boolean Sync(uint32_t timeout=ESP_TIMEOUT);
    // Process the input stream, call this in loop()
    void Process();
    // Busy wait for a response with a timeout in milliseconds, returns true if a response was recv'd
    boolean WaitReturn(uint32_t timeout=ESP_TIMEOUT);

    // Callback for wifi status changes that must be attached before calling Sync
    FP<void, void*> wifiCb;

  //private:
    Stream* _serial;
    boolean _debugEn;
    ELClientProtocol _proto;
    uint8_t _protoBuf[128];

    void init();
    void DBG(const char* info);
    void protoCompletedCb(void);
    void write(uint8_t data);
    void write(uint8_t* data, uint16_t len);
    uint16_t crc16Add(unsigned char b, uint16_t acc);
    uint16_t crc16Data(const unsigned char *data, uint16_t len, uint16_t acc);
};
#endif // _EL_CLIENT_H_
