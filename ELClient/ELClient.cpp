#include "ELClient.h"



void ELClient::protoCompletedCb(void) {
  ELClientPacket* packet = (ELClientPacket*)_proto.buf;
  uint16_t crc = 0, argc, len, resp_crc;
  uint8_t* data_ptr;
  argc = packet->argc;
  data_ptr = (uint8_t*)&packet->args;
  crc = crc16Data((unsigned const char*)&packet->cmd, 12, crc);

  while (argc--) {
    len = *((uint16_t*)data_ptr);
    crc = crc16Data((unsigned const char*)data_ptr, 2, crc);
    data_ptr += 2;
    while (len--) {
      crc = crc16Add(*data_ptr, crc);
      data_ptr++;
    }
  }

  resp_crc = *(uint16_t*)data_ptr;

  if (crc != resp_crc) {
    DBG((const char*)F("ARD: Invalid CRC"));
    return;
  }

  FP<void, void*> *fp;
  if (packet->callback != 0){
    fp = (FP<void, void*>*)packet->callback;

    return_cmd = packet->cmd;
    return_value = packet->_return;

    if (fp->attached())
      (*fp)((void*)packet);
  }
  else {
    if (packet->argc == 0) {
      is_return = true;
      return_cmd = packet->cmd;
      return_value = packet->_return;
    }
  }
}

void ELClient::write(uint8_t data) {
  switch (data) {
  case SLIP_END:
    _serial->write(SLIP_ESC);
    _serial->write(SLIP_ESC_END);
    break;
  case SLIP_ESC:
    _serial->write(SLIP_ESC);
    _serial->write(SLIP_ESC_ESC);
    break;
  default:
    _serial->write(data);
  }
}

void ELClient::write(uint8_t* data, uint16_t len) {
  while (len--)
    write(*data++);
}

uint16_t ELClient::Request(uint16_t cmd, uint32_t callback, uint32_t _return, uint16_t argc) {
  uint16_t crc = 0;
  _serial->write(SLIP_END);
  write((uint8_t*)&cmd, 2);
  crc = crc16Data((unsigned const char*)&cmd, 2, crc);

  write((uint8_t*)&callback, 4);
  crc = crc16Data((unsigned const char*)&callback, 4, crc);

  write((uint8_t*)&_return, 4);
  crc = crc16Data((unsigned const char*)&_return, 4, crc);

  write((uint8_t*)&argc, 2);
  crc = crc16Data((unsigned const char*)&argc, 2, crc);
  return crc;
}

uint16_t ELClient::Request(uint16_t crc_in, uint8_t* data, uint16_t len) {
  uint8_t temp = 0;
  uint16_t pad_len = len;
  while (pad_len % 4 != 0)
    pad_len++;
  write((uint8_t*)&pad_len, 2);
  crc_in = crc16Data((unsigned const char*)&pad_len, 2, crc_in);

  while (len--) {
    write(*data);
    crc_in = crc16Add(*data, crc_in);
    data++;
    if (pad_len > 0) pad_len--;
  }

  while (pad_len--) {
    write(temp);
    crc_in = crc16Add(*&temp, crc_in);
  }
  return crc_in;
}

uint16_t ELClient::Request(uint16_t crc_in, const __FlashStringHelper* data, uint16_t len) {
  uint8_t temp = 0;
  uint16_t pad_len = len;
  while (pad_len % 4 != 0)
    pad_len++;
  write((uint8_t*)&pad_len, 2);
  crc_in = crc16Data((unsigned const char*)&pad_len, 2, crc_in);

  PGM_P p = reinterpret_cast<PGM_P>(data);
  while (len--) {
    uint8_t c = pgm_read_byte(p++);
    write(c);
    crc_in = crc16Add(c, crc_in);
    if (pad_len > 0) pad_len--;
  }

  while (pad_len--) {
    write(temp);
    crc_in = crc16Add(*&temp, crc_in);
  }
  return crc_in;
}

uint16_t ELClient::Request(uint16_t crc) {
  write((uint8_t*)&crc, 2);
  return _serial->write(SLIP_END);
}

void ELClient::init() {
  _proto.buf = _protoBuf;
  _proto.bufSize = sizeof(_protoBuf);
  _proto.dataLen = 0;
  _proto.isEsc = 0;
}

ELClient::ELClient(Stream* serial) :
_serial(serial) {
  _debugEn = false;
  init();
}

ELClient::ELClient(Stream* serial, Stream* debug) :
_serial(serial), _debug(debug) {
  _debugEn = true;
  init();
}

void ELClient::DBG(const char* info) {
  if (_debugEn)
    _debug->println(info);
}

boolean ELClient::WaitReturn(uint32_t timeout) {
  is_return = false;
  return_value = 0;
  return_cmd = 0;
  uint32_t wait = millis();
  while (is_return == false && (millis() - wait < timeout)) {
    Process();
  }
  return is_return;
}

boolean ELClient::WaitReturn() {
  return WaitReturn(ESP_TIMEOUT);
}

void ELClient::Process() {
  char value;
  while (_serial->available()) {
    value = _serial->read();
    if (value == SLIP_ESC) {
      _proto.isEsc = 1;
    } else if (value == SLIP_END) {
      if (_proto.dataLen > 0) protoCompletedCb();
      _proto.dataLen = 0;
      _proto.isEsc = 0;
    } else {
      if (_proto.isEsc) {
        if (value == SLIP_ESC_END) value = SLIP_END;
        if (value == SLIP_ESC_ESC) value = SLIP_ESC;
        _proto.isEsc = 0;
      }
      if (_proto.dataLen < _proto.bufSize) {
        _proto.buf[_proto.dataLen++] = value;
      }
    }
  }
}

uint8_t ELClient::crc16Add(unsigned char b, uint8_t acc)
{
  acc ^= b;
  acc = (acc >> 8) | (acc << 8);
  acc ^= (acc & 0xff00) << 4;
  acc ^= (acc >> 8) >> 4;
  acc ^= (acc & 0xff00) >> 5;
  return acc;
}

uint8_t ELClient::crc16Data(const unsigned char *data, uint16_t len, uint8_t acc)
{
  uint16_t i;

  for (i = 0; i < len; ++i) {
    acc = crc16Add(*data, acc);
    ++data;
  }
  return acc;
}
