/**
  ******************************************************************************
  * @file    pmic.cpp
  * @author  Murali
  * @version V1.0.0
  * @date    23-Sep-2025
  * @brief   provide vbus service
  */


#include "pmic.h"
#include "../../libraries/Wire/src/Wire.h"

/**
  * @brief  Reads the bus voltage from the PMIC via I2C
  * @param  None
  * @retval float : The calculated bus voltage in volts
  */
float readBusVoltage(void)
{
    uint8_t vBusMsb;
    uint8_t vBusLsb;
 
    WirePMIC.begin();
    WirePMIC.beginTransmission(0x6B);
    WirePMIC.write(0x05);             // high byte = 0x05
    WirePMIC.write(0x07);             // low byte  = 0x07
    WirePMIC.write(0x01);
    WirePMIC.endTransmission();
 
    WirePMIC.beginTransmission(0x6B); // Start transmission to slave
    WirePMIC.write(0x05);             // high byte (0x05)
    WirePMIC.write(0x19);             // low byte  (0x19)
    WirePMIC.endTransmission(false);  // Send restart condition (false for no stop)
 
    WirePMIC.requestFrom(0x6B, 1);    // Request data from the slave
    vBusMsb = WirePMIC.read();        // Read the byte from the register
 
    WirePMIC.beginTransmission(0x6B); // Start transmission to slave
    WirePMIC.write(0x05);             // high byte (0x05)
    WirePMIC.write(0x1A);             // low byte  (0x1A)
    WirePMIC.endTransmission(false);  // Send restart condition (false for no stop)
 
    WirePMIC.requestFrom(0x6B, 1);    // Request data from the slave
    vBusLsb = WirePMIC.read();
 
    // We need only data from bit 0 and 1
    vBusLsb = (vBusLsb & 0x03);
    // Get the 10-bit ADC result
    uint16_t result = (vBusMsb << 2) | vBusLsb;
 
    return (result / 1023.0f) * 7.5f;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
