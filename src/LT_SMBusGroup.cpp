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

#include "LT_SMBusGroup.h"
    
LT_SMBusGroup::LT_SMBusGroup(LT_SMBus *smbus) : LT_SMBusBase()
{
  executor = smbus;
  queueing = false;
  head = NULL;
  tail = NULL;
}

LT_SMBusGroup::LT_SMBusGroup(LT_SMBus *smbus, uint32_t speed) : LT_SMBusBase(speed)
{
  executor = smbus;
  queueing = false;
  head = NULL;
  tail = NULL;
}

int LT_SMBusGroup::writeByte(uint8_t address, uint8_t command, uint8_t data)
{
  if (queueing)
    return addToQueue(new WriteByte(executor, address, command, data));
  else
    return executor->writeByte(address, command, data);
}

int LT_SMBusGroup::writeBytes(uint8_t *addresses, uint8_t *commands, uint8_t *data, uint8_t no_addresses)
{
  if (queueing)
    return addToQueue(new WriteBytes(executor, addresses, commands, data, no_addresses));
  else
    return executor->writeBytes(addresses, commands, data, no_addresses);
}

int LT_SMBusGroup::readByte(uint8_t address, uint8_t command)
{
  if (queueing)
    return executor->readByte(address, command);
  else
    return executor->readByte(address, command);
}

int LT_SMBusGroup::writeWord(uint8_t address, uint8_t command, uint16_t data)
{
  if (queueing)
    return addToQueue(new WriteWord(executor, address, command, data));
  else
    return executor->writeWord(address, command, data);
}

int LT_SMBusGroup::readWord(uint8_t address, uint8_t command)
{
  if (queueing)
    return executor->readWord(address, command);
  else
    return executor->readWord(address, command);
}

int LT_SMBusGroup::writeBlock(uint8_t address, uint8_t command,
                               uint8_t *block, uint16_t block_size)
{
  if (queueing)
    return addToQueue(new WriteBlock(executor, address, command, block, block_size));
  else
    return executor->writeBlock(address, command, block, block_size);
}

int LT_SMBusGroup::writeReadBlock(uint8_t address, uint8_t command,
                                      uint8_t *block_out, uint16_t block_out_size, uint8_t *block_in, uint16_t block_in_size)
{
  if (queueing)
    return 0;
  else
    return executor->writeReadBlock(address, command, block_out, block_out_size, block_in, block_in_size);
}

int LT_SMBusGroup::readBlock(uint8_t address, uint8_t command,
                                 uint8_t *block, uint16_t block_size)
{
  if (queueing)
    return 0;
  else
    return executor->readBlock(address, command, block, block_size);
}

int LT_SMBusGroup::sendByte(uint8_t address, uint8_t command)
{
  if (queueing)
    return addToQueue(new SendByte(executor, address, command));
  else
    return executor->sendByte(address, command);
}

int LT_SMBusGroup::readAlert(void)
{
//  if (queueing)
//    return addToQueue(new SendByte(executor, address, command));
//  else
    return executor->readAlert();
}

int LT_SMBusGroup::waitForAck(uint8_t address, uint8_t command)
{
//  if (queueing)
//    return addToQueue(new SendByte(executor, address, command));
//  else
    return executor->waitForAck(address, command);
}

uint8_t *LT_SMBusGroup::probeUnique(uint8_t command)
{
  return executor->probeUnique(command);
}

void LT_SMBusGroup::beginStoring()
{
  queueing = true;
  head = new Node();
  tail = head;
}

int LT_SMBusGroup::addToQueue(Executable *ex)
{
  if (!ex)
    return -1;

  Node *newNode = new Node();
  if (!newNode)
  {
    delete ex;
    return -1;
  }
  newNode->executable = ex;
  tail->next = newNode;
  tail = newNode;
  return 0;
}

void LT_SMBusGroup::execute()
{
  // Not implemented
  /*
  executor->i2cbus()->startGroupProtocol();
  tail = head;
  while ((tail = head->next) != NULL)
  {
    delete head;
    head = tail;
    if (!head->next)
      executor->i2cbus()->endGroupProtocol();
    head->executable->execute();
    delete head->executable;
  }
  delete head;
  head = 0;
  tail = 0;
  queueing = false;
  */
}


//private class methods

LT_SMBusGroup::Executable::Executable(LT_SMBus *e)
{
  executor = e;
}

LT_SMBusGroup::WriteByte::WriteByte(LT_SMBus *e, uint8_t a, uint8_t c, uint8_t d) : Executable(e)
{
  address = a;
  command = c;
  data = d;
}
void LT_SMBusGroup::WriteByte::execute()
{
  executor->writeByte(address, command, data);
}

LT_SMBusGroup::WriteBytes::WriteBytes(LT_SMBus *e, uint8_t *a, uint8_t *c, uint8_t *d, uint8_t n) : Executable(e)
{
  addresses = a;
  commands = c;
  data = d;
  no_addresses = n;
}
void LT_SMBusGroup::WriteBytes::execute()
{
  executor->writeBytes(addresses, commands, data, no_addresses);
};

LT_SMBusGroup::WriteWord::WriteWord(LT_SMBus *e, uint8_t a, uint8_t c, uint16_t d) : Executable(e)
{
  address = a;
  command = c;
  data = d;
}
void LT_SMBusGroup::WriteWord::execute()
{
  executor->writeWord(address, command, data);
};

LT_SMBusGroup::WriteBlock::WriteBlock(LT_SMBus *e, uint8_t a, uint8_t c, uint8_t *b, uint16_t bl) : Executable(e)
{
  address = a;
  command = c;
  block = b;
  block_size = bl;
}
void LT_SMBusGroup::WriteBlock::execute()
{
  executor->writeBlock(address, command, block, block_size);
};

LT_SMBusGroup::SendByte::SendByte(LT_SMBus *e, uint8_t a, uint8_t c) : Executable(e)
{
  address = a;
  command = c;
};
void LT_SMBusGroup::SendByte::execute()
{
  executor->sendByte(address, command);
};
