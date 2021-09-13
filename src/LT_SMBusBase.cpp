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

#define ENABLE_I2C 1

extern "C"
{
#if ENABLE_I2C
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>
#include <fcntl.h>
#ifdef DMALLOC
#include <dmalloc.h>
#else
#include <stdlib.h>
#endif
#include <unistd.h>
#include <sys/ioctl.h>
#else
#include <stdio.h>
#endif
}
#include <errno.h>
#include "LT_Exception.h"
#include "LT_SMBusBase.h"

#define FOUND_SIZE 0xFF

bool LT_SMBusBase::open_ = true;
uint8_t LT_SMBusBase::found_address_[FOUND_SIZE + 1];
int32_t LT_SMBusBase::file_;

LT_SMBusBase::LT_SMBusBase()
{
}

LT_SMBusBase::LT_SMBusBase(uint32_t speed){}

LT_SMBusBase::~LT_SMBusBase()
{
  if (LT_SMBusBase::open_)
  {
    //printf("Closing\n");
    close(LT_SMBusBase::file_);
    LT_SMBusBase::open_ = false;

  }
}

void LT_SMBusBase::clearBuffer()
{
  char buf[256];
  // This is intended to clear any data left over from a problem.
  // The known case of extra data is after a NACK.
  // Note that Rasp Pi also has a clock stretch bug.
  read(LT_SMBusBase::file_, &buf, 256);
}

void LT_SMBusBase::setPec()
{
  if (ioctl(LT_SMBusBase::file_, (unsigned long int)I2C_PEC, pec ? 1 : 0) < 0)
  {
    throw LT_Exception("Fail to set PEC");
  }
}

int LT_SMBusBase::writeByte(uint8_t address, uint8_t command, uint8_t data)
{
#if ENABLE_I2C
  setPec();
  if (ioctl(LT_SMBusBase::file_, (unsigned long int)I2C_SLAVE, address) < 0)
    throw LT_Exception("Write Byte: fail address");
  //printf("writeByte at address 0x%02x with command 0x%02x and data 0x%02x\n", address, command, data);

  if (i2c_smbus_write_byte_data(LT_SMBusBase::file_, command, data) == -1)
  {
    throw LT_Exception("Write Byte: fail data");
  }
  else
    return 0;
#else
  printf("Write Byte: addr 0x%x02, cmd 0x%x02, data 0x%x02\n", address, command, data);
  return 0;
#endif
}

void LT_SMBusBase::changeSpeed(uint32_t speed)
{
  throw LT_Exception("changeSpeed not implemented");
}

uint32_t LT_SMBusBase::getSpeed()
{
//  printf("getSpeed not implemented\n");
  return 0;
}

int LT_SMBusBase::writeBytes(uint8_t *addresses, uint8_t *commands, uint8_t *data, uint8_t no_addresses)
{
  uint16_t index = 0;
  int error = 0;

  setPec();
  while(index < no_addresses)
  {
    if (this->writeByte(addresses[index], commands[index], data[index]) == -1)
      error = -1;
    index++;
  }
  return error;
}

int LT_SMBusBase::readByte(uint8_t address, uint8_t command)
{
#if ENABLE_I2C
  __s32 result;

  setPec();  
  if (ioctl(LT_SMBusBase::file_, (unsigned long int)I2C_SLAVE, address) < 0)
    throw LT_Exception("Read Byte: fail address");
  //printf("readByte at address 0x%02x with command 0x%02x\n", address, command);

  if ((result = i2c_smbus_read_byte_data(LT_SMBusBase::file_, command)) == -1)
  {
    throw LT_Exception("Read Byte: fail data");
  }
  else

  return (int) result;
#else
  printf("Read Byte: addr 0x%x02, cmd 0x%x02\n", address, command);
  return 0;
#endif
}

int LT_SMBusBase::writeWord(uint8_t address, uint8_t command, uint16_t data)
{
#if ENABLE_I2C
  setPec();
  if (ioctl(LT_SMBusBase::file_, (unsigned long int)I2C_SLAVE, address) < 0)
    throw LT_Exception("Write Word: fail address");
  //printf("writeWord at address 0x%02x with command 0x%02x with data 0x%02x\n", address, command,data);

  if (i2c_smbus_write_word_data(LT_SMBusBase::file_, command, data) == -1)
  {
    throw LT_Exception("Write Word: fail data");
  }
  else
    return 0;
#else
  printf("Write Word: addr 0x%x02, cmd 0x%x02, data 0x%x04\n", address, command, data);
  return 0;
#endif
}

int LT_SMBusBase::readWord(uint8_t address, uint8_t command)
{
#if ENABLE_I2C
  __s32 result;

  setPec();
  if (ioctl(LT_SMBusBase::file_, (unsigned long int)I2C_SLAVE, address) < 0)
    throw LT_Exception("Read Word: fail address");
  //printf("readWord at address 0x%02x with command 0x%02x\n", address, command);
  if((result = i2c_smbus_read_word_data(LT_SMBusBase::file_, command)) == -1)
  {
    char msg[132];
    sprintf(msg, "Read Word: fail data with address 0x%02 command 0x%02x result %d", address, command, result);
    throw LT_Exception(msg);
  }
  else
    return (int) result;
#else
  printf("Read Word: addr 0x%x02, cmd 0x%x02\n", address, command);
  return 0;
#endif
}

int LT_SMBusBase::writeBlock(uint8_t address, uint8_t command,
    uint8_t *block, uint16_t block_size)
{
#if ENABLE_I2C
  unsigned long funcs;
  setPec();
  ioctl(LT_SMBusBase::file_, I2C_FUNCS, &funcs);
  if (funcs & I2C_FUNC_SMBUS_READ_BLOCK_DATA)
  {
    if (ioctl(LT_SMBusBase::file_, (unsigned long int)I2C_SLAVE, address) < 0)
      throw LT_Exception("Write Block: fail address");
    if (i2c_smbus_write_block_data(LT_SMBusBase::file_, address, command, block) == -1)
    {
      throw LT_Exception("Write Block: fail data");
    }
  }
  else
  {
    throw LT_Exception("Write Block: not supported");
  }

  return 0;
#else
  printf("Write Block: addr 0x%x02, cmd 0x%x02, size 0x%x02\n", address, command, block_size);
  return 0;
#endif
}

int LT_SMBusBase::writeReadBlock(uint8_t address, uint8_t command,
    uint8_t *block_out, uint16_t block_out_size, uint8_t* block_in, uint16_t block_in_size)
{
//  uint8_t *pos;
//  uint8_t block_read_size;
  throw LT_Exception("Write/Read Block: not supported");
}

int LT_SMBusBase::readBlock(uint8_t address, uint8_t command,
    uint8_t *block, uint16_t block_size)
{
#if ENABLE_I2C
  uint8_t count;
  unsigned long funcs;
  setPec();
  ioctl(LT_SMBusBase::file_, I2C_FUNCS, &funcs);
  if (funcs & I2C_FUNC_SMBUS_READ_BLOCK_DATA)
  {
    if (ioctl(LT_SMBusBase::file_, (unsigned long int)I2C_SLAVE, address) < 0)
      throw LT_Exception("Read Block: fail address");

    count = i2c_smbus_read_block_data(LT_SMBusBase::file_, command, block);
    if (count == -1)
    {
      throw LT_Exception("Read Block: fail data");
    }
    else
      return count;
  }
  else 
  {
    throw LT_Exception("Read Block: not supported");
  }
#else
  printf("Read Block: addr 0x%x02, cmd 0x%x02, size 0x%x02\n", address, command, block_size);
  return 0;
#endif
}

int LT_SMBusBase::sendByte(uint8_t address, uint8_t command)
{
#if ENABLE_I2C
  setPec();
  if (ioctl(LT_SMBusBase::file_, (unsigned long int)I2C_SLAVE, address) < 0)
    throw LT_Exception("Send Byte: fail address");

  if (i2c_smbus_write_byte(LT_SMBusBase::file_, command) == -1)
  {
    throw LT_Exception("Send Byte: fail data");
  }
  else
    return 0;
#else
  printf("Send Byte: addr 0x%x02, cmd 0x%x02\n", address, command);
  return 0;
#endif
}

int LT_SMBusBase::readAlert(void)
{
  uint8_t address;

  setPec();
  address = this->readByte(0x0C, 0x00);

  return address;
}

// General Note:
//
// Wait for ACK and probe will work with PMBus. However, they may not work
// well with all SMBus and I2CBus devices. Therefore, if you are not using
// PMBus devices, read the code and determine if they will work, or if there
// is an alternative implementation that will work, or an alternative way
// of using these functions from application code.

// This is not the best implementation of a wait for ack. First it makes
// the assumption that the command is a byte read, which is a bad assumption.
// Second, it does not STOP after the command ACKs the command byte, thus
// it could have side effects. Finally, it may not be compatible with an
// I2C device.
//
// The solution is to implement a wait for ack in the Linux driver. This
// implementation should offer some choices such as:
//
// - ACK address only
// - ACK address and command
// - ACK with read or write
// - ACK byte or word
// - ACK complete command
//
// The source of the problem is that linux is hiding the underlying driver
// layers that in an RTOS or Arduino are available to the programmer.
int LT_SMBusBase::waitForAck(uint8_t address, uint8_t command)
{
#if ENABLE_I2C
  setPec();
  if (ioctl(LT_SMBusBase::file_, (unsigned long int)I2C_SLAVE, address) < 0)
    throw LT_Exception("waitForAck: fail address");

  while (1)
  {
    if (i2c_smbus_read_byte_data(LT_SMBusBase::file_, command) >= 0)
      return 0;
  }
  throw LT_Exception("waitForAck: fail read");
#else
  printf("waitForAck: addr 0x%x02, cmd 0x%x02\n", address, command);
  return 0;
#endif
}

// Like the wait for ACK, this is not the best implementation. It
// would be enough to get an ACK on the address. That would make this
// compatible with all I2C devices, and all  SMBus devices. If all uses
// of the SMBus class (RTOS, Arduino...) can support only ACK of the address
// then the API can remove the command. Or this might be made more flexible
// like the discussion on the wait for ack command.
uint8_t* LT_SMBusBase::probe(uint8_t command)
{
#if ENABLE_I2C
  uint8_t   address;
//  uint8_t   buffer[1];
//  uint8_t   input[1];
  uint8_t   found = 0;
  int32_t   result;
//  buffer[0] = command;
  setPec();
  for (address = 0x10; address < 0x7F; address++)
  {
    if (address == 0x0C)
      continue;
    if (ioctl(LT_SMBusBase::file_, (unsigned long int)I2C_SLAVE, address) < 0)
      throw LT_Exception("Probe: fail address");
    result = i2c_smbus_read_byte_data(LT_SMBusBase::file_, command);
    if (result >= 0) 
    {
      if (found < FOUND_SIZE)
        found_address_[found++] = address;
    }
  }

  found_address_[found] = 0;
  return found_address_;
#else
  printf("probe: cmd 0x%x02\n", command);
  return 0;
#endif
}
                                                                                                                                 
uint8_t *LT_SMBusBase::probeUnique(uint8_t command)
{
  uint8_t   address;
//  uint8_t   buffer[1];
//  uint8_t   input[1];
  uint8_t   found = 0;
  int32_t   result;
//  buffer[0] = command;

//  return probe(command);

  setPec();
  for (address = 0x10; address < 0x7F; address++)
  {
    if (address == 0x0C)
      continue;
    if (address == 0x5B)
      continue;
    if (address == 0x5A)
      continue;
    if (address == 0x7C)
      continue;

    //printf("File %d Addr 0x%x\n", file_, address);./

    result = ioctl(LT_SMBusBase::file_, (unsigned long int)I2C_SLAVE, address);
    if (result == EBUSY)
      continue;
    else if (result < 0)
      throw LT_Exception("Probe Unique: fail address");

    result = i2c_smbus_read_byte_data(LT_SMBusBase::file_, command);

    //printf("probe data result %d\n", result);
    if (result >= 0)
    { 
      if (found < FOUND_SIZE)
        found_address_[found++] = address;
    }

  }

  found_address_[found] = 0;

  return found_address_;
}
