#ifndef _EL_CLIENT_MQTT_H_
#define _EL_CLIENT_MQTT_H_

#include <stdint.h>
#include <Arduino.h>
#include "FP.h"
#include "ELClient.h"

class ELClientMqtt {
  public:
    ELClientMqtt(ELClient* elc);
    boolean init();
    boolean lwt(const char* topic, const char* message, uint8_t qos, uint8_t retain);
    boolean lwt(const __FlashStringHelper* topic, const __FlashStringHelper* message, uint8_t qos, uint8_t retain);
    boolean lwt(const char* topic, const char* message);
    boolean lwt(const __FlashStringHelper* topic, const __FlashStringHelper* message);
    void subscribe(const char* topic, uint8_t qos);
    void subscribe(const __FlashStringHelper* topic, uint8_t qos);
    void subscribe(const char* topic);
    void subscribe(const __FlashStringHelper* topic);
    void publish(const char* topic, const uint8_t* data, const uint16_t len, uint8_t qos, uint8_t retain);
    void publish(const __FlashStringHelper* topic, const __FlashStringHelper* data, const uint16_t len, uint8_t qos, uint8_t retain);
    void publish(const char* topic, const __FlashStringHelper* data, const uint16_t len, uint8_t qos, uint8_t retain);
    void publish(const __FlashStringHelper* topic, const uint8_t* data, const uint16_t len, uint8_t qos, uint8_t retain);
    void publish(const char* topic, const char* data, uint8_t qos, uint8_t retain);
    void publish(const char* topic, const char* data);
    void publish(const __FlashStringHelper* topic, const __FlashStringHelper* data);
    void publish(const char* topic, const __FlashStringHelper* data);
    void publish(const __FlashStringHelper* topic, const char* data);

    FP<void, void*> connectedCb;
    FP<void, void*> disconnectedCb;
    FP<void, void*> publishedCb;
    FP<void, void*> dataCb;

  private:
    uint32_t remote_instance;
    ELClient* _elc;
};

#endif // _EL_CLIENT_MQTT_H_
