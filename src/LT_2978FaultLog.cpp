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

#include "LT_2978FaultLog.h"
#ifdef DMALLOC
#include <dmalloc.h>
#else
#include <stdlib.h>
#endif
#include <unistd.h>
    
#undef F
#define F(s) s
#define PSTR(s) s
#define snprintf_P(a,b,c,...) snprintf(a,b,c, __VA_ARGS__)

LT_2978FaultLog::LT_2978FaultLog(LT_PMBus *pmbus):LT_EEDataFaultLog(pmbus)
{
  faultLog2978 = NULL;

  buffer = NULL;
  voutPeaks = NULL;
  voutDatas = NULL;
}


bool LT_2978FaultLog::FaultLogLtc2978::isValidData(void *pos, uint8_t size)
{
  return (uint8_t *)pos >= (uint8_t *)this + firstValidByte && (uint8_t *)pos + (size - 1) <= (uint8_t *)this + lastValidByte;
}

/*
 * Read LTC2978 fault log
 *
 * address: PMBUS address
 */
void
LT_2978FaultLog::read(uint8_t address)
{
  uint16_t size = sizeof(struct LT_2978FaultLog::FaultLogLtc2978);
  uint8_t *data = (uint8_t *) malloc(size);
  if (data == 0)
    printf(F("bad malloc."));
  data[0] = 0x00;


  // Copy to RAM
  pmbus_->smbus()->sendByte(address, MFR_FAULT_LOG_RESTORE);
  // MFR_FAULT_LOG_RESTORE requires a delay (see datasheet), typical delay is 2ms, multiplying by 10 for safety
  usleep(20 * 1000);
  // Read block data with log
  pmbus_->smbus()->readBlock(address, MFR_FAULT_LOG, data, 255);

  struct LT_2978FaultLog::FaultLogLtc2978 *log = (struct LT_2978FaultLog::FaultLogLtc2978 *)data;

  log->firstValidByte = 47;

  log->lastValidByte = 237;

  uint8_t cycle_start = log->preamble.position_last;

  log->loops = (LT_2978FaultLog::FaultLogReadLoopLtc2978 *) (log->telemetryData - 39 + cycle_start);

  faultLog2978 = log;
}


void LT_2978FaultLog::release()
{
  free(faultLog2978);
  faultLog2978 = 0;
}

uint8_t *LT_2978FaultLog::getBinary()
{
  return (uint8_t *)faultLog2978;
}

uint16_t LT_2978FaultLog::getBinarySize()
{
  return 255;
}

void LT_2978FaultLog::dumpBinary()
{
  dumpBin((uint8_t *)faultLog2978, 255);
}

void LT_2978FaultLog::print()
{
  buffer = new char[FILE_TEXT_LINE_MAX];

  printTitle();

  printTime();

  printPeaks();

  printAllLoops();


  delete [] buffer;
}


void LT_2978FaultLog::printTitle()
{
  printf(F("LTC2978 Log Data\n"));
}

void LT_2978FaultLog::printTime()
{
  uint8_t *time = (uint8_t *)&faultLog2978->preamble.shared_time;
  snprintf_P(buffer, FILE_TEXT_LINE_MAX, PSTR("Fault Time 0x%02x%02x%02x%02x%02x%02x\n"), time[5], time[4], time[3], time[2], time[1], time[0]);
  printf("%s", buffer);
  printf("%ld", (long) getSharedTime200us(faultLog2978->preamble.shared_time));
  printf(F(" Ticks (200us each)\n"));
}

void LT_2978FaultLog::printPeaks()
{
  voutPeaks = new Peak16Words*[8];

  voutPeaks[0] = &faultLog2978->preamble.peaks.vout0_peaks;
  voutPeaks[1] = &faultLog2978->preamble.peaks.vout1_peaks;
  voutPeaks[2] = &faultLog2978->preamble.peaks.vout2_peaks;
  voutPeaks[3] = &faultLog2978->preamble.peaks.vout3_peaks;
  voutPeaks[4] = &faultLog2978->preamble.peaks.vout4_peaks;
  voutPeaks[5] = &faultLog2978->preamble.peaks.vout5_peaks;
  voutPeaks[6] = &faultLog2978->preamble.peaks.vout6_peaks;
  voutPeaks[7] = &faultLog2978->preamble.peaks.vout7_peaks;


  printf(F("\nPeak Values:\n"));
  printf(F("--------\n"));

  printFastChannel(0);
  printFastChannel(1);

  float vin_max, vin_min;
  vin_max = math_.lin11_to_float(getLin5_11WordVal(faultLog2978->preamble.peaks.vin_peaks.peak));
  vin_min = math_.lin11_to_float(getLin5_11WordVal(faultLog2978->preamble.peaks.vin_peaks.min));
  printf(F("Vin: Min: "));
  printf("%f", vin_min);
  printf(F(", Peak: "));
  printf("%f\n", vin_max);
  printf("\n");

  printFastChannel(2);
  printFastChannel(3);

  float temp_max, temp_min;
  temp_max = math_.lin11_to_float(getLin5_11WordVal(faultLog2978->preamble.peaks.temp_peaks.peak));
  temp_min = math_.lin11_to_float(getLin5_11WordVal(faultLog2978->preamble.peaks.temp_peaks.min));
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
}

void LT_2978FaultLog::printFastChannel(uint8_t index)
{
  float vout_peak, vout_min;
  vout_peak = math_.lin16_to_float(getLin16WordVal(voutPeaks[index]->peak), 0x13);
  vout_min = math_.lin16_to_float(getLin16WordVal(voutPeaks[index]->min), 0x13);
  printf(F("Vout"));
  printf("%d", index);
  printf(F(": Min: "));
  printf("%f", vout_min);
  printf(F(", Peak: "));
  printf("%f\n", vout_peak);
}

void LT_2978FaultLog::printAllLoops()
{
  voutDatas = new VoutData*[8];

  printf(F("Fault Log Loops Follow:\n"));
  printf(F("(most recent data first)\n"));

  for (int index = 0; index <= 5 && (index < 5 || faultLog2978->isValidData(&faultLog2978->loops[index])); index++)
  {
    printLoop(index);
  }

  delete [] voutDatas;
}

void LT_2978FaultLog::printLoop(uint8_t index)
{
  printf(F("-------\n"));
  printf(F("Loop: "));
  printf("%d\n", index);
  printf(F("-------\n"));

  voutDatas[0] = &faultLog2978->loops[index].vout_data0;
  voutDatas[1] = &faultLog2978->loops[index].vout_data1;
  voutDatas[2] = &faultLog2978->loops[index].vout_data2;
  voutDatas[3] = &faultLog2978->loops[index].vout_data3;
  voutDatas[4] = &faultLog2978->loops[index].vout_data4;
  voutDatas[5] = &faultLog2978->loops[index].vout_data5;
  voutDatas[6] = &faultLog2978->loops[index].vout_data6;
  voutDatas[7] = &faultLog2978->loops[index].vout_data7;

  uint8_t stat;
  float val;

  printLoopChannel(7);
  printLoopChannel(6);
  printLoopChannel(5);
  printLoopChannel(4);

  if (faultLog2978->isValidData(&faultLog2978->loops[index].temp_data.status_temp, 1))
  {
    printf(F("TEMPERATURE:\n"));
    stat = getRawByteVal(faultLog2978->loops[index].temp_data.status_temp);
    snprintf_P(buffer, FILE_TEXT_LINE_MAX, PSTR("  STATUS_TEMP: 0x%02x\n"), stat);
    printf("%s", buffer);
  }
  if (faultLog2978->isValidData(&faultLog2978->loops[index].temp_data.read_temp1))
  {
    val = math_.lin11_to_float(getLin5_11WordReverseVal(faultLog2978->loops[index].temp_data.read_temp1));
    printf(F("  READ_TEMP: "));
    printf("%f", val);
    printf(F(" C\n"));
  }

  printLoopChannel(3);
  printLoopChannel(2);

  if (faultLog2978->isValidData(&faultLog2978->loops[index].vin_data.status_vin, 1))
  {
    printf(F("VIN:\n"));
    stat = getRawByteVal(faultLog2978->loops[index].vin_data.status_vin);
    snprintf_P(buffer, FILE_TEXT_LINE_MAX, PSTR("  STATUS_INPUT: 0x%02x\n"), stat);
    printf("%s", buffer);
  }
  if (faultLog2978->isValidData(&faultLog2978->loops[index].vin_data.vin))
  {
    val = math_.lin11_to_float(getLin5_11WordReverseVal(faultLog2978->loops[index].vin_data.vin));
    printf(F("  READ_VIN: "));
    printf("%f", val);
    printf(F(" V\n"));
  }

  printLoopChannel(1);
  printLoopChannel(0);
  printf("\n");

}

void LT_2978FaultLog::printLoopChannel(uint8_t index)
{
  uint8_t stat;
  float val;

  if (faultLog2978->isValidData(&voutDatas[index]->status_mfr, 1))
  {
    printf(F("CHAN"));
    printf("%d", index);
    printf(F(":\n"));
    stat = getRawByteVal(voutDatas[index]->status_mfr);
    snprintf_P(buffer, FILE_TEXT_LINE_MAX, PSTR("  STATUS_MFR: 0x%02x\n"), stat);
    printf("%s", buffer);
  }
  if (faultLog2978->isValidData(&voutDatas[index]->status_vout, 1))
  {
    stat = getRawByteVal(voutDatas[index]->status_vout);
    snprintf_P(buffer, FILE_TEXT_LINE_MAX, PSTR("  STATUS_VOUT: 0x%02x\n"), stat);
    printf("%s", buffer);
  }
  if (faultLog2978->isValidData(&voutDatas[index]->read_vout))
  {
    val = math_.lin16_to_float(getLin16WordReverseVal(voutDatas[index]->read_vout), 0x13);
    printf(F("  READ_VOUT: "));
    printf("%f", val);
    printf(F(" V\n"));
  }
}



