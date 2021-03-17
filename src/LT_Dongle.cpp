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

#ifdef DMALLOC
#include <dmalloc.h>
#else
#include <stdlib.h>
#endif
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <string.h>
#include <unistd.h>
extern "C"
{
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>
}
#include "LT_Exception.h"
#include "LT_Dongle.h"

bool LT_Dongle::open_ = false;
int32_t LT_Dongle::file_;

LT_Dongle::LT_Dongle()
{
#if ENABLE_I2C
  if (!LT_Dongle::open_)
  {
    file_ = open("/dev/i2c-1", O_RDWR);
    if (LT_Dongle::file_ < 0)
      {
        throw LT_Exception("Fail to open");
      }
    clearBuffer();
    LT_Dongle::open_ = true;
    //printf ("Open NoPec handle %d\n", file_);
  }
#endif
}

LT_Dongle::LT_Dongle(char *dev){}

LT_Dongle::~LT_Dongle()
{
  if (LT_Dongle::open_)
  {
    //printf("Closing\n");
    close(LT_Dongle::file_);
    LT_Dongle::open_ = false;

  }
}

void LT_Dongle::clearBuffer()
{
  char buf[256];
  // This is intended to clear any data left over from a problem.
  // The known case of extra data is after a NACK.
  // Note that Rasp Pi also has a clock stretch bug.
  read(LT_Dongle::file_, &buf, 256);
}

void LT_Dongle::changeSpeed(uint32_t speed)
{
  throw LT_Exception("changeSpeed not implemented");
}

uint32_t LT_Dongle::getSpeed()
{
//  printf("getSpeed not implemented\n");
  return 0;
}

void LT_Dongle::setPec()
{
  if (ioctl(LT_Dongle::file_, (unsigned long int)I2C_PEC, 1) < 0)
  {
    throw LT_Exception("Fail to set PEC");
  }
}

void LT_Dongle::setNoPec()
{
  if (ioctl(LT_Dongle::file_, (unsigned long int)I2C_PEC, 0) < 0)
  {
    throw LT_Exception("Fail to set NO PEC");
  }
}

enum LT_Dongle::TransactionState tstate = LT_Dongle::STOPPED;
enum LT_Dongle::ProtocolState pstate = LT_Dongle::IDLE;

char address1;
char address2;
bool address = false;

#define SEND_MAX 256
char sendWriteData[SEND_MAX];
int sendWriteDataPosition = 0;
char sendReadData[SEND_MAX][SEND_MAX];
int sendReadDataPosition[SEND_MAX];
int sendReadDataIndex = 0;
bool send = false;

#define RECV_MAX 256
char recvData[RECV_MAX];
int recvDataPosition = 0;
bool recv = false;

int expectedBytes = 0;

void LT_Dongle::reset()
{
  tstate = STOPPED;
  pstate = IDLE;
  address = false;
  sendWriteDataPosition = 0;
  for (int i = 0; i < SEND_MAX; i++)
    sendReadDataPosition[i] = 0;
  sendReadDataIndex = 0;
  send = false;
  recvDataPosition = 0;
  recv = false;
  expectedBytes = 0;
}

char LT_Dongle::recvChar()
{
  char ch;
  ch = getchar();
  return ch;
}

char *LT_Dongle::sendString(const char *s)
{
  char *ret = new char[256];
  strcpy(ret, s);
  return ret;
}

void LT_Dongle::convertString(const char *s, int length, uint8_t *data)
{
  int i, j;
  
  //printf("convert %d characters\n", length);
  
  if (length % 2 > 0)
    throw LT_Exception("Cannot convert odd length string");
  
  for(i = 0; i < length; i=i+2)
  {
    for (j = 0; j < 2; j++)
    {
      if (j == 0)
        switch (s[i+j])
        {
          case '0' : data[i/2] = 0 << 4; break;
          case '1' : data[i/2] = 1 << 4; break;
          case '2' : data[i/2] = 2 << 4; break;
          case '3' : data[i/2] = 3 << 4; break;
          case '4' : data[i/2] = 4 << 4; break;
          case '5' : data[i/2] = 5 << 4; break;
          case '6' : data[i/2] = 6 << 4; break;
          case '7' : data[i/2] = 7 << 4; break;
          case '8' : data[i/2] = 8 << 4; break;
          case '9' : data[i/2] = 9 << 4; break;
          case 'A' : data[i/2] = 10 << 4; break;
          case 'B' : data[i/2] = 11 << 4; break;
          case 'C' : data[i/2] = 12 << 4; break;
          case 'D' : data[i/2] = 13 << 4; break;
          case 'E' : data[i/2] = 14 << 4; break;
          case 'F' : data[i/2] = 15 << 4; break;
          default: throw LT_Exception("Character not supported");
        }
      else
        switch (s[i+j])
        {
          case '0' : data[i/2] |= 0; break;
          case '1' : data[i/2] |= 1; break;
          case '2' : data[i/2] |= 2; break;
          case '3' : data[i/2] |= 3; break;
          case '4' : data[i/2] |= 4; break;
          case '5' : data[i/2] |= 5; break;
          case '6' : data[i/2] |= 6; break;
          case '7' : data[i/2] |= 7; break;
          case '8' : data[i/2] |= 8; break;
          case '9' : data[i/2] |= 9; break;
          case 'A' : data[i/2] |= 10; break;
          case 'B' : data[i/2] |= 11; break;
          case 'C' : data[i/2] |= 12; break;
          case 'D' : data[i/2] |= 13; break;
          case 'E' : data[i/2] |= 14; break;
          case 'F' : data[i/2] |= 15; break;
          default: throw LT_Exception("Character not supported");
        }      
    }
  }
}

char *LT_Dongle::toStateString(LT_Dongle::ProtocolState state)
{
  switch (state)
  {
    case IDLE: return (char*) "IDLE"; break;
    case ADDRESS1: return (char*) "ADDRESS1"; break;
    case ADDRESS2: return (char*) "ADDRESS2"; break;
    case SEND: return (char*) "SEND"; break;
    case READ_BYTE: return (char*) "READ_BYTE"; break;
    case READ_WORD: return (char*) "READ_WORD"; break;
    case READ_BLOCK: return (char*) "READ_BLOCK"; break;
    case READ_GPIO: return (char*) "READ_GPIO"; break;
    case WRITE_BYTE: return (char*) "WRITE_BYTE"; break;
    case WRITE_WORD: return (char*) "WRITE_WORD"; break;
    case WRITE_GPIO: return (char*) "WRITE_GPIO"; break;
    case DATA: return (char*) "DATA"; break;
    default: return (char*) "No State"; break;
  }
}


char *LT_Dongle::sendRecvBytes()
{
  uint8_t sdata[64];
  uint8_t rdata[64];
  uint8_t buf[64];
  struct i2c_rdwr_ioctl_data i2cData;
  struct i2c_msg msgs[2];
  int i,j;
  char *qdata = new char[64];
  char ch;
  int result;
  char *nada = new char[1];
  nada[0] = 0;
  char *nack = new char[3];
  nack[0] = 'N';
  nack[1] = 'N';
  nack[2] = 0;
    
  i2cData.msgs = msgs;
  i2cData.nmsgs = 2;
  
  if (send && !recv)
  {
    
    
#if ENABLE_I2C

    // Group Command Protocol will have sendWriteData and SendReadData
    // Read commands will have Send WriteData and SendReadData and recvReadData
    
    if (sendWriteDataPosition > 0)
    {
      printf("write sendWriteData %d\n", sendWriteDataPosition); 
      LT_Dongle::convertString(sendWriteData, sendWriteDataPosition, sdata);
      
      // Ignoring r/w bit.
      if ((result = ioctl(LT_Dongle::file_, (unsigned long int)I2C_SLAVE, sdata[0] >> 1)) < 0)
      {
        throw LT_Exception("Fail ioctl address");
      }
      if ((result = write(LT_Dongle::file_,sdata+1,sendWriteDataPosition/2 - 1)) < 0)
      {
        // The master cannot write past a NACK, so one N.
        return nack;
      }
    }

    for (int i = 0; i <= sendReadDataIndex; i++)
    {
      printf("loop %d %d\n", i, sendReadDataPosition[i]);
      if (sendReadDataPosition[i] > 0)
      {
        printf("write sendReadData %d %d\n", i, sendReadDataPosition[i]); 
        LT_Dongle::convertString(sendReadData[i], sendReadDataPosition[i], sdata);
        
        // Ignoring r/w bit.
        if ((result = ioctl(LT_Dongle::file_, (unsigned long int)I2C_SLAVE, sdata[0] >> 1)) < 0)
        {
          throw LT_Exception("Fail ioctl address");
        }
        if ((result = write(LT_Dongle::file_,sdata+1,sendReadDataPosition[i]/2 - 1)) < 0)
        {
          // The master cannot write past a NACK, so one N.
          return nack;
        }
      }
    }
  }

  if (send && recv)
  {
    LT_Dongle::convertString(sendWriteData, sendWriteDataPosition, sdata);
    LT_Dongle::convertString(sendReadData[0], sendReadDataPosition[0], rdata);
    printf("send %d && recv %d\n", sendWriteDataPosition, recvDataPosition);
    
    // This is not going to have RSTARTS in recvReadData
    // Need a multidimensional array.
    
    if (sendWriteDataPosition > 0)
    {
      printf("have send data/recv data\n");
      // Ignoring r/w bit
      i2cData.msgs[0].addr = sdata[0] >> 1;
      i2cData.msgs[0].flags = 0;
      i2cData.msgs[0].len = sendWriteDataPosition/2 - 1;
      i2cData.msgs[0].buf = sdata + 1;
      // Ignoring r/w bit.
      i2cData.msgs[1].addr = rdata[0] >> 1;
      i2cData.msgs[1].flags = I2C_M_RD | I2C_M_NOSTART;
      i2cData.msgs[1].len = recvDataPosition;
      i2cData.msgs[1].buf = buf;
      if ((result = ioctl(LT_Dongle::file_,I2C_RDWR, &i2cData)) < 0)
      {
        printf("nack\n");
        // The master cannot write or read past a NACK, so one N.
        return nack;
      }
      printf("ack\n");
      j = 0;
      for(i = 0; i < i2cData.msgs[1].len; i++)
      {
        ch = i2cData.msgs[1].buf[i] >> 4;
        if (ch >= 0 && ch <= 9)
          ch = ch + 0x30;
        else
          ch = ch + 0x41 - 10;
        qdata[j++] = ch;
        ch = i2cData.msgs[1].buf[i] & 0x0F;
        if (ch >= 0 && ch <= 9)
          ch = ch + 0x30;
        else
          ch = ch + 0x41 - 10;
        qdata[j++] = ch;
      }
//      qdata[j++] = '\n';
      qdata[j] = 0;
    }
    return qdata;
#endif
  }
  else
    return nada;
//  else
//    throw LT_Exception("No data to recv");
    
}

char *LT_Dongle::sendReadByte()
{
  uint8_t sdata[64];
  uint8_t adata[2];
  __s32 result;
  char *ret = new char[3];
  char *nada = new char[2];
  nada[0] = '\n' ;
  nada[1] = 0;
  char *nack = new char[3];
  nack[0] = 'N';
  nack[1] = 'N';
  nack[2] = 0;
  
  if (sendWriteDataPosition != 2)
    throw LT_Exception("Read Byte malformed");
  if (!address)
    throw LT_Exception("No Address");
    
  LT_Dongle::convertString(sendWriteData, sendWriteDataPosition, sdata);
    
  makeAddressString(address1, address2, adata);
  
  printf("readByte %d %d\n", adata[0], sdata[0]);
    
  if ((result = ioctl(LT_Dongle::file_, (unsigned long int)I2C_SLAVE, adata[0])) < 0)
  {
    throw LT_Exception("Fail ioctl address");
  }
  
      
  if ((result = i2c_smbus_read_byte_data(LT_Dongle::file_, sdata[0])) < 0)
    return nack;
  else
  {
    ret[0] = result;
    ret[1] = '\n';
    ret[2] = 0;
    return ret;
  }
}

char *LT_Dongle::sendReadWord()
{
  uint8_t sdata[64];
  uint8_t adata[2];
  __s32 result;
  char *ret = new char[4];
  char *nada = new char[2];
  nada[0] = '\n' ;
  nada[1] = 0;
  char *nack = new char[5];
  nack[0] = 'N';
  nack[1] = 'N';
  nack[2] = 'N';
  nack[3] = 'N';
  nack[4] = 0;
  
  if (sendWriteDataPosition != 2)
    throw LT_Exception("Read Word malformed");
  if (!address)
    throw LT_Exception("No Address");
    
  LT_Dongle::convertString(sendWriteData, sendWriteDataPosition, sdata);
    
  makeAddressString(address1, address2, adata);
  
  //printf("readWord %d %d\n", adata[0], sdata[0]);
    
  if ((result = ioctl(LT_Dongle::file_, (unsigned long int)I2C_SLAVE, adata[0])) < 0)
  {
    throw LT_Exception("Fail ioctl address");
  }
  
  if ((result = i2c_smbus_read_word_data(LT_Dongle::file_, sdata[0])) < 0)
    return nack;
  else
  {
    ret[0] = result << 8;
    ret[1] = result >> 8;
    ret[2] = '\n';
    ret[3] = 0;
    return ret;
  }
}

char *LT_Dongle::sendReadBlock()
{
  uint8_t sdata[64];
  uint8_t adata[2];
  char *block = new char[256];
  __s32 result;
  __s32 count;
  int i;
  
  struct i2c_rdwr_ioctl_data i2cData;
  struct i2c_msg msgs[2];
  int j;
  char *qdata = new char[256];
  char ch;
  uint8_t buf[256];
  char *nada = new char[2];
  nada[0] = '\n' ;
  nada[1] = 0;
  char *nack = new char[3];
  nack[0] = 'N';
  nack[1] = 'N';
  nack[2] = 0;
  
  i2cData.msgs = msgs;
  i2cData.nmsgs = 2;
  
  unsigned long funcs;
  result = ioctl(LT_Dongle::file_, I2C_FUNCS, &funcs);
  // printf("Funcs: %08x %d\n", funcs, funcs & I2C_FUNC_SMBUS_READ_BLOCK_DATA);
  
  if (sendWriteDataPosition != 2)
    throw LT_Exception("Read Block malformed");
  if (!address)
    throw LT_Exception("No Address");
    
  LT_Dongle::convertString(sendWriteData, sendWriteDataPosition, sdata);
    
  makeAddressString(address1, address2, adata);
  
  //printf("readBlock %d %d\n", adata[0], sdata[0]);
    
  if ((result = ioctl(LT_Dongle::file_, (unsigned long int)I2C_SLAVE, adata[0])) < 0)
  {
    throw LT_Exception("Fail ioctl address");
  }

  if ((funcs & I2C_FUNC_SMBUS_READ_BLOCK_DATA) > 0)
  {
    if ((count = i2c_smbus_read_block_data(LT_Dongle::file_, sdata[0], (uint8_t*)block)) < 0)
      return nack;
    else
    {
      block[count] = '\n';
      return block;
    }
  }
  else
  {
    if ((result = i2c_smbus_read_byte_data(LT_Dongle::file_, sdata[0])) < 0)
      return nack;
    else
    {
      i2cData.msgs[0].addr = adata[0];
      i2cData.msgs[0].flags = 0;
      i2cData.msgs[0].len = 1;
      i2cData.msgs[0].buf = sdata;
      i2cData.msgs[1].addr = adata[0];
      i2cData.msgs[1].flags = I2C_M_RD | I2C_M_NOSTART;
      i2cData.msgs[1].len = result + 1;
      i2cData.msgs[1].buf = buf;
      if ((result = ioctl(LT_Dongle::file_,I2C_RDWR, &i2cData)) < 0)
      {
        // The master cannot write or read past a NACK, so one N.
        return nack;
      }
      j = 0;
      for(i = 0; i < i2cData.msgs[1].len; i++)
      {
        ch = i2cData.msgs[1].buf[i] >> 4;
        if (ch >= 0 && ch <= 9)
          ch = ch + 0x30;
        else
          ch = ch + 0x41 - 10;
        qdata[j++] = ch;
        ch = i2cData.msgs[1].buf[i] & 0x0F;
        if (ch >= 0 && ch <= 9)
          ch = ch + 0x30;
        else
          ch = ch + 0x41 - 10;
        qdata[j++] = ch;
      }
//      qdata[j++] = '\n';
      qdata[j] = 0;

    } 
    return qdata;
  }
}

char *LT_Dongle::sendReadGpio()
{
  char *nada = new char[2];
  nada[0] = '\n' ;
  nada[1] = 0;
  
  return nada;
}

void LT_Dongle::makeAddressString(char l, char r, uint8_t *address)
{
  char s[3];
  s[0] = l;
  s[1] = r;
  s[2] = '\0';
  LT_Dongle::convertString(s, 2, address);
}

char *LT_Dongle::sendWriteByte()
{
  uint8_t sdata[64];
  uint8_t adata[2];
  int result;
  char *nada = new char[2];
  nada[0] = '\n' ;
  nada[1] = 0;
  char *nack = new char[3];
  nack[0] = 'N';
  nack[1] = 'N';
  nack[2] = 0;
  
  if (sendWriteDataPosition != 4)
    throw LT_Exception("Write Byte malformed");
  if (!address)
    throw LT_Exception("No Address");
    
  LT_Dongle::convertString(sendWriteData, sendWriteDataPosition, sdata);
    
  makeAddressString(address1, address2, adata);
  
  if ((result = ioctl(LT_Dongle::file_, (unsigned long int)I2C_SLAVE, adata[0])) < 0)
  {
    throw LT_Exception("Fail ioctl address");
  }
  
      
  if (i2c_smbus_write_byte_data(LT_Dongle::file_, sdata[0], sdata[1]) < 0)
    return nack;
  else
    return nada;
}

char *LT_Dongle::sendWriteWord()
{
  uint8_t sdata[64];
  uint8_t adata[2];
  int result;
  char *nada = new char[2];
  nada[0] = '\n' ;
  nada[1] = 0;
  char *nack = new char[5];
  nack[0] = 'N';
  nack[1] = 'N';
  nack[2] = 'N';
  nack[3] = 'N';
  nack[4] = 0;

  if (sendWriteDataPosition != 6)
    throw LT_Exception("Write Word malformed");
  if (!address)
    throw LT_Exception("No Address");
    
  LT_Dongle::convertString(sendWriteData, sendWriteDataPosition, sdata);
    
  makeAddressString(address1, address2, adata);
  
  if ((result = ioctl(LT_Dongle::file_, (unsigned long int)I2C_SLAVE, adata[0])) < 0)
  {
    throw LT_Exception("Fail ioctl address");
  }
  
      
  if (i2c_smbus_write_word_data(LT_Dongle::file_, sdata[0], (sdata[2] << 8) | sdata[1]) < 0)
    return nack;
  else
    return nada;
}

char *LT_Dongle::sendWriteGpio()
{
  char *nada = new char[2];
  nada[0] = '\n' ;
  nada[1] = 0;
  
  return nada;
}

char printStr[128];
char printChar[2];
char *ret;

// TODO: #define for replacing exception with reset.
// TODO: Add timout to reset machine.
char *LT_Dongle::processCommand(uint8_t command)
{
  char *echo = new char[3];
  echo[0] = 'e';
  echo[1] = '\n';
  echo[2] = 0;
  char *nada = new char[1];
  nada[0] = 0;
  char *nl = new char[2];
  nl[0] = '\n' ;
  nl[1] = 0;
  char *nack = new char[3];
  nack[0] = 'N';
  nack[1] = 'N';
  nack[2] = 0;
  printf("command %c %d %d \n", command, tstate, pstate);
  switch(tstate)
  {
    case LT_Dongle::STOPPED:
      printf("STOPPED\n");
      switch(command)
      {
        case 's': // START
          tstate = LT_Dongle::STARTED;
          break;
        case 'P': // PAUSE
          usleep(5000);
          printf("P\n");
          break;
        case 'Z': // LF
          tstate = LT_Dongle::STOPPED;
          printf("NL\n");
          return nl;
          break;
        case 'I': // DUMP EEPROM
          break;
        case 'i': // ID
          pstate = LT_Dongle::IDLE;
          return sendString("USBSMB,PIC,00,01,DC,LINUXD,---------------------");
          break;
        case 'M': // COMMAND
          tstate = LT_Dongle::MODE;
          break;          
        // SMBus
        case 'a': // ADDRESS
          tstate = LT_Dongle::SMBUS;
          pstate = LT_Dongle::ADDRESS1;
          break;
        case 'b': // READ BYTE
          expectedBytes = 2;
          tstate = LT_Dongle::SMBUS;
          pstate = LT_Dongle::READ_BYTE;
          break;
        case 'B': // SMBUS WRITE BYTE
          expectedBytes = 4;
          tstate = LT_Dongle::SMBUS;
          pstate = LT_Dongle::WRITE_BYTE;
          break;
        case 'w': // READ WORD
          expectedBytes = 2;
          tstate = LT_Dongle::SMBUS;
          pstate = LT_Dongle::READ_WORD;
          break;
        case 'W': // WRITE WORD
          expectedBytes = 6;
          tstate = LT_Dongle::SMBUS;
          pstate = LT_Dongle::WRITE_WORD;
          break;
        case 'k': // READ BLOCK
          expectedBytes = 2;
          tstate = LT_Dongle::SMBUS;
          pstate = LT_Dongle::READ_BLOCK;
          break;
        case 'g': // READ GPIO
          expectedBytes = 1;
          tstate = LT_Dongle::SMBUS;
          pstate = LT_Dongle::READ_GPIO;
          printf("0\n");
          break;
        case 'G': // WRITE GPIO
          expectedBytes = 2;
          tstate = LT_Dongle::SMBUS;
          pstate = LT_Dongle::WRITE_GPIO;
          break;
        case 'e': // ECHO
          return echo;
          break;
        default:
          //throw LT_Exception("Bad command:", command);
          break;
      }
      break;
    case LT_Dongle::SMBUS:
      printf("SMBUS\n");
      // If there is a Z in the middle of one of these, meaning the
      // expected bytes are getting fetched, it is not going to be
      // processed;
      switch(pstate)
      {
        case LT_Dongle::ADDRESS1:
          address1 = command;
          pstate = LT_Dongle::ADDRESS2;
          break;
        case LT_Dongle::ADDRESS2:
          address2 = command;
          address = true;
          tstate = LT_Dongle::STOPPED;
          pstate = LT_Dongle::IDLE;
          break;
        case LT_Dongle::READ_BYTE:
          sendWriteData[sendWriteDataPosition++] = command;
          if (--expectedBytes == 0)
          {
            ret = sendReadByte();
            reset();
            return ret;
          }
          break;
        case LT_Dongle::READ_WORD:
          sendWriteData[sendWriteDataPosition++] = command;
          if (--expectedBytes == 0)
          {
            ret = sendReadWord();
            reset();
            return ret;
          }
          break;
        case LT_Dongle::READ_BLOCK:
          sendWriteData[sendWriteDataPosition++] = command;
          if (--expectedBytes == 0)
          {
            ret = sendReadBlock();
            reset();
            return ret;
          }
          break;
        case LT_Dongle::READ_GPIO:
          sendWriteData[sendWriteDataPosition++] = command;
          if (--expectedBytes == 0)
          {
            ret = sendReadGpio();
            reset();
            return ret;
          }
          break;
        case LT_Dongle::WRITE_BYTE:
          sendWriteData[sendWriteDataPosition++] = command;
          if (--expectedBytes == 0)
          {
            ret = sendWriteByte();
            reset();
            return ret;
          }
          break;
        case LT_Dongle::WRITE_WORD:
          sendWriteData[sendWriteDataPosition++] = command;
          if (--expectedBytes == 0)
          {
            ret = sendWriteWord();
            reset();
            return ret;
          }
          break;
        case LT_Dongle::WRITE_GPIO:
          sendWriteData[sendWriteDataPosition++] = command;
          if (--expectedBytes == 0)
          {
            ret = sendWriteGpio();
            reset();
            return ret;
          }
          break;
        default:
          throw LT_Exception("STOPPED:SMBUS");
      }
      break;      
    case LT_Dongle::STARTED:
      printf("STARTED\n");
      switch(pstate)
      {
        case LT_Dongle::IDLE:
          switch(command)
          {
            case 'Z': // LF
              printf("Z\n");
              tstate = LT_Dongle::STOPPED;
              pstate = LT_Dongle::IDLE;
              return nl;
              break;
            case 's': // RESTART
              tstate = LT_Dongle::RESTARTED;
              break;
            case 'p': // STOP
              if (send)
              {
                ret = sendRecvBytes();
                reset();
                return ret;
              }
              else
                throw LT_Exception("STARTED:IDLE:p");
              tstate = LT_Dongle::STOPPED;
              break;
            case 'S': // SEND
              expectedBytes = 2;
              pstate = LT_Dongle::SEND;
              printf("Send\n");
              break;
            // This would be for something reading without a RSTART.
            case 'Q': // RECEIVE ACK
            case 'R': // RECEIVE NACK
              if (recvDataPosition >= RECV_MAX) throw LT_Exception("Overrun recv buffer");
                recvData[recvDataPosition++] = '0';
              recv = true;
              pstate = LT_Dongle::IDLE;
              break;
            default:
              throw LT_Exception("STARTED:IDLE:DEFAULT:", command);
          }
          break;
        case LT_Dongle::SEND:
          printf("send command %c for pos %d\n", command, sendWriteDataPosition);
          if (sendWriteDataPosition >= SEND_MAX) throw LT_Exception("Overrun send buffer");
          sendWriteData[sendWriteDataPosition++] = command;
          send = true;
          if (--expectedBytes == 0)
            pstate = LT_Dongle::IDLE;
          break;
      }
      break;
    case LT_Dongle::RESTARTED:
      printf("RESTARTED\n");
      switch(pstate)
      {
        case LT_Dongle::IDLE:
          switch(command)
          {
            case 'Z': // LF
              tstate = LT_Dongle::STOPPED;
              pstate = LT_Dongle::IDLE;
              return nl;
              break;
            case 's': // RESTART
              printf("bump index % d\n", sendReadDataIndex);
              sendReadDataIndex++;
              break;
            case 'p': // STOP
              if (send || recv)
              {
                ret = sendRecvBytes();
                reset();
                return ret;
              }
              else
                throw LT_Exception("RESTARTED:IDLE:p");
              tstate = LT_Dongle::STOPPED;
              break;
            case 'S': // SEND
              expectedBytes = 2;
              pstate = LT_Dongle::SEND;
              break;
            case 'Q': // RECEIVE ACK
            case 'R': // RECEIVE NACK
              if (recvDataPosition >= RECV_MAX) throw LT_Exception("Overrun recv buffer");
                recvData[recvDataPosition++] = '0';
              recv = true;
              pstate = LT_Dongle::IDLE;
              break;

            default:
              throw LT_Exception("RESTARTED:IDLE:QR");
          }
          break;
        case LT_Dongle::SEND:
          printf("send command %c for pos %d\n", command, sendReadDataPosition[sendReadDataIndex]);
          printf("send command %c for pos %d\n", command, sendReadDataPosition[sendReadDataIndex]);
          if (sendReadDataPosition[sendReadDataIndex] >= SEND_MAX) throw LT_Exception("Overrun send buffer");
          sendReadData[sendReadDataIndex][sendReadDataPosition[sendReadDataIndex]] = command;
          sendReadDataPosition[sendReadDataIndex] = sendReadDataPosition[sendReadDataIndex] + 1;
          printf("index %d pos %d\n", sendReadDataIndex, sendReadDataPosition[sendReadDataIndex]);
          send = true;
          if (--expectedBytes == 0)
            pstate = LT_Dongle::IDLE;
          break;
        default:
          throw LT_Exception("RESTARTED:IDLE:DEFAULT:", command);
      }
      break;
    case LT_Dongle::MODE:
      tstate = STOPPED;
      break;
    default:
      throw LT_Exception("DEFAULT");
      
  }
  return nada;
}





