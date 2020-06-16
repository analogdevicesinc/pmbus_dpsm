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

#include "LT_2977FaultLog.h"
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

LT_2977FaultLog::LT_2977FaultLog(LT_PMBus *pmbus):LT_CommandPlusFaultLog(pmbus)
{
  faultLog2977 = NULL;

  buffer = NULL;
  voutPeaks = NULL;
  chanStatuses = NULL;
  voutDatas = NULL;
}


bool LT_2977FaultLog::FaultLogLtc2977::isValidData(void *pos, uint8_t size)
{
  return (uint8_t *)pos >= (uint8_t *)this + firstValidByte && (uint8_t *)pos + (size - 1) <= (uint8_t *)this + lastValidByte;
}

/*
 * Read LTC2977 fault log
 *
 * address: PMBUS address
 */
void
LT_2977FaultLog::read(uint8_t address)
{
  // Copy to RAM
  pmbus_->smbus()->sendByte(address, MFR_FAULT_LOG_RESTORE);
  // Monitor BUSY bit
  while ((pmbus_->smbus()->readByte(address, MFR_COMMON) & (1 << 6)) == 0);

  uint16_t size = sizeof(struct LT_2977FaultLog::FaultLogLtc2977);
  uint8_t *data = (uint8_t *) malloc(size);
  if (data == 0)
    printf(F("bad malloc."));
#ifdef RAW_EEPROM
  getNvmBlock(address, 0, 128, 0xC0, data);
#else
  // Read block data with log
  pmbus_->smbus()->readBlock(address, MFR_FAULT_LOG, data, 255);
#endif

  struct LT_2977FaultLog::FaultLogLtc2977 *log = (struct LT_2977FaultLog::FaultLogLtc2977 *)data;

  log->firstValidByte = 72;

  uint8_t count = log->preamble.cyclic_data_count;
  if (count == 255)
    log->lastValidByte = 237;
  else
    log->lastValidByte = 237 - (166-count);

  uint8_t cycle_start = log->preamble.position_last;

  log->loops = (LT_2977FaultLog::FaultLogReadLoopLtc2977 *) (log->telemetryData - 45 + cycle_start);

  faultLog2977 = log;
}


void LT_2977FaultLog::release()
{
  free(faultLog2977);
  faultLog2977 = 0;
}

uint8_t *LT_2977FaultLog::getBinary()
{
  return (uint8_t *)faultLog2977;
}

uint16_t LT_2977FaultLog::getBinarySize()
{
  return 255;
}

void LT_2977FaultLog::dumpBinary()
{
  dumpBin((uint8_t *)faultLog2977, 255);
}

void LT_2977FaultLog::print()
{
  buffer = new char[FILE_TEXT_LINE_MAX];

  printTitle();

  printTime();

  printPeaks();

  printAllLoops();


  delete [] buffer;
}


void LT_2977FaultLog::printTitle()
{
  printf(F("LTC2977 Log Data\n"));
}

void LT_2977FaultLog::printTime()
{
  uint8_t *time = (uint8_t *)&faultLog2977->preamble.shared_time;
  snprintf_P(buffer, FILE_TEXT_LINE_MAX, PSTR("Fault Time 0x%02x%02x%02x%02x%02x%02x\n"), time[5], time[4], time[3], time[2], time[1], time[0]);
  printf("%s", buffer);
  printf("%ld", (long) getSharedTime200us(faultLog2977->preamble.shared_time));
  printf(F(" Ticks (200us each)\n"));
}

void LT_2977FaultLog::printPeaks()
{
  voutPeaks = new Peak16Words*[8];
  chanStatuses = new ChanStatus*[8];

  voutPeaks[0] = &faultLog2977->preamble.peaks.vout0_peaks;
  voutPeaks[1] = &faultLog2977->preamble.peaks.vout1_peaks;
  voutPeaks[2] = &faultLog2977->preamble.peaks.vout2_peaks;
  voutPeaks[3] = &faultLog2977->preamble.peaks.vout3_peaks;
  voutPeaks[4] = &faultLog2977->preamble.peaks.vout4_peaks;
  voutPeaks[5] = &faultLog2977->preamble.peaks.vout5_peaks;
  voutPeaks[6] = &faultLog2977->preamble.peaks.vout6_peaks;
  voutPeaks[7] = &faultLog2977->preamble.peaks.vout7_peaks;

  chanStatuses[0] = &faultLog2977->preamble.fault_log_status.chan_status0;
  chanStatuses[1] = &faultLog2977->preamble.fault_log_status.chan_status1;
  chanStatuses[2] = &faultLog2977->preamble.fault_log_status.chan_status2;
  chanStatuses[3] = &faultLog2977->preamble.fault_log_status.chan_status3;
  chanStatuses[4] = &faultLog2977->preamble.fault_log_status.chan_status4;
  chanStatuses[5] = &faultLog2977->preamble.fault_log_status.chan_status5;
  chanStatuses[6] = &faultLog2977->preamble.fault_log_status.chan_status6;
  chanStatuses[7] = &faultLog2977->preamble.fault_log_status.chan_status7;


  printf(F("\nPeak Values and Fast Status:\n"));
  printf(F("--------\n"));

  printFastChannel(0);
  printFastChannel(1);

  float vin_max, vin_min, temp_max, temp_min;
  vin_max = math_.lin11_to_float(getLin5_11WordVal(faultLog2977->preamble.peaks.vin_peaks.peak));
  vin_min = math_.lin11_to_float(getLin5_11WordVal(faultLog2977->preamble.peaks.vin_peaks.min));
  printf(F("Vin: Min: "));
  printf("%f", vin_min);
  printf(F(", Peak: "));
  printf("%f\n", vin_max);
  printf("\n");

  printFastChannel(2);
  printFastChannel(3);

  temp_max = math_.lin11_to_float(getLin5_11WordVal(faultLog2977->preamble.peaks.temp_peaks.peak));
  temp_min = math_.lin11_to_float(getLin5_11WordVal(faultLog2977->preamble.peaks.temp_peaks.min));
  printf(F("Temp: Min: "));
  printf("%f", temp_min);
  printf(F(", Peak: "));
  printf("%f\n", temp_max);
  printf("\n");

  printFastChannel(4);
  printFastChannel(5);
  printFastChannel(6);
  printFastChannel(7);

  delete [] voutPeaks;
  delete [] chanStatuses;
}

void LT_2977FaultLog::printFastChannel(uint8_t index)
{
  float vout_peak, vout_min;
  uint8_t status;
  vout_peak = math_.lin16_to_float(getLin16WordVal(voutPeaks[index]->peak), 0x13);
  vout_min = math_.lin16_to_float(getLin16WordVal(voutPeaks[index]->min), 0x13);
  printf(F("Vout"));
  printf("%d", index);
  printf(F(": Min: "));
  printf("%f", vout_min);
  printf(F(", Peak: "));
  printf("%f\n", vout_peak);
  printf(F("Fast Status"));
  printf("%d\n", index);
  status = getRawByteVal(chanStatuses[index]->status_vout);
  snprintf_P(buffer, FILE_TEXT_LINE_MAX, PSTR("  STATUS_VOUT%u: 0x%02x\n"), index, status);
  printf("%s", buffer);
  status = getRawByteVal(chanStatuses[index]->status_mfr);
  snprintf_P(buffer, FILE_TEXT_LINE_MAX, PSTR("  STATUS_MFR%u: 0x%02x\n"), index, status);
  printf("%s", buffer);
  status = getRawByteVal(chanStatuses[index]->mfr_status2);
  snprintf_P(buffer, FILE_TEXT_LINE_MAX, PSTR("  MFR_STATUS_2%u: 0x%02x\n"), index, status);
  printf("%s\n", buffer);
}

void LT_2977FaultLog::printAllLoops()
{
  voutDatas = new VoutData*[8];

  printf(F("Fault Log Loops Follow:\n"));
  printf(F("(most recent data first)\n"));

  for (int index = 0; index <= 4 && (index < 4 || faultLog2977->isValidData(&faultLog2977->loops[index])); index++)
  {
    printLoop(index);
  }

  delete [] voutDatas;
}

void LT_2977FaultLog::printLoop(uint8_t index)
{
  printf(F("-------\n"));
  printf(F("Loop: "));
  printf("%d\n", index);
  printf(F("-------\n"));

  voutDatas[0] = &faultLog2977->loops[index].vout_data0;
  voutDatas[1] = &faultLog2977->loops[index].vout_data1;
  voutDatas[2] = &faultLog2977->loops[index].vout_data2;
  voutDatas[3] = &faultLog2977->loops[index].vout_data3;
  voutDatas[4] = &faultLog2977->loops[index].vout_data4;
  voutDatas[5] = &faultLog2977->loops[index].vout_data5;
  voutDatas[6] = &faultLog2977->loops[index].vout_data6;
  voutDatas[7] = &faultLog2977->loops[index].vout_data7;

  uint8_t stat;
  float val;

  printLoopChannel(7);
  printLoopChannel(6);
  printLoopChannel(5);
  printLoopChannel(4);

  if (faultLog2977->isValidData(&faultLog2977->loops[index].temp_data.status_temp, 1))
  {
    printf(F("TEMPERATURE:\n"));
    stat = getRawByteVal(faultLog2977->loops[index].temp_data.status_temp);
    snprintf_P(buffer, FILE_TEXT_LINE_MAX, PSTR("  STATUS_TEMP: 0x%02x\n"), stat);
    printf("%s", buffer);
  }
  if (faultLog2977->isValidData(&faultLog2977->loops[index].temp_data.temp))
  {
    val = math_.lin11_to_float(getLin5_11WordReverseVal(faultLog2977->loops[index].temp_data.temp));
    printf(F("  READ_TEMP: "));
    printf("%f", val);
    printf(F(" C\n"));
  }

  printLoopChannel(3);
  printLoopChannel(2);

  if (faultLog2977->isValidData(&faultLog2977->loops[index].vin_data.status_vin, 1))
  {
    printf(F("VIN:\n"));
    stat = getRawByteVal(faultLog2977->loops[index].vin_data.status_vin);
    snprintf_P(buffer, FILE_TEXT_LINE_MAX, PSTR("  STATUS_INPUT: 0x%02x\n"), stat);
    printf("%s", buffer);
  }
  if (faultLog2977->isValidData(&faultLog2977->loops[index].vin_data.vin))
  {
    val = math_.lin11_to_float(getLin5_11WordReverseVal(faultLog2977->loops[index].vin_data.vin));
    printf(F("  READ_VIN: "));
    printf("%f", val);
    printf(F(" V\n"));
  }

  printLoopChannel(1);
  printLoopChannel(0);
}

void LT_2977FaultLog::printLoopChannel(uint8_t index)
{
  uint8_t stat;
  float val;

  if (faultLog2977->isValidData(&voutDatas[index]->mfr_status2, 1))
  {
    printf(F("CHAN"));
    printf("%d", index);
    printf(F(":\n"));
    stat = getRawByteVal(voutDatas[index]->mfr_status2);
    snprintf_P(buffer, FILE_TEXT_LINE_MAX, PSTR("  MFR_STATUS_2: 0x%02x\n"), stat);
    printf("%s", buffer);
  }
  if (faultLog2977->isValidData(&voutDatas[index]->status_mfr, 1))
  {
    stat = getRawByteVal(voutDatas[index]->status_mfr);
    snprintf_P(buffer, FILE_TEXT_LINE_MAX, PSTR("  STATUS_MFR: 0x%02x\n"), stat);
    printf("%s", buffer);
  }
  if (faultLog2977->isValidData(&voutDatas[index]->status_vout, 1))
  {
    stat = getRawByteVal(voutDatas[index]->status_vout);
    snprintf_P(buffer, FILE_TEXT_LINE_MAX, PSTR("  STATUS_VOUT: 0x%02x\n"), stat);
    printf("%s", buffer);
  }
  if (faultLog2977->isValidData(&voutDatas[index]->read_vout))
  {
    val = math_.lin16_to_float(getLin16WordReverseVal(voutDatas[index]->read_vout), 0x13);
    printf(F("  READ_VOUT: "));
    printf("%f", val);
    printf(F(" V\n"));
  }
}



