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

#include "LT_PMBusDevice.h"
    
void LT_PMBusDevice::probeSpeed()
{
  if (address_ == 0)
    maxSpeed_ = 0;
  else
  {
    LT_PMBusSpeedTest *speedTest = new LT_PMBusSpeedTest(pmbus_);
//    maxSpeed_ = speedTest->test(address_, 10);
    maxSpeed_ = 0;
    delete speedTest;
  }
}


LT_PMBus *LT_PMBusDevice::pmbus()
{
  return pmbus_;
}
LT_SMBus *LT_PMBusDevice::smbus()
{
  return pmbus_->smbus();
}

//! Change the pmbus
void LT_PMBusDevice::changePMBus(LT_PMBus *pmbus)
{
  pmbus_ = pmbus;
}


void LT_PMBusDevice::waitForAckNotBusy()
{
    pmbus_->smbus()->waitForAck(address_, 0x00);
    pmbus_->waitForNotBusy(address_);
}

//! Get the address
//! @return address
uint8_t LT_PMBusDevice::getAddress (
)
{
  return address_;
}

char *LT_PMBusDevice::getType(void)
{
  pmbus_->getModel(address_, model_);
  return (char *) model_;
}

void LT_PMBusDevice::setPage(uint8_t page)
{
  pmbus_->setPage(address_, page);
}

uint32_t LT_PMBusDevice::getMaxSpeed(void)
{
  return maxSpeed_;
}

void LT_PMBusDevice::setSpeed(uint32_t speed        //!< Speed
                     )
{
  uint8_t configAll;

  if (speed > 100000)
  {
    configAll = pmbus_->smbus()->readByte(address_, 0xD1);
    configAll |= 1 << 1;
    pmbus_->smbus()->writeByte(address_, 0xD1, configAll);
  }
  else
  {
    configAll = pmbus_->smbus()->readByte(address_, 0xD1);
    configAll &= ~(1 << 1);
    pmbus_->smbus()->writeByte(address_, 0xD1, configAll);
  }
 // pmbus_->smbus()->i2cbus()->changeSpeed(speed);
}

LT_PMBusRail **LT_PMBusDevice::getRails()
{
  return NULL;
}

void LT_PMBusDevice::enablePec()
{
  pmbus_->enablePec(address_);
}

void LT_PMBusDevice::disablePec()
{
  pmbus_->disablePec(address_);
}

bool LT_PMBusDevice::hasFaultLog()
{
  return false;
}

char *LT_PMBusDevice::getFaultLog()
{
  return NULL;
}

void LT_PMBusDevice::setVout(float voltage)
{
  if (hasCapability(HAS_VOUT))
    pmbus_->setVout(address_, voltage);
}

float LT_PMBusDevice::readVin(bool polling)
{
  if (hasCapability(HAS_VIN))
    return pmbus_->readVin(address_, polling);
  return 0.0;
}

float LT_PMBusDevice::readVout(bool polling)
{
  if (hasCapability(HAS_VOUT))
    return pmbus_->readVout(address_, polling);
  return 0.0;
}

float LT_PMBusDevice::readIin(bool polling)
{
  float current = 0.0;

  if (hasCapability(HAS_IIN))
    current = pmbus_->readIin(address_, polling);

  return current;
}

float LT_PMBusDevice::readIout(bool polling)
{
  float current = 0.0;

  if (hasCapability(HAS_IOUT))
    current = pmbus_->readIout(address_, polling);

  return current;
}

float LT_PMBusDevice::readPin(bool polling)
{
  float power = 0.0;

  if (hasCapability(HAS_PIN))
    power = pmbus_->readPin(address_, polling);

  return power;
}

float LT_PMBusDevice::readPout(bool polling)
{
  float power = 0.0;

  if (hasCapability(HAS_POUT))
    power = pmbus_->readPout(address_, polling);

  return power;
}

float LT_PMBusDevice::readExternalTemperature(bool polling)
{
  float temp = 0.0;

  if (hasCapability(HAS_TEMP))
    temp = pmbus_->readExternalTemperature(address_, polling);

  return temp;
}

float LT_PMBusDevice::readInternalTemperature(bool polling)
{
  float temp = 0.0;

  if (hasCapability(HAS_TEMP))
    temp = pmbus_->readInternalTemperature(address_, polling);

  return temp;
}

float LT_PMBusDevice::readDutyCycle(bool polling)
{
  float dc = 0.0;

  if (hasCapability(HAS_DC))
    dc = pmbus_->readDutyCycle(address_, polling);

  return dc;
}

uint16_t LT_PMBusDevice::readStatusWord()
{
  uint16_t sw = 0;

  if (hasCapability(HAS_STATUS_WORD))
    sw = pmbus_->readStatusWord(address_);

  return sw;
}

uint16_t LT_PMBusDevice::getMfrSpecialId()
{
  uint16_t id = 0;

  id = pmbus_->getMfrSpecialId(address_);

  return id;
}

void LT_PMBusDevice::clearFaults()
{
  pmbus_->clearAllFaults(address_);
}

void LT_PMBusDevice::marginHigh()
{
  pmbus_->marginHigh(address_);
}

void LT_PMBusDevice::marginLow()
{
  pmbus_->marginLow(address_);
}

void LT_PMBusDevice::marginOff()
{
  pmbus_->marginOff(address_);
}
