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

using namespace std;

static LT_SMBusNoPec *smbusNoPec;
static LT_SMBusPec *smbusPec;
static LT_PMBus *pmbusNoPec;
static LT_PMBus *pmbusPec;
static LT_SMBus *smbus;
static LT_PMBus *pmbus;
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
   






        while ((opt = getopt(argc, argv, "d:s:e:c:p:v:x:i ")) != -1) {
	        switch (opt) {
	        case 'd':
			printf("Operate with device %s\n", optarg);
			dev = optarg;
	        	break;
	        case 'p':
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
	        case 'i':
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
	        case 'v':
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
                                while (*device != NULL){
                                   if((*device)->getAddress() == opt_address){
					if((*device)->hasFaultLog()){
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
                   case 'e':
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
                                while (*device != NULL){
                                   if((*device)->getAddress() == opt_address){
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
                   case 'c':
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
                                if((*device)->getAddress() == opt_address){
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
                   case 's':
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
                                if((*device)->getAddress() == opt_address){
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
                   case 'x':
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
                                if((*device)->getAddress() == opt_address){
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
	        default: /* '?' */
				delete(detector);
				delete(pmbusPec);
				delete(pmbusNoPec);
				delete(smbusPec);
				delete(smbusNoPec);
	            fprintf(stderr, "Usage: %s [-d dev] ([-p file] | [-v address] | [-x address] |\n   [-e address] | [-s address] | [-c address] | [-i]\n", argv[0]);
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
    fprintf(stderr, "Usage: %s [-d dev] ([-p file] | [-v address] | [-x address] |\n   [-e address] | [-s address] | [-c address] | [-i])\n", argv[0]);
    exit(EXIT_FAILURE);
}

