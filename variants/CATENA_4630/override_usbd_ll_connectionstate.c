/*

Module: override_usbd_ll_connectionstate.c

Function:
        Override USBD_LL_ConnectionState() function for CATENA_461x variant.

Copyright notice and license information:
        Copyright 2018-2019 MCCI Corporation. All rights reserved.

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

#ifdef USBCON

#define ANALOG_CHANNEL_VBUS	4
#define READ_COUNT		1
#define	MULTIPLIER		3

/**
  * @brief  Get USB connection state
  * @param  None
  * @retval 0 if disconnected
  */
uint32_t USBD_LL_ConnectionState(void)
	{
	uint32_t vBus;

	vBus = Stm32ReadAnalog(ANALOG_CHANNEL_VBUS, READ_COUNT, MULTIPLIER);
	return vBus < 3000 ? 0 : 1;
	}

#endif // USBCON
