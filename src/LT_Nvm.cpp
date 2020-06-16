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

#include "LT_Nvm.h"
#include <stdio.h>
    
const char *icpData;
uint16_t flashLocation = 0;


LT_PMBus *pmbus__;
LT_SMBusNoPec *smbusNoPec__;
LT_SMBusPec *smbusPec__;

NVM::NVM(LT_PMBus *pmbus, LT_SMBusNoPec *smbusNoPec, LT_SMBusPec *smbusPec)
{
  pmbus__ = pmbus;
  smbusNoPec__ = smbusNoPec;
  smbusPec__ = smbusPec;
}

bool NVM::programWithData(const char *data)
{
  icpData = data;
  flashLocation = 0;

  reset_parse_hex();
  if (processRecordsOnDemand(get_record) == 0)
  {
    reset_parse_hex();
    return 0;
  }
  reset_parse_hex();

  return 1;
}

bool NVM::verifyWithData(const char *data)
{
  icpData = data;
  flashLocation = 0;

  reset_parse_hex();
  if (verifyRecordsOnDemand(get_record) == 0)
  {
    reset_parse_hex();
    return 0;
  }
  reset_parse_hex();

  return 1;
}

char *readFileData(const char *path)
{
  char ch[2];
  char *chp;
  char *line = NULL;
  int dlen = 0;
  char *data = NULL;
  size_t n = 0;
  void *r;
  int len = 0;
  ssize_t result;
  const char *mode = "r";
  FILE *fd = fopen(path, mode);

  if (NULL != fd)
  {
    while(1)
    {
      while (1)
      {
        chp = fgets(ch, 2, fd);
        if (NULL == chp)
        {
          break;
        }
        else if (':' == ch[0])
        {
          break;
        }
      }
      if (NULL == chp)
        break;
      result = getline(&line, &n, fd);
      if (result > 0)
      {
        line[result-1] = '\0'; // Remove \n
        if (len + result - 1 > dlen)
        {
          dlen += result;
          if ((r = realloc(data, dlen)) <= 0)
          {
            printf("Can't allocate memory");
            if (r != NULL)
              free(r);
            else
              free(data);

            free(line);
            return NULL;
          }
          else 
          {
            data = (char *)r;
          }
        }
        memcpy(data + len, ":", 1);
        memcpy(data + len + 1, line, result-1);
        free(line);
        line = NULL;
        len += result;
      }
    }
    fclose(fd);

    dlen += 1;
    if ((r = realloc(data, dlen + 1)) <= 0)
    {
      if (r != NULL)
        free(r);
      else
        free(data);

      printf("Can't allocate memory");
      return 0;
    }
    else
      data = (char *)r;
    data[dlen-1] = '\0';

    return data;
  }
  else
  {
    return NULL;
  }
}

bool NVM::programWithFileData(const char *path)
{
  char *data;
  if ((data = readFileData(path)) != NULL)
  {
    icpData = data;
    flashLocation = 0;

    reset_parse_hex();
    if (processRecordsOnDemand(get_record) == 0)
    {
      printf("Error parsing\n");
      reset_parse_hex();
      free(data);
      icpData = NULL;
      return 0;
    }
    reset_parse_hex();

    free(data);
    icpData = NULL;
    return 1;
  }
  else
  {
    printf("Can't open: %s", path);
    return 0;
  }
}

bool NVM::verifyWithFileData(const char *path)
{
  char *data;
  if ((data = readFileData(path)) != NULL)
  {
    icpData = data;
    flashLocation = 0;

    reset_parse_hex();
    if (verifyRecordsOnDemand(get_record) == 0)
    {
      reset_parse_hex();
      free(data);
      icpData = NULL;
      return 0;
    }
    reset_parse_hex();

    free(data);
    icpData = NULL;
    return 1;
  }
  else
  {
    printf("Can't open: %s", path);
    return 0;
  }
}

uint8_t get_hex_data(void)
{
  uint8_t c = '\0';
  c = *(icpData + flashLocation++);

  if ('\0' != c)
    return c;
  else
    return 0;
}

extern uint8_t filter_terminations(uint8_t (*get_data)(void));

uint8_t get_filtered_hex_data(void)
{
  return filter_terminations(get_hex_data);
}

uint8_t get_record_data(void)
{
  return parse_hex(get_filtered_hex_data);
}

pRecordHeaderLengthAndType get_record(void)
{
  return parse_record(get_record_data);
}
