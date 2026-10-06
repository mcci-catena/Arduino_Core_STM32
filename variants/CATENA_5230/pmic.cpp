/*

Module: pmic.cpp

Function:
        Read VBUS from the Catena 5230's nPM1300 PMIC, for USB connection
        detection.

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
        Murali, MCCI Corporation

Notes:
        This code is part of the core, so it must not use the Wire
        library: arduino-cli links Wire only into sketches that include
        <Wire.h>. It drives the PMIC's I2C bus with HAL polling transfers
        on its own handle. Polling uses no I2C interrupts, so it leaves
        alone the interrupt routing that twi.c sets up for WirePMIC on the
        same bus.

*/

#include "pmic.h"
#include "PortNames.h"
#include "pinmap.h"

namespace {

// nPM1300 7-bit I2C address; HAL wants it shifted left.
constexpr uint16_t kPmicAddress = 0x6B << 1;

// nPM1300 registers. Register addresses are 16 bits, sent MSB first.
constexpr uint16_t kRegTaskVbus7Measure = 0x0507;  // write 1: start a VBUS measurement
constexpr uint16_t kRegAdcVbusResultMsb = 0x0519;  // VBUS result, bits 9..2
constexpr uint16_t kRegAdcGp0ResultLsbs = 0x051A;  // bits 1..0: VBUS result bits 1..0

// full scale of the 10-bit VBUS result, in volts
constexpr float kVbusFullScale = 7.5f;

// timeout for each I2C transfer, in milliseconds
constexpr uint32_t kTimeoutMs = 10;

I2C_HandleTypeDef sPmicI2c;

void initPin(PinName pin, const PinMap *map)
    {
    const uint32_t function = pinmap_function(pin, map);
    GPIO_InitTypeDef init = {};

    init.Pin = STM_GPIO_PIN(pin);
    init.Mode = STM_PIN_MODE(function);
    init.Pull = STM_PIN_PUPD(function);
    init.Speed = GPIO_SPEED_FREQ_HIGH;
    init.Alternate = STM_PIN_AFNUM(function);
    HAL_GPIO_Init(set_GPIO_Port_Clock(STM_PORT(pin)), &init);
    }

// set up the pins and the I2C peripheral. Done on every read: WirePMIC
// may have reset or reconfigured the peripheral since the last one.
bool initI2c()
    {
    const PinName sda = digitalPinToPinName(PIN_WIRE_PMIC_SDA);
    const PinName scl = digitalPinToPinName(PIN_WIRE_PMIC_SCL);
    I2C_TypeDef * const instance = (I2C_TypeDef *) pinmap_merge_peripheral(
        pinmap_peripheral(sda, PinMap_I2C_SDA),
        pinmap_peripheral(scl, PinMap_I2C_SCL)
        );

    if (instance == NP)
        return false;

#if defined(I2C1_BASE)
    if (instance == I2C1)
        __HAL_RCC_I2C1_CLK_ENABLE();
#endif
#if defined(I2C2_BASE)
    if (instance == I2C2)
        __HAL_RCC_I2C2_CLK_ENABLE();
#endif
#if defined(I2C3_BASE)
    if (instance == I2C3)
        __HAL_RCC_I2C3_CLK_ENABLE();
#endif

    initPin(scl, PinMap_I2C_SCL);
    initPin(sda, PinMap_I2C_SDA);

    // same settings as twi.c uses for WirePMIC
    sPmicI2c.Instance = instance;
    sPmicI2c.Init.Timing = I2C_100KHz;
    sPmicI2c.Init.OwnAddress1 = 0x33 << 1;
    sPmicI2c.Init.OwnAddress2 = 0xFF;
    sPmicI2c.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    sPmicI2c.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    sPmicI2c.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    sPmicI2c.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    sPmicI2c.State = HAL_I2C_STATE_RESET;

    return HAL_I2C_Init(&sPmicI2c) == HAL_OK;
    }

bool writeReg(uint16_t reg, uint8_t value)
    {
    uint8_t buf[3] = { uint8_t(reg >> 8), uint8_t(reg), value };

    return HAL_I2C_Master_Transmit(&sPmicI2c, kPmicAddress, buf, sizeof(buf), kTimeoutMs) == HAL_OK;
    }

bool readReg(uint16_t reg, uint8_t &value)
    {
    uint8_t buf[2] = { uint8_t(reg >> 8), uint8_t(reg) };

    return HAL_I2C_Master_Transmit(&sPmicI2c, kPmicAddress, buf, sizeof(buf), kTimeoutMs) == HAL_OK &&
           HAL_I2C_Master_Receive(&sPmicI2c, kPmicAddress, &value, 1, kTimeoutMs) == HAL_OK;
    }

} // namespace

/**
  * @brief  Read the bus voltage from the PMIC via I2C
  * @param  None
  * @retval float : VBUS in volts, or a negative value if the PMIC
  *         could not be read
  */
float readBusVoltage(void)
{
    uint8_t vBusMsb;
    uint8_t vBusLsb;

    if (! initI2c() ||
        ! writeReg(kRegTaskVbus7Measure, 1) ||
        ! readReg(kRegAdcVbusResultMsb, vBusMsb) ||
        ! readReg(kRegAdcGp0ResultLsbs, vBusLsb))
        return -1.0f;

    // 10-bit result: MSB register holds bits 9..2, LSB register bits 1..0
    const uint16_t result = (uint16_t(vBusMsb) << 2) | (vBusLsb & 0x03);

    return (result / 1023.0f) * kVbusFullScale;
}
