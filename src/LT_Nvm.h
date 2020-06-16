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

#ifndef NVM_H_
#define NVM_H_

#include <stdbool.h>
#include <stdint.h>
#include "LT_PMBus.h"
#include "LT_SMBusNoPec.h"
#include "LT_SMBusPec.h"
#include "main_record_processor.h"
#include "hex_file_parser.h"


extern LT_PMBus *pmbus__;         //!< global accessor for IFU helper files
extern LT_SMBusNoPec *smbusNoPec__;   //!< global accessor for IFU helper files
extern LT_SMBusPec *smbusPec__;     //!< global accessor for IFU helper files


// extern uint8_t get_hex_data(void);

//! Data retrieving method.
extern uint8_t get_hex_data(void);
//! Data retrieving method.
extern uint8_t get_record_data(void);
//! Data retrieving method.
extern pRecordHeaderLengthAndType get_record(void);

class NVM
{
  private:
    uint8_t *addr;
    uint8_t numAddrs;

  public:
    //! Constructor.
    NVM(LT_PMBus *,         //!< reference to pmbus object for this library to use
        LT_SMBusNoPec *,    //!< reference to no pec smbus object
        LT_SMBusPec *       //!< reference to pec smbus object
       );

    //! Program with hex data.
    //! @return true if data loaded.
    bool programWithData(const char * //!< array of hex data
                        );

    //! Verifies board NVM with hex data..
    //! @return true if NVM configuration matches the hex data.
    bool verifyWithData(const char *);

    //! Program with hex file data.
    //! @return true if data loaded.
    bool programWithFileData(const char * //!< array of hex data
                        );

    //! Verifies board NVM with hex file data..
    //! @return true if NVM configuration matches the hex data.
    bool verifyWithFileData(const char *);

};

#endif /* NVM_H_ */
