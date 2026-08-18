/**
  ******************************************************************************
  * @file    pmic.h
  * @author  Murali
  * @version V1.0.0
  * @date    23-Sep-2025
  * @brief   Header for pmic.c module
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/