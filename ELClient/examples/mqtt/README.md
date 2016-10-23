MQTT example
============

This is a simple example of a sketch that subscribes to an mqtt topic and then publishes messages
to that topic, which it subsequently receives and prints.

**Important** For this sketch to work you must turn off the UART debug log in esp-link (on
the Debug Log page). The reason is that otherwise esp-link prints too much to its uart and then
misses incoming characters.
