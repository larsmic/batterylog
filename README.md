# batterylog
Terminal utility which displays a graph of the battery percentage logged by batteryloggerd.
## Usage
First install batteryloggerd, which is the logging daemon that generates the logfiles this programm reads from.
After that (and maybe waiting 10 minutes, so that batteryloggerd writes the first logentry) you can use batterylog.
## Installation
Build the binaries:
```bash
$ make
```
Moving the binaries and the configfile in your system to the right spot:
```bash
$ make DESTDIR="/" install 
```
## Configuration
The configfile lies under `/etc/batterylog.conf`, rules are explained there.
