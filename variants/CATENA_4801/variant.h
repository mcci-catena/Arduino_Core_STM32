/*
  Copyright (c) 2011 Arduino.  All right reserved.

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

#ifndef _VARIANT_ARDUINO_CATENA_4801_
#define _VARIANT_ARDUINO_CATENA_4801_

/*----------------------------------------------------------------------------
 *        Headers
 *----------------------------------------------------------------------------*/
#include "PeripheralPins.h"

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus

/*----------------------------------------------------------------------------
 *        Pins
 *----------------------------------------------------------------------------*/
extern const PinName digitalPin[];

// Enum defining pin names to match digital pin number --> Dx
// !!!
// !!! Copy the digitalPin[] array in variant.cpp
// !!! and remove all '_': PX_n --> PXn
// !!! For NC, suffix by _x where x is the number of NC:
// !!!   NC_1, NC_2,...
// !!! For duplicated pin name, suffix by _x where x is the number of pin:
// !!! PA7, PA7_2, PA7_3,...
enum {
  PA10, //D0      D0_RX
  PA9,  //D1      D1_TX
  PA7,  //D2      RADIO_MOSI   Murata internal
  PA6,  //D3      RADIO_MISO   Murata internal
  PB3,  //D4      RADIO_SCK    Murata internal
  PB6,  //D5      EN
  PA13, //D6      D6_SWDIO
  PA15, //D7      RADIO_NSS    Murata internal
  PC0,  //D8      RADIO_RESET  Murata internal
  PA14, //D9      D9_SWCLK
  PH1,  //D10     D10
  PB5,  //D11     D11
  PB7,  //D12     D12
  PB2,  //D13     D13
  PA0,  //D14/A0  A0_VBAT_DET
  PA5,  //D15/A1
  PA4,  //D16/A2
  PA3,  //D17/A3  A3_RS485_RX
  PA2,  //D18/A4  A4_RS485_TX
  PB12, //D19     NSS
  PB9,  //D20     SDA
  PB8,  //D21     SCL
  PB14, //D22     MISO
  PB15, //D23     MOSI
  PB13, //D24     SCK
  PB4,  //D25     RADIO_DIO_0  Murata internal
  PB1,  //D26     RADIO_DIO_1  Murata internal
  PB0,  //D27     RADIO_DIO_2  Murata internal
  PC13, //D28     RADIO_DIO_3  Murata internal
  PA1,  //D29     RADIO_ANT_SWITCH_RX       CRF1  Murata internal
  PC1,  //D30     RADIO_ANT_SWITCH_TX_BOOST CRF3  Murata internal
  PC2,  //D31     RADIO_ANT_SWITCH_TX_RFO   CRF2  Murata internal
  NC_1, //D32     Flash Write-Protect (not supported these platforms)
  PA8,  //D33     TCXO VDD, possibly VDD boost enable
  PEND
};

// This must be a literal with the same value as PEND
#define NUM_DIGITAL_PINS        34
// This must be a literal with a value less than or equal to to MAX_ANALOG_INPUTS
#define NUM_ANALOG_INPUTS       5
#define NUM_ANALOG_FIRST        14

//On-board LED pin number
#define LED_BUILTIN             D13
#define LED_RED                 LED_BUILTIN

//On-board user button
//#define USER_BTN                NC


//SPI definitions
#define RADIO_SS                D7
#define RADIO_MOSI              D2
#define RADIO_MISO              D3
#define RADIO_SCK               D4
#define RADIO_RESET             D8

#define FLASH_SS                D19
#define FLASH_MOSI              D23
#define FLASH_MISO              D22
#define FLASH_SCK               D24

#define PIN_SPI_SS              RADIO_SS   // Default for Arduino connector compatibility
#define PIN_SPI_MOSI            RADIO_MOSI // Default for Arduino connector compatibility
#define PIN_SPI_MISO            RADIO_MISO // Default for Arduino connector compatibility
#define PIN_SPI_SCK             RADIO_SCK  // Default for Arduino connector compatibility

//I2C Definitions
#define PIN_WIRE_SDA            D20 // Default for Arduino connector compatibility
#define PIN_WIRE_SCL            D21 // Default for Arduino connector compatibility

//Timer Definitions
//Do not use timer used by PWM pins when possible. See PinMap_PWM in PeripheralPins.c
#define TIMER_TONE              TIM3

//Do not use basic timer: OC is required
#define TIMER_SERVO             TIM2  //TODO: advanced-control timers don't work

// UART Definitions
// Define here Serial instance number to map on Serial generic name
#define SERIAL_UART_INSTANCE    1 //ex: 2 for Serial2 (USART2)
// Default pin used for 'Serial' instance (ex: ST-Link)
// Mandatory for Firmata
#define PIN_SERIAL_RX           D0
#define PIN_SERIAL_TX           D1

#ifdef __cplusplus
} // extern "C"
#endif
/*----------------------------------------------------------------------------
 *        Arduino objects - C++ only
 *----------------------------------------------------------------------------*/

#ifdef __cplusplus
// These serial port names are intended to allow libraries and architecture-neutral
// sketches to automatically default to the correct port name for a particular type
// of use.  For example, a GPS module would normally connect to SERIAL_PORT_HARDWARE_OPEN,
// the first hardware serial port whose RX/TX pins are not dedicated to another use.
//
// SERIAL_PORT_MONITOR        Port which normally prints to the Arduino Serial Monitor
//
// SERIAL_PORT_USBVIRTUAL     Port which is USB virtual serial
//
// SERIAL_PORT_LINUXBRIDGE    Port which connects to a Linux system via Bridge library
//
// SERIAL_PORT_HARDWARE       Hardware serial port, physical RX & TX pins.
//
// SERIAL_PORT_HARDWARE_OPEN  Hardware serial ports which are open for use.  Their RX & TX
//                            pins are NOT connected to anything by default.
#ifdef USBCON
# define SERIAL_PORT_MONITOR  SerialUSB
#else
# define SERIAL_PORT_MONITOR  Serial
#endif
#define SERIAL_PORT_HARDWARE  Serial
#endif

#endif /* _VARIANT_ARDUINO_CATENA_461x_ */
