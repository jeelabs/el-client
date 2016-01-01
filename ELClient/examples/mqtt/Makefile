HOST ?= esp-link
LIBRARYPATH = $(ARDUINODIR)/libraries ../../..
LIBRARIES = espduino
CPPFLAGS = 
SERIALDEV = net:$(HOST):2323
include ../arduino.mk

run: upload size
	nc $(HOST) 23
