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

#ifndef LT_SMBus_H_
#define LT_SMBus_H_

#include <stdint.h>

class LT_SMBus
{
  protected:
    bool pec_enabled_;

    uint8_t               running_pec_;     //!< Temporary pec calc value
    unsigned char         poly_;            //!< The poly used in the calc
    uint16_t              crc_polynomial_;  //!< The crc poly used in the calc


    //! Initialize the table used to speed up pec calculations
    //! @return void
    void constructTable(uint16_t user_polynomial  //!< The controlling polynomial
                       );

    //! Helper function for pec
    //! @return value
    uint8_t doCalculate(uint8_t data,
                        uint8_t begining_value  //!< The initial value
                       );

    LT_SMBus ();

  public:
    virtual ~LT_SMBus(){}

    //! Change the speed of the bus.
    virtual void changeSpeed(uint32_t speed  //!< the speed
                    ) = 0;

    //! Get the speed of the bus.
    virtual uint32_t getSpeed() = 0;

    //! Check if PEC is enabled
    //! @return true if enabled
    bool pecEnabled(void)
    {
      return pec_enabled_;
    }

    //! Clear the pec value so it can start a new calculation
    //! @return void
    void pecClear(void);

    //! Add a byte to the pec calculation
    //! @return void
    void pecAdd(uint8_t byte_value);

    //! Get the current pec result
    //! @return the pec
    uint8_t pecGet(void);

    //! Check CRC of block data organized as 31 data bytes plus CRC.
    //! Do not mix with PEC calculations.
    //! Return true if CRC does not match.
    bool checkCRC (uint8_t *data);

    //! Helper function for pec
    //! @return value
    uint8_t calculate(uint8_t *data,          //!< Data to be pec'ed
                      uint8_t begining_value, //!< Starting value for pec
                      uint8_t start_index,    //!< Starting index
                      uint8_t length          //!< Length of data
                     );

    //! Perform ARA
    //! @return error < 0
    virtual int readAlert(void) = 0;

    //! SMBus write byte command
    //! @return error < 0
    virtual int writeByte(uint8_t address,   //!< Slave address
                           uint8_t command,   //!< Command byte
                           uint8_t data       //!< Data to send
                          ) = 0;

    //! SMBus write byte command for a list of addresses
    //! @return error < 0
    virtual int writeBytes(uint8_t *addresses,     //!< Slave Addresses
                            uint8_t *commands,      //!< Command bytes
                            uint8_t *data,          //!< Data to send
                            uint8_t no_addresses
                           ) = 0;

    //! SMBus read byte command
    //! @return error < 0
    virtual int readByte(uint8_t address,     //!< Slave Address
                             uint8_t command      //!< Command byte
                            ) = 0;

    //! SMBus write word command
     //! @return error < 0
   virtual int writeWord(uint8_t address,   //!< Slave Address
                           uint8_t command,   //!< Command byte
                           uint16_t data      //!< Data to send
                          ) = 0;

    //! SMBus read word command
    //! @return error < 0
    virtual int readWord(uint8_t address,    //!< Slave Address
                              uint8_t command     //!< Command byte
                             ) = 0;

    //! SMBus write block command
    //! @return error < 0
    virtual int writeBlock(uint8_t address,    //!< Slave Address
                            uint8_t command,    //!< Command byte
                            uint8_t *block,     //!< Data to send
                            uint16_t block_size
                           ) = 0;

    //! SMBus write then read block command
    //! @return error < 0
    virtual int writeReadBlock(uint8_t address,         //!< Slave Address
                                   uint8_t command,         //!< Command byte
                                   uint8_t *block_out,      //!< Data to send
                                   uint16_t block_out_size, //!< Size of data to send
                                   uint8_t *block_in,       //!< Memory to receive data
                                   uint16_t block_in_size   //!< Size of receive data memory
                                  ) = 0;

    //! SMBus read block command
    //! @return error < 0 | count
    virtual int readBlock(uint8_t address,     //!< Slave Address
                              uint8_t command,     //!< Command byte
                              uint8_t *block,      //!< Memory to receive data
                              uint16_t block_size  //!< Size of receive data memory
                             ) = 0;

    //! SMBus send byte command
    //! @return error < 0
    virtual int sendByte(uint8_t address,    //!< Slave Address
                          uint8_t command     //!< Command byte
                         ) = 0;

    //! Read with the address and command in loop until ack, then issue stop
    //! @return error < 0
    virtual int waitForAck(uint8_t address,     //!< Slave Address
                               uint8_t command      //!< Command byte
                              ) = 0;

    //! SMBus bus probe
    //! @return array of addresses (caller must not delete return memory)
    virtual uint8_t *probe(uint8_t command    //!< Command byte
                          ) = 0;

    //! SMBus bus probe
    //! @return array of unique addresses (no global addresses)
    virtual uint8_t *probeUnique(uint8_t command    //!< Command byte
                                ) = 0;

};

#endif /* LT_SMBus_H_ */
