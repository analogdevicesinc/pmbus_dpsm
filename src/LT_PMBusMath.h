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

#ifndef LT_PMBusMath_H_
#define LT_PMBusMath_H_

class LT_PMBusMath
{

  public:

    typedef unsigned long   fl32_t;    // Type for the bit representation of "float"
    typedef unsigned int    lin11_t;   // Type for PMBus Linear11 mantissa
    typedef unsigned int    lin16_t;   // Type for PMBus Linear16 mantissa
    typedef unsigned int    lin16m_t;  // Type for PMBus Linear16 VOUT_MODE
    typedef int             slin11_t;  // A signed type for PMBus Linear11 exponent
    typedef unsigned char   uchar_t;   // Type for bit shifts

    fl32_t lin11_to_fl32 (lin11_t xin);
    fl32_t lin16_to_fl32 (lin16_t lin16_mant, lin16_t lin16_exp);
    lin16_t fl32_to_lin16 (fl32_t xin, lin16_t lin16_exp);
    lin11_t fl32_to_lin11 (fl32_t xin);

    float lin11_to_float (lin11_t xin);
    float lin16_to_float (lin16_t lin16_mant, lin16m_t vout_mode);
    lin11_t float_to_lin11 (float xin);
    lin16_t float_to_lin16 (float xin, lin16m_t vout_mode);

};

extern LT_PMBusMath math_;

#endif /* LT_PMBusMath_H_ */
