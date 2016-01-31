// Copyright (c) 2016 by B. Runnels and T. von Eicken

#include "ELClientRest.h"

typedef enum {
  HEADER_GENERIC = 0,
  HEADER_CONTENT_TYPE,
  HEADER_USER_AGENT
} HEADER_TYPE;

ELClientRest::ELClientRest(ELClient *e)
{
  _elc = e;
  remote_instance = -1;
}

void ELClientRest::restCallback(void *res)
{
  if (!res) return;

  ELClientResponse *resp = (ELClientResponse *)res;

  resp->popArg(&_status, sizeof(_status));
  _elc->_debug->print("REST code ");
  _elc->_debug->println(_status);

  _len = resp->popArgPtr(&_data);
}

int ELClientRest::begin(const char* host, uint16_t port, boolean security)
{
  uint8_t sec = !!security;
  restCb.attach(this, &ELClientRest::restCallback);

  _elc->Request(CMD_REST_SETUP, (uint32_t)&restCb, 3);
  _elc->Request(host, strlen(host));
  _elc->Request(&port, 2);
  _elc->Request(&sec, 1);
  _elc->Request();

  ELClientPacket *pkt = _elc->WaitReturn();
  if (pkt && (int32_t)pkt->value >= 0) {
    remote_instance = pkt->value;
    return 0;
  }
  return (int)pkt->value;
}

void ELClientRest::request(const char* path, const char* method, const char* data, int len)
{
  _status = 0;
  if (remote_instance < 0) return;
  if (data != 0 && len > 0) _elc->Request(CMD_REST_REQUEST, remote_instance, 3);
  else                      _elc->Request(CMD_REST_REQUEST, remote_instance, 2);
  _elc->Request(method, strlen(method));
  _elc->Request(path, strlen(path));
  if (data != NULL && len > 0) {
    _elc->Request(data, len);
  }

  _elc->Request();
}

void ELClientRest::request(const char* path, const char* method, const char* data)
{
  request(path, method, data, strlen(data));
}

void ELClientRest::get(const char* path, const char* data) { request(path, "GET", data); }
void ELClientRest::post(const char* path, const char* data) { request(path, "POST", data); }
void ELClientRest::put(const char* path, const char* data) { request(path, "PUT", data); }
void ELClientRest::del(const char* path) { request(path, "DELETE", 0); }

void ELClientRest::setHeader(const char* value)
{
  uint8_t header_index = HEADER_GENERIC;
  _elc->Request(CMD_REST_SETHEADER, remote_instance, 2);
  _elc->Request(&header_index, 1);
  _elc->Request(value, strlen(value));
  _elc->Request();
}

void ELClientRest::setContentType(const char* value)
{
  uint8_t header_index = HEADER_CONTENT_TYPE;
  _elc->Request(CMD_REST_SETHEADER, remote_instance, 2);
  _elc->Request(&header_index, 1);
  _elc->Request(value, strlen(value));
  _elc->Request();
}

void ELClientRest::setUserAgent(const char* value)
{
  uint8_t header_index = HEADER_USER_AGENT;
  _elc->Request(CMD_REST_SETHEADER, remote_instance, 2);
  _elc->Request(&header_index, 1);
  _elc->Request(value, strlen(value));
  _elc->Request();
}

uint16_t ELClientRest::getResponse(char* data, uint16_t maxLen)
{
  if (_status == 0) return 0;
  memcpy(data, _data, _len>maxLen?maxLen:_len);
  int16_t s = _status;
  _status = 0;
  return s;
}

uint16_t ELClientRest::waitResponse(char* data, uint16_t maxLen, uint32_t timeout)
{
  uint32_t wait = millis();
  while (_status == 0 && (millis() - wait < timeout)) {
    _elc->Process();
  }
  return getResponse(data, maxLen);
}
