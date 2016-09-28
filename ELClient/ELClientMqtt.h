/*! \file ELClientMqtt.h
    \brief Definitions for ELClientMqtt
    \author B. Runnels
    \author T. von Eicken
    \date 2016
*/
// Copyright (c) 2016 by B. Runnels and T. von Eicken

#ifndef _EL_CLIENT_MQTT_H_
#define _EL_CLIENT_MQTT_H_

#include <stdint.h>
#include "FP.h"
#include "ELClient.h"

// Class to send and receive MQTT messages. This class should be used with a singleton object
// because the esp-link implementation currently only supports a single MQTT server, so there is
// no value in instantiating multiple ELClientMqtt objects (although it's possible).
// All the server settings are made in esp-link and esp-link takes care to automatically
// reconnect and retry if the connection is lost. This means that on the arduino side the only
// code that is necessary is to send and receive messsages.
class ELClientMqtt {
  public:
    // Initialize with an ELClient object
    ELClientMqtt(ELClient* elc);

    // setup transmits the set of callbacks to esp-link. It assumes that the desired callbacks
    // have previously been attached using something like mqtt->connectedCb.attach(myCallbackFun).
    // After setup is called either the connectedCb or the disconnectedCb is invoked to provide
    // information about the initial connection status.
    void setup(void);

    // callbacks that can be attached prior to calling setup
    FP<void, void*> connectedCb;    /**< callback with no args when MQTT is connected */
    FP<void, void*> disconnectedCb; /**< callback with no args when MQTT is disconnected */
    FP<void, void*> publishedCb;    /**< not yet implemented */
    FP<void, void*> dataCb;         /**< callback when a message is received, called with two arguments: the topic and the message (max ~110 bytes for both) */

    // subscribe to a topic, the default qos is 0. When messages are recevied for the topic the
    // data callback is invoked.
    void subscribe(const char* topic, uint8_t qos=0);
    void subscribe(const __FlashStringHelper* topic, uint8_t qos=0);

    // publish a message to a topic
    void publish(const char* topic, const uint8_t* data,
        const uint16_t len, uint8_t qos=0, uint8_t retain=0);
    void publish(const char* topic, const char* data,
        uint8_t qos=0, uint8_t retain=0);
    void publish(const __FlashStringHelper* topic, const __FlashStringHelper* data,
        const uint16_t len, uint8_t qos=0, uint8_t retain=0);
    void publish(const char* topic, const __FlashStringHelper* data,
        const uint16_t len, uint8_t qos=0, uint8_t retain=0);
    void publish(const __FlashStringHelper* topic, const uint8_t* data,
        const uint16_t len, uint8_t qos=0, uint8_t retain=0);

    // set a last-will topic & message
    void lwt(const char* topic, const char* message, uint8_t qos=0, uint8_t retain=0);
    void lwt(const __FlashStringHelper* topic, const __FlashStringHelper* message,
        uint8_t qos=0, uint8_t retain=0);

  private:
    ELClient* _elc; /**< ELClient instance */
};

#endif // _EL_CLIENT_MQTT_H_
