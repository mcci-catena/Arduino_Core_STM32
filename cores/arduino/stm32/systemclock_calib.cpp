/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include <Arduino.h>
#include "stm32l0xx_hal.h"
#include "stm32l0xx.h"
#include <stdio.h>

#define CAPTURE_START              ((uint32_t) 0x00000001)
#define CAPTURE_ONGOING           ((uint32_t) 0x00000002)
#define CAPTURE_COMPLETED         ((uint32_t) 0x00000003)

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define CALIBRATION_MIN_ERROR
/* Enable this define to use the Fixed error method to calibrate the HSI and
   HSI14 oscillators */
/* #define FIXED_ERROR */
/* Enable this define to use the Measured curve error method to calibrate the HSI and
   HSI14 oscillators */
/* #define ERROR_CURVE */

#define INITIAL_ERROR             ((uint32_t)99999000)

#define ABS_RETURN(x)                             ((x < 0) ? (-x) : (x))
#define __HAL_GET_TIM_PRESCALER(__HANDLE__)       ((__HANDLE__)->Instance->PSC)

/* The LSE is divided by 8 => LSE/8 = 32768/8 = 4096 */
#define MSI_REFERENCE_FREQUENCY     ((uint32_t)4096) /* The reference frequency value in Hz */
#define MSI_NUMBER_OF_LOOPS         ((uint32_t)50)
#define MSI_TIMEOUT                 ((uint32_t)0xFFFF)

/* Get actual trimming settings of MSI */
#define GET_MSI_TRIMMING_VALUE()    ((RCC->ICSCR & RCC_ICSCR_MSITRIM) >> 24)

uint32_t	MSI_FreqMeasure(void);
uint32_t	MSI_FreqMeasure(void);
uint32_t	SystemClock_Calib(void);
void	MSI_MeasurementInit(void);
void	SetSysClock_MSI(void);
void	MSI_MeasurementInit(void);
void	MSI_RCC_AdjustCalibrationValue(char TrimmingValue);

/* variables */
int32_t   aFrequenceChangeTable[256]; /* 2^8 positions for MSI */
uint32_t  StartCalibration = 0;
uint32_t  MSIFrequencyBeforeCalib, MSIFrequencyAfterCalib;
uint32_t  __IO CaptureState = 0;
uint32_t  __IO Capture = 0;
uint32_t  IC1ReadValue1 = 0, IC1ReadValue2 = 0;
uint32_t  TrimmingCurveMeasured;
uint32_t  optimumCalibrationValue;
uint32_t  calibrationValue;
uint32_t  calibValue;

// RTC_HandleTypeDef hrtc;
TIM_HandleTypeDef htim21;

/* Private function prototypes -----------------------------------------------*/
static void TIM21_Init(void);

/**
  * @brief  The application entry point.
  * @retval int
  */

uint32_t MSI_CalibrateMinError(void)
{
  uint32_t  measuredFrequency = 0;
  uint32_t  sysClockFrequency = 0;
  uint32_t  optimumFrequency = 0;
  uint32_t  frequencyError = 0;
  uint32_t  optimumFrequencyError = INITIAL_ERROR; /* Large value */
  uint32_t  numberSteps = 0;       /* Number of steps: size of trimming bits */
  uint32_t  trimmingValue = 0;

  optimumCalibrationValue = 0;

  /* Set measurement environment */
  MSI_MeasurementInit();

  /* Get system clock frequency */
  sysClockFrequency = HAL_RCC_GetSysClockFreq();

  if (StartCalibration != 0)
  {
    /* MSITRIM is 8-bit length */
    numberSteps = 256; /* number of steps is 2^8 = 256 */
  }
  else
  {
    /* Without Calibration */
    numberSteps = 1;
  }

  /* Internal Osc frequency measurement for numberSteps */
  for (trimmingValue = 0; trimmingValue < numberSteps; trimmingValue++)
  {
    if (StartCalibration != 0)
    {
      /* Set the Intern Osc trimming bits to trimmingValue */
      MSI_RCC_AdjustCalibrationValue(trimmingValue);
    }

    /* Get actual frequency value */
    measuredFrequency = MSI_FreqMeasure();

    if (StartCalibration != 0)
    {
      /* Compute current frequency error corresponding to the current trimming value:
        measured value is subtracted from the typical one */
      frequencyError = ABS_RETURN((int32_t) (measuredFrequency - sysClockFrequency));

      /* Get the nearest frequency value to typical one */
      if (optimumFrequencyError > frequencyError)
      {
        optimumFrequencyError = frequencyError;
        optimumCalibrationValue = trimmingValue;
        optimumFrequency = measuredFrequency;
      }
    }
  }

  if (StartCalibration != 0)
  {
    /* Set trimming bits corresponding to the nearest frequency */
    MSI_RCC_AdjustCalibrationValue(optimumCalibrationValue);
    calibValue = optimumCalibrationValue;
    /* Return the intern oscillator frequency after calibration */
    return (optimumFrequency);
  }
  else
  {
    /* Return the intern oscillator frequency before calibration */
    return (measuredFrequency);
  }
}

/**
    @brief  Calibrates the internal oscillator with the maximum allowed
            error value set by user.
            If this value was not found, this function sets the oscillator
            to default value.
    @param  MaxAllowedError: maximum absolute value allowed of the MSI frequency
                             error given in Hz.
    @param  Freq: pointer to an uint32_t variable that will contain the value
                  of the internal oscillator frequency after calibration.
    @retval ErrorStatus:
               - SUCCESS: a frequency error =< MaxAllowedError was found.
               - ERROR: a frequency error =< MaxAllowedError was not found.
*/
ErrorStatus MSI_CalibrateFixedError(uint32_t MaxAllowedError, uint32_t* Freq)
{
  uint32_t  measuredFrequency;
  int32_t   frequencyError = 0;
  uint32_t  absFrequencyError = 0;
  uint32_t  sysclockFrequency = 0;
  uint32_t  max = 0xFF;   // number of steps is 2^8 = 256
  uint32_t  min = 0x00;
  uint32_t  mid = 0x00;
  uint32_t  trimmingIndex = 0;
  uint32_t  trimmingValue = 16;
  ErrorStatus calibrationStatus = ERROR;
  uint32_t  numberSteps = 256;         //* number of steps is 2^8 = 256

  // Set measurement environment
  MSI_MeasurementInit();

  //* Get system clock frequency
  sysclockFrequency = HAL_RCC_GetSysClockFreq();

  //****************************************************************************
  //* Start frequency measurement for current trimming value
  measuredFrequency = 0;

  //* Set the MSITRIMR register to trimmingValue to be ready for measurement
  __HAL_RCC_MSI_CALIBRATIONVALUE_ADJUST(numberSteps / 2);

  //****************************************************************************
  //* RC Frequency measurement for different values
  while ((trimmingIndex < 256) && (calibrationStatus == ERROR))
  {

    // Compute the middle
    mid = ((max + min) >> 1);
    trimmingValue = (mid - calibrationValue);

    // Set the MSITRIM bits to trimmingValue to be ready for measurement
    __HAL_RCC_MSI_CALIBRATIONVALUE_ADJUST(trimmingValue);

    //* Start frequency measurement for current trimming value
    measuredFrequency = 0;

    //* Get actual frequency value
    measuredFrequency = MSI_FreqMeasure();

    //* Compute current frequency error corresponding to the current trimming value
    frequencyError = (measuredFrequency - sysclockFrequency);
    absFrequencyError = ABS_RETURN(frequencyError);

    if (absFrequencyError < MaxAllowedError)
    {
      //* Calibration succeeded
      calibrationStatus = SUCCESS;

      calibrationValue = trimmingValue;

      //* Save the MSI measured value /
      *Freq = measuredFrequency;
    }
    else if (frequencyError < 0)
    {
      //* Update the minimum
      min = mid;
    }
    else
    {
      //* Update the maximum
      max = mid;
    }

    //* Increment trimming index
    trimmingIndex++;

  }

  //* If the frequency error set by the user was not found
  if (calibrationStatus == ERROR)
  {
    //* Set the MSITRIMR register to default value
    __HAL_RCC_MSI_CALIBRATIONVALUE_ADJUST(numberSteps / 2);
  }

  //* Return the calibration status: ERROR or SUCCESS
  return (calibrationStatus);
}

/**
    @brief  For all possible trimming values change of frequency is measured
    @retval None.
*/
void MSI_GetCurve(void)
{
  uint32_t measuredFrequency;
  uint32_t trimmingIndex = 0;
  uint32_t trimmingindexorig;
  uint32_t orig_frequency;
  uint32_t numberSteps;

  // Set measurement environment
  MSI_MeasurementInit();

  if (StartCalibration != 0)
  {
    // MSITRIM is 8-bit length
    numberSteps = 256; // number of steps is 2^8 = 256
  }
  else
  {
    // Without Calibration
    numberSteps = 1;
  }

  // Keep original values
  trimmingindexorig = GET_MSI_TRIMMING_VALUE();
  orig_frequency = MSI_FreqMeasure();

  // RC Frequency measurement for different values
  for (trimmingIndex = 0; trimmingIndex < numberSteps; trimmingIndex++)
  {

    // Set the MSITRIMR register to trimmingValue to be ready for measurement
    MSI_RCC_AdjustCalibrationValue(trimmingIndex);
    //************ Start measuring Internal Oscillator frequency ***************
    measuredFrequency = MSI_FreqMeasure();

    // Compute current frequency error corresponding to the current trimming value:
    // measured value is subtracted from the typical one
    aFrequenceChangeTable[trimmingIndex] =   ((int32_t)(measuredFrequency - orig_frequency));
  }

  if (TrimmingCurveMeasured == 0)
  {
    TrimmingCurveMeasured = 1;
  }

  MSI_RCC_AdjustCalibrationValue(trimmingindexorig);
}

/**
    @brief  Adjust calibration value (writing to trimming bits) of selected oscillator.
    @param  Freq: pointer to an uint32_t variable that will contain the value
                  of the internal oscillator frequency after calibration.
    @retval ErrorStatus:
               - SUCCESS: a frequency correction value was found.
               - ERROR: a frequency correction value was not found.
*/
ErrorStatus MSI_CalibrateCurve(uint32_t* Freq)
{

  uint32_t measuredFrequency;
  uint32_t i;
  uint32_t frequencyError;
  uint32_t size;
  uint32_t optimumfrequencyerror = INITIAL_ERROR; // Large value
  ErrorStatus returnvalue = ERROR;

  optimumCalibrationValue = 0;

  optimumCalibrationValue = GET_MSI_TRIMMING_VALUE();
  size = 32;

  // Get position
  measuredFrequency = MSI_FreqMeasure();

  // Find the closest difference
  for (i = 0; i < size; i++)
  {
    frequencyError = ABS_RETURN((int32_t) (MSI_VALUE - (int32_t)(measuredFrequency + aFrequenceChangeTable[i])));

    // Get the nearest frequency value to typical one
    if (frequencyError < optimumfrequencyerror)
    {
      optimumfrequencyerror = frequencyError;
      optimumCalibrationValue = i;
    }
  }

  if (optimumCalibrationValue < size)
  {
    __HAL_RCC_MSI_CALIBRATIONVALUE_ADJUST(optimumCalibrationValue);
    // Save the MSI measured value
    *Freq = measuredFrequency + aFrequenceChangeTable[optimumCalibrationValue];
    return returnvalue = SUCCESS;
  }

  return returnvalue;
}

/* the SystemClock_Calib provides the calibration value for system clocks */
uint32_t SystemClock_Calib()
{
  /************************** Start the Calibration ***************************/
  /* Enable Calibration */
  StartCalibration = 1;
  MSI_MeasurementInit();
  MSIFrequencyBeforeCalib = MSI_FreqMeasure();

#ifdef CALIBRATION_MIN_ERROR
  /* Calibrates internal oscillators to the minimum computed error */
  MSIFrequencyAfterCalib = MSI_CalibrateMinError();
  return optimumCalibrationValue;

#elif defined FIXED_ERROR // not CALIBRATION_MIN_ERROR
  // Calibrate MSI oscillator with the maximum allowed error in Hz
  // Fix the maximum value of the error frequency at +/- 20000Hz
  MSI_CalibrateFixedError(20000, &MSIFrequencyAfterCalib);
  return calibrationValue;

#elif defined  ERROR_CURVE  // not CALIBRATION_MIN_ERROR
  // Measure the MSI calibration curve
  MSI_GetCurve();
  // Choose the optiomal value on the curve
  MSI_CalibrateCurve((uint32_t*)&MSIFrequencyAfterCalib);
  return optimumCalibrationValue;

#else
  exit(0);
#endif
}

static void MX_NVIC_Init(void)
{
  /* TIM21_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(TIM21_IRQn, 1, 0);
	    /* Disable IRQ channel */
   HAL_NVIC_DisableIRQ(TIM21_IRQn);
 // HAL_NVIC_EnableIRQ(TIM21_IRQn);
}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
/* static void MX_RTC_Init(void)
{
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    calib_Error_Handler();
  }

} */

/**
  * @brief TIM21 Initialization Function
  * @param None
  * @retval None
  */
static void TIM21_Init(void)
{

  /* USER CODE END TIM21_Init 0 */
  TIM_IC_InitTypeDef sConfigIC = {0};

  /* Wait till LSE is ready */
  while (__HAL_RCC_GET_FLAG(RCC_FLAG_LSERDY) == RESET)
    {}

  timer_enable_clock(&htim21);

  htim21.Instance = TIM21;
  htim21.Init.Prescaler = 0;
  htim21.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim21.Init.Period = 0;
  htim21.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  if (HAL_TIM_IC_Init(&htim21) != HAL_OK)
  {
    calib_Error_Handler();
  }

  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV8;
  sConfigIC.ICFilter = 0;
  if (HAL_TIM_IC_ConfigChannel(&htim21, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
  {
    calib_Error_Handler();
  }

  if (HAL_TIMEx_RemapConfig(&htim21, TIM21_TI1_LSE) != HAL_OK)
  {
    calib_Error_Handler();
  }
  /* USER CODE BEGIN TIM21_Init 2 */

  /* Initialize interrupts */
  /* TIM21_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(TIM21_IRQn, 1, 0);

  /* Disable IRQ channel */
  HAL_NVIC_DisableIRQ(TIM21_IRQn);
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void calib_Error_Handler(void)
 {
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
 }

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if ((htim->Channel) == HAL_TIM_ACTIVE_CHANNEL_1)
	{
    if (CaptureState == CAPTURE_START)
    {
      /* Get the 1st Input Capture value */
      IC1ReadValue1 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
      CaptureState = CAPTURE_ONGOING;
    }
    else if (CaptureState == CAPTURE_ONGOING)
    {
      /* Get the 2nd Input Capture value */
      IC1ReadValue2 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);

      /* Capture computation */
      if (IC1ReadValue2 > IC1ReadValue1)
      {
        Capture = (IC1ReadValue2 - IC1ReadValue1);
      }
      else if (IC1ReadValue2 < IC1ReadValue1)
      {
        Capture = ((0xFFFF - IC1ReadValue1) + IC1ReadValue2);
      }
      else
      {
        Capture = -1;
        /* If capture values are equal, we have reached the limit of frequency
          measures */
        calib_Error_Handler();
      }

      CaptureState = CAPTURE_COMPLETED;
  }
 }
}

void SetSysClock_MSI(void)
{
  RCC_ClkInitTypeDef clk_init;
  RCC_OscInitTypeDef osc_init;

  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();

  /* Enable MSI oscillator */
  osc_init.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  osc_init.MSIState = RCC_MSI_ON;
  osc_init.MSICalibrationValue = 2;//RCC_MSICALIBRATION_DEFAULT;
  osc_init.MSIClockRange = RCC_MSIRANGE_5;
  osc_init.PLL.PLLState = RCC_PLL_NONE;

  if (HAL_RCC_OscConfig(&osc_init) != HAL_OK)
  {
    /* Initialization Error */
    calib_Error_Handler();
  }

  /* Select MSI as system clock source and configure the HCLK, PCLK1 clocks dividers */
  clk_init.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1);
  clk_init.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  clk_init.AHBCLKDivider = RCC_SYSCLK_DIV1;
  clk_init.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&clk_init, FLASH_LATENCY_0) != HAL_OK)
  {
    calib_Error_Handler();
  }
}


/**
    @brief Measures actual value of MSI
    @param  None.
    @retval Actual MSI frequency
*/
uint32_t MSI_FreqMeasure(void)
{
  uint32_t  measuredFrequency;
  char  loopcounter = 0;
  uint32_t  timeout = MSI_TIMEOUT;

  /* Start frequency measurement for current trimming value */
  measuredFrequency = 0;
  loopcounter = 0;
  /************ Start measuring Internal Oscillator frequency ***************/
  while (loopcounter <= MSI_NUMBER_OF_LOOPS)
  {
    CaptureState = CAPTURE_START;

    /* Enable capture 1 interrupt */
    HAL_TIM_IC_Start_IT(&htim21, TIM_CHANNEL_1);
    /* Enable the TIMx IRQ channel */
    HAL_NVIC_EnableIRQ(TIM21_IRQn);

		/* Wait for end of capture: two consecutive captures */
    while (CaptureState != CAPTURE_COMPLETED)
    {
      if (--timeout == 0)
      {
        return ERROR;
      }
    }

    /* Disable IRQ channel */
    HAL_NVIC_DisableIRQ(TIM21_IRQn);
    /* Disable TIMx */
    HAL_TIM_IC_Stop_IT(&htim21, TIM_CHANNEL_1);

    if (loopcounter != 0)
    {
      /* Compute the frequency (the Timer prescaler isn't included) */
      measuredFrequency += (uint32_t) (MSI_REFERENCE_FREQUENCY * Capture);
    }
    /* Increment loop counter */
    loopcounter++;
  }
  /**************************** END of Measurement **************************/

  /* Compute the average value corresponding the the current trimming value */
  measuredFrequency = (uint32_t)((__HAL_GET_TIM_PRESCALER(&htim21) + 1) * (measuredFrequency / MSI_NUMBER_OF_LOOPS));
  return measuredFrequency;
}


/**
    @brief Configures all the necessary peripherals necessary from frequency calibration.
    @param  None.
    @retval None.
*/
void MSI_MeasurementInit(void)
{
  SetSysClock_MSI();
  TIM21_Init();
}

/**
    @brief  Adjust calibration value (writing to trimming bits) of selected oscillator.
    @param  TrimmingValue: calibration value to be written in trimming bits.
    @retval None.
*/
void MSI_RCC_AdjustCalibrationValue(char TrimmingValue)
{
  __HAL_RCC_MSI_CALIBRATIONVALUE_ADJUST(TrimmingValue);
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
