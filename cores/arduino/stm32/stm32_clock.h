/*

Module: stm32_clock.h

Function:
        System clock calibration header for STM32 platform.

Copyright notice and license information:
        Copyright 2019 MCCI Corporation. All rights reserved.

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

#ifndef _STM32_CLOCK_H_          /* prevent multiple includes */
#define _STM32_CLOCK_H_

#include "stm32_def.h"

#ifdef __cplusplus
 extern "C" {
#endif

#ifdef STM32L0xx

uint32_t Stm32CalibrateSystemClock(void);
uint32_t Stm32MeasureMicrosPerRtcSecond(void);

#endif	/* STM32L0xx */

#ifdef __cplusplus
}
#endif

/**** end of stm32_clock.h ****/
#endif /* _STM32_CLOCK_H_ */
