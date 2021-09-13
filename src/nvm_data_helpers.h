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

#ifndef NVM_DATA_HELPERS_H_
#define NVM_DATA_HELPERS_H_

#ifdef DMALLOC
#include <dmalloc.h>
#else
#include <stdlib.h>
#endif
#include <stdint.h>
#include "LT_PMBus.h"
#include "LT_SMBusNoPec.h"
#include "LT_SMBusPec.h"
#include "record_type_definitions.h"    /* Record Type Definitions */


extern LT_SMBusNoPec *smbusNoPec__;
extern LT_SMBusPec *smbusPec__;

extern nvramNode_p nvramListNew(uint16_t dataIn, uint8_t pecIn, uint8_t addIn, uint8_t cmdIn);
extern void nvramListAdd(uint16_t dataIn, uint8_t pecIn, uint8_t addIn, uint8_t cmdIn, nvramNode_t *nvramList);
extern uint8_t writeNvmData(t_RECORD_NVM_DATA *pRecord);
extern uint8_t bufferNvmData(t_RECORD_NVM_DATA *pRecord);
extern void releaseRecord();
extern uint8_t readThenVerifyNvmData(t_RECORD_NVM_DATA *pRecord);

#endif /* NVM_DATA_HELPERS_H_ */
