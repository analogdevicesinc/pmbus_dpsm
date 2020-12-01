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
    Library File
*/

#include "hex_file_parser.h"
#include <stdio.h>
#include <string.h>
    
#undef F
#define F(s) s

uint8_t ascii_to_hex(uint8_t ascii)
{
  return 0;
}

uint8_t filter_terminations(uint8_t (*get_data)(void))
{
  uint8_t c = '\0';

  c = get_data();
  while (c == '\n' || c == '\r')
    c = get_data();
  return c;
}

uint8_t detect_colons(uint8_t (*get_data)(void))
{
  uint8_t c;

  c = get_data();
  if (c == ':')
  {
    printf(".");
  }
  return c;
}

/*
 * Parse a list of hex file bytes returning a list of ltc record bytes
 *
 * in_data:   buffer of hex data
 * in_length: length of the hex data
 * out_data:  preallocated buffer where the output data will be put
 * return:    actual size of the output data
 *
 * Notes:     CRC is ignored. No error processing; original data must be correct.
 *        Copies char input data so that it can be const, and frees when done.
 *        It is best to use const input so that memory usage is only temporary.
 */
uint16_t parse_hex_block(char *in_data, uint16_t in_length, uint8_t *out_data)
{
  uint16_t    in_position;
  uint16_t    out_position;
  uint8_t     start_code;
  uint16_t    byte_count;
  uint16_t    record_type;
  uint32_t    address;
  uint16_t    crc;
  uint8_t     save;
  uint16_t    i;

  in_position = 0;
  out_position = 0;

  while (1)
  {
    // Get colon
    start_code = in_data[in_position];
    in_position += 1;

    save = in_data[in_position+2];
    in_data[in_position+2] = '\0';
    byte_count = httoi(&in_data[in_position]);
    in_data[in_position+2] = save;
    in_position += 2;

    save = in_data[in_position+4];
    in_data[in_position+4] = '\0';
    address = httoi(&in_data[in_position]);
    in_data[in_position+4] = save;
    in_position += 4;

    save = in_data[in_position+2];
    in_data[in_position+2] = '\0';
    record_type = httoi(&in_data[in_position]);
    in_data[in_position+2] = save;
    in_position += 2;


    for (i = 0; i < byte_count; i++)
    {
      save = in_data[in_position+2];
      in_data[in_position+2] = '\0';
      out_data[out_position] = httoi(&in_data[in_position]);
      in_data[in_position+2] = save;
      in_position += 2;

      out_position += 1;
    }


    save = in_data[in_position+2];
    in_data[in_position+2] = '\0';
    crc = httoi(&in_data[in_position]);
    in_data[in_position+2] = save;
    in_position += 2;

    if (in_position >= in_length)
      break;
  }
  return out_position;
}

static uint8_t *parse_data = NULL;
static uint16_t parse_data_length = 0;
static uint16_t parse_data_position = 0;

void reset_parse_hex()
{
  if (parse_data != NULL)
    free(parse_data);
  parse_data = NULL;
  parse_data_length = 0;
  parse_data_position = 0;
}

/*
 * Parse a list of hex file bytes returning a list of ltc record bytes
 *
 * in_data:   Function to get one line of data to parse.
 * return:    One byte of data
 *
 * Notes:     CRC is ignored. No error processing; original data must be correct.
 *        Copies char input data so that it can be const, and frees when done.
 *        It is best to use const input so that memory usage is only temporary.
 */
uint8_t parse_hex(uint8_t (*get_data)(void))
{
  uint16_t    out_position;
  uint8_t     start_code;
  uint16_t    byte_count;
  uint16_t    record_type;
  uint32_t    address;
  uint16_t    crc;
  uint8_t     save;
  uint16_t    i;
  char      data[5];

  if (parse_data_position == parse_data_length)
  {
    if (parse_data != NULL)
      free(parse_data);

    // Get colon
    start_code = 0x00;
    while (start_code != ':')
      start_code = get_data();

    data[0] = get_data();
    data[1] = get_data();
    data[2] = '\0';
    byte_count = httoi(data);

    data[0] = get_data();
    data[1] = get_data();
    data[2] = get_data();
    data[3] = get_data();
    data[4] = '\0';
    address = httoi(data);

    data[0] = get_data();
    data[1] = get_data();
    data[2] = '\0';
    record_type = httoi(data);

    if (record_type == 0)
    {
      parse_data = (uint8_t *)malloc(byte_count);
      for (i = 0; i < byte_count; i++)
      {
        data[0] = get_data();
        data[1] = get_data();
        data[2] = '\0';
        parse_data[i] = httoi(data);
      }

      data[0] = get_data();
      data[1] = get_data();
      data[2] = '\0';
      crc = httoi(data);

      parse_data_position = 0;
      parse_data_length = byte_count;
    }
    else if (record_type == 1)
    {
      parse_data = (uint8_t *)malloc(8);
      parse_data[0] = 4;
      parse_data[1] = 0;
      parse_data[2] = 0x22;
      parse_data[3] = 0;
      parse_data[4] = 0;
      parse_data[5] = 0;
      parse_data[6] = 0;
      parse_data[7] = 0;
      parse_data_position = 0;
      parse_data_length = 8;
    }
  }
  return parse_data[parse_data_position++];
}

uint16_t parse_records(uint8_t *in_data, uint16_t in_length, tRecordHeaderLengthAndType **out_records)
{
  uint16_t    in_position;
  uint16_t  out_position;
  tRecordHeaderLengthAndType *record;

  in_position = 0;
  out_position = 0;

  while (1)
  {
    record = (tRecordHeaderLengthAndType *) &in_data[in_position];
    *(out_records+out_position) = record;
    in_position += (*out_records[out_position]).Length;
    out_position += 1;

    if (in_position >= in_length)
      break;
  }

  return out_position;
}

pRecordHeaderLengthAndType parse_record(uint8_t (*get_data)(void))
{
  uint8_t   *data;
  uint16_t  pos;
  tRecordHeaderLengthAndType recordA;
  pRecordHeaderLengthAndType recordB;

  // Build a header record only
  recordA.Length = (uint16_t)get_data() | (uint16_t)get_data() << 8;
  recordA.RecordType = (uint16_t)get_data() | (uint16_t)get_data() << 8;

//  printf("Rec A: %d, 0x%x\n", recordA.Length, recordA.RecordType);

  // Allocate enough memory for the whole record
  data = (uint8_t *)malloc(recordA.Length);

  // Copy the header portion of the data
  memcpy(data, &recordA, sizeof(tRecordHeaderLengthAndType));

  // Cast to the pointer for return
  recordB = (pRecordHeaderLengthAndType) data;

  //printf("Rec B1: %d, 0x%x\n", recordB->Length, recordB->RecordType);

  // Fill in the record
  switch (recordB->RecordType)
  {
    case RECORD_TYPE_PMBUS_WRITE_BYTE :
      ((t_RECORD_PMBUS_WRITE_BYTE *)recordB)->detailedRecordHeader.DeviceAddress = (uint16_t)get_data() | (uint16_t)get_data() << 8;
      ((t_RECORD_PMBUS_WRITE_BYTE *)recordB)->detailedRecordHeader.CommandCode = (uint8_t)get_data();
      ((t_RECORD_PMBUS_WRITE_BYTE *)recordB)->detailedRecordHeader.UsePec = (uint8_t)get_data();
      ((t_RECORD_PMBUS_WRITE_BYTE *)recordB)->dataByte = (uint8_t)get_data();
      break;
    case RECORD_TYPE_PMBUS_WRITE_WORD :
      ((t_RECORD_PMBUS_WRITE_WORD *)recordB)->detailedRecordHeader.DeviceAddress = (uint16_t)get_data() | (uint16_t)get_data() << 8;
      ((t_RECORD_PMBUS_WRITE_WORD *)recordB)->detailedRecordHeader.CommandCode = (uint8_t)get_data();
      ((t_RECORD_PMBUS_WRITE_WORD *)recordB)->detailedRecordHeader.UsePec = (uint8_t)get_data();
      ((t_RECORD_PMBUS_WRITE_WORD *)recordB)->dataWord = (uint16_t)get_data() | (uint16_t)get_data() << 8;
      break;
    case RECORD_TYPE_PMBUS_READ_BYTE_EXPECT :
      ((t_RECORD_PMBUS_READ_BYTE_EXPECT *)recordB)->detailedRecordHeader.DeviceAddress = (uint16_t)get_data() | (uint16_t)get_data() << 8;
      ((t_RECORD_PMBUS_READ_BYTE_EXPECT *)recordB)->detailedRecordHeader.CommandCode = (uint8_t)get_data();
      ((t_RECORD_PMBUS_READ_BYTE_EXPECT *)recordB)->detailedRecordHeader.UsePec = (uint8_t)get_data();
      ((t_RECORD_PMBUS_READ_BYTE_EXPECT *)recordB)->expectedDataByte = (uint8_t) get_data();
      break;
    case RECORD_TYPE_PMBUS_READ_WORD_EXPECT :
      ((t_RECORD_PMBUS_READ_WORD_EXPECT *)recordB)->detailedRecordHeader.DeviceAddress = (uint16_t)get_data() | (uint16_t)get_data() << 8;
      ((t_RECORD_PMBUS_READ_WORD_EXPECT *)recordB)->detailedRecordHeader.CommandCode = (uint8_t)get_data();
      ((t_RECORD_PMBUS_READ_WORD_EXPECT *)recordB)->detailedRecordHeader.UsePec = (uint8_t)get_data();
      ((t_RECORD_PMBUS_READ_WORD_EXPECT *)recordB)->expectedDataWord = (uint16_t) get_data() | (uint16_t)get_data() << 8;
      break;
    case RECORD_TYPE_PACKING_CODE :
      ((t_RECORD_PACKING_CODE *)recordB)->packingCode = (uint16_t) get_data() | (uint16_t)get_data() << 8;
      break;
    case RECORD_TYPE_NVM_DATA :
      ((t_RECORD_NVM_DATA *)recordB)->detailedRecordHeader.DeviceAddress = (uint16_t)get_data() | (uint16_t)get_data() << 8;
      ((t_RECORD_NVM_DATA *)recordB)->detailedRecordHeader.CommandCode = (uint8_t)get_data();
      ((t_RECORD_NVM_DATA *)recordB)->detailedRecordHeader.UsePec = (uint8_t)get_data();
      for (pos = 0; pos < recordA.Length - sizeof(tRecordHeaderLengthAndType) - sizeof(tRecordHeaderAddressAndCommandWithOptionalPEC); pos++)
        data[pos + sizeof(tRecordHeaderLengthAndType) + sizeof(tRecordHeaderAddressAndCommandWithOptionalPEC)] = get_data();
      break;
    case RECORD_TYPE_PMBUS_READ_BYTE_LOOP_MASK :
      ((t_RECORD_PMBUS_READ_BYTE_LOOP_MASK *)recordB)->detailedRecordHeader.DeviceAddress = (uint16_t)get_data() | (uint16_t)get_data() << 8;
      ((t_RECORD_PMBUS_READ_BYTE_LOOP_MASK *)recordB)->detailedRecordHeader.CommandCode = (uint8_t)get_data();
      ((t_RECORD_PMBUS_READ_BYTE_LOOP_MASK *)recordB)->detailedRecordHeader.UsePec = (uint8_t)get_data();
      ((t_RECORD_PMBUS_READ_BYTE_LOOP_MASK *)recordB)->byteMask = (uint8_t)get_data();
      ((t_RECORD_PMBUS_READ_BYTE_LOOP_MASK *)recordB)->expectedDataByte = (uint8_t)get_data();
      break;
    case RECORD_TYPE_PMBUS_READ_WORD_LOOP_MASK :
      ((t_RECORD_PMBUS_READ_WORD_LOOP_MASK *)recordB)->detailedRecordHeader.DeviceAddress = (uint16_t)get_data() | (uint16_t)get_data() << 8;
      ((t_RECORD_PMBUS_READ_WORD_LOOP_MASK *)recordB)->detailedRecordHeader.CommandCode = (uint8_t)get_data();
      ((t_RECORD_PMBUS_READ_WORD_LOOP_MASK *)recordB)->detailedRecordHeader.UsePec = (uint8_t)get_data();
      ((t_RECORD_PMBUS_READ_WORD_LOOP_MASK *)recordB)->wordMask = (uint16_t)get_data() | (uint16_t)get_data() << 8;
      ((t_RECORD_PMBUS_READ_WORD_LOOP_MASK *)recordB)->expectedDataWord = (uint16_t)get_data() | (uint16_t)get_data() << 8;
      break;
    case RECORD_TYPE_PMBUS_POLL_UNTIL_ACK_NOPEC :
      ((t_RECORD_PMBUS_POLL_READ_BYTE_UNTIL_ACK *)recordB)->detailedRecordHeader.DeviceAddress = (uint16_t)get_data() | (uint16_t)get_data() << 8;
      ((t_RECORD_PMBUS_POLL_READ_BYTE_UNTIL_ACK *)recordB)->detailedRecordHeader.CommandCode = (uint8_t)get_data();
      ((t_RECORD_PMBUS_POLL_READ_BYTE_UNTIL_ACK *)recordB)->timeout_in_ms = (uint16_t)get_data() | (uint16_t)get_data() << 8;
      break;
    case RECORD_TYPE_DELAY_MS :
      ((t_RECORD_DELAY_MS *)recordB)->numMs = (uint16_t) get_data() | (uint16_t)get_data() << 8;
      break;
    case RECORD_TYPE_PMBUS_SEND_BYTE :
      ((t_RECORD_PMBUS_SEND_BYTE *)recordB)->detailedRecordHeader.DeviceAddress = (uint16_t)get_data() | (uint16_t)get_data() << 8;
      ((t_RECORD_PMBUS_SEND_BYTE *)recordB)->detailedRecordHeader.CommandCode = (uint8_t)get_data();
      ((t_RECORD_PMBUS_SEND_BYTE *)recordB)->detailedRecordHeader.UsePec = (uint8_t)get_data();
      break;
    case RECORD_TYPE_PMBUS_WRITE_BYTE_NOPEC :
      ((t_RECORD_PMBUS_WRITE_BYTE_NOPEC *)recordB)->detailedRecordHeader.DeviceAddress = (uint16_t)get_data() | (uint16_t)get_data() << 8;
      ((t_RECORD_PMBUS_WRITE_BYTE_NOPEC *)recordB)->detailedRecordHeader.CommandCode = (uint8_t)get_data();
      ((t_RECORD_PMBUS_WRITE_BYTE_NOPEC *)recordB)->dataByte = (uint8_t)get_data();
      break;
    case RECORD_TYPE_PMBUS_WRITE_WORD_NOPEC :
      ((t_RECORD_PMBUS_WRITE_WORD_NOPEC *)recordB)->detailedRecordHeader.DeviceAddress = (uint16_t)get_data() | (uint16_t)get_data() << 8;
      ((t_RECORD_PMBUS_WRITE_WORD_NOPEC *)recordB)->detailedRecordHeader.CommandCode = (uint8_t)get_data();
      ((t_RECORD_PMBUS_WRITE_WORD_NOPEC *)recordB)->dataWord = (uint16_t)get_data() | (uint16_t)get_data() << 8;
      break;
    case RECORD_TYPE_PMBUS_READ_BYTE_EXPECT_NOPEC :
      ((t_RECORD_PMBUS_READ_BYTE_EXPECT_NOPEC *)recordB)->detailedRecordHeader.DeviceAddress = (uint16_t)get_data() | (uint16_t)get_data() << 8;
      ((t_RECORD_PMBUS_READ_BYTE_EXPECT_NOPEC *)recordB)->detailedRecordHeader.CommandCode = (uint8_t)get_data();
      ((t_RECORD_PMBUS_READ_BYTE_EXPECT_NOPEC *)recordB)->expectedDataByte = (uint8_t) get_data();
      break;
    case RECORD_TYPE_PMBUS_READ_WORD_EXPECT_NOPEC :
      ((t_RECORD_PMBUS_READ_WORD_EXPECT_NOPEC *)recordB)->detailedRecordHeader.DeviceAddress = (uint16_t)get_data() | (uint16_t)get_data() << 8;
      ((t_RECORD_PMBUS_READ_WORD_EXPECT_NOPEC *)recordB)->detailedRecordHeader.CommandCode = (uint8_t)get_data();
      ((t_RECORD_PMBUS_READ_WORD_EXPECT_NOPEC *)recordB)->expectedDataWord = (uint16_t) get_data() | (uint16_t)get_data() << 8;
      break;
    case RECORD_TYPE_PMBUS_READ_BYTE_LOOP_MASK_NOPEC :
      ((t_RECORD_PMBUS_READ_BYTE_LOOP_MASK_NOPEC *)recordB)->detailedRecordHeader.DeviceAddress = (uint16_t)get_data() | (uint16_t)get_data() << 8;
      ((t_RECORD_PMBUS_READ_BYTE_LOOP_MASK_NOPEC *)recordB)->detailedRecordHeader.CommandCode = (uint8_t)get_data();
      ((t_RECORD_PMBUS_READ_BYTE_LOOP_MASK_NOPEC *)recordB)->byteMask = (uint8_t)get_data();
      ((t_RECORD_PMBUS_READ_BYTE_LOOP_MASK_NOPEC *)recordB)->expectedDataByte = (uint8_t)get_data();
      break;
    case RECORD_TYPE_PMBUS_READ_WORD_LOOP_MASK_NOPEC :
      ((t_RECORD_PMBUS_READ_WORD_LOOP_MASK_NOPEC *)recordB)->detailedRecordHeader.DeviceAddress = (uint16_t)get_data() | (uint16_t)get_data() << 8;
      ((t_RECORD_PMBUS_READ_WORD_LOOP_MASK_NOPEC *)recordB)->detailedRecordHeader.CommandCode = (uint8_t)get_data();
      ((t_RECORD_PMBUS_READ_WORD_LOOP_MASK_NOPEC *)recordB)->wordMask = (uint16_t)get_data() | (uint16_t)get_data() << 8;
      ((t_RECORD_PMBUS_READ_WORD_LOOP_MASK_NOPEC *)recordB)->expectedDataWord = (uint16_t)get_data() | (uint16_t)get_data() << 8;
      break;
    case RECORD_TYPE_PMBUS_SEND_BYTE_NOPEC :
      ((t_RECORD_PMBUS_SEND_BYTE_NOPEC *)recordB)->detailedRecordHeader.DeviceAddress = (uint16_t)get_data() | (uint16_t)get_data() << 8;
      ((t_RECORD_PMBUS_SEND_BYTE_NOPEC *)recordB)->detailedRecordHeader.CommandCode = (uint8_t)get_data();
      break;
    case RECORD_TYPE_EVENT :
      ((t_RECORD_EVENT *)recordB)->eventId = (uint16_t) get_data() | (uint16_t)get_data() << 8;
      break;
    case RECORD_TYPE_PMBUS_READ_BYTE_EXPECT_MASK_NOPEC :
      ((t_RECORD_PMBUS_READ_BYTE_EXPECT_MASK_NOPEC *)recordB)->detailedRecordHeader.DeviceAddress = (uint16_t)get_data() | (uint16_t)get_data() << 8;
      ((t_RECORD_PMBUS_READ_BYTE_EXPECT_MASK_NOPEC *)recordB)->detailedRecordHeader.CommandCode = (uint8_t)get_data();
      ((t_RECORD_PMBUS_READ_BYTE_EXPECT_MASK_NOPEC *)recordB)->byteMask = (uint8_t)get_data();
      ((t_RECORD_PMBUS_READ_BYTE_EXPECT_MASK_NOPEC *)recordB)->expectedDataByte = (uint8_t)get_data();
      break;
    case RECORD_TYPE_PMBUS_READ_WORD_EXPECT_MASK_NOPEC :
      ((t_RECORD_PMBUS_READ_WORD_EXPECT_MASK_NOPEC *)recordB)->detailedRecordHeader.DeviceAddress = (uint16_t)get_data() | (uint16_t)get_data() << 8;
      ((t_RECORD_PMBUS_READ_WORD_EXPECT_MASK_NOPEC *)recordB)->detailedRecordHeader.CommandCode = (uint8_t)get_data();
      ((t_RECORD_PMBUS_READ_WORD_EXPECT_MASK_NOPEC *)recordB)->expectedDataWord = (uint16_t)get_data() | (uint16_t)get_data() << 8;
      ((t_RECORD_PMBUS_READ_WORD_EXPECT_MASK_NOPEC *)recordB)->wordMask = (uint16_t)get_data() | (uint16_t)get_data() << 8;
      break;
    case RECORD_TYPE_VARIABLE_META_DATA :
      ((t_RECORD_VARIABLE_META_DATA *)recordB)->metaDataType = (uint16_t) get_data() | (uint16_t)get_data() << 8;
      //printf("Meta size %d\n", ((t_RECORD_VARIABLE_META_DATA *)recordB)->metaDataType);
      if (((t_RECORD_VARIABLE_META_DATA *)recordB)->metaDataType == 0)
        ((t_RECORD_META_SET_GLOBAL_BASE_ADDRESS *)recordB)->globalBaseAddressInWordFormat = (uint16_t)get_data() | (uint16_t)get_data() << 8;
      else if (((t_RECORD_VARIABLE_META_DATA *)recordB)->metaDataType == 1)
        ((t_RECORD_META_OEM_SERIAL_NUMBER *)recordB)->serialNumber = (uint16_t)get_data() | (uint16_t)get_data() << 8;
      else
      {
        get_data();
        get_data();
      }
      break;
    case META_SET_GLOBAL_BASE_ADDRESS :
      ((t_RECORD_META_SET_GLOBAL_BASE_ADDRESS *)recordB)->metaHeader.metaDataType = (uint16_t) get_data() | (uint16_t)get_data() << 8;
      ((t_RECORD_META_SET_GLOBAL_BASE_ADDRESS *)recordB)->globalBaseAddressInWordFormat = (uint16_t) get_data() | (uint16_t)get_data() << 8;
      break;
    case RECORD_TYPE_MODIFY_WORD_NOPEC :
      ((t_RECORD_PMBUS_MODIFY_WORD_NO_PEC *)recordB)->detailedRecordHeader.DeviceAddress = (uint16_t)get_data() | (uint16_t)get_data() << 8;
      ((t_RECORD_PMBUS_MODIFY_WORD_NO_PEC *)recordB)->detailedRecordHeader.CommandCode = (uint8_t)get_data();
      ((t_RECORD_PMBUS_MODIFY_WORD_NO_PEC *)recordB)->wordMask = (uint16_t)get_data() | (uint16_t)get_data() << 8;
      ((t_RECORD_PMBUS_MODIFY_WORD_NO_PEC *)recordB)->desiredDataWord = (uint16_t)get_data() | (uint16_t)get_data() << 8;
      break;
    case RECORD_TYPE_MODIFY_BYTE_NOPEC :
      ((t_RECORD_PMBUS_MODIFY_BYTE_NO_PEC *)recordB)->detailedRecordHeader.DeviceAddress = (uint16_t)get_data() | (uint16_t)get_data() << 8;
      ((t_RECORD_PMBUS_MODIFY_BYTE_NO_PEC *)recordB)->detailedRecordHeader.CommandCode = (uint8_t)get_data();
      ((t_RECORD_PMBUS_MODIFY_BYTE_NO_PEC *)recordB)->byteMask = (uint8_t)get_data();
      ((t_RECORD_PMBUS_MODIFY_BYTE_NO_PEC *)recordB)->desiredDataByte = (uint8_t)get_data();
      break;
    case RECORD_TYPE_PMBUS_WRITE_EE_DATA :
      ((t_RECORD_TYPE_PMBUS_WRITE_EE_DATA *)recordB)->detailedRecordHeader.DeviceAddress = (uint16_t)get_data() | (uint16_t)get_data() << 8;
      ((t_RECORD_TYPE_PMBUS_WRITE_EE_DATA *)recordB)->detailedRecordHeader.CommandCode = (uint8_t)get_data();
      ((t_RECORD_TYPE_PMBUS_WRITE_EE_DATA *)recordB)->detailedRecordHeader.UsePec = (uint8_t)get_data();
      break;
    case RECORD_TYPE_PMBUS_READ_AND_VERIFY_EE_DATA :
      ((t_RECORD_TYPE_PMBUS_VERIFY_EE_DATA *)recordB)->detailedRecordHeader.DeviceAddress = (uint16_t)get_data() | (uint16_t)get_data() << 8;
      ((t_RECORD_TYPE_PMBUS_VERIFY_EE_DATA *)recordB)->detailedRecordHeader.CommandCode = (uint8_t)get_data();
      ((t_RECORD_TYPE_PMBUS_VERIFY_EE_DATA *)recordB)->detailedRecordHeader.UsePec = (uint8_t)get_data();
      break;
    case RECORD_TYPE_PMBUS_MODIFY_BYTE :
      ((t_RECORD_PMBUS_MODIFY_BYTE *)recordB)->detailedRecordHeader.DeviceAddress = (uint16_t)get_data() | (uint16_t)get_data() << 8;
      ((t_RECORD_PMBUS_MODIFY_BYTE *)recordB)->detailedRecordHeader.CommandCode = (uint8_t)get_data();
      ((t_RECORD_PMBUS_MODIFY_BYTE *)recordB)->detailedRecordHeader.UsePec = (uint8_t)get_data();
      ((t_RECORD_PMBUS_MODIFY_BYTE *)recordB)->byteMask = (uint8_t)get_data();
      ((t_RECORD_PMBUS_MODIFY_BYTE *)recordB)->desiredDataByte = (uint8_t)get_data();
      break;
    case RECORD_TYPE_PMBUS_MODIFY_WORD :
      ((t_RECORD_PMBUS_MODIFY_WORD *)recordB)->detailedRecordHeader.DeviceAddress = (uint16_t)get_data() | (uint16_t)get_data() << 8;
      ((t_RECORD_PMBUS_MODIFY_WORD *)recordB)->detailedRecordHeader.CommandCode = (uint8_t)get_data();
      ((t_RECORD_PMBUS_MODIFY_WORD *)recordB)->detailedRecordHeader.UsePec = (uint8_t)get_data();
      ((t_RECORD_PMBUS_MODIFY_WORD *)recordB)->wordMask = (uint16_t)get_data() | (uint16_t)get_data() << 8;
      ((t_RECORD_PMBUS_MODIFY_WORD *)recordB)->desiredDataWord = (uint16_t)get_data() | (uint16_t)get_data() << 8;
      break;
    case RECORD_TYPE_END_OF_RECORDS :
      ((t_RECORD_TYPE_END_OF_RECORDS *)recordB)->detailedRecordHeader.DeviceAddress = (uint16_t)get_data() | (uint16_t)get_data() << 8;
      ((t_RECORD_TYPE_END_OF_RECORDS *)recordB)->detailedRecordHeader.CommandCode = (uint8_t)get_data();
      ((t_RECORD_TYPE_END_OF_RECORDS *)recordB)->detailedRecordHeader.UsePec = (uint8_t)get_data();
      break;

    default :
      // Fill in the remaining data (should not get here)
      for (pos = 0; pos < recordB->Length - sizeof(tRecordHeaderLengthAndType); pos++)
        data[pos + sizeof(tRecordHeaderLengthAndType)] = get_data();
  }
//  printf("Create Rec: %d, 0x%x\n", recordB->Length, recordB->RecordType);

  return recordB;
}

pRecordHeaderLengthAndType print_record(pRecordHeaderLengthAndType (*get_record)(void))
{
  pRecordHeaderLengthAndType record = get_record();

  switch (record->RecordType)
  {
    case 0x01:
      printf(F("processWriteByteOptionalPEC(t_RECORD_PMBUS_WRITE_BYTE*);\n"));
      break;
    case 0x02:
      printf(F("processWriteWordOptionalPEC(t_RECORD_PMBUS_WRITE_WORD*);\n"));
      break;
    case 0x04:
      printf(F("processReadByteExpectOptionalPEC(t_RECORD_PMBUS_READ_BYTE_EXPECT*);\n"));
      break;
    case 0x05:
      printf(F("processReadWordExpectOptionalPEC(t_RECORD_PMBUS_READ_WORD_EXPECT*);\n"));
      break;
    case 0x09:
      printf(F("bufferNVMData(t_RECORD_NVM_DATA*);\n"));
      break;
    case 0x0A:
      printf(F("processReadByteLoopMaskOptionalPEC(t_RECORD_PMBUS_READ_BYTE_LOOP_MASK*);\n"));
      break;
    case 0x0B:
      printf(F("processReadWordLoopMaskOptionalPEC(t_RECORD_PMBUS_READ_WORD_LOOP_MASK*);\n"));
      break;
    case 0x0C:
      printf(F("processPollReadByteUntilAckNoPEC(t_RECORD_PMBUS_POLL_READ_BYTE_UNTIL_ACK*);\n"));
      break;
    case 0x0D:
      printf(F("processDelayMs(t_RECORD_DELAY_MS*);\n"));
      break;
    case 0x0E:
      printf(F("processSendByteOptionalPEC(t_RECORD_PMBUS_SEND_BYTE*);\n"));
      break;
    case 0x0F:
      printf(F("processWriteByteNoPEC(t_RECORD_PMBUS_WRITE_BYTE_NOPEC*);\n"));
      break;
    case 0x10:
      printf(F("processWriteWordNoPEC(t_RECORD_PMBUS_WRITE_WORD_NOPEC*);\n"));
      break;
    case 0x12:
      printf(F("processReadByteExpectNoPEC(t_RECORD_PMBUS_READ_BYTE_EXPECT_NOPEC*);\n"));
      break;
    case 0x13:
      printf(F("processReadWordExpectNoPEC(t_RECORD_PMBUS_READ_WORD_EXPECT_NOPEC*);\n"));
      break;
    case 0x15:
      printf(F("processReadByteLoopMaskNoPEC(t_RECORD_PMBUS_READ_BYTE_LOOP_MASK_NOPEC*);\n"));
      break;
    case 0x16:
      printf(F("processReadWordLoopMaskNoPEC(t_RECORD_PMBUS_READ_WORD_LOOP_MASK_NOPEC*);\n"));
      break;
    case 0x17:
      printf(F("processSendByteNoPEC(t_RECORD_PMBUS_SEND_BYTE_NOPEC*);\n"));
      break;
    case 0x18:
      printf(F("processEvent(t_RECORD_EVENT*);\n"));
      break;
    case 0x19:
      printf(F("processReadByteExpectMaskNoPEC(t_RECORD_PMBUS_READ_BYTE_EXPECT_MASK_NOPEC*);\n"));
      break;
    case 0x1A:
      printf(F("processReadWordExpectMaskNoPEC(t_RECORD_PMBUS_READ_WORD_EXPECT_MASK_NOPEC*);\n"));
      break;
    case 0x1B:
      printf(F("processVariableMetaData(t_RECORD_VARIABLE_META_DATA*);\n"));
      break;
    case 0x1C:
      printf(F("modifyWordNoPEC(t_RECORD_PMBUS_MODIFY_WORD_NO_PEC*);\n"));
      break;
    case 0x1D:
      printf(F("modifyByteNoPEC(t_RECORD_PMBUS_MODIFY_BYTE_NO_PEC*);\n"));
      break;
    case 0x1E:
      printf(F("writeNvmData(t_RECORD_NVM_DATA*);\n"));
      break;
    case 0x1F:
      printf(F("read_then_verifyNvmData(t_RECORD_NVM_DATA*);\n"));
      break;
    case 0x20:
      printf(F("modifyByteOptionalPEC(t_RECORD_PMBUS_MODIFY_BYTE*);\n"));
      break;
    case 0x21:
      printf(F("modifyWordOptionalPEC(t_RECORD_PMBUS_MODIFY_WORD*);\n"));
      break;
    default :
      printf(F("Unknown Record Type \n"));
      printf("%d", record->RecordType);
      break;
  }

  return record;
}
