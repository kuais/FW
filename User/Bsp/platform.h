#ifndef __PLATFORM__
#define __PLATFORM__

#include "os.h"
#include "stm32f10x.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*---------------port define--------------------*/
/* GPIO A */
#define ADC_PORT GPIOA
#define ADC_PIN1 GPIO_Pin_1

#define UART2_PORT GPIOA
#define UART2_PINTX GPIO_Pin_2
#define UART2_PINRX GPIO_Pin_3

#define SPI1_PORT GPIOA
#define SPI1_PINCS GPIO_Pin_4
#define SPI1_PINSCK GPIO_Pin_5
#define SPI1_PINMISO GPIO_Pin_6
#define SPI1_PINMOSI GPIO_Pin_7

#define UART1_PORT GPIOA
#define UART1_PINTX GPIO_Pin_9
#define UART1_PINRX GPIO_Pin_10

/* GPIO B */
#define BUTTON_PORT GPIOB
#define BUTTON_PIN GPIO_Pin_1

#define AT24CXX_PORT GPIOB
#define AT24CXX_SCL_IO GPIO_Pin_6
#define AT24CXX_SDA_IO GPIO_Pin_7

#define UART3_PORT GPIOB
#define UART3_PINTX GPIO_Pin_10
#define UART3_PINRX GPIO_Pin_11

#define SPI2_PORT GPIOB
#define SPI2_PINCS GPIO_Pin_12
#define SPI2_PINSCK GPIO_Pin_13
#define SPI2_PINMISO GPIO_Pin_14
#define SPI2_PINMOSI GPIO_Pin_15

/* GPIO C */
#define DS1302_PORT GPIOC
#define DS1302_SCLK GPIO_Pin_0 // 时钟Pin
#define DS1302_IO GPIO_Pin_1   // 数据Pin
#define DS1302_RST GPIO_Pin_2  // 复位Pin

#define W5500_PORT GPIOC
#define W5500_PININT GPIO_Pin_4
#define W5500_PINRST GPIO_Pin_5 // PC5

#define UART4_PORT GPIOC
#define UART4_PINTX GPIO_Pin_10
#define UART4_PINRX GPIO_Pin_11

#define UART5_PORT_TX GPIOC
#define UART5_PINTX GPIO_Pin_12
#define UART5_PORT_RX GPIOD
#define UART5_PINRX GPIO_Pin_2

#define POWER_BLE_PIN GPIO_Pin_0
#define POWER_ROUTER_PIN GPIO_Pin_2
#define POWER_KEYPAD_PIN3 GPIO_Pin_3
#define POWER_UART1_PIN GPIO_Pin_8
#define POWER_RELAYBOARD_PIN GPIO_Pin_9

#define POWER_BEEP_PIN GPIO_Pin_3
#define POWER_KEYPAD_PIN1 GPIO_Pin_6
#define POWER_KEYPAD_PIN2 GPIO_Pin_7
#define POWER_LED1_PIN GPIO_Pin_13

#define KEYPAD_PORT1 GPIOA
#define KEYPAD_INPUT2 GPIO_Pin_8

#define KEYPAD_PORT2 GPIOB
#define KEYPAD_INPUT1 GPIO_Pin_4

#define KEYPAD_PORT3 GPIOC
#define KEYPAD_INPUT3 GPIO_Pin_8
#define KEYPAD_INPUT4 GPIO_Pin_9

/*-------------------------------------------------*/
#define ENABLE_INT() __set_PRIMASK(0)  /* 开启全局中断 */
#define DISABLE_INT() __set_PRIMASK(1) /* 关闭全局中断 */

#define RCC_APB2Periph_GPIO_DISCONNECT RCC_APB2Periph_GPIOD
#define RCC_APB2Periph_ALLGPIO                                                                                                                \
    (RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | \
     RCC_APB2Periph_GPIOF | RCC_APB2Periph_GPIOG)

typedef void (*Action)(void); // 函数指针

#endif
