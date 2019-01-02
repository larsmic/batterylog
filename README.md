# batterylog
Terminal utility which displays a graph of the battery percentage logged by batteryloggerd.
## Usage
After installation (and waiting a maximum of 10 minutes, so that batteryloggerd writes the first logentry) you can use batterylog.
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

If the logging daemon should start at system startup, enable the unit with:
```bash
$ systemctl enable batteryloggerd.service
```
