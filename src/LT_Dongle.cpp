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
bool send = false;

char sendReadData[SEND_MAX];
int sendReadDataPosition = 0;

#define RECV_MAX 256
char recvReadData[RECV_MAX];
int recvReadDataPosition = 0;
bool recv = false;

int expectedBytes = 0;

void LT_Dongle::reset()
{
  tstate = STOPPED;
  pstate = IDLE;
  address = false;
  sendWriteDataPosition = 0;
  send = false;
  sendReadDataPosition = 0;
  recvReadDataPosition = 0;
  recv = false;
  expectedBytes = 0;
}

char LT_Dongle::recvChar()
{
  char ch;
  ch = getchar();
  return ch;
}

void LT_Dongle::sendString(const char *s)
{
  printf ("%s\n", s);
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
    case COMMAND: return (char*) "COMMAND"; break;
    case MODE: return (char*) "MODE"; break;
    case AUX: return (char*) "AUX"; break;
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


void LT_Dongle::sendRecvBytes()
{
  uint8_t sdata[64];
  uint8_t rdata[64];
  uint8_t buf[64];
  struct i2c_rdwr_ioctl_data i2cData;
  struct i2c_msg msgs[2];
  int result;
  
  i2cData.msgs = msgs;
  i2cData.nmsgs = 2;
  
  if (send && !recv)
  {
    LT_Dongle::convertString(sendWriteData, sendWriteDataPosition, sdata);
    
#if ENABLE_I2C
    
    if (sendWriteDataPosition > 0)
    {
      printf("sendByte at address 0x%02x with command 0x%02x\n", sdata[0], sdata[1]);


      if ((result = ioctl(LT_Dongle::file_, (unsigned long int)I2C_SLAVE, sdata[0] >> 1)) < 0)
      {
        printf("error ioctl %d handle %d\n", result, LT_Dongle::file_);
        throw LT_Exception("Fail address");
      }
        //printf("writeByte at address 0x%02x with command 0x%02x and data 0x%02x\n", address, command, sdata);

      //printf("position %d\n", sendWriteDataPosition/2);
      if ((result = write(LT_Dongle::file_,sdata+1,sendWriteDataPosition/2 -1)) < 0)
      {
       throw LT_Exception("Fail i2c write");
      }
//      switch (sendDataPosition/2)
//      {
//        case 2: // SEND BYTE
//          setNoPec();
//          if (i2c_smbus_write_byte(LT_Dongle::file_, data[1]) == -1)
//          {
//            throw LT_Exception("Send Byte: fail data");
//          }
//          break;
//        case 3:
//          setPec();
//          if (i2c_smbus_write_byte(LT_Dongle::file_, data[1]) == -1)
//          {
//            throw LT_Exception("Send Byte: fail data");
//          }
//          break;
//        case 3: // SEND BYTE PEC OR WRITE BYTE
//          setNoPec();
//          if (i2c_smbus_write_byte_data(LT_Dongle::file_, data[1], data[2]) == -1)
//          {
//            throw LT_Exception("Send Byte: fail data");
//          }
//          break;
//        case 4: // WRITE BYTE PEC
//          setPec();
//          if (i2c_smbus_write_byte_data(LT_Dongle::file_, data[1], data[2]) == -1)
//          {
//            throw LT_Exception("Send Byte: fail data");
//          }
//          break;
        // HOW TO KNOW IF GROUP?
        // Need raw i2c interface
//        default:
//          throw LT_Exception("No protocol for data");
//      }
    }
  }

  if (send && recv)
  {
    //printf("send/recv\n");
    LT_Dongle::convertString(sendWriteData, sendWriteDataPosition, sdata);
    LT_Dongle::convertString(sendReadData, sendReadDataPosition, rdata);
    
    if (sendReadDataPosition > 0)
    {
        //printf("writeByte at address 0x%02x with command 0x%02x and data 0x%02x\n", address, command, data);

      //printf("position %d\n", sendReadDataPosition/2);
      
      i2cData.msgs[0].addr = sdata[0] >> 1;
      i2cData.msgs[0].flags = 0;
      i2cData.msgs[0].len = sendWriteDataPosition/2 - 1;
      i2cData.msgs[0].buf = sdata + 1;
      i2cData.msgs[1].addr = rdata[0] >> 1;
      i2cData.msgs[1].flags = I2C_M_RD | I2C_M_NOSTART;
      i2cData.msgs[1].len = recvReadDataPosition;
      i2cData.msgs[1].buf = buf;
      if ((result = ioctl(LT_Dongle::file_,I2C_RDWR, &i2cData)) < 0)
      {
        printf("fail with %d\n", result);
       throw LT_Exception("Fail i2c write/read");
      }

    }
    
#endif
  }
//  else
//    throw LT_Exception("No data to recv");
    
}

void LT_Dongle::sendReadByte()
{
}

void LT_Dongle::sendReadWord()
{
}

void LT_Dongle::sendReadBlock()
{
}

void LT_Dongle::sendReadGpio()
{
}

void LT_Dongle::sendWriteByte()
{
}

void LT_Dongle::sendWriteWord()
{
}

void LT_Dongle::sendWriteGpio()
{
}

char printStr[128];
char printChar[2];
  
void LT_Dongle::processCommand(uint8_t command)
{
  switch(tstate)
  {
    case LT_Dongle::STOPPED:
      switch(command)
      {
        case 's': // START
          tstate = LT_Dongle::STARTED;
          break;
        case 'P': // PAUSE
          usleep(5000);
          break;
        case 'Z': // LF
          // TBD
          break;
        case 'I': // DUMP EEPROM
          break;
        case 'i': // ID
          sendString("USBSMB,PIC,00,01,DC,DC1613A,---------------------");
          pstate = LT_Dongle::IDLE;
          break;
        case 'M': // COMMAND
          pstate = LT_Dongle::COMMAND;
          break;          
        // SMBus
        case 'a': // ADDRESS
          pstate = LT_Dongle::ADDRESS1;
          break;
        case 'b': // READ BYTE
          expectedBytes = 2;
          pstate = LT_Dongle::READ_BYTE;
          break;
        case 'B': // SMBUS WRITE BYTE
          expectedBytes = 4;
          pstate = LT_Dongle::WRITE_BYTE;
          break;
        case 'w': // READ WORD
          expectedBytes = 2;
          pstate = LT_Dongle::READ_WORD;
          break;
        case 'W': // WRITE WORD
          expectedBytes = 6;
          pstate = LT_Dongle::WRITE_WORD;
          break;
        case 'k': // READ BLOCK
          expectedBytes = 2;
          pstate = LT_Dongle::READ_BLOCK;
          break;
        case 'g': // READ GPIO
          expectedBytes = 1;
          pstate = LT_Dongle::READ_GPIO;
          break;
        case 'G': // WRITE GPIO
          expectedBytes = 2;
          pstate = LT_Dongle::WRITE_GPIO;
          break;
        case 'e': // ECHO
          // TBD
          break;
          
        case LT_Dongle::ADDRESS1:
          address1 = command;
          pstate = LT_Dongle::ADDRESS2;
          break;
        case LT_Dongle::ADDRESS2:
          address2 = command;
          address = true;
          pstate = LT_Dongle::IDLE;
          break;
        case LT_Dongle::READ_BYTE:
          sendWriteData[sendWriteDataPosition++] = command;
          if (--expectedBytes == 0)
          {
            sendReadByte();
            reset();
          }
          break;
        case LT_Dongle::READ_WORD:
          sendWriteData[sendWriteDataPosition++] = command;
          if (--expectedBytes == 0)
          {
            sendReadWord();
            reset();
          }
          break;
        case LT_Dongle::READ_BLOCK:
          sendWriteData[sendWriteDataPosition++] = command;
          if (--expectedBytes == 0)
          {
            sendReadBlock();
            reset();
          }
          break;
        case LT_Dongle::READ_GPIO:
          sendWriteData[sendWriteDataPosition++] = command;
          if (--expectedBytes == 0)
          {
            sendReadGpio();
            reset();
          }
          break;
        case LT_Dongle::WRITE_BYTE:
          sendWriteData[sendWriteDataPosition++] = command;
          if (--expectedBytes == 0)
          {
            sendWriteByte();
            reset();
          }
          break;
        case LT_Dongle::WRITE_WORD:
          sendWriteData[sendWriteDataPosition++] = command;
          if (--expectedBytes == 0)
          {
            sendWriteWord();
            reset();
          }
          break;
        case LT_Dongle::WRITE_GPIO:
          sendWriteData[sendWriteDataPosition++] = command;
          if (--expectedBytes == 0)
          {
            sendWriteGpio();
            reset();
          }
          break;
        default:
          throw LT_Exception("");
      }
      break;
    case LT_Dongle::STARTED:
      switch(pstate)
      {
        case LT_Dongle::IDLE:
          switch(command)
          {
            case 's': // RESTART
              tstate = LT_Dongle::RESTARTED;
              break;
            case 'p': // STOP
              if (send)
              {
                sendRecvBytes();
                reset();
              }
              else
                throw LT_Exception("STARTED:IDLE:p");
              tstate = LT_Dongle::STOPPED;
              break;
            case 'S': // SEND
              expectedBytes = 2;
              pstate = LT_Dongle::SEND;
              break;
            default:
              throw LT_Exception("STARTED:IDLE:DEFAULT:", command);
          }
          break;
        case LT_Dongle::SEND:
          // printf("send command %c for pos %d\n", command, sendWriteDataPosition);
          if (sendWriteDataPosition >= SEND_MAX) throw LT_Exception("Overrun send buffer");
          sendWriteData[sendWriteDataPosition++] = command;
          send = true;
          if (--expectedBytes == 0)
            pstate = LT_Dongle::IDLE;
          break;
      }
      break;
    case LT_Dongle::RESTARTED:
      switch(pstate)
      {
        case LT_Dongle::IDLE:
          switch(command)
          {
            case 'p': // STOP
              if (send && recv)
              {
                sendRecvBytes();
                reset();
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
              if (recvReadDataPosition >= RECV_MAX) throw LT_Exception("Overrun recv buffer");
                recvReadData[recvReadDataPosition++] = '0';
              recv = true;
              pstate = LT_Dongle::IDLE;
              break;
            default:
              throw LT_Exception("RESTARTED:IDLE:QR");
          }
          break;
        case LT_Dongle::SEND:
          // printf("send command %c for pos %d\n", command, sendReadDataPosition);
          if (sendReadDataPosition >= SEND_MAX) throw LT_Exception("Overrun send buffer");
          sendReadData[sendReadDataPosition++] = command;
          // What about the case where there is an RSTART and write but to read?
          //recv = true;
          if (--expectedBytes == 0)
            pstate = LT_Dongle::IDLE;
          break;
        default:
          throw LT_Exception("RESTARTED:IDLE:DEFAULT:", command);
      }
      break;
    default:
      throw LT_Exception("DEFAULT");
      
  }

}





