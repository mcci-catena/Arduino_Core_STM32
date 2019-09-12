/*

Module: stm32_rtc.cpp

Function:
        RTC function for STM32 platform.

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

#ifdef __cplusplus
 extern "C" {
#endif

#ifdef STM32L0xx

static RTC_HandleTypeDef *gs_phRtc;
static volatile uint32_t *gs_pAlarmA;
static volatile uint32_t *gs_pAlarmB;

void RTC_IRQHandler(void)
        {
        HAL_RTC_AlarmIRQHandler(gs_phRtc);
        }

void HAL_RTC_AlarmAEventCallback(
        RTC_HandleTypeDef *     hRtc
        )
        {
        if (gs_pAlarmA != NULL)
                *gs_pAlarmA = 1;
        }

void HAL_RTC_AlarmBEventCallback(
        RTC_HandleTypeDef *     hRtc
        )
        {
        if (gs_pAlarmB != NULL)
                *gs_pAlarmB = 1;
        }

void HAL_RTC_MspInit(
        RTC_HandleTypeDef *     hRtc
        )
        {
        if (hRtc->Instance == RTC)
                {
                /* USER CODE BEGIN RTC_MspInit 0 */

                /* USER CODE END RTC_MspInit 0 */
                /* Peripheral clock enable */
                __HAL_RCC_RTC_ENABLE();
                /* USER CODE BEGIN RTC_MspInit 1 */
                HAL_NVIC_SetPriority(RTC_IRQn, TICK_INT_PRIORITY, 0U);
                HAL_NVIC_EnableIRQ(RTC_IRQn);
                /* USER CODE END RTC_MspInit 1 */
                }
        }

void HAL_RTC_MspDeInit(
        RTC_HandleTypeDef *     hRtc
        )
        {
        if (hRtc->Instance == RTC)
                {
                /* USER CODE BEGIN RTC_MspDeInit 0 */
                HAL_NVIC_DisableIRQ(RTC_IRQn);
                /* USER CODE END RTC_MspDeInit 0 */
                /* Peripheral clock disable */
                __HAL_RCC_RTC_DISABLE();
                /* USER CODE BEGIN RTC_MspDeInit 1 */

                /* USER CODE END RTC_MspDeInit 1 */
                }
        }

uint32_t HAL_AddTick(
        uint32_t delta
        )
        {
        extern __IO uint32_t uwTick;
        // copy old interrupt-enable state to flags.
        uint32_t const flags = __get_PRIMASK();

        // disable interrupts
        __set_PRIMASK(1);

        // observe uwTick, and advance it.
        uint32_t const tickCount = uwTick + delta;

        // save uwTick
        uwTick = tickCount;

        // restore interrupts (does nothing if ints were disabled on entry)
        __set_PRIMASK(flags);

        // return the new value of uwTick.
        return tickCount;
        }

bool Stm32RtcInit(
        RTC_HandleTypeDef *pRtc
        )
        {
        RTC_TimeTypeDef Time;
        RTC_DateTypeDef Date;
        uint32_t RtcClock;
        RTC_HandleTypeDef hRtc;

        if (pRtc == NULL)
                {
                pRtc = &hRtc;
                memset(pRtc, 0, sizeof(*pRtc));
                }
        else
                {
                gs_phRtc = pRtc;
                }

        pRtc->Instance = RTC;
        pRtc->Init.HourFormat = RTC_HOURFORMAT_24;
        RtcClock = __HAL_RCC_GET_RTC_SOURCE();
        if (RtcClock == RCC_RTCCLKSOURCE_LSI)
                {
                pRtc->Init.AsynchPrediv = 37 - 1; /* 37kHz / 37 = 1000Hz */
                pRtc->Init.SynchPrediv = 1000 - 1; /* 1000Hz / 1000 = 1Hz */
                }
        else if (RtcClock == RCC_RTCCLKSOURCE_LSE)
                {
                pRtc->Init.AsynchPrediv = 128 - 1; /* 32768Hz / 128 = 256Hz */
                pRtc->Init.SynchPrediv = 256 - 1; /* 256Hz / 256 = 1Hz */
                }
        else
                {
                /*
                || use HSE clock --
                || we don't support use of HSE as RTC because it's connected to
                || TCXO_OUT, and that's controlled by the LoRaWAN software.
                */
                Serial.println(
                        "?Stm32RtcInit:"
                        " HSE can not be used for RTC clock!\n"
                        );
                return false;
                }


        pRtc->Init.OutPut = RTC_OUTPUT_DISABLE;
        pRtc->Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
        pRtc->Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
        pRtc->Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;

        if (HAL_RTC_Init(pRtc) != HAL_OK)
                {
                Serial.println(
                        "?Stm32RtcInit:"
                        " HAL_RTC_Init() failed\n"
                        );
                return false;
                }

        /* Initialize RTC and set the Time and Date */
        if (HAL_RTCEx_BKUPRead(pRtc, RTC_BKP_DR0) != 0x32F2)
                {
                Time.Hours = 0x0;
                Time.Minutes = 0x0;
                Time.Seconds = 0x0;
                Time.SubSeconds = 0x0;
                Time.TimeFormat = RTC_HOURFORMAT12_AM;
                Time.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
                Time.StoreOperation = RTC_STOREOPERATION_RESET;

                if (HAL_RTC_SetTime(
                        pRtc,
                        &Time,
                        RTC_FORMAT_BIN
                        ) != HAL_OK)
                        {
                        Serial.println(
                                "?Stm32RtcInit:"
                                " HAL_RTC_SetTime() failed\n"
                                );
                        return false;
                        }

                /* Saturday 1st January 2000 */
                Date.WeekDay = RTC_WEEKDAY_SATURDAY;
                Date.Month = RTC_MONTH_JANUARY;
                Date.Date = 0x1;
                Date.Year = 0x0;

                if (HAL_RTC_SetDate(
                        pRtc,
                        &Date,
                        RTC_FORMAT_BIN
                        ) != HAL_OK)
                        {
                        Serial.println(
                                "?Stm32RtcInit:"
                                " HAL_RTC_SetDate() failed\n"
                                );
                        return false;
                        }

                HAL_RTCEx_BKUPWrite(pRtc, RTC_BKP_DR0, 0x32F2);
                }

        /* Enable Direct Read of the calendar registers (not through Shadow) */
        HAL_RTCEx_EnableBypassShadow(pRtc);

        HAL_RTC_DeactivateAlarm(pRtc, RTC_ALARM_A);
        HAL_RTC_DeactivateAlarm(pRtc, RTC_ALARM_B);
        return true;
        }

bool Stm32RtcSetAlarm(
        RTC_HandleTypeDef *pRtc,
        RTC_AlarmTypeDef *sAlarm,
        uint32_t Format,
        volatile uint32_t *pAlarmState
        )
        {
        HAL_StatusTypeDef Status;

        if (pRtc == NULL)
        	return false;

        if (gs_phRtc == NULL)
                gs_phRtc = pRtc;

        if (sAlarm->Alarm == RTC_ALARM_A)
                gs_pAlarmA = pAlarmState;
        else if (sAlarm->Alarm == RTC_ALARM_B)
                gs_pAlarmB = pAlarmState;

        Status = HAL_RTC_SetAlarm_IT(pRtc, sAlarm, Format);

        return (Status == HAL_OK);
        }

#endif  /* STM32L0xx */

#ifdef __cplusplus
} /* extern "C" */
#endif
