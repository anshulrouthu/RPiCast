RPiCast
=======

Screen casting application using RaspberryPi. Cast your entire desktop to TV while watching a movie or browsing, or for presentation. This is a work in progress. There is no audio support yet, but regular updates are made. This project is not yet stable but functional. This application is developed and tested in Ubuntu 12.04, however it should build & work in any linux machine.

Features
--------
- Device discovery support.
- Cast entire desktop to RPi (no audio yet).

Build Instructions
------------------
Run setup.sh first, This script will build ffmpeg (with x11grab & libx264 enabled), UnitTest++ and install all the built libraries in the staging dir where the makefile will look for these libs. It will also pull the cross compiler toolchain for building rpi apps and cross staging tarball from my dropbox and extract in rpi-staging for cross compiling rpi apps.

NOTE: setup.sh will setup cross compilation environment, hence should be run in development machine.

$ ./setup.sh (you should run this only first time after new checkout, this should take appx 8-10 mins)<br>
$ make cross-all all

You can find the native binaries in ./build/native and the rpi's binaries in ./build/pi
Transfer the binaries in ./build/pi directory to RaspberryPi

Usage
-----
RaspberryPi:<br>
$ ./rpicast-server  (this will start the server and keep looking for multicast ssdp requests, you can also make this as a service that startup during boot up so that you do not have to ssh into rpi and run it every time)

Ubuntu 12.04:<br>
$ ./rpicast  (this will give you a menu to perform additional operations)

Found Devices <br>
1. RPiCast [alarmpi] 192.168.1.4

MENU<br>
1. Search for RpiCast<br>
2. Cast Screen to RpiCast<br>
3. Capture to a file<br>

Once the device is discovered you can choose option 2 to cast the screen to the Rpi or 3 to record the screen to a file (Output.vid)

TODO
----
Comming soon

Known Issues
------------
1. Since it is currently using UDP socket to transmit video packets, some artifacts are observed with low network speed routers
2. There is a huge lag when first time casted due to delay in the openmax setup for first time(should be ok if you quit and re-cast a couple of times)
3. Some regression in rpicast-server demux module (this is severe and should be fixed soon)
4. No audio support yet

Report Bugs & Issues
====================
Please report bugs and issues to anshul.m67@gmail.com
