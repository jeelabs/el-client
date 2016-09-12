/*! \file ELClientResponse.cpp
    \brief Constructor and functions for ELClientResponse
*/
#include "ELClientResponse.h"

/*! ELClientResponse(ELClientPacket* packet)
@brief Constructor for ELClientResponse with ELClientPacket packet
@param packet
	Pointer to packet for response
@par Example
@code
	no example code yet
@endcode
*/
ELClientResponse::ELClientResponse(ELClientPacket* packet) {
  _cmd = packet;
  _arg_ptr = _cmd->args;
  _arg_num = 0;
}

/*! ELClientResponse(void* packet)
@brief Constructor for ELClientResponse with void packet
@param packet
	Pointer to packet for response
@par Example
@code
	no example code yet
@endcode
*/
ELClientResponse::ELClientResponse(void* packet) {
  _cmd = (ELClientPacket *)packet;
  _arg_ptr = _cmd->args;
  _arg_num = 0;
}

/*! popArgPtr(void **data)
@brief Extract pointer to an argument from the response packet
@param data
	Pointer to buffer for the argument pointer
@return <code>int16_t</code>
	Size of argument
@par Example
@code
	no example code yet
@endcode
*/
int16_t ELClientResponse::popArgPtr(void **data) {
  if (_arg_num >= _cmd->argc) return -1;

  uint16_t len = *(uint16_t*)_arg_ptr;
  uint16_t pad = (4-((len+2)&3))&3;    // pad to next multiple of 4, including length
  _arg_ptr += 2;
  _arg_num++;

  *data = _arg_ptr;
  _arg_ptr += len + pad;
  return len;
}

/*! popArg(void* d, uint16_t maxLen)
@brief Extract an argument from the response packet
@param d
	Pointer to buffer for the argument
@param maxLen
	Size of the buffer for the argument
@return <code>int16_t</code>
	Size of argument
@par Example
@code
	no example code yet
@endcode
*/
int16_t ELClientResponse::popArg(void* d, uint16_t maxLen) {
  if (_arg_num >= _cmd->argc) return -1;

  uint16_t len = *(uint16_t*)_arg_ptr;
  uint16_t pad = (4-((len+2)&3))&3;    // pad to next multiple of 4, including length
  _arg_ptr += 2;
  _arg_num++;

  uint8_t *data = (uint8_t *)d;
  uint16_t l = len > maxLen ? maxLen : len;
  uint8_t *p = _arg_ptr;
  while (l--)
    *data++ = *p++;

  _arg_ptr += len + pad;
  return len;
}

/*! popChar(char* buffer)
@brief Extract a character from the response packet
@param buffer
	Pointer to buffer for the character
@par Example
@code
	no example code yet
@endcode
*/
void ELClientResponse::popChar(char* buffer) {
  uint16_t len = *(uint16_t*)_arg_ptr;
  uint16_t pad = (4-((len+2)&3))&3;    // pad to next multiple of 4, including length
  _arg_ptr += 2;
  _arg_num++;

  uint8_t i;
  for (i = 0; i < len; i++) {
    buffer[i] = (char)*_arg_ptr++;
  }
  buffer[i] = '\0';

  _arg_ptr += pad;
}

/*! popString()
@brief Extract a string from the response packet
@return <code>String</code>
	String extracted from the response packet
@par Example
@code
	no example code yet
@endcode
*/
String ELClientResponse::popString() {
  String ret;
  uint16_t len = *(uint16_t*)_arg_ptr;
  uint16_t pad = (4-((len+2)&3))&3;    // pad to next multiple of 4, including length
  _arg_ptr += 2;
  _arg_num++;

  while (len--)
    ret += (char)*_arg_ptr++;

  _arg_ptr += pad;
  return ret;
}

/*! popString(String* data)
@brief Extract the pointer to a string from the response packet
@param data
	Pointer to be set to the string in the response packet
@par Example
@code
	no example code yet
@endcode
*/
void ELClientResponse::popString(String* data) {
  uint16_t len = *(uint16_t*)_arg_ptr;
  uint16_t pad = (4-((len+2)&3))&3;    // pad to next multiple of 4, including length
  _arg_ptr += 2;
  _arg_num++;

  while (len--)
    data->concat((char)*_arg_ptr++);

  _arg_ptr += pad;
}
