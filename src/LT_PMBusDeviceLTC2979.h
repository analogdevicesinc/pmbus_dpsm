/*
Copyright (c) 2021, Analog Devices Inc
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

#ifndef LT_PMBusDeviceLTC2979_H_
#define LT_PMBusDeviceLTC2979_H_

#include "LT_PMBusDeviceManager.h"
#include "LT_2977FaultLog.h"



class LT_PMBusDeviceLTC2979 : public LT_PMBusDeviceManager
{
  public:

    static uint32_t cap_;

    LT_PMBusDeviceLTC2979(LT_PMBus *pmbus, uint8_t address) : LT_PMBusDeviceManager(pmbus, address, 8)
    {
    }

    void reset()
    {
        pmbus_->restoreFromNvm(address_);
        pmbus_->smbus()->waitForAck(address_, 0x00);
        pmbus_->waitForNotBusy(address_);
    }

    uint32_t getCapabilities (
    )
    {
      return cap_;
    }

    //! Is/are these capability(s) supported?
    //! @return true if yes
    bool hasCapability(
      uint32_t capability          //!< List of capabilities
    )
    {
      return (cap_ & capability) == capability;
    }

    char *getType(void)
    {
      uint8_t *model = (uint8_t *) calloc(8,1);
      memcpy(model, "LTC2979", 7);
      return (char *) model;
    }

    uint8_t getNumPages(void)
    {
      return 8;
    }

    static LT_PMBusDevice *detect(LT_PMBus *pmbus, uint8_t address)
    {
      uint16_t id;
      LT_PMBusDeviceLTC2979 *device;

      id = pmbus->getMfrSpecialId(address);
      if (     (id & 0xFFF0) == 0x8060
            || (id & 0xFFF0) == 0x8070)
      {
        device = new LT_PMBusDeviceLTC2979(pmbus, address);
        device->probeSpeed();
        return device;
      }
      else
        return NULL;
    }

    void enableFaultLog()
    {
      LT_2977FaultLog *faultLog = new LT_2977FaultLog(pmbus_);
      faultLog->enableFaultLog(address_);
      delete faultLog;
    }

    void disableFaultLog()
    {
      LT_2977FaultLog *faultLog = new LT_2977FaultLog(pmbus_);
      faultLog->disableFaultLog(address_);
      delete faultLog;
    }

    bool hasFaultLog()
    {
      LT_2977FaultLog *faultLog = new LT_2977FaultLog(pmbus_);
      if (faultLog->hasFaultLog(address_))
      {
        delete faultLog;
        return true;
      }
      else
      {
        delete faultLog;
        return false;
      }
    }

    char *getFaultLog()
    {
      LT_2977FaultLog *faultLog = new LT_2977FaultLog(pmbus_);
      if (faultLog->hasFaultLog(address_))
      {
        faultLog->read(address_);
//        faultLog->print();
//        faultLog->dumpBinary();
        faultLog->release();
        delete faultLog;
        return NULL;
      }
      else
      {
        delete faultLog;
        return NULL;
      }
    }

    void printFaultLog()
    {
      LT_2977FaultLog *faultLog = new LT_2977FaultLog(pmbus_);
      if (faultLog->hasFaultLog(address_))
      {
        faultLog->read(address_);
        faultLog->print();
//        faultLog->dumpBinary();
        faultLog->release();
        delete faultLog;
      }
    }

    void clearFaultLog()
    {
      LT_2977FaultLog *faultLog = new LT_2977FaultLog(pmbus_);
      if (faultLog->hasFaultLog(address_))
      {
        faultLog->clearFaultLog(address_);
        delete faultLog;
      }
      else
      {
        delete faultLog;
      }
    }

    void storeFaultLog()
    {
      LT_2977FaultLog *faultLog = new LT_2977FaultLog(pmbus_);
      if (!faultLog->hasFaultLog(address_))
      {
        faultLog->storeFaultLog(address_);
        delete faultLog;
      }
      else
      {
        delete faultLog;
      }
    }
};

#endif /* LT_PMBusDeviceLTC2979_H_ */
