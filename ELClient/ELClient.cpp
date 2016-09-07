// Copyright (c) 2016 by B. Runnels and T. von Eicken

#include "ELClient.h"

#define SLIP_END  0300        // indicates end of packet
#define SLIP_ESC  0333        // indicates byte stuffing
#define SLIP_ESC_END  0334    // ESC ESC_END means END data byte
#define SLIP_ESC_ESC  0335    // ESC ESC_ESC means ESC data byte

//===== Input

// Process a received SLIP message
ELClientPacket* ELClient::protoCompletedCb(void) {
  // the packet starts with a ELClientPacket
  ELClientPacket* packet = (ELClientPacket*)_proto.buf;
  if (_debugEn) {
    _debug->print("ELC: got ");
    _debug->print(_proto.dataLen);
    _debug->print(" @");
    _debug->print((uint32_t)_proto.buf, 16);
    _debug->print(": ");
    _debug->print(packet->cmd, 16);
    _debug->print(" ");
    _debug->print(packet->value, 16);
    _debug->print(" ");
    _debug->print(packet->argc, 16);
    for (uint16_t i=8; i<_proto.dataLen; i++) {
      _debug->print(" ");
      _debug->print(*(uint8_t*)(_proto.buf+i), 16);
    }
    _debug->println();
  }

  // verify CRC
  uint16_t crc = crc16Data(_proto.buf, _proto.dataLen-2, 0);
  uint16_t resp_crc = *(uint16_t*)(_proto.buf+_proto.dataLen-2);
  if (crc != resp_crc) {
    DBG("ELC: Invalid CRC");
    return NULL;
  }

  // dispatch based on command
  if (packet->cmd == CMD_RESP_V) {
    // value response
    if (_debugEn) {
        _debug->print("RESP_V: ");
        _debug->println(packet->value);
    }
    return packet;
  } else if (packet->cmd == CMD_RESP_CB) {
    FP<void, void*> *fp;
    // callback reponse
    if (_debugEn) {
        _debug->print("RESP_CB: ");
        _debug->print(packet->value);
        _debug->print(" ");
        _debug->println(packet->argc);
    }
    fp = (FP<void, void*>*)packet->value;
    if (fp->attached()) {
      ELClientResponse resp(packet);
      (*fp)(&resp);
    }
    return NULL;
  } else {
    // command (NOT IMPLEMENTED)
    if (_debugEn) _debug->println("CMD??");
    return NULL;
  }
}

// Read all characters available on the serial input and process any messages that arrive, but
// stop if a non-callback response comes in
ELClientPacket *ELClient::Process() {
  int value;
  while (_serial->available()) {
    value = _serial->read();
    if (value == SLIP_ESC) {
      _proto.isEsc = 1;
    } else if (value == SLIP_END) {
      ELClientPacket *packet = _proto.dataLen >= 8 ? protoCompletedCb() : 0;
      _proto.dataLen = 0;
      _proto.isEsc = 0;
      if (packet != NULL) return packet;
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
  return NULL;
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
void ELClient::write(void* data, uint16_t len) {
  uint8_t *d = (uint8_t*)data;
  while (len--)
    write(*d++);
}

// Start a request. cmd=command, value=address of callback pointer or first arg,
// argc=additional argument count
void ELClient::Request(uint16_t cmd, uint32_t value, uint16_t argc) {
  crc = 0;
  _serial->write(SLIP_END);
  write(&cmd, 2);
  crc = crc16Data((unsigned const char*)&cmd, 2, crc);

  write(&argc, 2);
  crc = crc16Data((unsigned const char*)&argc, 2, crc);

  write(&value, 4);
  crc = crc16Data((unsigned const char*)&value, 4, crc);
}

// Append a block of data as an argument to the request
void ELClient::Request(const void* data, uint16_t len) {
  uint8_t *d = (uint8_t*)data;

  // write the length
  write(&len, 2);
  crc = crc16Data((unsigned const char*)&len, 2, crc);

  // output the data
  for (uint16_t l=len; l>0; l--) {
    write(*d);
    crc = crc16Add(*d, crc);
    d++;
  }

  // output padding
  uint16_t pad = (4-(len&3))&3;
  uint8_t temp = 0;
  while (pad--) {
    write(temp);
    crc = crc16Add(temp, crc);
  }
}

// Append a block of data located in flash as an argument to the request
void ELClient::Request(const __FlashStringHelper* data, uint16_t len) {
  // write the length
  write(&len, 2);
  crc = crc16Data((unsigned const char*)&len, 2, crc);

  // output the data
  PGM_P p = reinterpret_cast<PGM_P>(data);
  for (uint16_t l=len; l>0; l--) {
    uint8_t c = pgm_read_byte(p++);
    write(c);
    crc = crc16Add(c, crc);
  }

  // output padding
  uint16_t pad = (4-(len&3))&3;
  uint8_t temp = 0;
  while (pad--) {
    write(temp);
    crc = crc16Add(temp, crc);
  }
}

// Append the final CRC to the request and finish the request
void ELClient::Request(void) {
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
  if (_debugEn) _debug->println(info);
}

//===== Responses

// Wait for a response for a given timeout
ELClientPacket *ELClient::WaitReturn(uint32_t timeout) {
  uint32_t wait = millis();
  while (millis() - wait < timeout) {
    ELClientPacket *packet = Process();
    if (packet != NULL) return packet;
  }
  return NULL;
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
  // send sync request
  Request(CMD_SYNC, (uint32_t)&wifiCb, 0);
  Request();
  // empty the response queue hoping to find the wifiCb address
  ELClientPacket *packet;
  while ((packet = WaitReturn(timeout)) != NULL) {
    if (packet->value == (uint32_t)&wifiCb) { 
        if (_debugEn) _debug->println("SYNC!");
        return true;
    }
    if (_debugEn) {
        _debug->print("BAD: ");
        _debug->println(packet->value);
    }
  }
  // doesn't look like we got a real response
  return false;
}

void ELClient::GetWifiStatus(void) {
  Request(CMD_WIFI_STATUS, 0, 0);
  Request();
}
