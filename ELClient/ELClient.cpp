// Copyright (c) 2016 by B. Runnels and T. von Eicken

#include "ELClient.h"

//===== Input

// Process a received SLIP message
void ELClient::protoCompletedCb(void) {
  // the packet starts with a ELClientPacket
  ELClientPacket* packet = (ELClientPacket*)_proto.buf;
  uint16_t crc = 0, argc, len, resp_crc;
  uint8_t* data_ptr;
  argc = packet->argc;
  data_ptr = (uint8_t*)&packet->args;
  // start the CRC calculation
  crc = crc16Data((unsigned const char*)&packet->cmd, 12, crc);

  // calculate the CRC over all arguments
  while (argc--) {
    len = *((uint16_t*)data_ptr);
    crc = crc16Data((unsigned const char*)data_ptr, 2, crc);
    data_ptr += 2;
    while (len--) {
      crc = crc16Add(*data_ptr, crc);
      data_ptr++;
    }
  }

  // verify the CRC
  resp_crc = *(uint16_t*)data_ptr;
  if (crc != resp_crc) {
    DBG((const char*)F("ARD: Invalid CRC"));
    return;
  }

  // see whether we have a callback response
  FP<void, void*> *fp;
  if (packet->callback != 0){
    // got a callback to deliver
    fp = (FP<void, void*>*)packet->callback;

    return_cmd = packet->cmd;
    return_value = packet->_return;

    if (fp->attached())
      (*fp)((void*)packet);
  } else {
    // no callback to deliver, just signal that the call completed
    if (packet->argc == 0) { // FIXME: what is the purpose of this???
      is_return = true;
      return_cmd = packet->cmd;
      return_value = packet->_return;
    }
  }
}

// Read all characters available on the serial input and process any messages that arrive
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

//===== Output

// Write a byte to the output stream and perform SLIP escaping
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

// Write some bytes to the output stream
void ELClient::write(uint8_t* data, uint16_t len) {
  while (len--)
    write(*data++);
}

// Start a request. cmd=command, callback=address of callback pointer,
// _return=a token that is sent with the response, argc=argument count
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

// Append a block of data as an argument to the request
uint16_t ELClient::Request(uint16_t crc_in, void* data, uint16_t len) {
  uint8_t temp = 0;
  uint8_t *d = (uint8_t*)data;

  // all arguments get padded to a multiple of 4 bytes so they stay aligned, write the amount
  // of padding we're gonna perform
  uint16_t pad_len = len;
  while (pad_len % 4 != 0)
    pad_len++;
  write((uint8_t*)&pad_len, 2);
  crc_in = crc16Data((unsigned const char*)&pad_len, 2, crc_in);

  // output the data
  while (len--) {
    write(*d);
    crc_in = crc16Add(*d, crc_in);
    d++;
    if (pad_len > 0) pad_len--;
  }

  // output the padding
  while (pad_len--) {
    write(temp);
    crc_in = crc16Add(temp, crc_in);
  }
  return crc_in;
}

// Append a block of data located in flash as an argument to the request
uint16_t ELClient::Request(uint16_t crc_in, const __FlashStringHelper* data, uint16_t len) {
  uint8_t temp = 0;

  // output the length of the padding
  uint16_t pad_len = len;
  while (pad_len % 4 != 0)
    pad_len++;
  write((uint8_t*)&pad_len, 2);
  crc_in = crc16Data((unsigned const char*)&pad_len, 2, crc_in);

  // output the data
  PGM_P p = reinterpret_cast<PGM_P>(data);
  while (len--) {
    uint8_t c = pgm_read_byte(p++);
    write(c);
    crc_in = crc16Add(c, crc_in);
    if (pad_len > 0) pad_len--;
  }

  // output the padding
  while (pad_len--) {
    write(temp);
    crc_in = crc16Add(*&temp, crc_in);
  }
  return crc_in;
}

// Append the final CRC to the request and finish the request
void ELClient::Request(uint16_t crc) {
  write((uint8_t*)&crc, 2);
  _serial->write(SLIP_END);
}

//===== Initialization

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
_debug(debug), _serial(serial) {
  _debugEn = true;
  init();
}

void ELClient::DBG(const char* info) {
  if (_debugEn)
    _debug->println(info);
}

//===== Responses

// Wait for a response for a given timeout
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

//===== CRC helper functions

uint16_t ELClient::crc16Add(unsigned char b, uint16_t acc)
{
  acc ^= b;
  acc = (acc >> 8) | (acc << 8);
  acc ^= (acc & 0xff00) << 4;
  acc ^= (acc >> 8) >> 4;
  acc ^= (acc & 0xff00) >> 5;
  return acc;
}

uint16_t ELClient::crc16Data(const unsigned char *data, uint16_t len, uint16_t acc)
{
  for (uint16_t i=0; i<len; i++)
    acc = crc16Add(*data++, acc);
  return acc;
}

//===== Basic requests built into ElClient

boolean ELClient::Sync(uint32_t timeout) {
  // generate a hopefully unique number
  uint32_t nonce = micros();
  // send sync request
  uint16_t crc = Request(CMD_SYNC, 0, nonce, 1);
  crc = Request(crc, &wifiCb, 4);
  Request(crc);
  // empty the response queue hoping to find out magic number
  while (WaitReturn(timeout))
    if (return_value == nonce) return true;
  // doesn't look like we got a real response
  return false;
}
