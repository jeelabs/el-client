ESP_HOSTNAME ?= esp-link
LIBRARYPATH = $(ARDUINODIR)/libraries ../../..
LIBRARIES = ELClient
CPPFLAGS = 
SERIALDEV = net:$(ESP_HOSTNAME):2323
include ../arduino.mk

flash: all
	../avrflash $(ESP_HOSTNAME) test.hex
	nc $(ESP_HOSTNAME) 23

run: upload size
	nc $(ESP_HOSTNAME) 23
