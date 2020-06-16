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

#ifndef LT_PMBusDeviceManager_H_
#define LT_PMBusDeviceManager_H_

#ifndef DMALLOC
#include <stdlib.h>
#else
#include <dmalloc.h>
#endif
#include "LT_PMBusDevice.h"
#include "LT_PMBusRail.h"

class LT_PMBusDeviceManager : public LT_PMBusDevice
{
  private:
    uint8_t no_pages_;

  public:

    LT_PMBusDeviceManager(LT_PMBus *pmbus, uint8_t address, uint8_t no_pages) : LT_PMBusDevice(pmbus, address), no_pages_(no_pages) {}

    void setSpeed(uint32_t speed) 
    {
      //pmbus_->smbus()->i2cbus()->changeSpeed(speed);
    }
    
    LT_PMBusRail **getRails()
    {
      LT_PMBusRail **rails = NULL;
      tRailDef **railDef = NULL;
      uint8_t i;
      rails = (LT_PMBusRail **) malloc((no_pages_ + 1) * sizeof(LT_PMBusRail *));

      for (i = 0; i < no_pages_; i++)
      {
        railDef = (tRailDef **)malloc((no_pages_ + 1) * sizeof(tRailDef *));
        railDef[0] = new tRailDef;
        railDef[0]->address = address_;
        railDef[0]->pages = (uint8_t *) malloc(sizeof(uint8_t));
        railDef[0]->pages[0] = i;
        railDef[0]->noOfPages = 1;
        railDef[0]->controller = false;
        railDef[0]->multiphase = false;
        railDef[0]->capabilities = getCapabilities();
        railDef[1] = NULL;
        rails[i] = new LT_PMBusRail(pmbus_, railDef[0]->address, railDef);
      }

      rails[no_pages_] = NULL;
      return rails;
    }

};

#endif /* LT_PMBusDeviceManager_H_ */
