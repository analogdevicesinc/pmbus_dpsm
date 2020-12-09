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
#ifndef LT_CommandPlusFaultLog_H_
#define LT_CommandPlusFaultLog_H_

#include "LT_FaultLog.h"


class LT_CommandPlusFaultLog : public LT_FaultLog
{
  public:

    LT_CommandPlusFaultLog(LT_PMBus *pmbus):LT_FaultLog(pmbus)
    {

    }

    void getNvmBlock(uint8_t address, uint16_t offset, uint16_t numWords, uint8_t command, uint8_t *data) 
    {
      // Get device ready to give data using command plus
      pmbus_->smbus()->writeWord(address, command, 0x00EE);
      while ((pmbus_->smbus()->readByte(address, 0xEF) & 0x60) != 0x60); // Wait until not busy

      // Consume the size word.
      uint16_t w = pmbus_->smbus()->readWord(address, command + 1);
      while ((pmbus_->smbus()->readByte(address, 0xEF) & 0x60) != 0x60); // Wait until not busy

      // Consume words we need to ignore
      for (uint16_t i = 0; i < offset; i++)
      {
        while ((pmbus_->smbus()->readByte(address, 0xEF) & 0x60) != 0x60); // Wait until not busy
        pmbus_->smbus()->readWord(address, command + 1);
      }
      // Consume words of the fault log
      int pos = 0;
      for (uint16_t i = 0; i < numWords; i++)
      {
        while ((pmbus_->smbus()->readByte(address, 0xEF) & 0x60) != 0x60); // Wait until not busy
        w = pmbus_->smbus()->readWord(address, command + 1);
        // Endianess matters here
        data[pos] = 0xFF & w;
        pos++;
        data[pos] = 0xFF & (w >> 8);
        pos++;
      }
    }
};

#endif /* LT_CommandPlusFaultLog_H_ */
