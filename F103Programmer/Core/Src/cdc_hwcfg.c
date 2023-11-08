/*
 * cdc_hwcfg.c
 *
 *  Created on: Oct 5, 2023
 *      Author: wjunh
 */

#include "stm32f1xx_it.h"
#include "cdc_endp.h"
#include "cdc_hwcfg.h"
#include "usbd_cdc_if.h"
#include "usb_device.h"
#include "usbd_cdc.h"
#include "usbd_ioreq.h"
#include "usart.h" // 包含USART1相关的头文件
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
//ErrorStatus HSEStartUpStatus;
//EXTI_ConfigTypeDef EXTI_InitStructure;
extern USBD_HandleTypeDef hUsbDeviceFS;

__IO uint32_t packet_sent = 1;
extern __IO uint8_t Send_Buffer[CDC_DATA_FS_MAX_PACKET_SIZE] ;
__IO uint32_t packet_receive=1;
extern __IO uint8_t Receive_length;

uint32_t Send_length;

/* Extern variables ----------------------------------------------------------*/

//extern LINE_CODING linecoding;

/*******************************************************************************
* Function Name  : HexToChar.
* Description    : Convert Hex 32Bits value into char.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
static void IntToUnicode (uint32_t value , uint8_t *pbuf , uint8_t len)
{
  uint8_t idx = 0;

  for( idx = 0 ; idx < len ; idx ++)
  {
    if( ((value >> 28)) < 0xA )
    {
      pbuf[ 2* idx] = (value >> 28) + '0';
    }
    else
    {
      pbuf[2* idx] = (value >> 28) + 'A' - 10;
    }

    value = value << 4;

    pbuf[ 2* idx + 1] = 0;
  }
}

/*******************************************************************************
* Function Name  : Send DATA .
* Description    : send the data received from the STM32 to the PC through USB
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
uint32_t CDC_Send_DATA (uint8_t *ptrBuffer, uint8_t Send_length)
{
  /*if max buffer is Not reached*/
  if(Send_length <= CDC_DATA_FS_MAX_PACKET_SIZE)
  {
	/*Sent flag*/
	packet_sent = 0;
	/* send  packet to PMA*/
    CDC_Transmit_FS((unsigned char*)ptrBuffer, Send_length);
  }
  else
  {
    return 0;
  }
  return 1;
}

/*******************************************************************************
* Function Name  : Receive DATA .
* Description    : receive the data from the PC to STM32 and send it through USB
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
uint32_t CDC_Receive_DATA(void)
{
  USBD_CtlReceiveStatus(&hUsbDeviceFS);
  return 1 ;
}

/*******************************************************************************
* Function Name  : CDC_ReceiveDataLen.
* Description    : Returns number of bytes in receive buffer.
* Input          : None.
* Output         : None.
* Return         : Number of bytes.
*******************************************************************************/
uint32_t CDC_ReceiveDataLen(void)
{
  return Receive_length;
}

/*******************************************************************************
* Function Name  : CDC_ReceiveDataAck.
* Description    : Acknowledges received data has been handled.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void CDC_ReceiveDataAck(void)
{
  Receive_length = 0;
}

/*******************************************************************************
* Function Name  : CDC_IsPacketSent.
* Description    : Returns 1 if packet has been sent.
* Input          : None.
* Output         : None.
* Return         : 1/0.
*******************************************************************************/
int CDC_IsPacketSent(void)
{
  return packet_sent;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/