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
#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
extern "C"
{
#include <linux/i2c-dev.h>
}
#include "LT_Exception.h"
#include "LT_SMBusNoPec.h"

LT_SMBusNoPec::LT_SMBusNoPec() : LT_SMBusBase()
{
#if ENABLE_I2C
  if (!LT_SMBusBase::open_)
  {
    file_ = open("/dev/i2c-0", O_RDWR);
    if (LT_SMBusBase::file_ < 0)
      {
        throw LT_Exception("Fail to open");
      }
    clearBuffer();
    LT_SMBusBase::open_ = true;
    //printf ("Open NoPec handle %d\n", file_);
  }
#endif
  pec = false;
}

LT_SMBusNoPec::LT_SMBusNoPec(char *dev) : LT_SMBusBase()
{
#if ENABLE_I2C
  //printf("file_ is %d\n", LT_SMBusBase::file_);

  if (!LT_SMBusBase::open_)
    {
      LT_SMBusBase::file_ = open(dev, O_RDWR);

      if (LT_SMBusBase::file_ < 0)
        {
          throw LT_Exception("Fail to open");
        }
      clearBuffer();

      LT_SMBusBase::open_ = true;
      //printf ("Open NoPec device %s handle %d\n", dev, file_);
    }
#endif
  pec = false;
}

LT_SMBusNoPec::LT_SMBusNoPec(uint32_t speed)
{
#if ENABLE_I2C
  if (!LT_SMBusBase::open_)
    {
      LT_SMBusBase::file_ = open("/dev/i2c-0", O_RDWR);

      if (LT_SMBusBase::file_ < 0)
      {
        throw LT_Exception("Fail to open");
      }
      clearBuffer();

      LT_SMBusBase::open_ = true;
      //printf ("Open NoPec handle %d\n", file_);
    }
#endif
  pec = false;
}
