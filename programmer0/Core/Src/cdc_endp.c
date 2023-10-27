/*
 * cdc_endp.c
 *
 *  Created on: Oct 4, 2023
 *      Author: wjunh
 */

#include "cdc_endp.h"
#include "cdc_hwcfg.h"
#include "usbd_cdc_if.h"
#include "usb_device.h"
#include "usbd_cdc.h"
#include "usbd_ioreq.h"
#include "usart.h" // 包含USART1相关的头文件
#include "stdio.h"

extern USBD_HandleTypeDef hUsbDeviceHS;
extern __IO uint32_t packet_sent;
uint32_t Receive_length;

/*******************************************************************************
* Function Name  : EP1_IN_Callback
* Description    :
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/

void EP1_IN_Callback ()
{
  packet_sent = 1;
}

/*******************************************************************************
* Function Name  : EP3_OUT_Callback
* Description    :
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
// 定义循环缓冲区和索引
#define PACKET_SIZE 64
#define CIRC_BUF_SIZE 34 /* 62 * 34 = ~2K of data (max. NAND page) */

typedef uint8_t packet_buf_t[PACKET_SIZE];

typedef struct {
    packet_buf_t pbuf;
    uint32_t len;
} packet_t;

static packet_t circ_buf[CIRC_BUF_SIZE];
static volatile uint8_t head, size, tail = CIRC_BUF_SIZE - 1;

uint32_t USB_Data_Peek(uint8_t **data)
{
  if (!size)
    return 0;

  *data = circ_buf[head].pbuf;

  return circ_buf[head].len;
}

uint32_t USB_Data_Get(uint8_t **data)
{
  uint32_t len;

  if (!size)
    return 0;

  *data = circ_buf[head].pbuf;
  len = circ_buf[head].len;
  head = (head + 1) % CIRC_BUF_SIZE;
  __disable_irq();
  size--;
  __enable_irq();

  return len;
}

static inline void USB_DataRx_Sched_Internal(void)
{
  if (size < CIRC_BUF_SIZE)
//    SetEPRxValid(ENDP3);
	  USBD_CtlReceiveStatus(&hUsbDeviceHS);
}

void USB_DataRx_Sched(void)
{
  __disable_irq();
  USB_DataRx_Sched_Internal();
  __enable_irq();
}

// CDC接收数据回调函数
/*void EP3_OUT_Callback(void)*/
void EP3_OUT_Callback(uint8_t **Buf, uint32_t *Len)
{
	Receive_length = USBD_GetRxCount(&hUsbDeviceHS, CDC_OUT_EP);
    if (size < CIRC_BUF_SIZE)
    {
    	printf("E3_OUT run \r\n");
        // 循环缓冲区索引移动
        tail = (tail + 1) % CIRC_BUF_SIZE;
        // 将接收到的数据复制到循环缓冲区
        // 将数据从 Buf 复制到 circ_buf[tail].pbuf
        memcpy(circ_buf[tail].pbuf, Buf, Receive_length);
        circ_buf[tail].len = Receive_length;
        size++;
        USB_DataRx_Sched_Internal();
    }

    // 发送数据到USART1
//    HAL_UART_Transmit(&huart1, circ_buf[tail].pbuf, Receive_length, HAL_MAX_DELAY);

    // 发送数据回USB HS
//    CDC_Transmit_HS(circ_buf[tail].pbuf, Receive_length); // 假设存在一个名为USBD_CDC_Transmit_HS的函数
}

