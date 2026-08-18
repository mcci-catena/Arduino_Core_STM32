/*
  Copyright (c) 2011 Arduino.  All right reserved.
  Copyright (c) 2023 MCCI Corporation.  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "variant.h"
#include "stm32l0xx_ll_rcc.h"

#ifdef __cplusplus
extern "C" {
#endif

// Pin number
// This array allows to map Arduino pin number(Dx or x)
// to STM32 PinName (PX_n) -- don't forget to also change variant.h
const PinName digitalPin[] = {
//PX_n,  //Dx
  PA_10, //D0      D0_RX
  PA_9,  //D1      D1_TX
  PB_15, //D2      RADIO_MOSI   Murata internal
  PB_14, //D3      RADIO_MISO   Murata internal
  PB_13, //D4      RADIO_SCK    Murata internal
  PB_6,  //D5      D5
  PA_13, //D6      D6_SWDIO
  PB_12, //D7      RADIO_NSS    Murata internal
  PB_1,  //D8      RADIO_RESET  Murata internal
  PA_14, //D9      D9_SWCLK
  PA_7,  //D10     D10
  PA_6,  //D11     D11
  PB_7,  //D12     D12
  PB_2,  //D13     D13_LED
  PA_4,  //D14/A0  A0
  PA_3,  //D15/A1  A1
  PA_2,  //D16/A2  A2
  PA_0,  //D17/A3  A3
  PA_1,  //D18/A4  A4
  PA_8,  //D19     NSS
  PB_9,  //D20     SDA
  PB_8,  //D21     SCL
  PB_4,  //D22     MISO
  PB_5,  //D23     MOSI
  PB_3,  //D24     SCK
  PB_0,  //D25     RADIO_DIO_1   (main IRQ)
  PA_15, //D26     RADIO_ANT_SWITCH_RX   (CRF1, TX/RX control)
  PA_5,  //D27     ACC_INT_1
  PB_11, //D28     SDA_PMIC
  PB_10, //D29     SCL_PMIC
  PC_2,  //D30     RADIO_BUSY Murata internal
};

#ifdef __cplusplus
}
#endif

// ----------------------------------------------------------------------------

