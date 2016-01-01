#include "ELClientMqtt.h"

ELClientMqtt::ELClientMqtt(ELClient* elc) :_elc(elc) {}

boolean ELClientMqtt::init() {
  uint16_t crc;
  uint32_t cb_data;

  crc = _elc->Request(CMD_MQTT_INIT, 0, 1, 4);

  cb_data = (uint32_t)&connectedCb;
  crc = _elc->Request(crc, (uint8_t*)&cb_data, 4);

  cb_data = (uint32_t)&disconnectedCb;
  crc = _elc->Request(crc, (uint8_t*)&cb_data, 4);

  cb_data = (uint32_t)&publishedCb;
  crc = _elc->Request(crc, (uint8_t*)&cb_data, 4);

  cb_data = (uint32_t)&dataCb;
  crc = _elc->Request(crc, (uint8_t*)&cb_data, 4);

  _elc->Request(crc);

  if (_elc->WaitReturn() == false || _elc->return_cmd == 0 || _elc->return_value == 0)
    return false;
  remote_instance = _elc->return_value;
  return true;
}

boolean ELClientMqtt::lwt(const char* topic, const char* message, uint8_t qos, uint8_t retain) {
  uint16_t crc;

  crc = _elc->Request(CMD_MQTT_LWT, 0, 1, 5);
  crc = _elc->Request(crc, (uint8_t*)&remote_instance, 4);
  crc = _elc->Request(crc, (uint8_t*)topic, strlen(topic));
  crc = _elc->Request(crc, (uint8_t*)message, strlen(message));
  crc = _elc->Request(crc, (uint8_t*)&qos, 1);
  crc = _elc->Request(crc, (uint8_t*)&retain, 1);
  _elc->Request(crc);
  if (_elc->WaitReturn() && _elc->return_value)
    return true;
  return false;
}

boolean ELClientMqtt::lwt(const __FlashStringHelper* topic, const __FlashStringHelper* message, uint8_t qos, uint8_t retain) {
  uint16_t crc;

  crc = _elc->Request(CMD_MQTT_LWT, 0, 1, 5);
  crc = _elc->Request(crc, (uint8_t*)&remote_instance, 4);
  crc = _elc->Request(crc, topic, strlen_P((const char*)topic));
  crc = _elc->Request(crc, message, strlen_P((const char*)message));
  crc = _elc->Request(crc, (uint8_t*)&qos, 1);
  crc = _elc->Request(crc, (uint8_t*)&retain, 1);
  _elc->Request(crc);
  if (_elc->WaitReturn() && _elc->return_value)
    return true;
  return false;
}

boolean ELClientMqtt::lwt(const char* topic, const char* message) {
  return lwt(topic, message, 0, 0);
}

boolean ELClientMqtt::lwt(const __FlashStringHelper* topic, const __FlashStringHelper* message) {
  return lwt(topic, message, 0, 0);
}

//void MQTT::connect(const char* host, uint32_t port, boolean security) {
//  uint16_t crc;
//  crc = _elc->Request(CMD_MQTT_CONNECT, 0, 0, 4);
//  crc = _elc->Request(crc, (uint8_t*)&remote_instance, 4);
//  crc = _elc->Request(crc, (uint8_t*)host, strlen(host));
//  crc = _elc->Request(crc, (uint8_t*)&port, 4);
//  crc = _elc->Request(crc, (uint8_t*)&security, 1);
//  _elc->Request(crc);
//}
//
//void MQTT::connect(const __FlashStringHelper* host, uint32_t port, boolean security) {
//  uint16_t crc;
//  crc = _elc->Request(CMD_MQTT_CONNECT, 0, 0, 4);
//  crc = _elc->Request(crc, (uint8_t*)&remote_instance, 4);
//  crc = _elc->Request(crc, host, strlen_P((const char*)host));
//  crc = _elc->Request(crc, (uint8_t*)&port, 4);
//  crc = _elc->Request(crc, (uint8_t*)&security, 1);
//  _elc->Request(crc);
//}
//
//void MQTT::connect(const char* host, uint32_t port) {
//  connect(host, port, false);
//}
//
//void MQTT::connect(const __FlashStringHelper* host, uint32_t port) {
//  connect(host, port, false);
//}

//void MQTT::disconnect() {
//  uint16_t crc;
//  crc = _elc->Request(CMD_MQTT_DISCONNECT, 0, 0, 1);
//  crc = _elc->Request(crc, (uint8_t*)&remote_instance, 4);
//  _elc->Request(crc);
//}

void ELClientMqtt::subscribe(const char* topic, uint8_t qos) {
  uint16_t crc;
  crc = _elc->Request(CMD_MQTT_SUBSCRIBE, 0, 0, 3);
  crc = _elc->Request(crc, (uint8_t*)&remote_instance, 4);
  crc = _elc->Request(crc, (uint8_t*)topic, strlen(topic));
  crc = _elc->Request(crc, (uint8_t*)&qos, 1);
  _elc->Request(crc);
}

void ELClientMqtt::subscribe(const __FlashStringHelper* topic, uint8_t qos) {
  uint16_t crc;
  crc = _elc->Request(CMD_MQTT_SUBSCRIBE, 0, 0, 3);
  crc = _elc->Request(crc, (uint8_t*)&remote_instance, 4);
  crc = _elc->Request(crc, topic, strlen_P((const char*)topic));
  crc = _elc->Request(crc, (uint8_t*)&qos, 1);
  _elc->Request(crc);
}

void ELClientMqtt::subscribe(const char* topic) {
  subscribe(topic, 0);
}

void ELClientMqtt::subscribe(const __FlashStringHelper* topic) {
  subscribe(topic, 0);
}

void ELClientMqtt::publish(const char* topic, const uint8_t* data, const uint16_t len, uint8_t qos, uint8_t retain) {
  uint16_t crc;
  crc = _elc->Request(CMD_MQTT_PUBLISH, 0, 0, 6);
  crc = _elc->Request(crc, (uint8_t*)&remote_instance, 4);
  crc = _elc->Request(crc, (uint8_t*)topic, strlen(topic));
  crc = _elc->Request(crc, (uint8_t*)data, len);
  crc = _elc->Request(crc, (uint8_t*)&len, 2);
  crc = _elc->Request(crc, (uint8_t*)&qos, 1);
  crc = _elc->Request(crc, (uint8_t*)&retain, 1);
  _elc->Request(crc);
}

void ELClientMqtt::publish(const __FlashStringHelper* topic, const __FlashStringHelper* data, const uint16_t len, uint8_t qos, uint8_t retain) {
  uint16_t crc;
  crc = _elc->Request(CMD_MQTT_PUBLISH, 0, 0, 6);
  crc = _elc->Request(crc, (uint8_t*)&remote_instance, 4);
  crc = _elc->Request(crc, topic, strlen_P((const char*)topic));
  crc = _elc->Request(crc, data, len);
  crc = _elc->Request(crc, (uint8_t*)&len, 2);
  crc = _elc->Request(crc, (uint8_t*)&qos, 1);
  crc = _elc->Request(crc, (uint8_t*)&retain, 1);
  _elc->Request(crc);
}

void ELClientMqtt::publish(const char* topic, const __FlashStringHelper* data, const uint16_t len, uint8_t qos, uint8_t retain) {
  uint16_t crc;
  crc = _elc->Request(CMD_MQTT_PUBLISH, 0, 0, 6);
  crc = _elc->Request(crc, (uint8_t*)&remote_instance, 4);
  crc = _elc->Request(crc, (uint8_t*)topic, strlen(topic));
  crc = _elc->Request(crc, data, len);
  crc = _elc->Request(crc, (uint8_t*)&len, 2);
  crc = _elc->Request(crc, (uint8_t*)&qos, 1);
  crc = _elc->Request(crc, (uint8_t*)&retain, 1);
  _elc->Request(crc);
}

void ELClientMqtt::publish(const __FlashStringHelper* topic, const uint8_t* data, const uint16_t len, uint8_t qos, uint8_t retain) {
  uint16_t crc;
  crc = _elc->Request(CMD_MQTT_PUBLISH, 0, 0, 6);
  crc = _elc->Request(crc, (uint8_t*)&remote_instance, 4);
  crc = _elc->Request(crc, topic, strlen_P((const char*)topic));
  crc = _elc->Request(crc, (uint8_t*)data, len);
  crc = _elc->Request(crc, (uint8_t*)&len, 2);
  crc = _elc->Request(crc, (uint8_t*)&qos, 1);
  crc = _elc->Request(crc, (uint8_t*)&retain, 1);
  _elc->Request(crc);
}

void ELClientMqtt::publish(const char* topic, const char* data, uint8_t qos, uint8_t retain) {
  publish(topic, (const uint8_t*)data, strlen(data), qos, retain);
}

void ELClientMqtt::publish(const char* topic, const char* data) {
  publish(topic, (const uint8_t*)data, strlen(data), 0, 0);
}

void ELClientMqtt::publish(const __FlashStringHelper* topic, const __FlashStringHelper* data) {
  publish(topic, data, strlen_P((const char*)data), 0, 0);
}

void ELClientMqtt::publish(const char* topic, const __FlashStringHelper* data) {
  publish(topic, data, strlen_P((const char*)data), 0, 0);
}

void ELClientMqtt::publish(const __FlashStringHelper* topic, const char* data) {
  publish(topic, (const uint8_t*)data, strlen(data), 0, 0);
}

