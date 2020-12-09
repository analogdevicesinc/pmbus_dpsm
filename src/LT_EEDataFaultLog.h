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

#ifndef LT_EEDataFaultLog_H_
#define LT_EEDataFaultLog_H_

#include "LT_FaultLog.h"


class LT_EEDataFaultLog : public LT_FaultLog
{
  public:

    LT_EEDataFaultLog(LT_PMBus *pmbus):LT_FaultLog(pmbus)
    {

    }

    void getNvmBlock(uint8_t address, uint16_t offset, uint16_t numWords, bool crc16, uint8_t *data) 
    {
      bool pecMatch = true;
      uint16_t v;

      // Tell the device to get ready and set the index at the beginning of EEPROM
      while ((pmbus_->smbus()->readByte(address, 0xEF) & 0x60) != 0x60); // Wait until not busy
      pmbus_->smbus()->writeByte(address, 0xBD, 0x2B);
      while ((pmbus_->smbus()->readByte(address, 0xEF) & 0x60) != 0x60); // Wait until not busy
      pmbus_->smbus()->writeByte(address, 0xBD, 0x91);
      while ((pmbus_->smbus()->readByte(address, 0xEF) & 0x60) != 0x60); // Wait until not busy
      pmbus_->smbus()->writeByte(address, 0xBD, 0xE4);
      // Read the ID
      while ((pmbus_->smbus()->readByte(address, 0xEF) & 0x60) != 0x60); // Wait until not busy
      v = pmbus_->smbus()->readWord(address, 0xBF);
      //printf("Packing Code: 0x%x\n", v);
      // Read the size and ingore it because we know it will not change
      while ((pmbus_->smbus()->readByte(address, 0xEF) & 0x60) != 0x60); // Wait until not busy
      v = pmbus_->smbus()->readWord(address, 0xBF);
      //printf("Size: 0x%x\n", v);
      // Consume words we need to ignore
      for (uint16_t i = 0; i < offset; i++)
      {
        while ((pmbus_->smbus()->readByte(address, 0xEF) & 0x60) != 0x60); // Wait until not busy
        v = pmbus_->smbus()->readWord(address, 0xBF);
        //printf("Ignore: 0x%x\n", v);
      }
      // Consume words of the fault log and check the CRC
      int pos = 0;
      pmbus_->smbus()->pecClear();
      for (uint16_t i = 0; i < numWords; i++)
      {
        while ((pmbus_->smbus()->readByte(address, 0xEF) & 0x60) != 0x60); // Wait until not busy
        uint16_t w = pmbus_->smbus()->readWord(address, 0xBF);
        //printf("Store: 0x%x\n", w);
        // printf("Take %x\n", w);
        // Toss CRC every 32 bytes if doing CRC8
        if (!crc16 && ((i + 1) % 16 == 0))
        {
          data[pos] = 0xFF & w; // If this is not obvious, consider the endianess. This is the 32nd byte.
          pmbus_->smbus()->pecAdd(data[pos]);
          pos++;
          uint8_t calcPec = pmbus_->smbus()->pecGet();
          uint8_t devPec = 0xFF & (w >> 8); // This depends on endianess as well.
          pmbus_->smbus()->pecClear();
          if (calcPec != devPec)
          {
            // printf("PEC mismatch %x %x\n", calcPec, devPec);
            pecMatch = false;
          }
    //            printf("PEC 0x%x 0x%x\n", calcPec, devPec);
        }
        else   // And endianess applies here too.
        {
          data[pos] = 0xFF & w;
          // printf("Save %x\n", data[pos]);
          if (!crc16)
            pmbus_->smbus()->pecAdd(data[pos]);
          pos++;
          data[pos] = 0xFF & (w >> 8);
          // printf("Save %x\n", data[pos]);
          if (!crc16)
            pmbus_->smbus()->pecAdd(data[pos]);
          pos++;
        }
      }

      // It is possible to have a partial good/bad CRC. The device will not load any Fault Log data
      // to RAM if there is ANY CRC mismatch. Then the read Fault Log will return all zero data or
      // random data depending on revision of silicon. This code mimics the most recent silicon
      // and returns all zeros if there is a CRC mismatch.
      // printf("PEC match val %d\n", pecMatch);
      if (!crc16 && !pecMatch)
      //   memset (data, 0x00, numWords*2);

      pmbus_->smbus()->writeByte(address, 0xBD, 0x00);
      while ((pmbus_->smbus()->readByte(address, 0xEF) & 0x60) != 0x60); // Wait until not busy
      //printf("First word: 0x%x\n", data[0]);
    }
};

#endif /* LT_EEDataFaultLog_H_ */
