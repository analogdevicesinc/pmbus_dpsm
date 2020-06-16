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

#ifndef LT_SMBusBase_H_
#define LT_SMBusBase_H_

#include <stdio.h>
#include "LT_SMBus.h"

class LT_SMBusBase : public LT_SMBus
{
  protected:
    static bool open_;          //!< Used to ensure initialisation of i2c once
    static uint8_t found_address_[];
    static int32_t file_;
    bool pec;
    
    LT_SMBusBase();
    LT_SMBusBase(uint32_t speed);
    ~LT_SMBusBase();

    void setPec();
 public:

    void clearBuffer();

    //! Change the speed of the bus.
    void changeSpeed(uint32_t speed  //!< the speed
                    );

    //! Get the speed of the bus.
    uint32_t getSpeed();

    //! SMBus write byte command
    //! @return error < 0
    int writeByte(uint8_t address,     //!< Slave address
                   uint8_t command,     //!< Command byte
                   uint8_t data         //!< Data to send
                  );

    //! SMBus write byte command for a list of addresses
    //! @return error < 0
    int writeBytes(uint8_t *addresses,         //!< Slave Addresses
                    uint8_t *commands,          //!< Command bytes
                    uint8_t *data,              //!< Data to send
                    uint8_t no_addresses
                   );

    //! SMBus read byte command
    //! @return error < 0
    int readByte(uint8_t address,        //!< Slave Address
                     uint8_t command         //!< Command byte
                    );

    //! SMBus write word command
    //! @return error < 0
    int writeWord(uint8_t address,     //!< Slave Address
                   uint8_t command,     //!< Command byte
                   uint16_t data        //!< Data to send
                  );

    //! SMBus read word command
    //! @return error < 0
    int readWord(uint8_t address,      //!< Slave Address
                      uint8_t command       //!< Command byte
                     );

    //! SMBus write block command
    //! @return error < 0
    int writeBlock(uint8_t address,        //!< Slave Address
                    uint8_t command,        //!< Command byte
                    uint8_t *block,         //!< Data to send
                    uint16_t block_size
                   );

    //! SMBus write then read block command
    //! @return error < 0 | count
    int writeReadBlock(uint8_t address,         //!< Slave Address
                           uint8_t command,         //!< Command byte
                           uint8_t *block_out,      //!< Data to send
                           uint16_t block_out_size, //!< Size of data to send
                           uint8_t *block_in,       //!< Memory to receive data
                           uint16_t block_in_size   //!< Size of receive data memory
                          );

    //! SMBus read block command
    //! @return error < 0
    int readBlock(uint8_t address,         //!< Slave Address
                      uint8_t command,         //!< Command byte
                      uint8_t *block,          //!< Memory to receive data
                      uint16_t block_size      //!< Size of receive data memory
                     );

    //! SMBus send byte command
    //! @return error < 0
    int sendByte(uint8_t address,      //!< Slave Address
                  uint8_t command       //!< Command byte
                 );

    //! Perform ARA
    //! @return error < 0
    int readAlert(void);

    //! Read with the address and command in loop until ack, then issue stop
    //! @return error < 0
    int waitForAck(uint8_t address,        //!< Slave Address
                       uint8_t command         //!< Command byte
                      );

    //! SMBus bus probe
    //! @return array of addresses
    uint8_t *probe(uint8_t command      //!< Command byte
                  );

    //! SMBus bus probe
    //! @return array of unique addresses (no global addresses)
    uint8_t *probeUnique(uint8_t command      //!< Command byte
                        );

};

#endif /* LT_SMBusBase_H_ */
