/*

Module: override_usbd_ll_connectionstate.c

Function:
        Override USBD_LL_ConnectionState() function for CATENA_4551 variant.

Copyright notice and license information:
        Copyright 2018-2026 MCCI Corporation. All rights reserved.

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
        ChaeHee Won, MCCI Corporation

*/

#include <Arduino.h>
#include <usbd_conf.h>

//
// adc_read_value() reads near 0 with no USB host present and rises well above
// this threshold when VBUS is driven; this is a raw ADC count, not a
// voltage.
//
// Vbus is divided by 3, and the ADC full-scale range is 3.3V, so volts at VBUS are
// fraction of full scale * 9.9V. All approximate, of course, because Vdda is not
// a precision 3.3V source, and the resistors in the external divider are up to
// 1% off.
//
#ifndef USB_VBUS_PRESENT_THRESHOLD_ADC
#define USB_VBUS_PRESENT_THRESHOLD_ADC (250 * 4)        // 250 counts/1024, scaled for /4096 == 2.4 volts
#endif

// the pin that VBUS is wired to on this platform.
#ifndef USB_VBUS_DETECT_PIN
#define USB_VBUS_DETECT_PIN     18
#endif

#ifdef USBCON

/**
  * @brief  Get USB connection state
  * @param  None
  * @retval 0 if disconnected
  */
USBD_LL_ConnectionState_WEAK uint32_t USBD_LL_ConnectionState(void)
{
  // we can't use analogRead directly because the application can
  // change the resolution with analogReadResolution() and there's
  // no API to read it back.
  // adc_read_value() calls the HAL which returns 12-bit counts.
  PinName p = analogInputToPinName(USB_VBUS_DETECT_PIN);
  if(p != NC) {
    uint32_t vBus;
    // errors in adc_read_value / analogRead return 0, so we
    // return 0 on failure.
    vBus = adc_read_value(p);
    return vBus > USB_VBUS_PRESENT_THRESHOLD_ADC ? 1 : 0;
  } else {
    // return 0 on failure.
    return 0;
  }
}

#endif // USBCON
