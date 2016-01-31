#ifndef _EL_CLIENT_RESPONSE_H_
#define _EL_CLIENT_RESPONSE_H_

#if _MSC_VER
#define PACKED
#else
#define PACKED __attribute__ ((__packed__))
#endif

#include <Arduino.h>

typedef struct PACKED {
  uint16_t cmd;            // command to execute
  uint16_t argc;           // number of arguments
  uint32_t value;          // callback to invoke, NULL if none; or response value
  uint8_t  args[0];
} ELClientPacket;

class ELClientResponse {
  public:
    // Create a response from a packet, this is done internally in ELClient
    ELClientResponse(ELClientPacket* packet);
    ELClientResponse(void *packet);

    // Accessors to the response fields
    uint16_t argc() { return _cmd->argc; }
    uint16_t cmd() { return _cmd->cmd; }
    uint32_t value() { return _cmd->value; }

    // Return the length of the next argument
    uint16_t argLen() { return *(uint16_t*)_arg_ptr; }
    // Pop one argument from the response, returns the actual length. Returns -1 if there is
    // no arg left.
    int16_t popArg(void* data, uint16_t maxLen);
    // Pop one argument as a poiner from the response, returns the actual length.
    int16_t popArgPtr(void **data);
    // Pop one argument into a string buffer and append a null character. The buffer needs to
    // be large enough (argLen()+1)
    void popChar(char* buffer);
    // Pop one argument into a String buffer
    String popString();
    // Pop one argument into a String buffer
    void popString(String* data);

  private:
    uint16_t _arg_num;
    uint8_t* _arg_ptr;
    ELClientPacket* _cmd;
};

#endif // _EL_CLIENT_RESPONSE_H_
