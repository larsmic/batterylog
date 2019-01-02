SHELL = /bin/sh

all: batteryloggerd batterylog

batteryloggerd: batteryloggerd.o settings.o
	gcc -o batteryloggerd $^

batterylog: batterylog.o ui.o settings.o
	gcc -o batterylog $^

%.o: %.c defines.h settings.h
	gcc -o $@ -c $< -I.

.DEFAULT_GOAL := all

.PHONY: install

install:
	mkdir -p $(DESTDIR)usr/bin/
	cp ./batterylog $(DESTDIR)usr/bin/batterylog
	mkdir -p $(DESTDIR)etc/
	cp ./batterylog.conf $(DESTDIR)etc/batterylog.conf
	
	mkdir -p $(DESTDIR)usr/bin/
	cp ./batteryloggerd $(DESTDIR)usr/bin/batteryloggerd
	mkdir -p $(DESTDIR)etc/systemd/system
	cp ./batteryloggerd.service $(DESTDIR)etc/systemd/system/batteryloggerd.service
