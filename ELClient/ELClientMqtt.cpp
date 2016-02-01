// Copyright (c) 2016 by B. Runnels and T. von Eicken

#include <Arduino.h>
#include "ELClientMqtt.h"

// constructor
ELClientMqtt::ELClientMqtt(ELClient* elc) :_elc(elc) {}

// setup -- push callbacks to esp-link
void ELClientMqtt::setup(void) {
  Serial.print("ConnectedCB is 0x"); Serial.println((uint32_t)&connectedCb, 16);
  _elc->Request(CMD_MQTT_SETUP, 0, 4);
  uint32_t cb = (uint32_t)&connectedCb;
  _elc->Request(&cb, 4);
  cb = (uint32_t)&disconnectedCb;
  _elc->Request(&cb, 4);
  cb = (uint32_t)&publishedCb;
  _elc->Request(&cb, 4);
  cb = (uint32_t)&dataCb;
  _elc->Request(&cb, 4);
  _elc->Request();
}

// LWT

void ELClientMqtt::lwt(const char* topic, const char* message, uint8_t qos, uint8_t retain) {
  _elc->Request(CMD_MQTT_LWT, 0, 4);
  _elc->Request(topic, strlen(topic));
  _elc->Request(message, strlen(message));
  _elc->Request(&qos, 1);
  _elc->Request(&retain, 1);
  _elc->Request();
}

void ELClientMqtt::lwt(const __FlashStringHelper* topic, const __FlashStringHelper* message,
    uint8_t qos, uint8_t retain)
{
  _elc->Request(CMD_MQTT_LWT, 0, 4);
  _elc->Request(topic, strlen_P((const char*)topic));
  _elc->Request(message, strlen_P((const char*)message));
  _elc->Request(&qos, 1);
  _elc->Request(&retain, 1);
  _elc->Request();
}

// SUBSCRIBE

void ELClientMqtt::subscribe(const char* topic, uint8_t qos) {
  _elc->Request(CMD_MQTT_SUBSCRIBE, 0, 2);
  _elc->Request(topic, strlen(topic));
  _elc->Request(&qos, 1);
  _elc->Request();
}

void ELClientMqtt::subscribe(const __FlashStringHelper* topic, uint8_t qos) {
  _elc->Request(CMD_MQTT_SUBSCRIBE, 0, 2);
  _elc->Request(topic, strlen_P((const char*)topic));
  _elc->Request(&qos, 1);
  _elc->Request();
}

// PUBLISH

void ELClientMqtt::publish(const char* topic, const uint8_t* data, const uint16_t len,
    uint8_t qos, uint8_t retain)
{
  _elc->Request(CMD_MQTT_PUBLISH, 0, 5);
  _elc->Request(topic, strlen(topic));
  _elc->Request(data, len);
  _elc->Request(&len, 2);
  _elc->Request(&qos, 1);
  _elc->Request(&retain, 1);
  _elc->Request();
}

void ELClientMqtt::publish(const char* topic, const char* data, uint8_t qos, uint8_t retain)
{
  publish(topic, (uint8_t*)data, strlen(data), qos, retain);
}

void ELClientMqtt::publish(const __FlashStringHelper* topic, const __FlashStringHelper* data,
    const uint16_t len, uint8_t qos, uint8_t retain)
{
  _elc->Request(CMD_MQTT_PUBLISH, 0, 5);
  _elc->Request(topic, strlen_P((const char*)topic));
  _elc->Request(data, len);
  _elc->Request(&len, 2);
  _elc->Request(&qos, 1);
  _elc->Request(&retain, 1);
  _elc->Request();
}

void ELClientMqtt::publish(const char* topic, const __FlashStringHelper* data,
    const uint16_t len, uint8_t qos, uint8_t retain)
{
  _elc->Request(CMD_MQTT_PUBLISH, 0, 5);
  _elc->Request(topic, strlen(topic));
  _elc->Request(data, len);
  _elc->Request(&len, 2);
  _elc->Request(&qos, 1);
  _elc->Request(&retain, 1);
  _elc->Request();
}

void ELClientMqtt::publish(const __FlashStringHelper* topic, const uint8_t* data,
    const uint16_t len, uint8_t qos, uint8_t retain)
{
  _elc->Request(CMD_MQTT_PUBLISH, 0, 5);
  _elc->Request(topic, strlen_P((const char*)topic));
  _elc->Request(data, len);
  _elc->Request(&len, 2);
  _elc->Request(&qos, 1);
  _elc->Request(&retain, 1);
  _elc->Request();
}
