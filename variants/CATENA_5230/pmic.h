/*

Module: pmic.h

Function:
        Header for pmic.cpp, which reads VBUS from the Catena 5230's
        nPM1300 PMIC, for USB connection detection.

Copyright notice and license information:
        Copyright 2025-2026 MCCI Corporation. All rights reserved.

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
        Murali, MCCI Corporation  September 2025

*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PMIC_H
#define __PMIC_H

/* Includes ------------------------------------------------------------------*/

#ifdef __cplusplus
#include <Arduino.h>
extern "C" {
#endif

float readBusVoltage(void);

#ifdef __cplusplus
} // extern "C"
#endif


#endif /* __PMIC_H */
