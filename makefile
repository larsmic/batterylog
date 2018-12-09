SHELL = /bin/sh


batterylog: main.o ui.o settings.o
	gcc -o batterylog $^

%.o: %.c header.h
	gcc -o $@ -c $< -I.

.DEFAULT_GOAL := batterylog

.PHONY: install

install:
	mkdir -p $(DESTDIR)usr/bin/
	cp ./batterylog $(DESTDIR)usr/bin/batterylog

