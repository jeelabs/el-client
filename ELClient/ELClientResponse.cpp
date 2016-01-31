#include "ELClientResponse.h"

ELClientResponse::ELClientResponse(ELClientPacket* packet) {
  _cmd = packet;
  _arg_ptr = _cmd->args;
  _arg_num = 0;
}

ELClientResponse::ELClientResponse(void* packet) {
  _cmd = (ELClientPacket *)packet;
  _arg_ptr = _cmd->args;
  _arg_num = 0;
}

int16_t ELClientResponse::popArgPtr(void **data) {
  if (_arg_num >= _cmd->argc) return -1;

  uint16_t length = *(uint16_t*)_arg_ptr;
  _arg_ptr += 2;
  _arg_num++;

  *data = _arg_ptr;
  _arg_ptr += length;
  _arg_ptr += (4-(length&3))&3; // account for padding
  return length;
}

int16_t ELClientResponse::popArg(void* d, uint16_t maxLen) {
  if (_arg_num >= _cmd->argc) return -1;

  uint16_t length = *(uint16_t*)_arg_ptr;
  _arg_ptr += 2;
  _arg_num++;

  uint8_t *data = (uint8_t *)d;
  uint16_t len = length > maxLen ? maxLen : length;
  uint16_t l = len;
  uint8_t *p = _arg_ptr;
  while (l--)
    *data++ = *p++;

  _arg_ptr += length;
  _arg_ptr += (4-(length&3))&3; // account for padding

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
  _arg_ptr += (4-(len&3))&3; // account for padding
}

String ELClientResponse::popString() {
  String ret;
  uint16_t len = *(uint16_t*)_arg_ptr;
  _arg_ptr += 2;
  while (len--)
    ret += (char)*_arg_ptr++;
  _arg_num++;
  _arg_ptr += (4-(len&3))&3; // account for padding
  return ret;
}

void ELClientResponse::popString(String* data) {
  uint16_t len = *(uint16_t*)_arg_ptr;
  _arg_ptr += 2;
  while (len--)
    data->concat((char)*_arg_ptr++);
  _arg_ptr += (4-(len&3))&3; // account for padding
  _arg_num++;
}
