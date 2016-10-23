/*! \file ELClientResponse.h
    \brief Definitions for ELClientResponse
*/
#ifndef _EL_CLIENT_RESPONSE_H_
#define _EL_CLIENT_RESPONSE_H_

#if _MSC_VER
#define PACKED
#else
#define PACKED __attribute__ ((__packed__))
#endif

#include <Arduino.h>

#define VARIABLE_ARG_NUM 255

typedef struct PACKED {
  uint16_t cmd;            /**< Command to execute */
  uint16_t argc;           /**< Number of arguments */
  uint32_t value;          /**< Callback to invoke, NULL if none; or response value */
  uint8_t  args[0];        /**< Arguments */
} ELClientPacket; /**< Packet structure  */

// ELClientResponse is a parser for responses. The constructor initializes the parser based on
// a packet and the various pop functions consume one response argument at a time.
class ELClientResponse {
  public:
    // Create a response from a packet, this is done internally in ELClient
    ELClientResponse(ELClientPacket* packet);
    ELClientResponse(void *packet);

    // Accessors to the response fields
    uint16_t argc() { return _cmd->argc; } /**< Get number of arguments  */
    uint16_t cmd() { return _cmd->cmd; } /**< Get command  */
    uint32_t value() { return _cmd->value; } /**< Get returned value  */

    // Return the length of the next argument
    uint16_t argLen() { return *(uint16_t*)_arg_ptr; } /**< Get length of argument  */
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
    uint16_t _arg_num; /**< Argument number */
    uint8_t* _arg_ptr; /**< Pointer to argument */
    ELClientPacket* _cmd; /**< Pointer to packet */
};

#endif // _EL_CLIENT_RESPONSE_H_
