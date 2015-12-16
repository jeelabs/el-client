#ifndef _EL_CLIENT_RESPONSE_H_
#define _EL_CLIENT_RESPONSE_H_

#if _MSC_VER
#define PACKED
#else
#define PACKED __attribute__ ((__packed__))
#endif

#include <Arduino.h>

typedef struct PACKED {
  uint16_t len;
  uint8_t data;
} ELClientPacketArgs;

typedef struct PACKED {
  uint16_t cmd;
  uint32_t callback;
  uint32_t _return;
  uint16_t argc;
  ELClientPacketArgs args;
} ELClientPacket;

class ELClientResponse {
  public:
    ELClientResponse(void* response);
    uint16_t getArgc();
    int32_t popArgs(uint8_t* data, uint16_t maxLen);
    void popChar(char* buffer);
    uint16_t argLen();
    String popString();
    void popString(String* data);

  private:
    uint16_t _arg_num;
    uint8_t* _arg_ptr;
    ELClientPacket* _cmd;
};

#endif // _EL_CLIENT_RESPONSE_H_