/*

Module: override_usbd_ll_connectionstate.c

Function:
        Override USBD_LL_ConnectionState() function for MODEL_4917 variant.

Copyright notice and license information:
        Copyright 2018-2023 MCCI Corporation. All rights reserved.

        This library is free software; you can redistribute it and/or
        modify it under the terms of the GNU Lesser General Public
        License as published by the Free Software Foundation; either
        version 2.1 of the License, or (at your option) any later version.

        This library is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
        See the GNU Lesser General Public License for more details.

        You should have received a copy of the GNU Lesser General Public
        License along with this library; if not, write to the Free
        Software Foundation, Inc., 51 Franklin St, Fifth Floor,
        Boston, MA  02110-1301  USA

Author:
        Dhinesh Kumar Pitchai, MCCI Corporation

*/

#include <Arduino.h>
#include <usbd_conf.h>

// analogRead() reads near 0 with no USB host present and rises well above
// this threshold when VBUS is driven; this is a raw ADC count, not a
// voltage, since no HAL conversion is applied here.
#ifndef USB_VBUS_PRESENT_THRESHOLD_ADC
#define USB_VBUS_PRESENT_THRESHOLD_ADC 270
#endif

#ifdef USBCON

/**
  * @brief  Get USB connection state
  * @param  None
  * @retval 0 if disconnected
  */
USBD_LL_ConnectionState_WEAK uint32_t USBD_LL_ConnectionState(void)
{
  uint32_t vBus;

  vBus = analogRead(18);
  return vBus > USB_VBUS_PRESENT_THRESHOLD_ADC ? 1 : 0;
}

#endif // USBCON
