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

#ifndef LT_Dongle_H_
#define LT_Dongle_H_

#include <stdint.h>

class LT_Dongle
{
  protected:
    static bool open_;          //!< Used to ensure initialisation of i2c once
    static int32_t file_;


  public:

    enum TransactionState { STARTED, RESTARTED, STOPPED };
    enum ProtocolState {  IDLE, ADDRESS1, ADDRESS2, SEND,
                        COMMAND, MODE, AUX, READ_BYTE, READ_WORD, READ_BLOCK, READ_GPIO, WRITE_BYTE, WRITE_WORD, WRITE_GPIO, DATA };


    LT_Dongle ();
    LT_Dongle(char *dev);
    
    ~LT_Dongle();

    void clearBuffer();
    
    //! Change the speed of the bus.
    void changeSpeed(uint32_t speed  //!< the speed
                    );

    //! Get the speed of the bus.
    uint32_t getSpeed();

    void setPec();
    void setNoPec();
    
    void reset();
    
    char recvChar();
    void sendString(const char *s);
    
    void convertString(const char *s, int length, uint8_t *data);
    
    char *toStateString(ProtocolState state);
    
    void sendRecvBytes();
    void sendReadByte();
    void sendReadWord();
    void sendReadBlock();
    void sendReadGpio();
    void sendWriteByte();
    void sendWriteWord();
    void sendWriteGpio();

    //! Process command
    void processCommand(uint8_t command
                                );

};

#endif /* LT_Dongle_H_ */
