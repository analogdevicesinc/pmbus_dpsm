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

#ifndef RECORD_TYPE_BASIC_DEFINITIONS_
#define RECORD_TYPE_BASIC_DEFINITIONS_

#include <stdint.h>

#pragma pack(push, 1)
/** DEFINITIONS ****************************************************/
#define SUCCESS 1
#define FAILURE 0

/** Basic Record Type Definitions **********************************/
typedef struct tRecordHeader
{
  uint16_t Length;
  uint16_t RecordType;
} tRecordHeaderLengthAndType, *pRecordHeaderLengthAndType, * *ppRecordHeaderLengthAndType;

typedef struct
{
  uint16_t DeviceAddress;
  uint8_t CommandCode;
  uint8_t UsePec;
} tRecordHeaderAddressAndCommandWithOptionalPEC;

typedef struct
{
  uint16_t DeviceAddress;
  uint8_t CommandCode;
} tRecordHeaderAddressAndCommandWithoutPEC;
#pragma pack(pop)
#endif
