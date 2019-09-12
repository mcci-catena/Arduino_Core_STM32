/*

Module: stm32_adc.cpp

Function:
         Read analog function for STM32 platform.

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

constexpr unsigned long ADC_TIMEOUT = 10;
constexpr int STM32L0_ADC_CHANNEL_VREFINT = 17;
constexpr int STM32L0_ADC_CHANNEL_TEMPSENSOR = 18;

constexpr uint32_t AdcClockModeAsync = 0;
constexpr uint32_t AdcClockModePclk2 = ADC_CFGR2_CKMODE_0;
/* the clock we'll use: */
constexpr uint32_t AdcClockMode = AdcClockModeAsync;

#ifdef __cplusplus
 extern "C" {
#endif

#ifdef STM32L0xx

static bool AdcCalibrate(void);
static bool AdcDisable(void);
static bool AdcEnable(void);
static bool AdcGetValue(uint32_t *value);
static void AdcStart(void);

static bool
AdcReadAnalog(
	uint32_t Channel,
	uint32_t ReadCount,
	uint32_t Multiplier,
	uint32_t *pValue
	)
	{
	uint32_t	vRef;
	uint32_t	vChannel;
	bool		fStatusOk;

	if (pValue == NULL)
		return false;

	*pValue = 0;
	if (Channel > STM32L0_ADC_CHANNEL_VREFINT)
		return false;

	/* make sure that the RCC is generating the low-frequency clock */
	__HAL_RCC_ADC1_CLK_ENABLE();

	if (HAL_RCC_GetHCLKFreq() < 16000000)
		{
		/* Set the Low Frequency Mode */
		/* ADC->CCR = ADC_CCR_LFMEN; -- it's not working with MSI clock */

		/* ADC requires HSI clock, so enable it now */
		LL_RCC_HSI_Enable();
		while (LL_RCC_HSI_IsReady() != 1U)
			{
			/* Wait for HSI ready */
			}
		}

	fStatusOk = true;

	if (fStatusOk)
		{
		*pValue = 2;

		// make sure the clock is configured
		// ADC1->CFGR2 = (ADC1->CFGR2 & ~ADC_CFGR2_CKMODE) | AdcClockMode;

		fStatusOk = AdcCalibrate();
		}

	// calibration turns off the ADC. Turn it back on.
	if (fStatusOk)
		{
		*pValue = 3;

		// Set the ADC clock source
		ADC1->CFGR2 = (ADC1->CFGR2 & ~ADC_CFGR2_CKMODE) | AdcClockMode;
		ADC1->CR = ADC_CR_ADVREGEN;
		fStatusOk = AdcEnable();
		}

	if (fStatusOk)
		{
		*pValue = 4;
		ADC1->CFGR1 = ADC_CFGR1_WAIT | ADC_CFGR1_SCANDIR;
		ADC1->CHSELR = (1u << STM32L0_ADC_CHANNEL_VREFINT) |
			       (1u << Channel);
		ADC1->SMPR = ADC_SMPR_SMPR;
		ADC->CCR = ADC_CCR_VREFEN;

		/* start ADC, which will take 2 readings */
		AdcStart();

		fStatusOk = AdcGetValue(&vRef);
		if (fStatusOk)
			{
			*pValue = 5;
			if (Channel != STM32L0_ADC_CHANNEL_VREFINT)
				fStatusOk = AdcGetValue(&vChannel);
			else
				vChannel = vRef;
			}

		if (fStatusOk)
			{
			*pValue = 6;

			ADC1->CHSELR = (1u << Channel);
			for (auto i = 1; i < ReadCount; ++i)
				{
				++*pValue;

				AdcStart();
				fStatusOk = AdcGetValue(&vChannel);
				if (! fStatusOk)
					break;
				}
			}
		}

	AdcDisable();
	ADC1->CR &= ~ADC_CR_ADVREGEN;
	ADC->CCR &= ~ADC_CCR_VREFEN;

	if (HAL_RCC_GetHCLKFreq() < 16000000)
		{
		LL_RCC_HSI_Disable();
		}

	__HAL_RCC_ADC1_FORCE_RESET();
	__HAL_RCC_ADC1_RELEASE_RESET();
	__HAL_RCC_ADC1_CLK_DISABLE();

	if (fStatusOk)
		{
		uint16_t *	pVrefintCal;
		uint32_t	vRefInt_cal;
		uint32_t	vResult;

		pVrefintCal = (uint16_t *) (0x1FF80078);
		// add a factor of 4 to vRefInt_cal, we'll take it out below
		vRefInt_cal = (*pVrefintCal * 3000 + 1) / (4096 / 4);

		vResult = vChannel * Multiplier;
	
		vResult = vResult * vRefInt_cal / vRef;
		// remove the factor of 4, and round
		*pValue = (vResult + 2) >> 2;

//		Serial.print("vRef=");
//		Serial.print(vRef);
//		Serial.print("vChannel=");
//		Serial.print(vChannel);
//		Serial.print("*pVrefintCal=");
//		Serial.println(*pVrefintCal);
		}

	return fStatusOk;
	}

static bool AdcDisable(void)
	{
	uint32_t uTime;

	if (ADC1->CR & ADC_CR_ADSTART)
		ADC1->CR |= ADC_CR_ADSTP;

	uTime = millis();
	while (ADC1->CR & ADC_CR_ADSTP)
		{
		if ((millis() - uTime) > ADC_TIMEOUT)
			{
//			Serial.print("?AdcDisable: CR=");
//			Serial.println(ADC1->CR, HEX);
			return false;
			}
		}

	ADC1->CR |= ADC_CR_ADDIS;
	uTime = millis();
	while (ADC1->CR & ADC_CR_ADEN)
		{
		if ((millis() - uTime) > ADC_TIMEOUT)
			{
//			Serial.print("?AdcDisable: CR=");
//			Serial.println(ADC1->CR, HEX);
			return false;
			}
		}
	return true;
	}

static bool AdcCalibrate(void)
	{
	uint32_t uTime;

	if (ADC1->CR & ADC_CR_ADEN)
		{
		ADC1->CR &= ~ADC_CR_ADEN;

		// if (! AdcDisable())
		// 	return false;
		}

	uTime = millis();

	/* turn on the cal bit */
	ADC1->ISR = ADC_ISR_EOCAL;
	ADC1->CR |= ADC_CR_ADCAL;

	while (! (ADC1->ISR & ADC_ISR_EOCAL))
		{
		if ((millis() - uTime) > ADC_TIMEOUT)
			{
//			Serial.print("?AdcCalibrate: CCR=");
//			Serial.print(ADC1->CCR, HEX);
//			Serial.print(" CR=");
//			Serial.print(ADC1->CR, HEX);
//			Serial.print(" ISR=");
//			Serial.println(ADC1->ISR, HEX);
			return false;
			}
		}

	// uint32_t calData = ADC1->DR;

	/* turn off eocal */
	ADC1->ISR = ADC_ISR_EOCAL;
	return true;
	}

static bool AdcEnable(void)
	{
	uint32_t uTime;

	if (ADC1->CR & ADC_CR_ADEN)
		return true;

	/* clear the done bit */
	ADC1->ISR = ADC_ISR_ADRDY;
	ADC1->CR |= ADC_CR_ADEN;

	if (ADC1->CFGR1 & ADC_CFGR1_AUTOFF)
		return true;

	uTime = millis();
	while (!(ADC1->ISR & ADC_ISR_ADRDY))
		{
		if ((millis() - uTime) > ADC_TIMEOUT)
			{
//			Serial.print("?AdcEnable: CR=");
//			Serial.print(ADC1->CR, HEX);
//			Serial.print(" ISR=");
//			Serial.println(ADC1->ISR, HEX);
			return false;
			}
		}
	return true;
	}

static bool AdcGetValue(uint32_t *value)
	{
	uint32_t rAdcIsr;
	uint32_t uTime;

	uTime = millis();
	while (! ((rAdcIsr = ADC1->ISR) & (ADC_ISR_EOC | ADC_ISR_EOSEQ)))
		{
		if ((millis() - uTime) > ADC_TIMEOUT)
			{
			*value = 0x0FFFu;
//			Serial.print("?AdcGetValue: ISR=");
//			Serial.println(rAdcIsr, HEX);
			return false;
			}
		}

	/* rarely, EOSEQ gets set early... so wait if needed */
	if (! (rAdcIsr & ADC_ISR_EOC))
		{
		for (unsigned i = 0; i < 256u; ++i)
			{
			if ((rAdcIsr = ADC1->ISR) & ADC_ISR_EOC)
				break;
			}
		}

	if (rAdcIsr & ADC_ISR_EOC)
		{
		*value = ADC1->DR;
		ADC1->ISR = ADC_ISR_EOC;
		return true;
		}

//	Serial.print("?AdcGetValue: ISR=");
//	Serial.println(rAdcIsr, HEX);

	// no more data in this sequence
	*value = 0x0FFFu;
	return false;
	}

static void AdcStart(void)
	{
	ADC1->ISR = (ADC_ISR_EOC | ADC_ISR_EOSEQ);
	ADC1->CR |= ADC_CR_ADSTART;
	}

uint32_t Stm32ReadAnalog(
	uint32_t Channel,
	uint32_t ReadCount,
	uint32_t Multiplier
	)
	{
	uint32_t	vResult;
	bool		fStatusOk;

	fStatusOk = AdcReadAnalog(Channel, ReadCount, Multiplier, &vResult);
	if (! fStatusOk)
		{
		Serial.print("?Stm32ReadAnalog(");
		Serial.print(Channel);
		Serial.print("): failed ");
		Serial.println(vResult);
		vResult = 0x0FFFu;
		}

	return vResult;
	}

#endif	/* STM32L0xx */

#ifdef __cplusplus
} /* extern "C" */
#endif
