#include "ELClientRest.h"
ELClientRest::ELClientRest(ELClient *e)
{
  _elc = e;
  remote_instance = 0;
  timeout = DEFAULT_REST_TIMEOUT;
}
void ELClientRest::restCallback(void *resp)
{
  response = true;
  res = resp;
}

boolean ELClientRest::begin(const char* host, uint16_t port, boolean security)
{
  uint8_t sec = 0;
  if (security)
    sec = 1;
  restCb.attach(this, &ELClientRest::restCallback);

  uint16_t crc = _elc->Request(CMD_REST_SETUP, (uint32_t)&restCb, 1, 3);
  crc = _elc->Request(crc, (uint8_t*)host, strlen(host));
  crc = _elc->Request(crc, (uint8_t*)&port, 2);
  crc = _elc->Request(crc, (uint8_t*)&sec, 1);
  _elc->Request(crc);

  if (_elc->WaitReturn(timeout) && _elc->return_value != 0){
    remote_instance = _elc->return_value;
    return true;
  }
  return false;
}

boolean ELClientRest::begin(const char* host)
{
  return begin(host, 80, false);
}
void ELClientRest::request(const char* path, const char* method, const char* data, int len)
{
  if (remote_instance == 0)
    return;
  uint16_t crc;
  if (len > 0)
    crc = _elc->Request(CMD_REST_REQUEST, 0, 0, 5);
  else
    crc = _elc->Request(CMD_REST_REQUEST, 0, 0, 3);
  crc = _elc->Request(crc, (uint8_t*)&remote_instance, 4);
  crc = _elc->Request(crc, (uint8_t*)method, strlen(method));
  crc = _elc->Request(crc, (uint8_t*)path, strlen(path));
  if (len > 0){
    crc = _elc->Request(crc, (uint8_t*)&len, 2);
    crc = _elc->Request(crc, (uint8_t*)data, len);
  }

  _elc->Request(crc);
}
void ELClientRest::request(const char* path, const char* method, const char* data)
{
  request(path, method, data, strlen(data));
}
void ELClientRest::get(const char* path, const char* data)
{
  request(path, "GET", data);
}
void ELClientRest::get(const char* path)
{
  request(path, "GET", 0, 0);
}
void ELClientRest::post(const char* path, const char* data)
{
  request(path, "POST", data);
}
void ELClientRest::put(const char* path, const char* data)
{
  request(path, "PUT", data);
}
void ELClientRest::del(const char* path, const char* data)
{
  request(path, "DELETE", data);
}

void ELClientRest::setHeader(const char* value)
{
  uint8_t header_index = HEADER_GENERIC;
  uint16_t crc = _elc->Request(CMD_REST_SETHEADER, 0, 0, 3);
  crc = _elc->Request(crc, (uint8_t*)&remote_instance, 4);
  crc = _elc->Request(crc, (uint8_t*)&header_index, 1);
  crc = _elc->Request(crc, (uint8_t*)value, strlen(value));
  _elc->Request(crc);
}
void ELClientRest::setContentType(const char* value)
{
  uint8_t header_index = HEADER_CONTENT_TYPE;
  uint16_t crc = _elc->Request(CMD_REST_SETHEADER, 0, 0, 3);
  crc = _elc->Request(crc, (uint8_t*)&remote_instance, 4);
  crc = _elc->Request(crc, (uint8_t*)&header_index, 1);
  crc = _elc->Request(crc, (uint8_t*)value, strlen(value));
  _elc->Request(crc);
}
void ELClientRest::setUserAgent(const char* value)
{
  uint8_t header_index = HEADER_USER_AGENT;
  uint16_t crc = _elc->Request(CMD_REST_SETHEADER, 0, 0, 3);
  crc = _elc->Request(crc, (uint8_t*)&remote_instance, 4);
  crc = _elc->Request(crc, (uint8_t*)&header_index, 1);
  crc = _elc->Request(crc, (uint8_t*)value, strlen(value));
  _elc->Request(crc);
}
void ELClientRest::setTimeout(uint32_t ms)
{
  timeout = ms;
}

uint16_t ELClientRest::getResponse(char* data, uint16_t maxLen)
{
  response = false;
  uint32_t wait = millis();
  while (response == false && (millis() - wait < timeout)) {
    _elc->Run();
  }
  if (response){
    ELClientResponse resp(res);

    uint32_t len = resp.popArgs((uint8_t*)data, maxLen);
    data[len < maxLen ? len : maxLen - 1] = 0;
    return _elc->return_value;
  }


  return 0;
}
