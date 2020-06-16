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

#include "LT_PMBusSpeedTest.h"
    
LT_PMBusSpeedTest::LT_PMBusSpeedTest(LT_PMBus *pmbus):pmbus_(pmbus)
{
}

uint32_t LT_PMBusSpeedTest::test(uint8_t address, uint8_t tries)
{
  bool nok;

  LT_SMBus *smbus = pmbus_->smbus();
  smbus->changeSpeed(400000);
  nok = false;
  for (int i = 0; i < tries; i++)
  {
    pmbus_->setPage(address, 0x00);
    nok |= pmbus_->getPage(address) != 0x00;
    if (nok) break;
    pmbus_->setPage(address, 0xFF);
    nok |= pmbus_->getPage(address) != 0xFF;
    if (nok) break;
  }
  if (!nok) return 400000;

  smbus->changeSpeed(100000);
  nok = false;
  for (int i = 0; i < tries; i++)
  {
    pmbus_->setPage(address, 0x00);
    nok |= pmbus_->getPage(address) != 0x00;
    if (nok) break;
    pmbus_->setPage(address, 0xFF);
    nok |= pmbus_->getPage(address) != 0xFF;
    if (nok) break;
  }
  if (!nok) return 100000;

  // Arduino Mega did not run at 10kHz.
  smbus->changeSpeed(20000);
  nok = false;
  for (int i = 0; i < tries; i++)
  {
    pmbus_->setPage(address, 0x00);
    nok |= pmbus_->getPage(address) != 0x00;
    if (nok) break;
    pmbus_->setPage(address, 0xFF);
    nok |= pmbus_->getPage(address) != 0xFF;
    if (nok) break;
  }
  if (!nok) return 10000;
  return 0;
}
