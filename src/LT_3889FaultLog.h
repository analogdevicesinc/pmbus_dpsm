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

#ifndef LT_3889FaultLog_H_
#define LT_3889FaultLog_H_

#include "LT_SMBus.h"
#include "LT_FaultLog.h"
#include "LT_PMBusMath.h"
#include "LT_EEDataFaultLog.h"

//! class that handles LTC3889 fault logs.
//! contains structs for interpreting the data read from the part.
class LT_3889FaultLog : public LT_EEDataFaultLog
{

  public:
#pragma pack(push, 1)


    /********************************************************************
     * LTC3889 types
     ********************************************************************/

    struct FaultLogTelemetrySummaryLtc3889
    {
      public:
        struct Lin16WordReverse mfr_vout_peak_p0;
        struct Lin16WordReverse mfr_vout_peak_p1;

        struct Lin5_11WordReverse mfr_iout_peak_p0;
        struct Lin5_11WordReverse mfr_iout_peak_p1;

        struct Lin5_11WordReverse mfr_vin_peak;
        struct Lin5_11WordReverse read_temperature_1_p0;
        struct Lin5_11WordReverse read_temperature_1_p1;
        struct Lin5_11WordReverse read_temperature_2;
    };

    struct FaultLogPreambleLtc3889
    {
      public:
        uint16_t lt;
        uint16_t mfr_special_id;
        uint8_t position_fault;
        struct FaultLogTimeStamp shared_time;
        struct FaultLogTelemetrySummaryLtc3889 peaks;
    };

    struct FaultLogReadLoopLtc3889
    {
      public:
        struct Lin16WordReverse read_vout_p0;
        struct Lin16WordReverse read_vout_p1;

        struct Lin5_11WordReverse read_iout_p0;
        struct Lin5_11WordReverse read_iout_p1;

        struct Lin5_11WordReverse read_vin ;
        struct Lin5_11WordReverse read_iin;

        struct RawByte status_vout_p0;
        struct RawByte status_vout_p1;

        struct RawWordReverse status_word_p0;
        struct RawWordReverse status_word_p1;

        struct RawByte status_mfr_specificP0;
        struct RawByte status_mfr_specificP1;
    };

    struct FaultLogLtc3889
    {
      public:
        struct FaultLogPreambleLtc3889 preamble;
        struct FaultLogReadLoopLtc3889 fault_log_loop[6];
    };

#pragma pack(pop)

  protected:
    FaultLogLtc3889   *faultLog3889;

  public:
    //! Constructor
    LT_3889FaultLog(LT_PMBus *pmbus   //!< pmbus object reference for this fault log handler to use.
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
    struct FaultLogLtc3889 *get()
    {
      return faultLog3889;
    }

    //! Frees the memory reserved for the fault log.
    void release();

  private:
    char *buffer;

    void printTitle();
    void printTime();
    void printPeaks();
    void printAllLoops();
    void printLoop(uint8_t index);

};

#endif /* LT_FaultLog_H_ */
