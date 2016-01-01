LIBRARYPATH = $(ARDUINODIR)/libraries ../../..
LIBRARIES = espduino
CPPFLAGS = 
SERIALDEV = net:esp-link:23
include ../arduino.mk

run: upload size
	nc esp-link 23
