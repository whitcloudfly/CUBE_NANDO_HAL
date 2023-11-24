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
    HAL_GPIO_WritePin(GPIOA, RED_Pin|YELLOW_Pin, GPIO_PIN_SET);


    /*Configure GPIO pin : PtPin */
    GPIO_InitStruct.Pin = RED_Pin|YELLOW_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    HAL_GPIO_WritePin(GPIOA, RED_Pin|YELLOW_Pin, GPIO_PIN_RESET); // 将引脚0和引脚1的状态置为低电平
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
    led_set(GPIOA, RED_Pin, on); // 设置引脚LED1的状态
}

void led_rd_set(bool on)
{
    led_set(GPIOA, YELLOW_Pin, on); // 设置引脚LED2的状态
}
