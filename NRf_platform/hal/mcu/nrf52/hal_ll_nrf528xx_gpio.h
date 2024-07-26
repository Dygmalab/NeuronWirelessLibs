
/*
 * The MIT License (MIT)
 *
 * Copyright (C) 2022  Dygma Lab S.L.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
 
#ifndef __HAL_LL_NRF528XX_GPIO_H_
#define __HAL_LL_NRF528XX_GPIO_H_

#define HAL_MCU_GPIO_PORT_GET(port)     (port << 5)
#define HAL_MCU_GPIO_PIN_GET(port, pin) (HAL_MCU_GPIO_PORT_GET(port) | pin)

/*
 * TODO: Different NRF52 chip variants support different GPIOs. This GPIO set is derived from the nRF52833 chip.
 *       Think about making the hal_mcu_gpio_pin_t more chip-related.
 */

/* configuration */
typedef enum
{
    HAL_MCU_GPIO_PIN_0_00 = HAL_MCU_GPIO_PIN_GET(0, 0),
    HAL_MCU_GPIO_PIN_0_01 = HAL_MCU_GPIO_PIN_GET(0, 1),
    HAL_MCU_GPIO_PIN_0_02 = HAL_MCU_GPIO_PIN_GET(0, 2),
    HAL_MCU_GPIO_PIN_0_03 = HAL_MCU_GPIO_PIN_GET(0, 3),
    HAL_MCU_GPIO_PIN_0_04 = HAL_MCU_GPIO_PIN_GET(0, 4),
    HAL_MCU_GPIO_PIN_0_05 = HAL_MCU_GPIO_PIN_GET(0, 5),
    HAL_MCU_GPIO_PIN_0_06 = HAL_MCU_GPIO_PIN_GET(0, 6),
    HAL_MCU_GPIO_PIN_0_07 = HAL_MCU_GPIO_PIN_GET(0, 7),
    HAL_MCU_GPIO_PIN_0_08 = HAL_MCU_GPIO_PIN_GET(0, 8),
    HAL_MCU_GPIO_PIN_0_09 = HAL_MCU_GPIO_PIN_GET(0, 9),
    HAL_MCU_GPIO_PIN_0_10 = HAL_MCU_GPIO_PIN_GET(0, 10),
    HAL_MCU_GPIO_PIN_0_11 = HAL_MCU_GPIO_PIN_GET(0, 11),
    HAL_MCU_GPIO_PIN_0_12 = HAL_MCU_GPIO_PIN_GET(0, 12),
    HAL_MCU_GPIO_PIN_0_13 = HAL_MCU_GPIO_PIN_GET(0, 13),
    HAL_MCU_GPIO_PIN_0_14 = HAL_MCU_GPIO_PIN_GET(0, 14),
    HAL_MCU_GPIO_PIN_0_15 = HAL_MCU_GPIO_PIN_GET(0, 15),
    HAL_MCU_GPIO_PIN_0_16 = HAL_MCU_GPIO_PIN_GET(0, 16),
    HAL_MCU_GPIO_PIN_0_17 = HAL_MCU_GPIO_PIN_GET(0, 17),
    HAL_MCU_GPIO_PIN_0_18 = HAL_MCU_GPIO_PIN_GET(0, 18),
    HAL_MCU_GPIO_PIN_0_19 = HAL_MCU_GPIO_PIN_GET(0, 19),
    HAL_MCU_GPIO_PIN_0_20 = HAL_MCU_GPIO_PIN_GET(0, 20),
    HAL_MCU_GPIO_PIN_0_21 = HAL_MCU_GPIO_PIN_GET(0, 21),
    HAL_MCU_GPIO_PIN_0_22 = HAL_MCU_GPIO_PIN_GET(0, 22),
    HAL_MCU_GPIO_PIN_0_23 = HAL_MCU_GPIO_PIN_GET(0, 23),
    HAL_MCU_GPIO_PIN_0_24 = HAL_MCU_GPIO_PIN_GET(0, 24),
    HAL_MCU_GPIO_PIN_0_25 = HAL_MCU_GPIO_PIN_GET(0, 25),
    HAL_MCU_GPIO_PIN_0_26 = HAL_MCU_GPIO_PIN_GET(0, 26),
    HAL_MCU_GPIO_PIN_0_27 = HAL_MCU_GPIO_PIN_GET(0, 27),
    HAL_MCU_GPIO_PIN_0_28 = HAL_MCU_GPIO_PIN_GET(0, 28),
    HAL_MCU_GPIO_PIN_0_29 = HAL_MCU_GPIO_PIN_GET(0, 29),
    HAL_MCU_GPIO_PIN_0_30 = HAL_MCU_GPIO_PIN_GET(0, 30),
    HAL_MCU_GPIO_PIN_0_31 = HAL_MCU_GPIO_PIN_GET(0, 31),
    HAL_MCU_GPIO_PIN_1_00 = HAL_MCU_GPIO_PIN_GET(1, 0),
    HAL_MCU_GPIO_PIN_1_01 = HAL_MCU_GPIO_PIN_GET(1, 1),
    HAL_MCU_GPIO_PIN_1_02 = HAL_MCU_GPIO_PIN_GET(1, 2),
    HAL_MCU_GPIO_PIN_1_03 = HAL_MCU_GPIO_PIN_GET(1, 3),
    HAL_MCU_GPIO_PIN_1_04 = HAL_MCU_GPIO_PIN_GET(1, 4),
    HAL_MCU_GPIO_PIN_1_05 = HAL_MCU_GPIO_PIN_GET(1, 5),
    HAL_MCU_GPIO_PIN_1_06 = HAL_MCU_GPIO_PIN_GET(1, 6),
    HAL_MCU_GPIO_PIN_1_07 = HAL_MCU_GPIO_PIN_GET(1, 7),
    HAL_MCU_GPIO_PIN_1_08 = HAL_MCU_GPIO_PIN_GET(1, 8),
    HAL_MCU_GPIO_PIN_1_09 = HAL_MCU_GPIO_PIN_GET(1, 9),
} hal_mcu_gpio_pin_t;

#endif /* __HAL_LL_NRF528XX_GPIO_H_ */
