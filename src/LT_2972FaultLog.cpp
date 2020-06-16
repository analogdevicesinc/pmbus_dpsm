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

#include "LT_2972FaultLog.h"
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

LT_2972FaultLog::LT_2972FaultLog(LT_PMBus *pmbus):LT_CommandPlusFaultLog(pmbus)
{
  faultLog2972 = NULL;

  buffer = NULL;
  voutPeaks = NULL;
  ioutPeaks = NULL;
  tempPeaks = NULL;
  chanStatuses = NULL;
  voutDatas = NULL;
  ioutDatas = NULL;
  poutDatas = NULL;
  tempDatas = NULL;
}


bool LT_2972FaultLog::FaultLogLtc2972::isValidData(void *pos, uint8_t size)
{
  return (uint8_t *)pos >= (uint8_t *)this + firstValidByte && (uint8_t *)pos + (size - 1) <= (uint8_t *)this + lastValidByte;
}

/*
 * Read LTC2972 fault log
 *
 * address: PMBUS address
 */
void
LT_2972FaultLog::read(uint8_t address)
{
  // Copy to RAM
  pmbus_->smbus()->sendByte(address, MFR_FAULT_LOG_RESTORE);
  // Monitor BUSY bit
  while ((pmbus_->smbus()->readByte(address, MFR_COMMON) & (1 << 6)) == 0);

  uint16_t size = sizeof(struct LT_2972FaultLog::FaultLogLtc2972);  
  uint8_t *data = (uint8_t *) malloc(size);
  if (data == 0)
    printf(F("bad malloc."));
#ifdef RAW_EEPROM
  getNvmBlock(address, 0, 128, 0xC8, data);
#else
  // Read block data with log
  pmbus_->smbus()->readBlock(address, MFR_FAULT_LOG, data, 255);
#endif

  struct LT_2972FaultLog::FaultLogLtc2972 *log = (struct LT_2972FaultLog::FaultLogLtc2972 *)data;

  log->firstValidByte = 55; // first byte after preamble

  log->lastValidByte = 238; // last byte of loop data

  uint8_t cycle_start = log->preamble.position_last;

  printf("pos last %d\n", cycle_start);

  //35 corresponds to datasheet p77, last line 2nd column
  log->loops = (LT_2972FaultLog::FaultLogReadLoopLtc2972 *) (log->telemetryData - 35 + cycle_start);

  faultLog2972 = log;
}


void LT_2972FaultLog::release()
{
  free(faultLog2972);
  faultLog2972 = 0;
}

uint8_t *LT_2972FaultLog::getBinary()
{
  return (uint8_t *)faultLog2972;
}

uint16_t LT_2972FaultLog::getBinarySize()
{
  return 255;
}

void LT_2972FaultLog::dumpBinary()
{
  dumpBin((uint8_t *)faultLog2972, 255);
}

void LT_2972FaultLog::print()
{
  buffer = new char[FILE_TEXT_LINE_MAX];

  printTitle();

  printTime();

  printPeaks();

  printAllLoops();


  delete [] buffer;
}


void LT_2972FaultLog::printTitle()
{
  printf(F("LTC2972 Log Data\n"));
}

void LT_2972FaultLog::printTime()
{
  uint8_t *time = (uint8_t *)&faultLog2972->preamble.shared_time;
  snprintf_P(buffer, FILE_TEXT_LINE_MAX, PSTR("Fault Time 0x%02x%02x%02x%02x%02x%02x\n"), time[5], time[4], time[3], time[2], time[1], time[0]);
  printf("%s", buffer);
  printf("%ld", (long) getSharedTime200us(faultLog2972->preamble.shared_time));
  printf(F(" Ticks (200us each)\n"));
}

void LT_2972FaultLog::printPeaks()
{
  voutPeaks = new Peak16Words*[4];
  ioutPeaks = new Peak5_11Words*[4];
  tempPeaks = new Peak5_11Words*[4];
  chanStatuses = new ChanStatus*[4];

  voutPeaks[0] = &faultLog2972->preamble.peaks.vout0_peaks;
  voutPeaks[1] = &faultLog2972->preamble.peaks.vout1_peaks;

  ioutPeaks[0] = &faultLog2972->preamble.peaks.iout0_peaks;
  ioutPeaks[1] = &faultLog2972->preamble.peaks.iout1_peaks;

  tempPeaks[0] = &faultLog2972->preamble.peaks.temp0_peaks;
  tempPeaks[1] = &faultLog2972->preamble.peaks.temp1_peaks;

  chanStatuses[0] = &faultLog2972->preamble.fault_log_status.chan_status0;
  chanStatuses[1] = &faultLog2972->preamble.fault_log_status.chan_status1;

  printf(F("\nPeak Values and Fast Status:\n"));
  printf(F("--------\n"));

  printFastChannel(0);

  float vin_max, vin_min;
  vin_max = math_.lin11_to_float(getLin5_11WordVal(faultLog2972->preamble.peaks.vin_peaks.peak));
  vin_min = math_.lin11_to_float(getLin5_11WordVal(faultLog2972->preamble.peaks.vin_peaks.min));
  printf(F("Vin: Min: "));
  printf("%f", vin_min);
  printf(F(", Peak: "));
  printf("%f\n", vin_max);

  float iin_max, iin_min;
  iin_max = math_.lin11_to_float(getLin5_11WordVal(faultLog2972->preamble.peaks.iin_peaks.peak));
  iin_min = math_.lin11_to_float(getLin5_11WordVal(faultLog2972->preamble.peaks.iin_peaks.min));
  printf(F("Iin: Min: "));
  printf("%f", iin_min);
  printf(F(", Peak: "));
  printf("%f\n", iin_max);
  printf("\n");

  float pin_max, pin_min;
  pin_max = math_.lin11_to_float(getLin5_11WordVal(faultLog2972->preamble.peaks.pin_peaks.peak));
  pin_min = math_.lin11_to_float(getLin5_11WordVal(faultLog2972->preamble.peaks.pin_peaks.min));
  printf(F("Pin: Min: "));
  printf("%f", pin_min);
  printf(F(", Peak: "));
  printf("%f\n", pin_max);
  printf("\n");

  printFastChannel(1);

  delete [] voutPeaks;
  delete [] ioutPeaks;
  delete [] tempPeaks;
  delete [] chanStatuses;
}

void LT_2972FaultLog::printFastChannel(uint8_t index)
{
  float vout_peak, vout_min, iout_peak, iout_min, temp_peak, temp_min;
  uint8_t status;
  vout_peak = math_.lin16_to_float(getLin16WordVal(voutPeaks[index]->peak), 0x13);
  vout_min = math_.lin16_to_float(getLin16WordVal(voutPeaks[index]->min), 0x13);
  printf(F("Vout"));
  printf("%d", index);
  printf(F(": Min: "));
  printf("%f", vout_min);
  printf(F(", Peak: "));
  printf("%f\n", vout_peak);
  temp_peak = math_.lin11_to_float(getLin5_11WordVal(tempPeaks[index]->peak));
  temp_min = math_.lin11_to_float(getLin5_11WordVal(tempPeaks[index]->min));
  printf(F("Temp"));
  printf("%d", index);
  printf(F(": Min: "));
  printf("%f", temp_min);
  printf(F(", Peak: "));
  printf("%f\n", temp_peak);
  iout_peak = math_.lin11_to_float(getLin5_11WordVal(ioutPeaks[index]->peak));
  iout_min = math_.lin11_to_float(getLin5_11WordVal(ioutPeaks[index]->min));
  printf(F("Iout"));
  printf("%d", index);
  printf(F(": Min: "));
  printf("%f", iout_min);
  printf(F(", Peak: "));
  printf("%f\n", iout_peak);

  printf(F("Fast Status"));
  printf("%d\n", index);
  status = getRawByteVal(chanStatuses[index]->status_vout);
  snprintf_P(buffer, FILE_TEXT_LINE_MAX, PSTR("  STATUS_VOUT%u: 0x%02x\n"), index, status);
  printf("%s", buffer);
  status = getRawByteVal(chanStatuses[index]->status_iout);
  snprintf_P(buffer, FILE_TEXT_LINE_MAX, PSTR("  STATUS_IOUT%u: 0x%02x\n"), index, status);
  printf("%s", buffer);
  status = getRawByteVal(chanStatuses[index]->status_mfr_specific);
  snprintf_P(buffer, FILE_TEXT_LINE_MAX, PSTR("  STATUS_MFR%u: 0x%02x\n"), index, status);
  printf("%s\n", buffer);
}

void LT_2972FaultLog::printAllLoops()
{
  voutDatas = new VoutData*[4];
  ioutDatas = new IoutData*[4];
  poutDatas = new PoutData*[4];
  tempDatas = new TempData*[4];

  printf(F("Fault Log Loops Follow:\n"));
  printf(F("(most recent data first)\n"));

  for (int index = 0; index <= 4 && (index < 4 || faultLog2972->isValidData(&faultLog2972->loops[index])); index++)
  {
    printLoop(index);
  }

  delete [] voutDatas;
  delete [] ioutDatas;
  delete [] poutDatas;
  delete [] tempDatas;
}

void LT_2972FaultLog::printLoop(uint8_t index)
{
  printf(F("-------\n"));
  printf(F("Loop: "));
  printf("%d\n", index);
  printf(F("-------\n"));

  voutDatas[0] = &faultLog2972->loops[index].vout_data0;
  voutDatas[1] = &faultLog2972->loops[index].vout_data1;

  ioutDatas[0] = &faultLog2972->loops[index].iout_data0;
  ioutDatas[1] = &faultLog2972->loops[index].iout_data1;

  poutDatas[0] = &faultLog2972->loops[index].pout_data0;
  poutDatas[1] = &faultLog2972->loops[index].pout_data1;

  tempDatas[0] = &faultLog2972->loops[index].temp_data0;
  tempDatas[1] = &faultLog2972->loops[index].temp_data1;

  uint8_t stat;
  float val;

  printLoopChannel(1);

  if (faultLog2972->isValidData(&faultLog2972->loops[index].vin_data.status_vin, 1))
  {
    printf(F("VIN:\n"));
    stat = getRawByteVal(faultLog2972->loops[index].vin_data.status_vin);
    snprintf_P(buffer, FILE_TEXT_LINE_MAX, PSTR("  STATUS_INPUT: 0x%02x\n"), stat);
    printf("%s", buffer);
  }
  if (faultLog2972->isValidData(&faultLog2972->loops[index].vin_data.vin))
  {
    val = math_.lin11_to_float(getLin5_11WordReverseVal(faultLog2972->loops[index].vin_data.vin));
    printf(F("  READ_VIN: "));
    printf("%f", val);
    printf(F(" V\n"));
  }
  if (faultLog2972->isValidData(&faultLog2972->loops[index].iin_data.read_iin))
  {
    val = math_.lin11_to_float(getLin5_11WordReverseVal(faultLog2972->loops[index].iin_data.read_iin));
    printf(F("  READ_IIN: "));
    printf("%f", val);
    printf(F(" A\n"));
  }
  if (faultLog2972->isValidData(&faultLog2972->loops[index].pin_data.read_pin))
  {
    val = math_.lin11_to_float(getLin5_11WordReverseVal(faultLog2972->loops[index].pin_data.read_pin));
    printf(F("  READ_PIN: "));
    printf("%f", val);
    printf(F(" W\n"));
  }

  printLoopChannel(0);

  if (faultLog2972->isValidData(&faultLog2972->loops[index].read_temp2))
  {
    printf(F("Chip Temp:\n"));
    val = math_.lin11_to_float(getLin5_11WordReverseVal(faultLog2972->loops[index].read_temp2));
    printf(F("  CHIP TEMP: "));
    printf("%f", val);
    printf(F(" C\n"));
  }


}

void LT_2972FaultLog::printLoopChannel(uint8_t index)
{
  uint8_t stat;
  float val;

  if (faultLog2972->isValidData(&poutDatas[index]->read_pout, 2))
  {
    printf(F("CHAN"));
    printf("%d", index);
    printf(F(":\n"));
    val = math_.lin11_to_float(getLin5_11WordReverseVal(poutDatas[index]->read_pout));
    printf(F("  READ POUT: "));
    printf("%f", val);
    printf(F(" W \n"));
  }
  if (faultLog2972->isValidData(&ioutDatas[index]->read_iout, 2))
  {
    val = math_.lin11_to_float(getLin5_11WordReverseVal(ioutDatas[index]->read_iout));
    printf(F("  READ IOUT: "));
    printf("%f", val);
    printf(F(" A \n"));
  }
  if (faultLog2972->isValidData(&ioutDatas[index]->status_iout, 1))
  {
    stat = getRawByteVal(ioutDatas[index]->status_iout);
    snprintf_P(buffer, FILE_TEXT_LINE_MAX, PSTR("  STATUS IOUT: 0x%02x\n"), stat);
    printf("%s", buffer);
  }
  if (faultLog2972->isValidData(&tempDatas[index]->status_temp, 1))
  {
    stat = getRawByteVal(tempDatas[index]->status_temp);
    snprintf_P(buffer, FILE_TEXT_LINE_MAX, PSTR("  STATUS TEMP: 0x%02x\n"), stat);
    printf("%s", buffer);
  }
  if (faultLog2972->isValidData(&tempDatas[index]->read_temp1, 2))
  {
    val = math_.lin11_to_float(getLin5_11WordReverseVal(tempDatas[index]->read_temp1));
    printf(F("  READ TEMP: "));
    printf("%f", val);
    printf(F(" C\n"));
  }
  if (faultLog2972->isValidData(&voutDatas[index]->status_mfr_specific, 1))
  {
    stat = getRawByteVal(voutDatas[index]->status_mfr_specific);
    snprintf_P(buffer, FILE_TEXT_LINE_MAX, PSTR("  STATUS_MFR: 0x%02x\n"), stat);
    printf("%s", buffer);
  }
  if (faultLog2972->isValidData(&voutDatas[index]->status_mfr_2, 1))
  {
    stat = getRawByteVal(voutDatas[index]->status_mfr_2);
    snprintf_P(buffer, FILE_TEXT_LINE_MAX, PSTR("  STATUS_MFR_2: 0x%02x\n"), stat);
    printf("%s", buffer);
  }
  if (faultLog2972->isValidData(&voutDatas[index]->status_vout, 1))
  {
    stat = getRawByteVal(voutDatas[index]->status_vout);
    snprintf_P(buffer, FILE_TEXT_LINE_MAX, PSTR("  STATUS_VOUT: 0x%02x\n"), stat);
    printf("%s", buffer);
  }
  if (faultLog2972->isValidData(&voutDatas[index]->read_vout))
  {
    val = math_.lin16_to_float(getLin16WordReverseVal(voutDatas[index]->read_vout), 0x13);
    printf(F("  READ_VOUT: "));
    printf("%f", val);
    printf(F(" V\n"));
  }
}



