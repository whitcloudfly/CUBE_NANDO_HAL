/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */
/*代码是一个关于LED控制的C代码，通过调用不同的函数实现LED的初始化和控制。*/

#include "led.h"
#include "gpio.h"

void led_init()
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOE_CLK_ENABLE(); // 初始化GPIOE时钟
    __HAL_RCC_GPIOC_CLK_ENABLE(); // 初始化GPIOC时钟

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(LED4_GPIO_Port, LED4_Pin, GPIO_PIN_SET);

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOC, LED1_Pin|LED2_Pin|LED3_Pin, GPIO_PIN_SET);

    /*Configure GPIO pin : PtPin */
    GPIO_InitStruct.Pin = LED4_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED4_GPIO_Port, &GPIO_InitStruct);

    /*Configure GPIO pins : PCPin PCPin PCPin */
    GPIO_InitStruct.Pin = LED1_Pin|LED2_Pin|LED3_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    HAL_GPIO_WritePin(GPIOC, LED1_Pin | LED2_Pin, GPIO_PIN_RESET); // 将引脚0和引脚1的状态置为低电平
}

static void led_set(GPIO_TypeDef *gpiox, uint16_t pin, bool on)
{
    if (on)
    	HAL_GPIO_WritePin(gpiox, pin, GPIO_PIN_SET); // 设置引脚为高电平
    else
    	HAL_GPIO_WritePin(gpiox, pin, GPIO_PIN_RESET); // 设置引脚为低电平
}

void led_wr_set(bool on)
{
    led_set(GPIOC, LED1_Pin, on); // 设置引脚LED1的状态
}

void led_rd_set(bool on)
{
    led_set(GPIOC, LED2_Pin, on); // 设置引脚LED2的状态
}
