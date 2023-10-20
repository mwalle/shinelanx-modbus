/****************************************************************************
 * boards/arm/stm32/shinelanx/src/shinelanx.h
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

#ifndef __BOARDS_ARM_STM32_SHINELANX_SRC_SHINELANX_H
#define __BOARDS_ARM_STM32_SHINELANX_SRC_SHINELANX_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* LED definitions **********************************************************/

#define GPIO_LED0      (GPIO_OUTPUT|GPIO_CNF_OUTPP|GPIO_MODE_50MHz|\
                        GPIO_OUTPUT_CLEAR|GPIO_PORTC|GPIO_PIN7)

#define GPIO_LED1      (GPIO_OUTPUT|GPIO_CNF_OUTPP|GPIO_MODE_50MHz|\
                        GPIO_OUTPUT_SET|GPIO_PORTB|GPIO_PIN1)

#define GPIO_LED2      (GPIO_OUTPUT|GPIO_CNF_OUTPP|GPIO_MODE_50MHz|\
                        GPIO_OUTPUT_SET|GPIO_PORTB|GPIO_PIN0)

#define GPIO_LED3      (GPIO_OUTPUT|GPIO_CNF_OUTPP|GPIO_MODE_50MHz|\
                        GPIO_OUTPUT_SET|GPIO_PORTC|GPIO_PIN5)

#define LED_DRIVER_PATH "/dev/userleds"

/* The Shine LAN-X supports one button connected to PA3 and is low-active */
#define MIN_IRQBUTTON  BUTTON_USER
#define MAX_IRQBUTTON  BUTTON_USER
#define NUM_IRQBUTTONS 1

#define GPIO_BTN_USER  (GPIO_INPUT|GPIO_CNF_INFLOAT|GPIO_EXTI|GPIO_PORTA|GPIO_PIN3)

/* ENC28J60 */

#define GPIO_ENC28J60_CS    (GPIO_OUTPUT|GPIO_CNF_OUTPP|GPIO_MODE_50MHz|\
                             GPIO_OUTPUT_SET|GPIO_PORTB|GPIO_PIN12)

#define GPIO_ENC28J60_RESET (GPIO_OUTPUT|GPIO_CNF_OUTPP|GPIO_MODE_50MHz|\
                             GPIO_OUTPUT_CLEAR|GPIO_PORTC|GPIO_PIN8)

#define GPIO_ENC28J60_INTR  (GPIO_INPUT|GPIO_CNF_INPULLUP|GPIO_MODE_INPUT|\
                             GPIO_EXTI|GPIO_PORTC|GPIO_PIN6)

/* USB pull-up */

#define GPIO_USB_PULLUP     (GPIO_OUTPUT|GPIO_CNF_OUTOD|GPIO_MODE_50MHz|\
                             GPIO_OUTPUT_SET|GPIO_PORTA|GPIO_PIN8)
/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

#ifndef __ASSEMBLY__

/****************************************************************************
 * Name: stm32_bringup
 *
 * Description:
 *   Perform architecture specific initialization
 *
 *   CONFIG_BOARDCTL=y:
 *     If CONFIG_NSH_ARCHINITIALIZE=y:
 *       Called from the NSH library (or other application)
 *     Otherwise, assumed to be called from some other application.
 *
 *   Otherwise CONFIG_BOARD_LATE_INITIALIZE=y:
 *     Called from board_late_initialize().
 *
 *   Otherwise, bad news:  Never called
 *
 ****************************************************************************/

int stm32_bringup(void);

/****************************************************************************
 * Name: stm32_spidev_initialize
 *
 * Description:
 *   Called to configure SPI chip select GPIO pins for the ShineLAN-X
 *   board.
 *
 ****************************************************************************/

void stm32_spidev_initialize(void);

/****************************************************************************
 * Name: stm32_usbinitialize
 *
 * Description:
 *   Called to setup USB-related GPIO pins for the ShineLAN-X board.
 *
 ****************************************************************************/

void stm32_usbinitialize(void);

/****************************************************************************
 * Name: stm32_autoled_initialize
 *
 * Description:
 *   Initialize NuttX-controlled LED logic
 *
 ****************************************************************************/

#ifdef CONFIG_ARCH_LEDS
void stm32_autoled_initialize(void);
#endif

#endif /* __ASSEMBLY__ */
#endif /* __BOARDS_ARM_STM32_SHINELANX_SRC_SHINELANX_H */
