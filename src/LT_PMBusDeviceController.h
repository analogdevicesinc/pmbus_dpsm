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

#ifndef LT_PMBusDeviceController_H_
#define LT_PMBusDeviceController_H_

#ifdef DMALLOC
#include <dmalloc.h>
#else
#include <stdlib.h>
#endif
#include "LT_PMBusDevice.h"
#include "LT_PMBusRail.h"

class LT_PMBusDeviceController : public LT_PMBusDevice
{
  private:
    uint8_t no_pages_;

  public:

    LT_PMBusDeviceController(LT_PMBus *pmbus, uint8_t address, uint8_t no_pages) : LT_PMBusDevice(pmbus, address), no_pages_(no_pages){}

    LT_PMBusRail **getRails()
    {
      LT_PMBusRail **rails = NULL;
      tRailDef **railDef0 = NULL;
      tRailDef **railDef1 = NULL;
      void *m;
      uint8_t rail_address, last_rail_address;
      uint8_t no_rails = 0;
//    Serial.println("controller get rails ");

      no_rails = 0;
      pmbus_->setPage(address_, 0);
      last_rail_address = (rail_address = pmbus_->getRailAddress(address_));
      if (rail_address == 0x80)
      {
        railDef0 = (tRailDef **)malloc(2*sizeof(tRailDef *));
        railDef0[1] = NULL;
        railDef0[0] = new tRailDef;
        railDef0[0]->address = address_;
        railDef0[0]->pages = (uint8_t *) malloc(1);
        railDef0[0]->pages[0] = 0;
        railDef0[0]->noOfPages = 1;
        railDef0[0]->controller = true;
        railDef0[0]->multiphase = false;
        railDef0[0]->capabilities = getCapabilities();
        last_rail_address = address_;
        no_rails++;
      }
      else
      {
        m = malloc(2*sizeof(tRailDef *));
        if (m == NULL)
          free(m);
        else
        {
          railDef0 = (tRailDef **)m;
          railDef0[1] = NULL;
          railDef0[0] = new tRailDef;
          railDef0[0]->address = address_;
          railDef0[0]->pages = (uint8_t *) malloc(1);
          railDef0[0]->pages[0] = 0;
          railDef0[0]->noOfPages = 1;
          railDef0[0]->controller = true;
          railDef0[0]->multiphase = true;
          railDef0[0]->capabilities = getCapabilities();
        }
        no_rails++;
      }

      if (no_pages_ > 0) // Only handles 1/2 channel controllers
      {
        pmbus_->setPage(address_, 1);
        rail_address = pmbus_->getRailAddress(address_);
        if (rail_address == 0x80)
        {
          m = malloc(2*sizeof(tRailDef *));
          if (m == NULL)
            free(m);
          else
          {
            railDef1 = (tRailDef **)m;
            railDef1[1] = NULL;
            railDef1[0] = new tRailDef;
            railDef1[0]->address = address_;
            railDef1[0]->pages = (uint8_t *) malloc(1);
            railDef1[0]->pages[0] = 1;
            railDef1[0]->noOfPages = 1;
            railDef1[0]->controller = true;
            railDef1[0]->multiphase = false;
            railDef1[0]->capabilities = getCapabilities();
            rail_address = address_;
          }
          no_rails++;
        }
        else
        {
          if (last_rail_address == rail_address) // Both pages in same rail.
          {
            railDef0[0]->pages = (uint8_t *) realloc(railDef0[0]->pages, 2);
            railDef0[0]->pages[1] = 1;
            railDef0[0]->noOfPages = 2;
          }
          else
          {
            m = malloc(2 *sizeof(tRailDef *));
            if (m == NULL)
              free(m);
            else
            {
              railDef1 = (tRailDef **)m;
              railDef1[1] = NULL;
              railDef1[0] = new tRailDef;
              railDef1[0]->address = address_;
              railDef1[0]->pages = (uint8_t *) malloc(1);
              railDef1[0]->pages[0] = 1;
              railDef1[0]->noOfPages = 1;
              railDef1[0]->controller = true;
              railDef1[0]->multiphase = true;
              railDef1[0]->capabilities = getCapabilities();
            }
            no_rails++;
          }
        }
      }

      if (no_rails > 0)
      {
        rails = (LT_PMBusRail **) malloc((no_rails + 1) * sizeof(LT_PMBusRail *));
        rails[0] = new LT_PMBusRail(pmbus_, last_rail_address, railDef0);

        if (no_rails > 1)
        {
          rails[1] = new LT_PMBusRail(pmbus_, rail_address, railDef1); // Will cause problems when freeing.
        }
      }

      rails[no_rails] = NULL;
      return rails;
    }

};

#endif /* LT_PMBusDeviceController_H_ */
