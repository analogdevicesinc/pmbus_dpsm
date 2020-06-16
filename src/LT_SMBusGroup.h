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

#ifndef LT_SMBusGroup_H_
#define LT_SMBusGroup_H_

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "LT_SMBusBase.h"

class LT_SMBusGroup : public LT_SMBusBase
{
  private:
    class Executable
    {
      protected:
        LT_SMBus *executor;

        Executable(LT_SMBus *e);

      public:
        virtual ~Executable(){}
        virtual void execute() = 0;
    };

    class WriteByte : public Executable
    {
        uint8_t address;
        uint8_t command;
        uint8_t data;

      public:
        WriteByte(LT_SMBus *e, uint8_t a, uint8_t c, uint8_t d);
        void execute();
    };

    class WriteBytes : public Executable
    {
        uint8_t *addresses;
        uint8_t *commands;
        uint8_t *data;
        uint8_t no_addresses;

      public:
        WriteBytes(LT_SMBus *e, uint8_t *a, uint8_t *c, uint8_t *d, uint8_t n);
        void execute();
    };

    class WriteWord : public Executable
    {
        uint8_t address;
        uint8_t command;
        uint16_t data;

      public:
        WriteWord(LT_SMBus *e, uint8_t a, uint8_t c, uint16_t d);
        void execute();
    };

    class WriteBlock : public Executable
    {
        uint8_t address;
        uint8_t command;
        uint8_t *block;
        uint16_t block_size;

      public:
        WriteBlock(LT_SMBus *e, uint8_t a, uint8_t c, uint8_t *b, uint16_t bl);
        void execute();
    };

    class SendByte : public Executable
    {
        uint8_t address;
        uint8_t command;

      public:
        SendByte(LT_SMBus *e, uint8_t a, uint8_t c);
        void execute();
    };

    class Node
    {
      public:
        Executable *executable;
        Node *next;
    };


  private:
    LT_SMBus *executor;
    bool queueing;
    Node *head;
    Node *tail;

    int addToQueue(Executable *);

  public:

    LT_SMBusGroup(LT_SMBus *);
    LT_SMBusGroup(LT_SMBus *, uint32_t speed);
    virtual ~LT_SMBusGroup(){}

    //! SMBus write byte command
    //! @return error < 0
    int writeByte(uint8_t address,   //!< Slave address
                   uint8_t command,   //!< Command byte
                   uint8_t data       //!< Data to send
                  );

    //! SMBus write byte command for a list of addresses
    //! @return error < 0
    int writeBytes(uint8_t *addresses,     //!< Slave Addresses
                    uint8_t *commands,      //!< Command bytes
                    uint8_t *data,          //!< Data to send
                    uint8_t no_addresses
                   );

    //! SMBus read byte command
    //! @return error < 0
    int readByte(uint8_t address,     //!< Slave Address
                     uint8_t command      //!< Command byte
                    );

    //! SMBus write word command
    //! @return error < 0
    int writeWord(uint8_t address,   //!< Slave Address
                   uint8_t command,   //!< Command byte
                   uint16_t data      //!< Data to send
                  );

    //! SMBus read word command
    //! @return error < 0
    int readWord(uint8_t address,    //!< Slave Address
                      uint8_t command     //!< Command byte
                     );

    //! SMBus write block command
    //! @return error < 0
    int writeBlock(uint8_t address,    //!< Slave Address
                    uint8_t command,    //!< Command byte
                    uint8_t *block,     //!< Data to send
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
    int readBlock(uint8_t address,     //!< Slave Address
                      uint8_t command,     //!< Command byte
                      uint8_t *block,      //!< Memory to receive data
                      uint16_t block_size  //!< Size of receive data memory
                     );

    //! SMBus send byte command
    //! @return error < 0
    int sendByte(uint8_t address,    //!< Slave Address
                  uint8_t command     //!< Command byte
                 );

    //! Perform ARA
    //! @return error < 0
    int readAlert(void);

    //! Read with the address and command in loop until ack, then issue stop
    //! @return error < 0
    int waitForAck(uint8_t address,        //!< Slave Address
                       uint8_t command         //!< Command byte
                      );
    
    uint8_t *probeUnique(uint8_t command      //!< Command byte
                        );
    //! Group Protocol Begin
    //! @return void
    void beginStoring();

    //! Group Protocol Execute queued commands
    //! @return void
    void execute();
};

#endif /* LT_SMBusGroup_H_ */
