/*
  Copyright (c) 2011 Arduino.  All right reserved.
  Copyright (c) 2019 MCCI Corporation. All rights reserved.

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

#include "Arduino.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
  The time provider (#223). NULL: the SysTick time below. A table is
  const, so only the pointer is shared; it is written with interrupts
  masked and read once per call.
*/
static const ArduinoTimeProvider_t * volatile s_pTimeProvider;

// true if the table's nSize covers the given field.
#define PROVIDER_HAS(p, field) \
  ((p)->nSize >= offsetof(ArduinoTimeProvider_t, field) + sizeof((p)->field) && (p)->field != NULL)

const ArduinoTimeProvider_t *setArduinoTimeProvider(const ArduinoTimeProvider_t *pProvider)
{
  const ArduinoTimeProvider_t *pPrevious;
  const uint32_t primask = __get_PRIMASK();

  __disable_irq();
  pPrevious = s_pTimeProvider;
  if (pProvider == NULL || PROVIDER_HAS(pProvider, pGetTime))
    s_pTimeProvider = pProvider;
  __set_PRIMASK(primask);
  return pPrevious;
}

const ArduinoTimeProvider_t *getArduinoTimeProvider(void)
{
  return s_pTimeProvider;
}

int arduinoTimeProvider_delayMicroseconds(uint32_t us)
{
  const ArduinoTimeProvider_t * const p = s_pTimeProvider;

  if (p == NULL || ! PROVIDER_HAS(p, pDelayMicroseconds))
    return 0;
  p->pDelayMicroseconds(p, us);
  return 1;
}

uint32_t millis( void )
{
  const ArduinoTimeProvider_t * const p = s_pTimeProvider;

  if (p != NULL)
    return (uint32_t) p->pGetTime(p, kArduinoTime_Millis);
  return GetCurrentMilli() ;
}

// Interrupt-compatible version of micros
uint32_t micros( void )
{
  const ArduinoTimeProvider_t * const p = s_pTimeProvider;

  if (p != NULL)
    return (uint32_t) p->pGetTime(p, kArduinoTime_Micros);
  return GetCurrentMicro();
}

void delay( uint32_t ms )
{
  if (ms == 0)
      return;

  const ArduinoTimeProvider_t * const p = s_pTimeProvider;

  if (p != NULL)
  {
    if (PROVIDER_HAS(p, pDelay))
    {
      p->pDelay(p, ms);
      return;
    }

    // follow the provider's clock, not SysTick. Count whole milliseconds
    // of micros(), so the delay is never short however far into a
    // millisecond it starts, and nothing overflows for long delays.
    uint32_t start = micros();
    while (ms > 0)
    {
      yield();
      while (ms > 0 && (micros() - start) >= 1000)
      {
        --ms;
        start += 1000;
      }
    }
    return;
  }
  uint32_t startVAL = SysTick->VAL;
  uint32_t start = GetCurrentMilli();
  uint32_t minVAL = (SysTick->LOAD + 1) / 1000;
  if (startVAL < minVAL)
      startVAL = minVAL;
  do {
      yield();
  } while (GetCurrentMilli() - start < ms);
  while (SysTick->VAL > startVAL);
}

#ifdef __cplusplus
}
#endif
