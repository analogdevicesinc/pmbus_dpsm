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

#include "LT_3880FaultLog.h"
#ifdef DMALLOC
#include <dmalloc.h>
#else
#include <stdlib.h>
#endif
    
#undef F
#define F(s) s
#define PSTR(s) s
#define snprintf_P(a,b,c,...) snprintf(a,b,c, __VA_ARGS__)
    
#define RAW_EEPROM

LT_3880FaultLog::LT_3880FaultLog(LT_PMBus *pmbus):LT_EEDataFaultLog(pmbus)
{

  faultLog3880 = NULL;

  buffer = NULL;
}


/*
 * Read LTC3880 fault log
 *
 * address: PMBUS address
 */
void
LT_3880FaultLog::read(uint8_t address)
{
#ifdef RAW_EEPROM
  uint8_t *data = (uint8_t *) malloc(sizeof(uint8_t) * 54 * 2); // Becuse CRC is stripped, the acutal data size is smaller

  getNvmBlock(address, 192*2, 54, true, data);
#else
  uint8_t *data = (uint8_t *) malloc(147);
  data[0] = 0x00;

  pmbus_->smbus()->readBlock(address, MFR_FAULT_LOG, data, 147);
#endif
  faultLog3880 = (struct LT_3880FaultLog::FaultLogLtc3880 *) (data);
}


void LT_3880FaultLog::release()
{
  free(faultLog3880);
  faultLog3880 = 0;
}

uint8_t *LT_3880FaultLog::getBinary()
{
  return (uint8_t *)faultLog3880;
}

uint16_t LT_3880FaultLog::getBinarySize()
{
  return 147;
}

void LT_3880FaultLog::dumpBinary()
{
  dumpBin((uint8_t *)faultLog3880, 147);
}

void LT_3880FaultLog::print()
{
  buffer = new char[FILE_TEXT_LINE_MAX];

  printTitle();

  printTime();

  printPeaks();

  printAllLoops();


  delete [] buffer;
}


void LT_3880FaultLog::printTitle()
{
  printf(F("LTC3880 Log Data\n"));

  uint8_t position = faultLog3880->preamble.position_fault;
  switch (position)
  {
    case 0xFF :
      snprintf_P(buffer, FILE_TEXT_LINE_MAX, PSTR("Fault Position MFR_FAULT_LOG_STORE%s\n"),"");
      break;
    case 0x00 :
      snprintf_P(buffer, FILE_TEXT_LINE_MAX, PSTR("Fault Position TON_MAX_FAULT Channel 0%s\n"),"");
      break;
    case 0x01 :
      snprintf_P(buffer, FILE_TEXT_LINE_MAX, PSTR("Fault Position VOUT_OV_FAULT Channel 0%s\n"),"");
      break;
    case 0x02 :
      snprintf_P(buffer, FILE_TEXT_LINE_MAX, PSTR("Fault Position VOUT_UV_FAULT Channel 0%s\n"),"");
      break;
    case 0x03 :
      snprintf_P(buffer, FILE_TEXT_LINE_MAX, PSTR("Fault Position IOUT_OC_FAULT Channel 0%s\n"),"");
      break;
    case 0x05 :
      snprintf_P(buffer, FILE_TEXT_LINE_MAX, PSTR("Fault Position OT_FAULT Channel 0%s\n"),"");
      break;
    case 0x06 :
      snprintf_P(buffer, FILE_TEXT_LINE_MAX, PSTR("Fault Position UT_FAULT Channel 0%s\n"),"");
      break;
    case 0x07 :
      snprintf_P(buffer, FILE_TEXT_LINE_MAX, PSTR("Fault Position VIN_OV_FAULT Channel 0%s\n"),"");
      break;
    case 0x0A :
      snprintf_P(buffer, FILE_TEXT_LINE_MAX, PSTR("Fault Position MFR_OT_FAULT Channel 0%s\n"),"");
      break;
    case 0x10 :
      snprintf_P(buffer, FILE_TEXT_LINE_MAX, PSTR("Fault Position TON_MAX_FAULT Channel 1%s\n"),"");
      break;
    case 0x11 :
      snprintf_P(buffer, FILE_TEXT_LINE_MAX, PSTR("Fault Position VOUT_OV_FAULT Channel 1%s\n"),"");
      break;
    case 0x12 :
      snprintf_P(buffer, FILE_TEXT_LINE_MAX, PSTR("Fault Position VOUT_UV_FAULT Channel 1%s\n"),"");
      break;
    case 0x13 :
      snprintf_P(buffer, FILE_TEXT_LINE_MAX, PSTR("Fault Position IOUT_OC_FAULT Channel 1%s\n"),"");
      break;
    case 0x15 :
      snprintf_P(buffer, FILE_TEXT_LINE_MAX, PSTR("Fault Position OT_FAULT Channel 1%s\n"),"");
      break;
    case 0x16 :
      snprintf_P(buffer, FILE_TEXT_LINE_MAX, PSTR("Fault Position UT_FAULT Channel 1%s\n"),"");
      break;
    case 0x17 :
      snprintf_P(buffer, FILE_TEXT_LINE_MAX, PSTR("Fault Position VIN_OV_FAULT Channel 1%s\n"),"");
      break;
    case 0x1A :
      snprintf_P(buffer, FILE_TEXT_LINE_MAX, PSTR("Fault Position MFR_OT_FAULT Channel 1%s\n"),"");
      break;
  }

  printf("%s", buffer);
}

void LT_3880FaultLog::printTime()
{
  uint8_t *time = (uint8_t *)&faultLog3880->preamble.shared_time;
  snprintf_P(buffer, FILE_TEXT_LINE_MAX, PSTR("Fault Time 0x%02x%02x%02x%02x%02x%02x\n"), time[5], time[4], time[3], time[2], time[1], time[0]);
  printf("%s", buffer);
  printf("%ld", (long) getSharedTime200us(faultLog3880->preamble.shared_time));
  printf(F(" Ticks (200us each)\n"));
}

void LT_3880FaultLog::printPeaks()
{
  printf(F("\nHeader Information:\n--------\n"));

  printf(F("VOUT Peak 0 "));
  printf("%f\n", math_.lin16_to_float(getLin16WordReverseVal(faultLog3880->preamble.peaks.mfr_vout_peak_p0), 0x14));
  printf(F("VOUT Peak 1 "));
  printf("%f\n", math_.lin16_to_float(getLin16WordReverseVal(faultLog3880->preamble.peaks.mfr_vout_peak_p1), 0x14));
  printf(F("IOUT Peak 0 "));
  printf("%f\n", math_.lin11_to_float(getLin5_11WordReverseVal(faultLog3880->preamble.peaks.mfr_iout_peak_p0)));
  printf(F("IOUT Peak 1 "));
  printf("%f\n", math_.lin11_to_float(getLin5_11WordReverseVal(faultLog3880->preamble.peaks.mfr_iout_peak_p1)));
  printf(F("VIN Peak "));
  printf("%f\n", math_.lin11_to_float(getLin5_11WordReverseVal(faultLog3880->preamble.peaks.mfr_vin_peak)));
  printf(F("Temp External Last Event Page 0 "));
  printf("%f\n", math_.lin11_to_float(getLin5_11WordReverseVal(faultLog3880->preamble.peaks.read_temperature_1_p0)));
  printf(F("Temp External Last Event Page 1 "));
  printf("%f\n", math_.lin11_to_float(getLin5_11WordReverseVal(faultLog3880->preamble.peaks.read_temperature_1_p1)));
  printf(F("Temp Internal Last Event "));
  printf("%f\n", math_.lin11_to_float(getLin5_11WordReverseVal(faultLog3880->preamble.peaks.read_temperature_2)));
  printf(F("Temp External Peak Page 0 "));
  printf("%f\n", math_.lin11_to_float(getLin5_11WordReverseVal(faultLog3880->preamble.peaks.mfr_temperature_1_peak_p0)));
  printf(F("Temp External Peak Page 1 "));
  printf("%f\n", math_.lin11_to_float(getLin5_11WordReverseVal(faultLog3880->preamble.peaks.mfr_temperature_1_peak_p1)));
}

void LT_3880FaultLog::printAllLoops()
{
  printf(F("\nFault Log Loops Follow:\n"));
  printf(F("(most recent data first)\n"));

  for (int index = 0; index < 4; index++)
  {
    printLoop(index);
  }
}

void LT_3880FaultLog::printLoop(uint8_t index)
{
  printf(F("-------\n"));
  printf(F("Loop: "));
  printf("%d\n", index);
  printf(F("-------\n"));

  printf(F("Input: "));
  printf("%f", math_.lin11_to_float(getLin5_11WordReverseVal(faultLog3880->fault_log_loop[index].read_vin)));
  printf(F(" V, "));
  printf("%f", math_.lin11_to_float(getLin5_11WordReverseVal(faultLog3880->fault_log_loop[index].read_iin)));
  printf(F(" A\n"));
  printf(F("Chan0: "));
  printf("%f", math_.lin16_to_float(getLin16WordReverseVal(faultLog3880->fault_log_loop[index].read_vout_p0), 0x14));
  printf(F(" V, "));
  printf("%f", math_.lin11_to_float(getLin5_11WordReverseVal(faultLog3880->fault_log_loop[index].read_iout_p0)));
  printf(F(" A\n"));
  snprintf_P(buffer, FILE_TEXT_LINE_MAX, PSTR("  STATUS_VOUT: 0x%02x\n"), getRawByteVal(faultLog3880->fault_log_loop[index].status_vout_p0));
  printf("%s", buffer);
  snprintf_P(buffer, FILE_TEXT_LINE_MAX, PSTR("  STATUS_MFR_SPECIFIC: 0x%02x\n"), getRawByteVal(faultLog3880->fault_log_loop[index].status_mfr_specificP0));
  printf("%s", buffer);
  snprintf_P(buffer, FILE_TEXT_LINE_MAX, PSTR("  STATUS_WORD: 0x%04x\n"), getRawWordReverseVal(faultLog3880->fault_log_loop[index].status_word_p0));
  printf("%s", buffer);
  printf(F("Chan1: "));
  printf("%f", math_.lin16_to_float(getLin16WordReverseVal(faultLog3880->fault_log_loop[index].read_vout_p1), 0x14));
  printf(F(" V, "));
  printf("%f", math_.lin11_to_float(getLin5_11WordReverseVal(faultLog3880->fault_log_loop[index].read_iout_p1)));
  printf(F(" A\n"));
  snprintf_P(buffer, FILE_TEXT_LINE_MAX, PSTR("  STATUS_VOUT: 0x%02x\n"), getRawByteVal(faultLog3880->fault_log_loop[index].status_vout_p1));
  printf("%s", buffer);
  snprintf_P(buffer, FILE_TEXT_LINE_MAX, PSTR("  STATUS_MFR_SPECIFIC: 0x%02x\n"), getRawByteVal(faultLog3880->fault_log_loop[index].status_mfr_specificP1));
  printf("%s", buffer);
  snprintf_P(buffer, FILE_TEXT_LINE_MAX, PSTR("  STATUS_WORD: 0x%04x\n"), getRawWordReverseVal(faultLog3880->fault_log_loop[index].status_word_p1));
  printf("%s", buffer);
}
