/*

Module: stm32_clock.cpp

Function:
         System clock calibration function for STM32 platform.

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

#include <Arduino.h>
#include <stm32_clock.h>

extern "C" {

#ifdef STM32L0xx

uint32_t Stm32CalibrateSystemClock(void)
    {
    uint32_t Calib;
    uint32_t CalibNew;
    uint32_t CalibLow;
    uint32_t CalibHigh;
    uint32_t mSecond;
    uint32_t mSecondNew;
    uint32_t mSecondLow;
    uint32_t mSecondHigh;
    bool fHaveSeenLow;
    bool fHaveSeenHigh;
    const bool fCalibrateMSI =  HAL_RCC_GetHCLKFreq() < 16000000;

    if (fCalibrateMSI)
        {
        Calib = (RCC->ICSCR & RCC_ICSCR_MSITRIM) >> 24;
        }
    else
        {
        Calib = (RCC->ICSCR & RCC_ICSCR_HSITRIM) >> 8;
        }

    /* preapre to loop, setting suitable defaults */
    CalibNew = Calib;
    CalibLow = 0;
    CalibHigh = 0;
    mSecondLow = 0;
    mSecondHigh = 2000000;
    fHaveSeenLow = fHaveSeenHigh = false;

    /* loop until we have a new value */
    do	{
        /* meassure the # of micros per RTC second */
        mSecond = Stm32MeasureMicrosPerRtcSecond();

        /* invariant: */
        if (Calib == CalibNew)
            mSecondNew = mSecond;

        /* if mSecond is low, this meaans we must increase the system clock */
        if (mSecond <= 1000000)
            {
//          Serial.print('-');
            /*
            || the following condition establishes that we're
            || below the target frequency, but closer than we've been
            || before (mSecondLow is the previous "low" limit). If
            || so, we reduce the limit, and capture the "low" calibration
            || value.
            */
            if (mSecond > mSecondLow)
                {
                mSecondLow = mSecond;
                CalibLow = Calib; /* save previous calibration value */
                fHaveSeenLow = true;
                }

            /*
            || if we are low, and we have never exceeded the high limit,
            || we can  increase the clock.
            */
            if (! fHaveSeenHigh)
                {
                if (fCalibrateMSI)
                    {
                    if (Calib < 0xFF)
                        {
                        ++Calib;
                        __HAL_RCC_MSI_CALIBRATIONVALUE_ADJUST(Calib);
                        }
                    else
                        break;
                    }
                else
                    {
                    if (Calib < 0x1F)
                        {
                        ++Calib;
                        __HAL_RCC_HSI_CALIBRATIONVALUE_ADJUST(Calib);
                        }
                    else
                        {
                        break;
                        }
                    }

                /* let the clock settle */
                delay(500);
                }
            }

        /* if mSecond is high, we must reduce the system clock */
        else
            {
//          Serial.print('+');
            /*
            || the following condition establishes that we're
            || above the target frequency, but closer than we've been
            || before (mSecondHigh is the previous "high" limit). If
            || so, we reduce the limit, and capture the calibration
            || value.
            */
            if (mSecond < mSecondHigh)
                {
                mSecondHigh = mSecond;
                CalibHigh = Calib;
                fHaveSeenHigh = true;
                }

            /*
            || if we are above the target frequency, and we have
            || never raised the frequence, we can lower the
            || frequency
            */
            if (! fHaveSeenLow)
                {
                if (Calib == 0)
                    break;

                --Calib;
                if (fCalibrateMSI)
                    {
                    __HAL_RCC_MSI_CALIBRATIONVALUE_ADJUST(Calib);
                    }
                else
                    {
                    __HAL_RCC_HSI_CALIBRATIONVALUE_ADJUST(Calib);
                    }
                delay(500);
                }
            }
        } while ((Calib != CalibNew) && (! fHaveSeenLow || !fHaveSeenHigh));

    //
    // We are going to take higher calibration value first and
    // it allows us not to call LMIC_setClockError().
    //
    if (fHaveSeenHigh)
        {
        mSecondNew = mSecondHigh;
        CalibNew = CalibHigh;
        }
    else if (fHaveSeenLow)
        {
        mSecondNew = mSecondLow;
        CalibNew = CalibLow;
        }
    else
        {
        // Use original value
        Serial.println(
            "?Stm32_CalibrateSystemClock: can't calibrate"
            );
        }

    if (CalibNew != Calib)
        {
//      Serial.print(CalibNew < Calib ? '+' : '-');
        if (fCalibrateMSI)
            {
            __HAL_RCC_MSI_CALIBRATIONVALUE_ADJUST(CalibNew);
            }
        else
            {
            __HAL_RCC_HSI_CALIBRATIONVALUE_ADJUST(CalibNew);
            }
        delay(500);
        }

//  Serial.print(" 0x");
//  Serial.println(CalibNew, HEX);
    return CalibNew;
    }

uint32_t
Stm32MeasureMicrosPerRtcSecond(
    void
    )
    {
    uint32_t second;
    uint32_t now;
    uint32_t start;
    uint32_t end;

    /* get the starting time */
    second = RTC->TR & (RTC_TR_ST | RTC_TR_SU);

    /* wait for a new second to start, and capture micros() in start */
    do	{
        now = RTC->TR & (RTC_TR_ST | RTC_TR_SU);
        start = micros();
        } while (second == now);

    /* update our second of interest */
    second = now;

    /* no point in watching the register until we get close */
    delay(500);

    /* wait for the next second to start, and capture micros() */
    do	{
        now = RTC->TR & (RTC_TR_ST | RTC_TR_SU);
        end = micros();
        } while (second == now);

    /* return the delta */
    return end - start;
    }

#endif	/* STM32L0xx */

} /* extern "C" */
