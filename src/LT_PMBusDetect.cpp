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

#include <string.h>
    
#include "LT_PMBusDetect.h"
#include "LT_PMBusDeviceLTC3880.h"
#include "LT_PMBusDeviceLTC3882.h"
#include "LT_PMBusDeviceLTC3883.h"
#include "LT_PMBusDeviceLTC3884.h"
#include "LT_PMBusDeviceLTC3886.h"
#include "LT_PMBusDeviceLTC3887.h"
#include "LT_PMBusDeviceLTC3888.h"
#include "LT_PMBusDeviceLTC3889.h"
#include "LT_PMBusDeviceLTC7880.h"
#include "LT_PMBusDeviceLTM4664.h"
#include "LT_PMBusDeviceLTM4675.h"
#include "LT_PMBusDeviceLTM4676.h"
#include "LT_PMBusDeviceLTM4677.h"
#include "LT_PMBusDeviceLTM4678.h"
#include "LT_PMBusDeviceLTM4680.h"
#include "LT_PMBusDeviceLTM4686.h"
#include "LT_PMBusDeviceLTM4700.h"
#include "LT_PMBusDeviceLTC2972.h"
#include "LT_PMBusDeviceLTC2974.h"
#include "LT_PMBusDeviceLTC2975.h"
#include "LT_PMBusDeviceLTC2977.h"
#include "LT_PMBusDeviceLTC2978.h"
#include "LT_PMBusDeviceLTC2979.h"
#include "LT_PMBusDeviceLTC2980.h"
#include "LT_PMBusDeviceLTM2987.h"

LT_PMBusDetect::LT_PMBusDetect(LT_PMBus *pmbus):pmbus_(pmbus)
{
  devices_ = NULL;
  rails_ = NULL;
  deviceCnt_ = 0;
  railCnt_ = 0;
}

LT_PMBusDetect::~LT_PMBusDetect()
{
  if (deviceCnt_ > 0)
  {
    while (deviceCnt_ > 0)
    {
      deviceCnt_--;
      if (devices_[deviceCnt_] != NULL)
        delete (devices_[deviceCnt_]);
    }
    free(devices_);
  }

  if (railCnt_ > 0)
  {
    while (railCnt_ > 0)
    {
      railCnt_--;
      if (rails_[railCnt_] != NULL)
      {
        delete (rails_[railCnt_]);
      }
    }
    free(rails_);
  }
}

LT_PMBusDevice **LT_PMBusDetect::getDevices(
)
{
  return devices_;
}

LT_PMBusRail **LT_PMBusDetect::getRails(
)
{
  return rails_;
}


void LT_PMBusDetect::detect ()
{
  uint8_t *addresses;
  LT_PMBusDevice *device;
  unsigned int i, j;

  if (deviceCnt_ > 0)
  {
    while (deviceCnt_ > 0)
    {
      deviceCnt_--;
      if (devices_[deviceCnt_] != NULL)
        delete (devices_[deviceCnt_]);
    }
    free(devices_);
  }

  if (railCnt_ > 0)
  {
    while (railCnt_ > 0)
    {
      railCnt_--;
      if (rails_[railCnt_] != NULL)
      {
        delete (rails_[railCnt_]);
      }
    }
    free(rails_);
  }

  addresses = pmbus_->smbus()->probeUnique(0x00);

  // May be more than required. Can add code to trim based on deviceCnt_
  // +1 and calloc so there is a terminating NULL
  devices_ = (LT_PMBusDevice **) calloc(strlen((char *)addresses) + 1, sizeof(LT_PMBusDevice *));

  // Check each device type one by one. Trading generality and composability
  // for performance. For better performance, cache IDs or write decoder here
  // and then call detect on the correct device first time.
  for (i = 0; i < strlen((char *)addresses); i++)
  {
    if ((device = LT_PMBusDeviceLTC3880::detect(pmbus_, addresses[i])) != NULL)
      devices_[deviceCnt_++] = device;
    else if ((device = LT_PMBusDeviceLTC3882::detect(pmbus_, addresses[i])) != NULL)
      devices_[deviceCnt_++] = device;
    else if ((device = LT_PMBusDeviceLTC3883::detect(pmbus_, addresses[i])) != NULL)
      devices_[deviceCnt_++] = device;
    else if ((device = LT_PMBusDeviceLTC3884::detect(pmbus_, addresses[i])) != NULL)
      devices_[deviceCnt_++] = device;
    else if ((device = LT_PMBusDeviceLTC3886::detect(pmbus_, addresses[i])) != NULL)
      devices_[deviceCnt_++] = device;
    else if ((device = LT_PMBusDeviceLTC3887::detect(pmbus_, addresses[i])) != NULL)
      devices_[deviceCnt_++] = device;
    else if ((device = LT_PMBusDeviceLTC3888::detect(pmbus_, addresses[i])) != NULL)
      devices_[deviceCnt_++] = device;
    else if ((device = LT_PMBusDeviceLTC3889::detect(pmbus_, addresses[i])) != NULL)
      devices_[deviceCnt_++] = device;
    else if ((device = LT_PMBusDeviceLTC7880::detect(pmbus_, addresses[i])) != NULL)
      devices_[deviceCnt_++] = device;
    else if ((device = LT_PMBusDeviceLTM4664::detect(pmbus_, addresses[i])) != NULL)
      devices_[deviceCnt_++] = device;
    else if ((device = LT_PMBusDeviceLTM4675::detect(pmbus_, addresses[i])) != NULL)
      devices_[deviceCnt_++] = device;
    else if ((device = LT_PMBusDeviceLTM4676::detect(pmbus_, addresses[i])) != NULL)
      devices_[deviceCnt_++] = device;
    else if ((device = LT_PMBusDeviceLTM4677::detect(pmbus_, addresses[i])) != NULL)
      devices_[deviceCnt_++] = device;
    else if ((device = LT_PMBusDeviceLTM4678::detect(pmbus_, addresses[i])) != NULL)
      devices_[deviceCnt_++] = device;
    else if ((device = LT_PMBusDeviceLTM4680::detect(pmbus_, addresses[i])) != NULL)
      devices_[deviceCnt_++] = device;
    else if ((device = LT_PMBusDeviceLTM4686::detect(pmbus_, addresses[i])) != NULL)
      devices_[deviceCnt_++] = device;
    else if ((device = LT_PMBusDeviceLTM4700::detect(pmbus_, addresses[i])) != NULL)
      devices_[deviceCnt_++] = device;
    else if ((device = LT_PMBusDeviceLTC2972::detect(pmbus_, addresses[i])) != NULL)
      devices_[deviceCnt_++] = device;
    else if ((device = LT_PMBusDeviceLTC2974::detect(pmbus_, addresses[i])) != NULL)
      devices_[deviceCnt_++] = device;
    else if ((device = LT_PMBusDeviceLTC2975::detect(pmbus_, addresses[i])) != NULL)
      devices_[deviceCnt_++] = device;
    else if ((device = LT_PMBusDeviceLTC2977::detect(pmbus_, addresses[i])) != NULL)
      devices_[deviceCnt_++] = device;
    else if ((device = LT_PMBusDeviceLTC2978::detect(pmbus_, addresses[i])) != NULL)
      devices_[deviceCnt_++] = device;
    else if ((device = LT_PMBusDeviceLTC2979::detect(pmbus_, addresses[i])) != NULL)
      devices_[deviceCnt_++] = device;
    else if ((device = LT_PMBusDeviceLTC2980::detect(pmbus_, addresses[i])) != NULL)
      devices_[deviceCnt_++] = device;
    else if ((device = LT_PMBusDeviceLTM2987::detect(pmbus_, addresses[i])) != NULL)
      devices_[deviceCnt_++] = device;
//    else devices_[deviceCnt_++] = NULL;

  }

  // Get all the rails, while merging duplicates.
  for (i = 0; i < deviceCnt_; i++)
  {
    LT_PMBusRail **rails;
    LT_PMBusRail **new_rail;
    void *m;
    bool merged;

    new_rail = rails = devices_[i]->getRails();

    // Loop if device has rail support, and if there are rails to process.
    while (new_rail != NULL && *new_rail != NULL)
    {

      // See if we are already in the list.
      merged = false;
      for (j = 0; j < railCnt_; j++)
      {
        if (rails_[j]->isMultiphase() && rails_[j]->getAddress() == (*new_rail)->getAddress())
        {
          rails_[j]->merge(*new_rail);
          delete (*new_rail);
          merged = true;
          break;
        }
      }
      if (!merged)
      {
        if (rails_ == NULL)
          rails_ = (LT_PMBusRail **) malloc(sizeof(LT_PMBusRail *));
        else
        {
          m = realloc(rails_, (railCnt_ + 1) * sizeof(LT_PMBusRail *));
          if (m != NULL)
//            free(m);
//          else
            rails_ = (LT_PMBusRail **) m;
        }
        if (rails_ != NULL)
          rails_[railCnt_++] = *new_rail;
      }

      new_rail++;
    }
    delete rails;
  }

  rails_ = (LT_PMBusRail **) realloc(rails_, (railCnt_ + 1) * sizeof(LT_PMBusRail *));

  rails_[railCnt_] = NULL;
}