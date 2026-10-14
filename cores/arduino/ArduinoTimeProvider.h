/*

Module:  ArduinoTimeProvider.h

Function:
        Let an external provider supply millis(), micros(), delay(), and
        delayMicroseconds() (mcci-catena/Arduino_Core_STM32#223).

Copyright notice and license information:
        Copyright (C) 2026 MCCI Corporation. All rights reserved.

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
        Terry Moore, MCCI Corporation   October 2026

*/

#ifndef _ArduinoTimeProvider_h_
#define _ArduinoTimeProvider_h_         /* prevent multiple includes */

#include <stdint.h>

/// \brief defined if this core has setArduinoTimeProvider().
#define ARDUINO_CORE_STM32_TIME_PROVIDER        1

/// \brief delayMicroseconds() hands waits of at least this many
///     microseconds to the provider's pDelayMicroseconds(), if it has
///     one; shorter waits stay on SysTick.
#ifndef ARDUINO_TIME_PROVIDER_DELAYUS_MIN
# define ARDUINO_TIME_PROVIDER_DELAYUS_MIN      100
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
        {
        kArduinoTime_Millis = 0,
        kArduinoTime_Micros = 1,
        } ArduinoTimeSelect_t;

typedef struct ArduinoTimeProvider_s ArduinoTimeProvider_t;

/*
  A provider's table. Register it with setArduinoTimeProvider(). Every
  function must work from interrupt handlers and with interrupts masked,
  as millis() and micros() do. A provider that needs state embeds the
  table in a larger const structure and recovers it from pThis.
*/
struct ArduinoTimeProvider_s
        {
        /// size of this structure, so the table can grow; a field beyond
        /// nSize is treated as absent.
        uint32_t        nSize;

        /// required: the current time in milliseconds or microseconds, as
        /// 64 bits; millis() and micros() use the low 32. For a value of
        /// which it does not handle, return 0.
        uint64_t        (*pGetTime)(const ArduinoTimeProvider_t *pThis, ArduinoTimeSelect_t which);

        /// optional: delay(); may idle the CPU. NULL: the core loops on
        /// millis(), calling yield().
        void            (*pDelay)(const ArduinoTimeProvider_t *pThis, uint32_t ms);

        /// optional: delayMicroseconds() for waits of at least
        /// ARDUINO_TIME_PROVIDER_DELAYUS_MIN. NULL: the core counts SysTick.
        void            (*pDelayMicroseconds)(const ArduinoTimeProvider_t *pThis, uint32_t us);
        };

/// \brief register a time provider.
///
/// \param pProvider the new table, or NULL to go back to the core's
///     SysTick time. A table without pGetTime, or whose nSize does not
///     cover pGetTime, is refused: nothing changes.
///
/// \returns the provider that was registered before (NULL for the
///     core's own), so that a caller can chain to it; or, if pProvider
///     was refused, the provider still registered.
const ArduinoTimeProvider_t *setArduinoTimeProvider(const ArduinoTimeProvider_t *pProvider);

/// \brief the registered provider, or NULL.
const ArduinoTimeProvider_t *getArduinoTimeProvider(void);

/// \brief for delayMicroseconds(): hand the wait to the provider.
/// \returns nonzero if the provider waited; zero if the caller must.
int arduinoTimeProvider_delayMicroseconds(uint32_t us);

#ifdef __cplusplus
}
#endif

#endif /* _ArduinoTimeProvider_h_ */
