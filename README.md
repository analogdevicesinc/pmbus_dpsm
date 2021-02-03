# LT_PMBus

LT_PMbus is a port of the Linduino PSM library code and an example applicaiton.
The application communicates with PSM devices using /dev/i2c.

The purpose of the code is to teach PMBus programming of PSM parts and offer a
starting code base for projects. It is not intended to be industrial strength
software.

It is the responsibility of the user to add their own logging, error handling,
and other features normally found in industial code.

As such it is provided without warrantee (see LICENSE), but contributions,
bug fixes, and improvements are welcome to continually improve the code.

## Features

* Text menu system
* Command line without menu for device programming
* Telemetry
* Status
* On/Off/Margin control
* Fault log dumping and formatting
* In System Programming
* PMBus and SMBus layers
* Device abstractions

## License

See the LICENSE file.

## Dependencies

PMBus relies on a REPEATED START from the dev/i2c master.

## Targets

This code has be used on Rasp Pi and with a DLN-2. Some Raspberry Pi versons
require enabling of the REPEATED START via linux commands. If REPEATED START
does not work, search the internet for help. The DLN-2 was mainly used
for debugging code on a laptop and is not recommended for production use.

### Raspberry Pi 4

Follow these steps:

* Use the default OS that comes with the board
* Enable I2C with raspi-config Tool
* sudo apt-get install i2c-tools
* sudo apt-get install libi2c-dev
* Clone the code
* cd pmbus_dpsm
* ./configure
* ./make
* Connect a DC1962 or another demo board to the I2C lines
* src/LT_PMBusApp -i

This will run the application in interactive mode with a menu.

## Building Application

### Automake

Be sure to set the target as you would for any autoconfig project.

$ ./configure
$ make

### Automake Clean

$ make clean
$ make distclean

### CMake

$ mkdir build
$ cd build
$ cmake ..
$ make

### CMake Clean

$ make clean

## Dongle

The application can emulate a DC1613A dongle.

### Configure gadgetfs

Edit the boot files:

* Add dtoverlay=dwc2 to /boot/config.txt
* Add modules-load=dwc2 to /boot/cmdline.txt

Perform the following setup steps:

* sudo modprobe gadgetfs (test if neccessary or can be added to modules-load like dwc2,gadgetfs)
* sudo mkdir /dev/gadget
* sudo mount -t gadgetfs none /dev/gadget

### Running the Application

To run the Application as a CLI for manual testing, run it with the -l option.
