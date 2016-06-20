ELClient
========
This is a Wifi library for arduino that uses the SLIP protocol to communicate via serial with
an ESP8266 module running the [esp-link firmware](https://github.com/jeelabs/esp-link).

This library requires esp-link v2.2.beta2 or later.

Features
========
- C++ classes to manage the communication with esp-link
- Support outbound REST requests
- Support MQTT pub/sub
- Support additional commands to query esp-link about wifi and such

- MQTT functionality: 
    + MQTT protocol itself implemented by esp-link
    + Support subscribing, publishing, LWT, keep alive pings and all QoS levels 0&1

- REST functionality:
    + Support methods GET, POST, PUT, DELETE
    + setContent type, set header, set User Agent

Examples
========
Currently two examples are provided that are known to work and that come with HEX files ready
to load into an Atmega 328 based arduino:
- A REST example that fetches the current time from a server on the internet and prints it.
  This example is in `./ELClient/examples/rest`
- An MQTT example that publishes messages to an MQTT server and subscribes so it receives and
  prints its own messages.

The "thingspeak" and "demo" examples are currently not maintained and therefore won't work as-is.
