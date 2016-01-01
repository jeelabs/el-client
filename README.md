ELClient
========
This is Wifi library for arduino that uses the SLIP protocol to communicate via serial with ESP8266 modules running [esp-link firmware](https://github.com/jeelabs/esp-link).

Features
========
- Rock Solid wifi network client for arduino
- **More reliable** than AT COMMAND library
- **ELClient can add additional callbacks that the esp-link firmware can utilize to add additional custom functionality
- MQTT module: 
    + MQTT client runs on the esp-link
    + Support subscribing, publishing, LWT, keep alive pings and all 3 QoS levels (it should be a fully functional client).
    + Easy to setup and use
- REST module:
    + Support method GET, POST, PUT, DELETE
    + setContent type, set header, set User Agent
    + Easy to used API
    
Installations
========
**1. Clone this project:**

```bash
git clone https://github.com/jeelabs/el-client
cd el-client
```

**2. Program ESP8266:**

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



