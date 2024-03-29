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

Open Source Licenses
--------------------

This software makes use of additional third-party open-source software,
released under, and subject to, the following license:

Shmoulette / httoi

Download page: https://github.com/christianboutin/Shmoulette/blob/master/src/util/httoi.cpp
Online license: https://github.com/christianboutin/Shmoulette/blob/master/License.md

MIT License

Copyright (c) 2011-2020 Exequor Studios Inc.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

*/

/*! @file
    @ingroup LTPSM_InFlightUpdate
    Library Header File
*/

#ifndef HEX_FILE_PARSER_H_
#define HEX_FILE_PARSER_H_

#ifdef DMALLOC
#include <dmalloc.h>
#else
#include <stdlib.h>
#endif
#include <stdint.h>
#include "httoi.h"

#include "record_type_definitions.h"

extern uint8_t filter_terminations(uint8_t (*get_data)(void));
extern uint8_t detect_colons(uint8_t (*get_data)(void));
extern void reset_parse_hex(void);
extern uint16_t parse_hex_block(char *in_data, uint16_t in_length, uint8_t *out_data);
extern uint8_t parse_hex(uint8_t (*get_data)(void));
extern uint16_t parse_records(uint8_t *in_data, uint16_t in_length, tRecordHeaderLengthAndType **out_records);
extern pRecordHeaderLengthAndType parse_record(uint8_t (*get_data)(void));
extern pRecordHeaderLengthAndType print_record(pRecordHeaderLengthAndType (*get_data)(void));

#endif
