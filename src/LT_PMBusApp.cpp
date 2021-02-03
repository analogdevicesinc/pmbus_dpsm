/*
Copyright (c) 2020, Analog Devices Inc
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:
  * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
  * Neither the name of the Analog Devices, Inc. nor the names of its
    contributors may be used to endorse or promote products derived from this
    software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL ANALOG DEVICES, INC. BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdint.h>
#include <stdio.h>
#include <signal.h>
#ifdef DMALLOC
#include <dmalloc.h>
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#define exit(T) return(T);
#else
#include <stdlib.h>
#endif
#include <unistd.h>
#include <mcheck.h>
#include <sys/ioctl.h>
#include <fcntl.h>
// #include <sys/types.h>
// #include <stdint.h>
// #include <string.h>
#include <pthread.h>
#include <errno.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadgetfs.h>

#include <sys/resource.h>
#include "LT_Exception.h"
#include "LT_SMBusNoPec.h"
#include "LT_SMBusPec.h"
#include "LT_PMBus.h"
#include "LT_PMBusMath.h"
#include <LT_PMBusDevice.h>
#include <LT_SMBusGroup.h>
#include <LT_PMBusRail.h>
#include <LT_PMBusDetect.h>
#include <LT_Nvm.h>
#include "data.h"
#include <LT_Dongle.h>

#include "usbstring.c"

using namespace std;

static LT_SMBusNoPec *smbusNoPec;
static LT_SMBusPec *smbusPec;
static LT_PMBus *pmbusNoPec;
static LT_PMBus *pmbusPec;
static LT_SMBus *smbus;
static LT_PMBus *pmbus;
static LT_Dongle *dongle;
static LT_PMBusDetect *detector;
static LT_PMBusDevice **devices;
static LT_PMBusDevice **device;
static LT_PMBusRail **rails;
static LT_PMBusRail **rail;

void print_title(void);
void print_prompt(void);
void menu_1_basic_commands(void);
void menu_2_basic_commands(void);
void menu_3_basic_commands(void);

#define FETCH(_var_)                            \
    memcpy(cp, &_var_, _var_.bLength);          \
    cp += _var_.bLength;

#define CONFIG_VALUE 2

// Specific to controller
#define USB_DEV "/dev/gadget/fe980000.usb"
#define USB_EPIN "/dev/gadget/ep1in"
#define USB_EPOUT "/dev/gadget/ep2out"

enum {
    STRINGID_MANUFACTURER = 1,
    STRINGID_PRODUCT,
    STRINGID_SERIAL,
    STRINGID_CONFIG_HS,
    STRINGID_CONFIG_LS,
    STRINGID_INTERFACE,
    STRINGID_MAX
};

struct io_thread_args {
    unsigned stop;
    int fd_in, fd_out;
};

static struct io_thread_args thread_args;

static struct usb_string stringtab [] = {
    { STRINGID_MANUFACTURER, "MyOwnGadget", },
    { STRINGID_PRODUCT,      "Custom gadget", },
    { STRINGID_SERIAL,       "0001", },
    { STRINGID_CONFIG_HS,    "High speed configuration", },
    { STRINGID_CONFIG_LS,    "Low speed configuration", },
    { STRINGID_INTERFACE,    "Custom interface", },
    { STRINGID_MAX, NULL},
};

static struct usb_gadget_strings strings = {
    .language = 0x0409, /* en-us */
    .strings = stringtab,
};

static struct usb_endpoint_descriptor ep_descriptor_in;
static struct usb_endpoint_descriptor ep_descriptor_out;

void wait_for_nvm()
{
  usleep(4000000); // Allow time for action to complete.
}

void program_nvm (char *path)
{
  bool worked;

  NVM *nvm = new NVM(pmbusNoPec, smbusNoPec, smbusPec);

  printf("Please wait for programming EEPROM...\n");
  if (path == NULL)
  	worked = nvm->programWithData(isp_data);
  else
	worked = nvm->programWithFileData(path);

  wait_for_nvm();
  printf("Programming Complete (RAM and EEPROM may not match until reset)\n");

  delete(nvm);
}

void verify_nvm (char *path)
{
  bool worked;

  NVM *nvm = new NVM(pmbusNoPec, smbusNoPec, smbusPec);

  printf("Please wait for verification of EEPROM...\n");
  if (path == NULL)
  	worked = nvm->verifyWithData(isp_data);
  else
  	worked = nvm->verifyWithFileData(path);

  wait_for_nvm();

  if (worked == 0)
    printf("Verification complete: Invalid EEPROM data\n");
  else
  {
    printf("Verification complete: Valid EEPROM data\n");
  }

  delete(nvm);

  // Ensure there is no write protect so that the clear faults option works.
  pmbus->disableWriteProtectGlobal();
}

void restore_nvm ()
{
  printf("Restore User All\n");
  pmbus->restoreFromNvmGlobal();
  wait_for_nvm();
  printf("Restore Complete (EEPROM written to RAM)\n");
}

// Not working
// void reset_all_devices ()
// {
//   printf("Resseting all devices\n");
//    pmbus->startGroupProtocol();

// 	device = (devices = detector->getDevices());
// 	while (*device != NULL)
// 	{
// 		(*device)->reset();
// 		device++;
// 	}

// 	pmbus->executeGroupProtocol();

// 	device = (devices = detector->getDevices());
// 	while (*device != NULL)
// 	{
// 		(*device)->waitForAckNotBusy();
// 		device++;
// 	}
//   printf("All devices reset (RAM == EEPROM)\n");
// }

int loop()
{
	char input[12];
	char *inputp;
	uint user_command;
	uint8_t res;
	uint8_t model[7];
	uint8_t *addresses = NULL;

	do
		inputp = fgets(input, 11, stdin);
	while (input[0] == '\n');

	sscanf(input, "%d", &user_command);       //! Reads the user command

	switch (user_command)
	{
	case 1:
	  	menu_1_basic_commands();
	  	break;
	case 2 :
	  	menu_2_basic_commands();
	  	break;
	case 3 :
	  	menu_3_basic_commands();
	  	break;
	case 4 :
		device = (devices = detector->getDevices());
		while (*device != NULL)
		{
			printf("  Device: ");
			((*device)->getType());
			printf(" Address: ");
			printf("0x%x", (*device)->getAddress());

			printf(", Speed: ");
			printf("0x%x\n", (*device)->getMaxSpeed());

			// Switch the device to Pec
			pmbusNoPec->enablePec((*device)->getAddress());
			(*device)->changePMBus(pmbusPec);

			device++;
		}

		rail = (rails = detector->getRails());
		while (*rail != NULL)
		{
			printf("  Rail: ");
			printf(" Address: ");
			printf("0x%x", (*rail)->getAddress());
			printf(" Pages: ");
			printf("0x%x\n", (*rail)->getNoPages());
			(*rail)->changePMBus(pmbusPec);
			rail++;
		}
	  	break;
	case 5 :
		device = (devices = detector->getDevices());
		while (*device != NULL)
		{
			(*device)->enablePec();
			device++;
		}
		smbus = smbusPec;
		pmbus = pmbusPec;

		delete(detector);
		detector = new LT_PMBusDetect(pmbus);
		detector->detect();
		devices = detector->getDevices();
		break;
	case 6 :
		device = (devices = detector->getDevices());
		while (*device != NULL)
		{
			(*device)->disablePec();
			device++;
		}
		smbus = smbusNoPec;
		pmbus = pmbusNoPec;
		delete(detector);
		detector = new LT_PMBusDetect(pmbus);
		detector->detect();
		devices = detector->getDevices();
		break;
	case 7 :
		addresses = smbus->probe(0);
		while(*addresses != 0)
		{
			printf("ADDR 0x%x\n", *addresses++);
		}
break;
	case 8 :
		 pmbus->resetGlobal();
		 break;
	case 9 :
	  	return 0;
	default:
		 printf("Incorrect Option");
		 break;
	}

	print_prompt();
	return (1);
}

// Function Definitions

//! Prints the title block when program first starts.
void print_title()
{
	printf("\n***************************************************************\n");
	printf("* PSM Management Program                                        *\n");
	printf("*                                                               *\n");
	printf("*****************************************************************\n");
}

//! Prints main menu.
void print_prompt()
{
	printf("\n  1-Basic Commands\n");
	printf("  2-Fault Log\n");
	printf("  3-In Flight Update\n");
	printf("  4-System Tree\n");
	printf("  5-PEC On\n");
	printf("  6-PEC Off\n");
	printf("  7-Bus Probe\n");
	printf("  8-Reset\n");
	printf("  9-Quit\n");
	printf("\nEnter a command:");
}

//! Prints a warning if the demo board is not detected.
void print_warning_prompt()
{
	printf("\nWarning: Demo board not detected. Linduino will attempt to proceed.");
}

void print_all_voltages()
{
	float   voltage;
	device = devices;
	while((*device) != NULL)
	{
		if((*device)->hasCapability(HAS_VOUT))
		{
			for (int page = 0; page < (*device)->getNumPages(); page++)
			{
				(*device)->setPage(page);
				voltage = (*device)->readVout(true);
				printf("VOUT %f @ 0x%02x:%d\n", voltage, (*device)->getAddress(), page);
			}
		}
		device++;
	}
}

void print_all_currents()
{
	float   current;
	device = devices;
	while((*device) != NULL)
	{
		if((*device)->hasCapability(HAS_IOUT))
		{
			for (int page = 0; page < (*device)->getNumPages(); page++)
			{
				(*device)->setPage(page);
				current = (*device)->readIout(true);
				printf("IOUT %f @ 0x%02x:%d\n", current, (*device)->getAddress(), page);
			}
		}
		device++;
	}
}

void print_all_status()
{
	uint16_t w;

	device = devices;
	while((*device) != NULL)
	{
		if((*device)->hasCapability(HAS_STATUS_WORD))
		{
			for (int page = 0; page < (*device)->getNumPages(); page++)
			{
				w = (*device)->readStatusWord();
				printf("STATUS_WORD 0x%04x @ 0x%02x:%d\n", w, (*device)->getAddress(), page);
			}
		}
		device++;
	}
}

void sequence_off_on()
{
	pmbus->sequenceOffGlobal();
	usleep(2000 * 1000);
	pmbus->sequenceOnGlobal();
}

void margin_high()
{
	pmbus->marginHighGlobal();
}

void margin_low()
{
	pmbus->marginLowGlobal();
}

void margin_off()
{
	pmbus->sequenceOnGlobal();
}

void menu_1_basic_commands()
{
	uint32_t user_command;
	char input[12];
	char *inputp;

	do
	{
		//! Displays the Read/Write menu
		printf("\nRead/Write\n\n");
		printf("  1-Read All Voltages\n");
		printf("  2-Read All Currents\n");
		printf("  3-Read All Status\n");
		printf("  4-Sequence Off/On\n");
		printf("  5-Margin High\n");
		printf("  6-Margin Low\n");
		printf("  7-Margin Off\n");
		printf("  m-Main Menu\n");
		printf("\nEnter a command: ");

		do
			inputp = fgets(input, 11, stdin);
		while (input[0] == '\n');
		if (input[0] == 'm')                                // Print m if it is entered
		{
			break;
		}
		else
		{
			sscanf(input, "%d", &user_command);                //! Reads the user command
		}

		//! Reads or writes to a LTC3676 and prints result.
		switch (user_command)
		{
		  case 1:
			print_all_voltages();
			break;
		  case 2:
			print_all_currents();
			break;
		  case 3:
			print_all_status();
			break;
		  case 4:
			sequence_off_on();
			break;
		  case 5:
			margin_high();
			break;
		  case 6:
			margin_low();
			break;
		  case 7:
			margin_off();
			break;
		  default:
			if (user_command != 'm')
			  printf("Invalid Selection\n");
			break;
		}
	}
	while (user_command != 'm');
}

void menu_2_basic_commands()
{
	uint32_t user_command;
	char input[12];
	char *inputp;
  	uint16_t timeout;

	do
	{
		//! Displays the Read/Write menu
		printf("\n  1-Dump Fault Logs\n");
		printf("  2-Clear Fault Logs\n");
		printf("  3-Clear Faults\n");
		printf("  4-Enable Fault Logs\n");
		printf("  5-Disable Fault Logs\n");
		printf("  7-Reset\n");
		printf("  8-Store Fault Log\n");
		printf("  m-Main Menu\n");
		printf("\nEnter a command:");

		do
			inputp = fgets(input, 11, stdin);
		while (input[0] == '\n');
		if (input[0] == 'm')                                // Print m if it is entered
		{
			break;
		}
		else
		{
			sscanf(input, "%d", &user_command);                //! Reads the user command
		}

	    uint8_t status;

	    switch (user_command)                          //! Prints the appropriate submenu
	    {
	      case 1:
	      	device = (devices = detector->getDevices());
			while (*device != NULL)
			{
				if((*device)->hasFaultLog())
				{
					printf("Fault log for 0x%02x\n", (*device)->getAddress());					
					(*device)->printFaultLog();
				}
				else
					printf("No fault log for 0x%02x\n", (*device)->getAddress());
				device++;
			}
	      	break;
	      case 2:
	      	device = (devices = detector->getDevices());
			while (*device != NULL)
			{
				(*device)->clearFaultLog();
				device++;
			}
	        break;
	      case 3:
	      	device = (devices = detector->getDevices());
			while (*device != NULL)
			{
				(*device)->clearFaults();
				device++;
			}
	        break;
	      case 4:
	      	device = (devices = detector->getDevices());
			while (*device != NULL)
			{
				(*device)->enableFaultLog();
				device++;
			}
		break;
	      case 5:
	      	device = (devices = detector->getDevices());
			while (*device != NULL)
			{
				(*device)->disableFaultLog();
				device++;
			}
			break;
	      case 7:
	        pmbus->restoreFromNvmGlobal();
	//        usleep(2000 * 1000);
	        pmbus->resetGlobal();
	        break;
	      case 8:
	      	device = (devices = detector->getDevices());
			while (*device != NULL)
			{
				(*device)->storeFaultLog();
				device++;
			}
	        break;
	      default:
	        printf("Incorrect Option");
	        break;
	    }
	}
	while (user_command != 'm');
}

void menu_3_basic_commands()
{
	int user_command;
	char input[256];
	char *inputp;

	do
	{
		//! Displays the Read/Write menu
		printf("\n  1-Program (Data)\n");
		printf("  2-Verify (Data)\n");
		printf("  3-Program, Verify and Apply (Data)\n");
		printf("  4-Program (File)\n");
		printf("  5-Verify (File)\n");
		printf("  6-Program, Verify and Apply (File)\n");
		printf("  7-Restore\n");
		printf("  8-Clear Faults\n");
		printf("  m-Main Menu\n");
		printf("\nEnter a command:");

		do
			inputp = fgets(input, 11, stdin);
		while (input[0] == '\n');
		if (input[0] == 'm')                                // Print m if it is entered
		{
			break;
		}
		else
		{
			sscanf(input, "%d", &user_command);                //! Reads the user command
		}

	    uint8_t status;

	    switch (user_command)                          //! Prints the appropriate submenu
	    {
			case 1:
				program_nvm(NULL);
				break;
			case 2:
				verify_nvm(NULL);
				// Lock the NVM in case the verify failed.
				pmbus->smbus()->writeByte(0x5b, MFR_EE_UNLOCK, 0x00);
				break;
			case 3:
				program_nvm(NULL);
				verify_nvm(NULL);
				pmbus->resetGlobal();
				break;
			case 4:
				printf("Enter File Path: ");
				if (NULL == fgets(input, sizeof(input), stdin))
					printf("No File Given\n");
				else 
				{
					input[strlen(input)-1] = '\0';
					if (-1 != access(input, R_OK))
						program_nvm(input);
					else
						printf("Bad File Given: %s\n", input);	
				}				
				break;
			case 5:
				printf("Enter File Path: ");
				if (NULL == fgets(input, sizeof(input), stdin))
					printf("No File Given\n");
				else 
				{
					input[strlen(input)-1] = '\0';
					if (-1 != access(input, R_OK))
						verify_nvm(input);
					else
						printf("Bad File Given: %s\n", input);
				}					
				break;
			case 6:
				printf("Enter File Path: ");
				if (NULL == fgets(input, sizeof(input), stdin))
					printf("No File Given\n");
				else 
				{
					input[strlen(input)-1] = '\0';
					if (-1 != access(input, R_OK))
					{
						program_nvm(input);
						verify_nvm(input);
						pmbus->resetGlobal();
					}
					else
						printf("Bad File Given: %s\n", input);	
				}				
				break;
			case 7:
				restore_nvm();
				break;
			case 8:
				pmbus->clearFaultsGlobal();
				break;
	    }
	}
	while (user_command != 'm');
}

static void* io_thread(void* arg)
{
    struct io_thread_args* thread_args = (struct io_thread_args*)arg;
    fd_set read_set, write_set;
    struct timeval timeout;
    int ret, max_read_fd, max_write_fd;
    char buffer[512];

    max_read_fd = max_write_fd = 0;

    if (thread_args->fd_in > max_write_fd) max_write_fd = thread_args->fd_in;
    if (thread_args->fd_out > max_read_fd) max_read_fd  = thread_args->fd_out;

    while (!thread_args->stop)
    {
        FD_ZERO(&read_set);
        FD_SET(thread_args->fd_out, &read_set);
        timeout.tv_sec = 0;
        timeout.tv_usec = 10000; // 10ms

        memset(buffer, 0, sizeof(buffer));
        ret = select(max_read_fd+1, &read_set, NULL, NULL, &timeout);

        // Timeout
        if (ret == 0)
            continue;

        // Error
        if (ret < 0)
            break;

        ret = read (thread_args->fd_out, buffer, sizeof(buffer));

        if (ret > 0)
            printf("Read %d bytes : %s\n", ret, buffer);
        else
            printf("Read error %d(%m)\n", ret);

        FD_ZERO(&write_set);
        FD_SET(thread_args->fd_in, &write_set);

        memset(buffer, 0, sizeof(buffer));
        ret = select(max_write_fd+1, NULL, &write_set, NULL, NULL);

        // Error
        if (ret < 0)
            break;

        strcpy(buffer, "My name is USBond !");

        ret = write (thread_args->fd_in, buffer, strlen(buffer)+1);

        printf("Write status %d (%m)\n", ret);
    }

    close (thread_args->fd_in);
    close (thread_args->fd_out);

    thread_args->fd_in = -1;
    thread_args->fd_out = -1;

    return NULL;
}

static int init_ep(int* fd_in, int* fd_out)
{
    uint8_t init_config[2048];
    uint8_t* cp;
    int ret = -1;
    uint32_t send_size;

    // Configure ep1 (low/full speed + high speed)
    *fd_in = open(USB_EPIN, O_RDWR);

    if (*fd_in <= 0)
    {
        printf("Unable to open %s (%m)\n", USB_EPIN);
        goto end;
    }

    *(uint32_t*)init_config = 1;
    cp = &init_config[4];

    FETCH(ep_descriptor_in);
    FETCH(ep_descriptor_in);

    send_size = (uint32_t)cp-(uint32_t)init_config;
    ret = write(*fd_in, init_config, send_size);

    if (ret != send_size)
    {
        printf("Write error %d (%m)\n", ret);
        goto end;
    }

    printf("ep1 configured\n");

    // Configure ep2 (low/full speed + high speed)
    *fd_out = open(USB_EPOUT, O_RDWR);

    if (*fd_out <= 0)
    {
        printf("Unable to open %s (%m)\n", USB_EPOUT);
        goto end;
    }

    *(uint32_t*)init_config = 1;
    cp = &init_config[4];

    FETCH(ep_descriptor_out);
    FETCH(ep_descriptor_out);

    send_size = (uint32_t)cp-(uint32_t)init_config;
    ret = write(*fd_out, init_config, send_size);

    if (ret != send_size)
    {
        printf("Write error %d (%m)\n", ret);
        goto end;
    }

    printf("ep2 configured\n");

    ret = 0;

end:
    return ret;
}

static void handle_setup_request(int fd, struct usb_ctrlrequest* setup)
{
    int status;
    uint8_t buffer[512];
    pthread_t thread;

    printf("Setup request %d\n", setup->bRequest);

    switch (setup->bRequest)
    {
    case USB_REQ_GET_DESCRIPTOR:
        if (setup->bRequestType != USB_DIR_IN)
            goto stall;
        switch (setup->wValue >> 8)
        {
            case USB_DT_STRING:
                printf("Get string id #%d (max length %d)\n", setup->wValue & 0xff,
                    setup->wLength);
                status = usb_gadget_get_string (&strings, setup->wValue & 0xff, buffer);
                // Error 
                if (status < 0)
                {
                    printf("String not found !!\n");
                    break;
                }
                else
                {
                    printf("Found %d bytes\n", status);
                }
                write (fd, buffer, status);
                return;
        default:
            printf("Cannot return descriptor %d\n", (setup->wValue >> 8));
        }
        break;
    case USB_REQ_SET_CONFIGURATION:
        if (setup->bRequestType != USB_DIR_OUT)
        {
            printf("Bad dir\n");
            goto stall;
        }
        switch (setup->wValue) {
        case CONFIG_VALUE:
            printf("Set config value\n");
            if (!thread_args.stop)
            {
                thread_args.stop = 1;
                usleep(200000); // Wait for termination
            }
            if (thread_args.fd_in <= 0)
            {
                status = init_ep (&thread_args.fd_in, &thread_args.fd_out);
            }
            else
                status = 0;
            if (!status)
            {
                thread_args.stop = 0;
                pthread_create(&thread, NULL, io_thread, &thread_args);
            }
            break;
        case 0:
            printf("Disable threads\n");
            thread_args.stop = 1;
            break;
        default:
            printf("Unhandled configuration value %d\n", setup->wValue);
            break;
        }        
        // Just ACK
        status = read (fd, &status, 0);
        return;
    case USB_REQ_GET_INTERFACE:
        printf("GET_INTERFACE\n");
        buffer[0] = 0;
        write (fd, buffer, 1);
        return;
    case USB_REQ_SET_INTERFACE:
        printf("SET_INTERFACE\n");
        ioctl (thread_args.fd_in, GADGETFS_CLEAR_HALT);
        ioctl (thread_args.fd_out, GADGETFS_CLEAR_HALT);
        // ACK
        status = read (fd, &status, 0);
        return;
    }

stall:
    printf("Stalled\n");
    // Error
    if (setup->bRequestType & USB_DIR_IN)
        read (fd, &status, 0);
    else
        write (fd, &status, 0);
}

static void handle_ep0(int fd)
{
    int ret, nevents, i;
    fd_set read_set;
    struct usb_gadgetfs_event events[5];

    while (1)
    {
        FD_ZERO(&read_set);
        FD_SET(fd, &read_set);

        select(fd+1, &read_set, NULL, NULL, NULL);

        ret = read(fd, &events, sizeof(events));

        if (ret < 0)
        {
            printf("Read error %d (%m)\n", ret);
            goto end;        
        }

        nevents = ret / sizeof(events[0]);

        printf("%d event(s)\n", nevents);

        for (i=0; i<nevents; i++)
        {
            switch (events[i].type)
            {
            case GADGETFS_CONNECT:
                printf("EP0 CONNECT\n");
                break;
            case GADGETFS_DISCONNECT:
                printf("EP0 DISCONNECT\n");
                break;
            case GADGETFS_SETUP:
                printf("EP0 SETUP\n");
                handle_setup_request(fd, &events[i].u.setup);
                break;
            case GADGETFS_NOP:
            case GADGETFS_SUSPEND:
                break;
            }
        }
    }

end:
    return;
}


int dongleLoop()
{
    int fd=-1, ret, err=-1;
    uint32_t send_size;
    struct usb_config_descriptor config;
    struct usb_config_descriptor config_hs;
    struct usb_device_descriptor device_descriptor;
    struct usb_interface_descriptor if_descriptor;
    uint8_t init_config[2048];
    uint8_t* cp;

    fd = open(USB_DEV, O_RDWR|O_SYNC);

    if (fd <= 0)
    {
        printf("Unable to open %s (%m)\n", USB_DEV);
        return 1;
    }

    *(uint32_t*)init_config = 0;
    cp = &init_config[4];

    device_descriptor.bLength = USB_DT_DEVICE_SIZE;
    device_descriptor.bDescriptorType = USB_DT_DEVICE;
    device_descriptor.bDeviceClass = USB_CLASS_COMM;
    device_descriptor.bDeviceSubClass = 0;
    device_descriptor.bDeviceProtocol = 0;
    //device_descriptor.bMaxPacketSize0 = 255; Set by driver
    device_descriptor.idVendor = 0xAA; // My own id
    device_descriptor.idProduct = 0xBB; // My own id
    device_descriptor.bcdDevice = 0x0200; // Version
    // Strings
    device_descriptor.iManufacturer = STRINGID_MANUFACTURER;
    device_descriptor.iProduct = STRINGID_PRODUCT;
    device_descriptor.iSerialNumber = STRINGID_SERIAL;
    device_descriptor.bNumConfigurations = 1; // Only one configuration

    ep_descriptor_in.bLength = USB_DT_ENDPOINT_SIZE;
    ep_descriptor_in.bDescriptorType = USB_DT_ENDPOINT;
    ep_descriptor_in.bEndpointAddress = USB_DIR_IN | 1;
    ep_descriptor_in.bmAttributes = USB_ENDPOINT_XFER_BULK;
    ep_descriptor_in.wMaxPacketSize = 512; // HS size

    ep_descriptor_out.bLength = USB_DT_ENDPOINT_SIZE;
    ep_descriptor_out.bDescriptorType = USB_DT_ENDPOINT;
    ep_descriptor_out.bEndpointAddress = USB_DIR_OUT | 2;
    ep_descriptor_out.bmAttributes = USB_ENDPOINT_XFER_BULK;
    ep_descriptor_out.wMaxPacketSize = 512; // HS size

    if_descriptor.bLength = sizeof(if_descriptor);
    if_descriptor.bDescriptorType = USB_DT_INTERFACE;
    if_descriptor.bInterfaceNumber = 0;
    if_descriptor.bAlternateSetting = 0;
    if_descriptor.bNumEndpoints = 2;
    if_descriptor.bInterfaceClass = USB_CLASS_COMM;
    if_descriptor.bInterfaceSubClass = 0;
    if_descriptor.bInterfaceProtocol = 0;
    if_descriptor.iInterface = STRINGID_INTERFACE;

    config_hs.bLength = sizeof(config_hs);
    config_hs.bDescriptorType = USB_DT_CONFIG;
    config_hs.wTotalLength = config_hs.bLength +
        if_descriptor.bLength + ep_descriptor_in.bLength + ep_descriptor_out.bLength;
    config_hs.bNumInterfaces = 1;
    config_hs.bConfigurationValue = CONFIG_VALUE;
    config_hs.iConfiguration = STRINGID_CONFIG_HS;
    config_hs.bmAttributes = USB_CONFIG_ATT_ONE | USB_CONFIG_ATT_SELFPOWER;
    config_hs.bMaxPower = 1;

    config.bLength = sizeof(config);
    config.bDescriptorType = USB_DT_CONFIG;
    config.wTotalLength = config.bLength +
        if_descriptor.bLength + ep_descriptor_in.bLength + ep_descriptor_out.bLength;
    config.bNumInterfaces = 1;
    config.bConfigurationValue = CONFIG_VALUE;
    config.iConfiguration = STRINGID_CONFIG_LS;
    config.bmAttributes = USB_CONFIG_ATT_ONE | USB_CONFIG_ATT_SELFPOWER;
    config.bMaxPower = 1;

    FETCH(config);
    FETCH(if_descriptor);
    FETCH(ep_descriptor_in);
    FETCH(ep_descriptor_out);

    FETCH(config_hs);
    FETCH(if_descriptor);
    FETCH(ep_descriptor_in);
    FETCH(ep_descriptor_out);

    FETCH(device_descriptor);

    // Configure ep0
    send_size = (uint32_t)cp-(uint32_t)init_config;
    ret = write(fd, init_config, send_size);

    if (ret != send_size)
    {
        printf("Write error %d (%m)\n", ret);
        goto end;
    }

    printf("ep0 configured\n");
    
    handle_ep0(fd);

end:
    if (fd != -1) close(fd);

    return err;
}

int dongleCommandLineInterface()
{
	char ch;
	
	ch = dongle->recvChar();
	if (ch == '\r' || ch == '\n')
		return 1;
	if (ch == 'q')
		return 0;
	
	dongle->processCommand(ch);
	return 1;
}

static void sig_abort(int signo)
{
  delete smbus;
  abort();
}


void serial_init(int argc, char * argv[]);
int main(int argc, char * argv[]) {
	int opt;
    char *file;
    char *holder;
    char *dev = NULL;
    int opt_address;
    

    signal(SIGINT, sig_abort);

    try
    {

	    // CFLAGS=-g
	    // export CFLAGS
	    // .configure i686
	    // MALLOC_TRACE=~/pmbus_dpsm/memory.txt
	    // export MALLOC_TRACE
	    // echo "" > memory.txt
	    // src/LT_PMBusApp
	    // mtrace src/LT_PMBusTest memory.txt
	    // addr2line -e src/LT_PMBusTest 0x00000000019ef640
	    // gdb src/LT_PMBusApp
	    // start
	    // info symbol sym
   
        while ((opt = getopt(argc, argv, "d:s:e:c:p:v:x:i:g :l ")) != -1) {
	        switch (opt) {
	        case 'd': // Device
			printf("Operate with device %s\n", optarg);
			dev = optarg;
	        	break;
	        case 'p': // Program using file
			printf("Program with file %s\n", optarg);
			mtrace();
			if (dev != NULL)
			{
				smbusNoPec = new LT_SMBusNoPec(dev);
				smbusPec = new LT_SMBusPec(dev);
			}
			else
			{
				smbusNoPec = new LT_SMBusNoPec();
				smbusPec = new LT_SMBusPec();
			}
			pmbusNoPec = new LT_PMBus(smbusNoPec);
			pmbusPec = new LT_PMBus(smbusPec);
			smbus = smbusNoPec;
			pmbus = pmbusNoPec;
			program_nvm(optarg);
			verify_nvm(optarg);
			pmbus->resetGlobal();
			muntrace();
			delete(detector);
			delete(pmbusPec);
			delete(pmbusNoPec);
			delete(smbusPec);
			delete(smbusNoPec);
			exit(EXIT_SUCCESS);
			break;
	        case 'i': // Interactive
	        	mtrace();
	        	if (dev != NULL)
		    	{
				smbusNoPec = new LT_SMBusNoPec(dev);
				smbusPec = new LT_SMBusPec(dev);
			}
			else
			{
				smbusNoPec = new LT_SMBusNoPec();
				smbusPec = new LT_SMBusPec();
			}
			pmbusNoPec = new LT_PMBus(smbusNoPec);
			pmbusPec = new LT_PMBus(smbusPec);
			smbus = smbusNoPec;
			pmbus = pmbusNoPec;
			detector = new LT_PMBusDetect(pmbus);
			detector->detect();
			devices = detector->getDevices();
			print_title();
			print_prompt();
			while(loop());
			muntrace();
			delete(detector);
			delete(pmbusPec);
			delete(pmbusNoPec);
			delete(smbusPec);
			delete(smbusNoPec);
			exit(EXIT_SUCCESS);
	            break;
	        case 'v': // Dump Fault Logs
                        mtrace();
	        	if (dev != NULL)
			{
				smbusNoPec = new LT_SMBusNoPec(dev);
				smbusPec = new LT_SMBusPec(dev);
			}
			else
			{
				smbusNoPec = new LT_SMBusNoPec();
				smbusPec = new LT_SMBusPec();
			}
			pmbusNoPec = new LT_PMBus(smbusNoPec);
			pmbusPec = new LT_PMBus(smbusPec);
			smbus = smbusNoPec;
			pmbus = pmbusNoPec;
			detector = new LT_PMBusDetect(pmbus);
			detector->detect();
			devices = detector->getDevices();
			device = devices;
			holder = optarg;
			opt_address = strtol(holder, NULL, 16);
			while (*device != NULL)
			{
				if((*device)->getAddress() == opt_address)
				{
					if((*device)->hasFaultLog())
					{
						printf("Fault log for 0x%02x\n", (*device)->getAddress());					
						(*device)->printFaultLog();
					}
					else
					printf("No fault log for 0x%02x\n", (*device)->getAddress());
				}
				device++;
			}
			delete(detector);
			delete(pmbusPec);
			delete(pmbusNoPec);
			delete(smbusPec);
			delete(smbusNoPec);
			exit(EXIT_SUCCESS);
			break;
                   case 'e': // Enable Fault Logs
                        mtrace();
	        	if (dev != NULL)
			{
				smbusNoPec = new LT_SMBusNoPec(dev);
				smbusPec = new LT_SMBusPec(dev);
			}
			else
			{
				smbusNoPec = new LT_SMBusNoPec();
				smbusPec = new LT_SMBusPec();
			}
			pmbusNoPec = new LT_PMBus(smbusNoPec);
			pmbusPec = new LT_PMBus(smbusPec);
			smbus = smbusNoPec;
			pmbus = pmbusNoPec;
			detector = new LT_PMBusDetect(pmbus);
			detector->detect();
			devices = detector->getDevices();
			device = devices;
			holder = optarg;
			opt_address = strtol(holder, NULL, 16);
			opt_address = strtol(holder, NULL, 16);
			while (*device != NULL)
			{
				if((*device)->getAddress() == opt_address)
				{
					(*device)->enableFaultLog();
					printf("Fault log for 0x%02x Enabled\n", (*device)->getAddress());					
				}
				device++;
			}
			delete(detector);
			delete(pmbusPec);
			delete(pmbusNoPec);
			delete(smbusPec);
			delete(smbusNoPec);
			exit(EXIT_SUCCESS);
	        	break;
                   case 'c': // Clear Fault Logs
                        mtrace();
	        	if (dev != NULL)
			{
				smbusNoPec = new LT_SMBusNoPec(dev);
				smbusPec = new LT_SMBusPec(dev);
			}
			else
			{
				smbusNoPec = new LT_SMBusNoPec();
				smbusPec = new LT_SMBusPec();
			}
			pmbusNoPec = new LT_PMBus(smbusNoPec);
			pmbusPec = new LT_PMBus(smbusPec);
			smbus = smbusNoPec;
			pmbus = pmbusNoPec;
			detector = new LT_PMBusDetect(pmbus);
			detector->detect();
			devices = detector->getDevices();
			device = devices;
			holder = optarg;
			opt_address = strtol(holder, NULL, 16);
			while (*device != NULL)
			{
				if((*device)->getAddress() == opt_address)
				{
					(*device)->clearFaultLog();
					printf("Clearing Fault log for 0x%02x \n", (*device)->getAddress());					
				}
				device++;
			}
			delete(detector);
			delete(pmbusPec);
			delete(pmbusNoPec);
			delete(smbusPec);
			delete(smbusNoPec);
			exit(EXIT_SUCCESS);
	        	break;
                   case 's': // Store Fault Logs
                        mtrace();
	        	if (dev != NULL)
			{
				smbusNoPec = new LT_SMBusNoPec(dev);
				smbusPec = new LT_SMBusPec(dev);
			}
			else
			{
				smbusNoPec = new LT_SMBusNoPec();
				smbusPec = new LT_SMBusPec();
			}
			pmbusNoPec = new LT_PMBus(smbusNoPec);
			pmbusPec = new LT_PMBus(smbusPec);
			smbus = smbusNoPec;
			pmbus = pmbusNoPec;
			detector = new LT_PMBusDetect(pmbus);
			detector->detect();
			devices = detector->getDevices();
			device = devices;
			holder = optarg;
			opt_address = strtol(holder, NULL, 16);
			while (*device != NULL)
			{
				if((*device)->getAddress() == opt_address)
				{
					(*device)->storeFaultLog();
					printf("Storing Fault log for 0x%02x \n", (*device)->getAddress());					
				}
				device++;
			}
			delete(detector);
			delete(pmbusPec);
			delete(pmbusNoPec);
			delete(smbusPec);
			delete(smbusNoPec);
			exit(EXIT_SUCCESS);
	        	break;
                   case 'x': // Disable Fault Logs
                        mtrace();
	        	if (dev != NULL)
			{
				smbusNoPec = new LT_SMBusNoPec(dev);
				smbusPec = new LT_SMBusPec(dev);
			}
			else
			{
				smbusNoPec = new LT_SMBusNoPec();
				smbusPec = new LT_SMBusPec();
			}
			pmbusNoPec = new LT_PMBus(smbusNoPec);
			pmbusPec = new LT_PMBus(smbusPec);
			smbus = smbusNoPec;
			pmbus = pmbusNoPec;
			detector = new LT_PMBusDetect(pmbus);
			detector->detect();
			devices = detector->getDevices();
			device = devices;
			holder = optarg;
			opt_address = strtol(holder, NULL, 16);
			while (*device != NULL)
			{
				if((*device)->getAddress() == opt_address)
				{
					(*device)->disableFaultLog();
					printf("Disabling Fault log for 0x%02x \n", (*device)->getAddress());					
				}
				device++;
			}
			delete(detector);
			delete(pmbusPec);
			delete(pmbusNoPec);
			delete(smbusPec);
			delete(smbusNoPec);
			exit(EXIT_SUCCESS);
	        	break;
		case 'g' : // DC1613A Dongle
	        	mtrace();
	        	if (dev != NULL)
			{
				dongle = new LT_Dongle(dev);
			}
			else
			{
				dongle = new LT_Dongle();
			}
			while(dongleLoop());
			muntrace();
			delete(dongle);
			exit(EXIT_SUCCESS);
		case 'l' : // DC1613A Dongle CLI
	        	mtrace();
	        	if (dev != NULL)
			{
				dongle = new LT_Dongle(dev);
			}
			else
			{
				dongle = new LT_Dongle();
			}
			while(dongleCommandLineInterface());
			muntrace();
			delete(dongle);
			exit(EXIT_SUCCESS);
		default: /* '?' Help */
			delete(detector);
			delete(pmbusPec);
			delete(pmbusNoPec);
			delete(smbusPec);
			delete(smbusNoPec);
			fprintf(stderr, "Usage: %s [-d dev] ([-p file] | [-v address] | [-x address] |\n   [-e address] | [-s address] | [-c address] | [-i] | [-g] | [-l]\n", argv[0]);
			exit(EXIT_FAILURE);
	        }
	    }
	} catch (LT_Exception ex)
	{
		fprintf(stderr, "Error: %s\n", ex.what());
		delete smbus;
		exit(EXIT_FAILURE);
	}

	delete(detector);
	delete(pmbusPec);
	delete(pmbusNoPec);
	delete(smbusPec);
	delete(smbusNoPec);
	fprintf(stderr, "Usage: %s [-d dev] ([-p file] | [-v address] | [-x address] |\n   [-e address] | [-s address] | [-c address] | [-i] | [-g] | [-l])\n", argv[0]);
	exit(EXIT_FAILURE);
}

