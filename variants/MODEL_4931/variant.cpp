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
//PX_n, //Dx
  PA_10, //D0      D0_UART_RX
  PA_9,  //D1      D1_UART_TX
  PA_7,  //D2      RADIO_MOSI   Murata internal
  PA_6,  //D3      RADIO_MISO   Murata internal
  PB_3,  //D4      RADIO_SCK    Murata internal
  PB_6,  //D5      D5_TEMP1
  PA_13, //D6      D6_SWDIO
  PA_15, //D7      RADIO_NSS    Murata internal
  PC_0,  //D8      RADIO_RESET  Murata internal
  PA_14, //D9      D9_SWCLK
  PA_4,  //D10     D10_TEMP2
  PB_5,  //D11     D11_nCS
  PB_7,  //D12     D12_PULSE2
  PB_2,  //D13     D13_LED
  PA_0,  //D14/A0  A0_VBUS_MON
  PA_5,  //D15/A1  A1_VBAT_MON
  PH_1,  //D16     D16_PULSE1
  PA_3,  //D17/A3  A3_RS485_RX
  PA_2,  //D18/A4  A4_RS485_TX
  PB_12, //D19     D19_NSS
  PB_9,  //D20     SDA
  PB_8,  //D21     SCL
  PB_14, //D22     MISO
  PB_15, //D23     MOSI
  PB_13, //D24     SCK
  PB_4,  //D25     RADIO_DIO_0
  PB_1,  //D26     RADIO_DIO_1
  PB_0,  //D27     RADIO_DIO_2
  PC_13, //D28     RADIO_DIO_3
  PA_1,  //D29     RADIO_ANT_SWITCH_RX       CRF1
  PC_1,  //D30     RADIO_ANT_SWITCH_TX_BOOST CRF3
  PC_2,  //D31     RADIO_ANT_SWITCH_TX_RFO   CRF2
  NC,    //D32     Flash Write-Protect (not supported these platforms)
  PA_8,  //D33     RADIO_TCXO      TCXO VDD, possibly VDD boost enable.
};

#ifdef __cplusplus
}
#endif