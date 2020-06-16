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

This code has be used on Rasp Pi and with a DLN-2. Some Rasp Pi versons
require enabling of the REPEATED START via linux commands. If REPEATED START
does not work, search the internet for help. The DLN-2 was mainly used
for debugging code on a laptop and is not recommended for production use.

## Make

The build envionrment is autoconfig. Be sure to set the target as you would
for any autoconfig project.

$ ./configure
$ make

## Clean

$ make clean
$ make distclean