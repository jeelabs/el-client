/*! \file ELClientMqtt.cpp
    \brief Constructor and functions for ELClientMqtt
    \author B. Runnels
    \author T. von Eicken
    \date 2016
*/
// Copyright (c) 2016 by B. Runnels and T. von Eicken

#include <Arduino.h>
#include "ELClientMqtt.h"

// constructor
/*! ELClientMqtt(ELClient* elc)
@brief Constructor for ELClientMqtt
@par Example
@code
  ELClientMqtt(ELClient* elc);
@endcode
*/
ELClientMqtt::ELClientMqtt(ELClient* elc) :_elc(elc) {}

/*! setup(void)
@brief Setup mqtt
@details Send callback functions for MQTT events to the ESP
@par Example
@code
  mqtt.connectedCb.attach(mqttConnected);
  mqtt.disconnectedCb.attach(mqttDisconnected);
  mqtt.publishedCb.attach(mqttPublished);
  mqtt.dataCb.attach(mqttData);
  mqtt.setup();
@endcode
*/
void ELClientMqtt::setup(void) {
  Serial.print(F("ConnectedCB is 0x")); Serial.println((uint32_t)&connectedCb, 16);
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

/*! lwt(const char* topic, const char* message, uint8_t qos, uint8_t retain)
@brief Set MQTT last will.
@details Sends the "last will" to the ESP.
@param topic
  Last will topic name
@param message
  Last will message
@param qos
  (optional) Requested qos level, default 0
@param retain
  (optional) Requested retain level, default 0
@warning At the moment only qos level 0 is implemented and supported!
@par Example
@code
  Serial.println("ARDUINO: setup mqtt lwt");
  mqtt.lwt("/lwt", "offline", 0, 0); //or mqtt.lwt("/lwt", "offline");
@endcode
*/
void ELClientMqtt::lwt(const char* topic, const char* message, uint8_t qos, uint8_t retain) {
  _elc->Request(CMD_MQTT_LWT, 0, 4);
  _elc->Request(topic, strlen(topic));
  _elc->Request(message, strlen(message));
  _elc->Request(&qos, 1);
  _elc->Request(&retain, 1);
  _elc->Request();
}

/*! lwt(const __FlashStringHelper* topic, const __FlashStringHelper* message, uint8_t qos, uint8_t retain)
@brief Set MQTT last will.
@details Sends the "last will" to the ESP with the topic and message stored in program memory
@param topic
  Last will topic name
@param message
  Last will message
@param qos
  (optional) Requested qos level, default 0
@param retain
  (optional) Requested retain level, default 0
@warning At the moment only qos level 0 is implemented and supported!
@par Example
@code
  no example code yet
@endcode
*/
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

/*! subscribe(const char* topic, uint8_t qos)
@brief Subscribe to MQTT topic
@details Sends the MQTT subscription request to the ESP
@param topic
  Topic name
@param qos
  (optional) Requested qos level, default 0
@warning At the moment only qos level 0 is implemented and supported!
@par Example
@code
  mqtt.subscribe("/esp-link/1");
  mqtt.subscribe("/hello/world/#");
@endcode
*/
void ELClientMqtt::subscribe(const char* topic, uint8_t qos) {
  _elc->Request(CMD_MQTT_SUBSCRIBE, 0, 2);
  _elc->Request(topic, strlen(topic));
  _elc->Request(&qos, 1);
  _elc->Request();
}

/*! subscribe(const __FlashStringHelper* topic, uint8_t qos)
@brief Subscribe to MQTT topic
@details Sends the MQTT subscription request to the ESP with the topic and message stored in program memory
@param topic
  Topic name
@param qos
  (optional) Requested qos level, default 0
@warning At the moment only qos level 0 is implemented and supported!
@par Example
@code
  no example code yet
@endcode
*/
void ELClientMqtt::subscribe(const __FlashStringHelper* topic, uint8_t qos) {
  _elc->Request(CMD_MQTT_SUBSCRIBE, 0, 2);
  _elc->Request(topic, strlen_P((const char*)topic));
  _elc->Request(&qos, 1);
  _elc->Request();
}

// PUBLISH

/*! publish(const char* topic, const uint8_t* data, const uint16_t len, uint8_t qos, uint8_t retain)
@brief Subscribe to MQTT topic
@details Sends the MQTT subscription request to the ESP
@param topic
  Topic name
@param data
  Pointer to data buffer
@param len
  Size of data buffer
@param qos
  (optional) Requested qos level, default 0
@param retain
  (optional) Requested retain level, default 0
@warning At the moment only qos level 0 is implemented and supported!
@par Example
@code
  char buf[12];
  itoa(count++, buf, 10);
  mqtt.publish("/esp-link/1", buf);
  itoa(count+99, buf, 10);
  mqtt.publish("/hello/world/arduino", buf, 12);
@endcode
*/
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

/*! publish(const char* topic, const char* data, uint8_t qos, uint8_t retain)
@brief Subscribe to MQTT topic
@details Sends the MQTT subscription request to the ESP. Data must be null-terminated
@param topic
  Topic name
@param data
  Pointer to data buffer
@param qos
  (optional) Requested qos level, default 0
@param retain
  (optional) Requested retain level, default 0
@warning At the moment only qos level 0 is implemented and supported!
@par Example
@code
  char buf[12];
  itoa(count++, buf, 10);
  mqtt.publish("/esp-link/1", buf);
  itoa(count+99, buf, 10);
  mqtt.publish("/hello/world/arduino", buf);
@endcode
*/
void ELClientMqtt::publish(const char* topic, const char* data, uint8_t qos, uint8_t retain)
{
  publish(topic, (uint8_t*)data, strlen(data), qos, retain);
}

/*! publish(const __FlashStringHelper* topic, const __FlashStringHelper* data, const uint16_t len, uint8_t qos, uint8_t retain)
@brief Subscribe to MQTT topic
@details Sends the MQTT subscription request to the ESP with the topic and data stored in program memory
@param topic
  Topic name
@param data
  Pointer to data buffer
@param len
  Size of data buffer
@param qos
  (optional) Requested qos level, default 0
@param retain
  (optional) Requested retain level, default 0
@warning At the moment only qos level 0 is implemented and supported!
@par Example
@code
  no example code yet
@endcode
*/
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

/*! ELClientMqtt::publish(const char* topic, const __FlashStringHelper* data, const uint16_t len, uint8_t qos, uint8_t retain)
@brief Subscribe to MQTT topic
@details Sends the MQTT subscription request to the ESP with the data stored in program memory
@param topic
  Topic name
@param data
  Pointer to data buffer
@param len
  Size of data buffer
@param qos
  (optional) Requested qos level, default 0
@param retain
  (optional) Requested retain level, default 0
@warning At the moment only qos level 0 is implemented and supported!
@par Example
@code
  no example code yet
@endcode
*/
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

/*! publish(const __FlashStringHelper* topic, const uint8_t* data, const uint16_t len, uint8_t qos, uint8_t retain)
@brief Subscribe to MQTT topic
@details Sends the MQTT subscription request to the ESP with the topic stored in program memory
@param topic
  Topic name
@param data
  Pointer to data buffer
@param len
  Size of data buffer
@param qos
  (optional) Requested qos level, default 0
@param retain
  (optional) Requested retain level, default 0
@warning At the moment only qos level 0 is implemented and supported!
@par Example
@code
  no example code yet
@endcode
*/
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
