#ifndef _EL_CLIENT_H_
#define _EL_CLIENT_H_

#include <avr/pgmspace.h>
#include <HardwareSerial.h>
#include <Arduino.h>
#include "ELClientResponse.h"
#include "FP.h"
#include "ringbuf.h"

#define ESP_TIMEOUT 2000

#define SLIP_START 0x7E
#define SLIP_END  0x7F
#define SLIP_REPL 0x7D
#define SLIP_ESC(x) (x ^ 0x20)

typedef enum {
  CMD_IS_READY = 0,  
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
  uint8_t isBegin;
} ELClientProtocol;

class ELClient {
  public:
    ELClient(Stream* serial, Stream* debug, uint8_t chip_pd = -1);
    ELClient(Stream* serial, uint8_t chip_pd = -1);
    Stream* _debug;

    uint32_t return_value;
    uint16_t return_cmd;
    boolean is_return;
    
    void Run();
    uint16_t Request(uint16_t cmd, uint32_t callback, uint32_t _return, uint16_t argc);
    uint16_t Request(uint16_t crc_in, uint8_t* data, uint16_t len);
    uint16_t Request(uint16_t crc_in, const __FlashStringHelper* data, uint16_t len);
    uint16_t Request(uint16_t crc);    
    void Enable();
    void Disable();
    boolean WaitReturn(uint32_t timeout);
    boolean WaitReturn();

  private:
    Stream* _serial;
    boolean _debugEn;
    ELClientProtocol _proto;
    uint8_t _protoBuf[128];
    int _chip_pd;

    void init();
    void DBG(const char* info);
    void protoCompletedCb(void);
    void write(uint8_t data);
    void write(uint8_t* data, uint16_t len);
    uint8_t crc16Add(unsigned char b, uint8_t acc);
    uint8_t crc16Data(const unsigned char *data, uint16_t len, uint8_t acc);
};
#endif // _EL_CLIENT_H_
