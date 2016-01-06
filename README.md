ELClient
========
This is a Wifi library for arduino that uses the SLIP protocol to communicate via serial with
an ESP8266 module running the [esp-link firmware](https://github.com/jeelabs/esp-link).

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

Installation
============
**1. Clone this project:**

```bash
git clone https://github.com/jeelabs/el-client
cd el-client
```

**2. Program the ESP8266:**

- Wiring: ![Program Connection diagram](fritzing/program_esp8266_bb.png?raw=true)
- Program release firmware:

```python
esp8266/tools/esptool.py -p COM1 write_flash 0x00000 esp-link/0x00000.bin 0x40000 esp-link/0x40000.bin
```

**3. Wiring:**
![Program Connection diagram](fritzing/esp8266_bb.png?raw=true)

**4. Import arduino library and run example:**

Example read DHT11 and send to [thingspeak.com](http://thingspeak.com)
=========
- ```espduino/examples/thingspeak/thingspeak.ino```
- Using DHT11 library from: [https://github.com/RobTillaart/Arduino](https://github.com/RobTillaart/Arduino)

![](images/thingspeak.png)
