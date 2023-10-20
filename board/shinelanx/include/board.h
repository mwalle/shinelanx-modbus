/****************************************************************************
 * boards/arm/stm32/shinelanx/include/board.h
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

#ifndef __BOARDS_ARM_STM32_SHINELANX_INCLUDE_BOARD_H
#define __BOARDS_ARM_STM32_SHINELANX_INCLUDE_BOARD_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#ifndef __ASSEMBLY__
#  include <stdint.h>
#endif

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Clocking *****************************************************************/

/* HSI - Internal 8 MHz RC Oscillator
 * LSI - 32 KHz RC
 * HSE - 8 MHz from crystal
 * LSE - unused
 */

#define STM32_BOARD_XTAL        8000000ul        /* X1 on board */

#define STM32_HSI_FREQUENCY     8000000ul
#define STM32_LSI_FREQUENCY     40000            /* Between 30kHz and 60kHz */
#define STM32_HSE_FREQUENCY     STM32_BOARD_XTAL
#undef STM32_LSE_FREQUENCY

/* PLL source is HSE/1, PLL multipler is 9:
 *   PLL frequency is 8MHz (XTAL) x 9 = 72MHz
 */

#define STM32_CFGR_PLLSRC       RCC_CFGR_PLLSRC
#define STM32_CFGR_PLLXTPRE     0
#define STM32_CFGR_PLLMUL       RCC_CFGR_PLLMUL_CLKx9
#define STM32_PLL_FREQUENCY     (9*STM32_BOARD_XTAL)

/* Use the PLL and set the SYSCLK source to be the PLL */

#define STM32_SYSCLK_SW         RCC_CFGR_SW_PLL
#define STM32_SYSCLK_SWS        RCC_CFGR_SWS_PLL
#define STM32_SYSCLK_FREQUENCY  STM32_PLL_FREQUENCY

/* AHB clock (HCLK) is SYSCLK (72MHz) */

#define STM32_RCC_CFGR_HPRE     RCC_CFGR_HPRE_SYSCLK
#define STM32_HCLK_FREQUENCY    STM32_SYSCLK_FREQUENCY

/* APB2 clock (PCLK2) is HCLK (72MHz) */

#define STM32_RCC_CFGR_PPRE2    RCC_CFGR_PPRE2_HCLK
#define STM32_PCLK2_FREQUENCY   STM32_HCLK_FREQUENCY
#define STM32_APB2_CLKIN        (STM32_PCLK2_FREQUENCY)   /* Timers 1 and 8, 15-17 */

/* APB1 clock (PCLK1) is HCLK/2 (36MHz) */

#define STM32_RCC_CFGR_PPRE1    RCC_CFGR_PPRE1_HCLKd2
#define STM32_PCLK1_FREQUENCY   (STM32_HCLK_FREQUENCY/2)

/* APB2 TIM 1 will receive PCLK2 (72MHz) */

#define STM32_APB2_TIM1_CLKIN   (STM32_PCLK2_FREQUENCY)

/* APB1 TIM 2-4 will be twice PCLK1 (72MHz) */

#define STM32_APB1_TIM2_CLKIN   (2*STM32_PCLK1_FREQUENCY)
#define STM32_APB1_TIM3_CLKIN   (2*STM32_PCLK1_FREQUENCY)
#define STM32_APB1_TIM4_CLKIN   (2*STM32_PCLK1_FREQUENCY)

/* USB divider -- Divide PLL clock by 1.5 */

#define STM32_CFGR_USBPRE       0

/* LED definitions **********************************************************/

/* The ShineLAN-X has one (red) power LED, one (blue) debug LED and a user RGB
 * LED. Unfortunately, the RGB LED isn't connected to PWM outputs, so we can
 * just use seven different colors.
 *
 * The debug LED is connected to PC7, the RGB LED is connected to PB1 (red),
 * PB0 (green) and PC5 (blue).
 */

/* LED index values for use with board_userled() */

#define BOARD_LED1       0
#define BOARD_LED2       1
#define BOARD_LED3       2
#define BOARD_NLEDS      3

/* LED bits for use with board_userled_all() */

#define BOARD_LED1_BIT   (1 << BOARD_LED1)
#define BOARD_LED2_BIT   (1 << BOARD_LED2)
#define BOARD_LED3_BIT   (1 << BOARD_LED3)

/* If CONFIG_ARCH_LEDs is defined, then NuttX will control the debug LED.
 * The following definitions describe how NuttX controls the LED:
 *
 *   SYMBOL              Meaning                  LED state
 *   ------------------  -----------------------  ----------
 *   LED_STARTED         NuttX has been started   OFF
 *   LED_HEAPALLOCATE    Heap has been allocated  OFF
 *   LED_IRQSENABLED     Interrupts enabled       OFF
 *   LED_STACKCREATED    Idle stack created       ON
 *   LED_INIRQ           In an interrupt          No change
 *   LED_SIGNAL          In a signal handler      No change
 *   LED_ASSERTION       An assertion failed      No change
 *   LED_PANIC           The system has crashed   Blinking
 *   LED_IDLE            STM32 is is sleep mode   Not used
 */

#define LED_STARTED      0
#define LED_HEAPALLOCATE 1
#define LED_IRQSENABLED  2
#define LED_STACKCREATED 3
#define LED_INIRQ        4
#define LED_SIGNAL       5
#define LED_ASSERTION    6
#define LED_PANIC        7

/* Button definitions *******************************************************/

/* The ShineLAN-X supports one buttons, which is connected to the I/O PA3. */

#define BUTTON_USER      0
#define NUM_BUTTONS      1

#define BUTTON_USER_BIT  (1 << BUTTON_USER)

/* Alternate function pin selections ****************************************/

/* DMA channels *************************************************************/

#endif /* __BOARDS_ARM_STM32_SHINELANX_INCLUDE_BOARD_H */
