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

#ifndef PMBUSRAIL_H_
#define PMBUSRAIL_H_

#include <stdbool.h>
#include <stdint.h>
#ifdef DMALLOC
#include <dmalloc.h>
#else
#include <stdlib.h>
#endif
#include <string.h>
#include <math.h>
#include "LT_PMBus.h"

typedef struct
{
  uint8_t address; // or char strVal[20];
  uint8_t *pages;
  uint8_t noOfPages;
  bool controller;
  bool multiphase;
  uint32_t capabilities;
} tRailDef;

//! PMBusRail communication. For Multiphase Rails.
class LT_PMBusRail
{
  private:

    LT_PMBus *pmbus_;
    uint8_t railAddress_;
    uint8_t model_[9];

  protected:
    tRailDef **railDef_;

  public:

    //! Construct a LT_PMBus.
    LT_PMBusRail(LT_PMBus *pmbus,     //!< SMBus for communication. Use the PEC or non-PEC version.
                 uint8_t railAddress, //!< The rail address.
                 tRailDef **railDef);  //!< A list of addresses with pages that make up the rail. NULL terminated.

    ~LT_PMBusRail();

    //! Change the pmbus
    void changePMBus(LT_PMBus *pmbus);

    //! Get ther rail address
    uint8_t getAddress();

    //! Ask if devices is a PSM controller
    bool isController ();

    //! Get the number of pages in the rail
    uint8_t getNoPages();

    //! Ask if the rail is multiphase
    bool isMultiphase();

    //! Get a list of capabilities
    uint32_t getCapabilities();

    //! Ask if the rail has the given capability
    uint32_t hasCapability(uint32_t capability);

    //! Merge a rail into this one. No delete of incomming rail.
    void merge(LT_PMBusRail *rail //!< Rail to merge
              );

    //! Set the output voltage of a polyphase rail
    //! @return void
    void setVout(float voltage //!< Rail voltage
                );

    //! Read the input voltage of a polyphase rail
    //! @return voltage
    float readVin(bool polling  //!< true for polling
                 );

    //! Read the output voltage of a polyphase rail
    //! @return voltage
    float readVout(bool polling //!< true for polling
                  );

    //! Read the input current of a polyphase rail
    //! @return current
    float readIin(bool polling  //!< true for polling
                 );

    //! Read the output current of a polyphase rail
    //! @return current
    float readIout(bool polling //!< true for polling
                  );

    //! Read the input power of a polyphase rail
    //! @return current
    float readPin(bool polling //!< true for polling
                 );

    //! Read the output power of a polyphase rail
    //! @return current
    float readPout(bool polling //!< true for polling
                  );

    //! Read the external temperature of a polyphase rail
    //! @return current
    float readExternalTemperature(bool polling //!< true for polling
                                 );

    //! Read the internal temperature of a polyphase rail
    //! @return current
    float readInternalTemperature(bool polling //!< true for polling
                                 );

    //! Read the efficiency (calculated)
    //! @return efficiency or zero if not supported
    float readEfficiency(bool polling //!< true for polling
                        );

    //! Read the average duty cycle
    //! @return duty cycle or zero if not supported
    float readDutyCycle(bool polling //!< true for polling
                       );

    //! Read the phase balance (calculated)
    //! @return phase balance as 100 * (max-min)/total current
    float readPhaseBalance(bool polling //!< true for polling
                          );

    //! Read the transient
    //! @return a value in ms.
    float readTransient(bool polling //!< true for polling
                       );

    //! Read the status word of a polyphase rail
    //! @return word
    uint16_t readStatusWord();

    //! Read the special of a polyphase rail
    //! @return word
    uint16_t getMfrSpecialId();

    //! Clear faults of polyphase rail
    //! @return void
    void clearFaults();

    //! Sequence on a polyphase rail
    //! @return void
    void sequenceOn();

    //! Turn off a polyphase rail
    //! @return void
    void immediateOff();

    //! Sequence off a polyphase rail
    //! @return void
    void sequenceOff();

    //! Margin high polyphase rail
    //! @return void
    void marginHigh();

    //! Margin low polyphase rail
    //! @return void
    void marginLow();

    //! Margin off polyphase rail
    //! @return void
    void marginOff();

};

#endif /* PMBUS_H_ */

