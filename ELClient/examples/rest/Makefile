HOST ?= esp-link
LIBRARYPATH = $(ARDUINODIR)/libraries ../../..
LIBRARIES = ELClient
CPPFLAGS = 
SERIALDEV = net:$(HOST):2323
include ../arduino.mk

flash: all
	../avrflash $(HOST) rest.hex
	nc $(HOST) 23

run: upload size
	nc $(HOST) 23
