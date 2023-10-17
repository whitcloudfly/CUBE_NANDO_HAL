/*
 * usbd_init.h
 *
 *  Created on: Sep 15, 2023
 *      Author: wjunh
 */

#ifndef _USBD_INIT_H_
#define _USBD_INIT_H_

#include <stdio.h>
#include <stdlib.h>
#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "usbd_def.h"
#include "usbd_core.h"
#include "usbd_cdc.h"
#include "usb_device.h"
//#include "types.h"
#include "stdarg.h"
#include "usbd_cdc_if.h"

typedef struct USB_ISR
{
	void (*USBTXOver)(void);    //USB发送回调函数
	void (*USBRXOperation)(uint8_t *RxData,uint32_t *Len);  //USB接收回调函数

}USB_ISR;

extern USB_ISR Usb_ISR;

/*******************************************************************************
* 函数  : STM32_USBD_Init
* 描述  : USB初始化函数
* 输入  : Tx_Over: 发送回调函数
		  RxFunction: 接收回调函数
* 输出  : None
* 返回  : None
* 注意  : None
*******************************************************************************/

void STM32_USBD_Init(void Tx_Over(void), void (*RxFunction)(uint8_t *RX_Data,uint32_t *Len));

/*******************************************************************************
* 函数  : USBD_SendBuff
* 描述  : USB发送函数
* 输入  :Buf: 发送数据缓存区
		 Len: 发送数据长度
* 输出  : None
* 返回  : None
* 注意  : None
*******************************************************************************/
void USBD_SendBuff(uint8_t* Buf, uint16_t Len);

/*******************************************************************************
* 函数  : usb_printf
* 描述  : printf重定义函数
* 输入  : None
* 输出  : None
* 返回  : None
* 注意  : None
*******************************************************************************/
void usb_printf(const char *format, ...);

/*******************************************************************************
* 函数  : RecvMemcpy
* 描述  : 接收数据回调函数
* 输入  : None
* 输出  : None
* 返回  : None
* 注意  : None
*******************************************************************************/
void RecvMemcpy(uint8_t *str,uint32_t *Len);

/*******************************************************************************
* 函数  : RecvProcess
* 描述  : 接收数据处理函数
* 输入  : None
* 输出  : None
* 返回  : None
* 注意  : None
*******************************************************************************/
void RecvProcess(uint8_t *str,uint32_t *Len);

#endif
