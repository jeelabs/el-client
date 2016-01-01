#include "ELClientResponse.h"

ELClientResponse::ELClientResponse(void* response) {
  _cmd = (ELClientPacket*)response;
  _arg_ptr = (uint8_t*)&_cmd->args;
  _arg_num = 0;
}

uint16_t ELClientResponse::getArgc() {
  return _cmd->argc;
}

uint16_t ELClientResponse::argLen() {
  return *(uint16_t*)_arg_ptr;
}

int32_t ELClientResponse::popArgs(uint8_t* data, uint16_t maxLen) {
  uint16_t length, len, incLen = 0;

  if (_arg_num >= _cmd->argc)
    return -1;

  length = *(uint16_t*)_arg_ptr;
  len = length;
  _arg_ptr += 2;

  while (length--) {
    *data++ = *_arg_ptr++;
    incLen++;
    if (incLen > maxLen) {
      _arg_num++;
      _arg_ptr += length;
      return maxLen;
    }
  }
  _arg_num++;
  return len;
}

void ELClientResponse::popChar(char* buffer) {
  uint16_t len = *(uint16_t*)_arg_ptr;
  _arg_ptr += 2;
  uint8_t i;
  for (i = 0; i < len; i++) {
    buffer[i] = (char)*_arg_ptr++;
  }
  buffer[i] = '\0';
  _arg_num++;
}

String ELClientResponse::popString() {
  String ret;
  uint16_t len = *(uint16_t*)_arg_ptr;
  _arg_ptr += 2;
  while (len--)
    ret += (char)*_arg_ptr++;
  _arg_num++;
  return ret;
}

void ELClientResponse::popString(String* data) {
  uint16_t len = *(uint16_t*)_arg_ptr;
  _arg_ptr += 2;
  while (len--)
    data->concat((char)*_arg_ptr++);
  _arg_num++;
}