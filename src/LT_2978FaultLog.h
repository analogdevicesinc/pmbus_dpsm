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

#ifndef LT_2978FaultLog_H_
#define LT_2978FaultLog_H_

#include "LT_PMBus.h"
#include "LT_FaultLog.h"
#include "LT_PMBusMath.h"
#include "LT_EEDataFaultLog.h"

//! class that handles LTC2978 fault logs.
//! contains structs for interpreting the data read from the part.
class LT_2978FaultLog : public LT_EEDataFaultLog
{

  public:
#pragma pack(push, 1)


    /********************************************************************
     * LTC2978 types
     ********************************************************************/

    struct VoutData
    {
      public:
        struct RawByte status_mfr;
        struct RawByte status_vout;
        struct Lin16WordReverse read_vout;
    };

    struct TempData
    {
      public:
        struct RawByte status_temp;
        struct Lin5_11WordReverse read_temp1;
    };

    struct VinStatus
    {
      public:
        struct RawByte status_vin;
    };

    struct VinData
    {
      public:
        struct RawByte status_vin;
        struct Lin5_11WordReverse vin;
    };

    struct Peak16Words
    {
      public:
        struct Lin16Word peak;
        struct Lin16Word min;
    };

    struct Peak5_11Words
    {
      public:
        struct Lin5_11Word peak;
        struct Lin5_11Word min;
    };

    struct FaultLogPeaksLtc2978
    {
      public:
        struct Peak16Words vout0_peaks;
        struct Peak16Words vout1_peaks;
        struct Peak5_11Words vin_peaks;
        struct Peak16Words vout2_peaks;
        struct Peak16Words vout3_peaks;
        struct Peak5_11Words temp_peaks;
        struct Peak16Words vout4_peaks;
        struct Peak16Words vout5_peaks;
        struct Peak16Words vout6_peaks;
        struct Peak16Words vout7_peaks;
    };

    struct FaultLogPreambleLtc2978
    {
      public:
        uint8_t position_last;
        struct FaultLogTimeStamp shared_time;
        struct FaultLogPeaksLtc2978 peaks;
    };

    struct FaultLogReadLoopLtc2978
    {
      public:
        struct VoutData vout_data7;
        struct VoutData vout_data6;
        struct VoutData vout_data5;
        struct VoutData vout_data4;
        struct RawByte reserved;
        struct TempData temp_data;
        struct VoutData vout_data3;
        struct VoutData vout_data2;
        struct RawByte reserved2;
        struct VinData vin_data;
        struct VoutData vout_data1;
        struct VoutData vout_data0;
    };

    struct FaultLogLtc2978
    {
      public:
        struct FaultLogPreambleLtc2978 preamble;
        uint8_t telemetryData[208];
        struct FaultLogReadLoopLtc2978 *loops; //loops[0] - loops[5]; lays over telemetry data based on Fault-Log Pointer; 0, 4 have potential for invalid/phantom data; 5 necessarily contains some or all invalid/phantom data; based on following valid byte pointers
        uint8_t firstValidByte; //relative to struct.  firstValidByte should reference start of telemetryData (47).  See datasheet.
        uint8_t lastValidByte; //relative to struct.  lastValidByte should reference end of telemetryData (237).  See datasheet.

        bool isValidData(void *pos, uint8_t size = 2); //helper function to identify loop data in valid locations.  Pass in the pointer to the data, and size of the data (length in bytes).
    };

#pragma pack(pop)

  protected:
    FaultLogLtc2978   *faultLog2978;

  public:
    //! Constructor
    LT_2978FaultLog(LT_PMBus *pmbus //!< pmbus object reference for this fault log handler to use.
                   );

    //! Pretty prints this part's fault log to a Print inheriting object, or Serial if none specified.
    void print();

    //! Get binary of the fault log or NULL if no log
    uint8_t *getBinary();

    //! Get size of binary data
    uint16_t getBinarySize();

    //! Dumps binary of the fault log to a Print inheriting object, or Serial if none specified.
    void dumpBinary();

    //! Reads the fault log from the specified address, reserves memory to hold the data.
    //! @return a reference to the data read from the part.
    void read(uint8_t address  //!< the address to read the fault log from.
             );

    // ! Get the fault log data
    struct FaultLogLtc2978 *get()
    {
      return faultLog2978;
    }

    //! Frees the memory reserved for the fault log.
    void release();

  private:
    char *buffer;
    Peak16Words **voutPeaks;
    VoutData **voutDatas;

    void printTitle();
    void printTime();
    void printPeaks();
    void printFastChannel(uint8_t index);
    void printAllLoops();
    void printLoop(uint8_t index);
    void printLoopChannel(uint8_t index);

};

#endif /* LT_FaultLog_H_ */
