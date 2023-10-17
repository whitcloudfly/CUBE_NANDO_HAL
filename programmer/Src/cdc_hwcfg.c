/*
 * cdc_hwcfg.c
 *
 *  Created on: Oct 5, 2023
 *      Author: wjunh
 */

#include "stm32f4xx_it.h"
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
//EXTI_InitTypeDef EXTI_InitStructure;
__IO uint32_t packet_sent = 1;
extern USBD_HandleTypeDef hUsbDeviceHS;
extern __IO uint8_t Send_Buffer[VIRTUAL_COM_PORT_DATA_SIZE] ;
__IO uint32_t packet_receive = 1;
extern __IO uint8_t Receive_length;

uint32_t Send_length;
//static void IntToUnicode (uint32_t value , uint8_t *pbuf , uint8_t len);
/* Extern variables ----------------------------------------------------------*/

//extern LINE_CODING linecoding;

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
  if(Send_length <= VIRTUAL_COM_PORT_DATA_SIZE)
  {
/*packet_sent = 0;
  memcpy(CDC_IN_EP, (unsigned char*)ptrBuffer, Send_length);
  CDC_Transmit_HS((unsigned char*)ptrBuffer, Send_length);
单次发送
 */
	  packet_sent = 0;
//	  uint8_t result = USBD_OK;
//      memcpy(CDC_IN_EP, (unsigned char*)ptrBuffer, Send_length);
      CDC_Transmit_HS((unsigned char*)ptrBuffer, Send_length);
//      HAL_Delay(10000);
//	  return USBD_OK;
//      CDC_TransmitCplt_HS(uint8_t *Buf, uint32_t *Len, uint8_t epnum)

	  /* USER CODE BEGIN 12 */
/*	  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceHS.pClassData;
	  if (hcdc->TxState != 0){
	    return USBD_BUSY;
	    return packet_sent = 1;
	  }
	  USBD_CDC_SetTxBuffer(&hUsbDeviceHS, *ptrBuffer, Send_length);
	  result = USBD_CDC_TransmitPacket(&hUsbDeviceHS);*/
	  /* USER CODE END 12 */

//      CDC_Transmit_HS((unsigned char*)ptrBuffer, Send_length);
//    HAL_Delay(20000);
//    USBD_CtlSendStatus(&hUsbDeviceHS);
  }
  else
  {
    return 0;
    return USBD_OK;
  }
  return 1;
  return USBD_BUSY;
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
  USBD_CtlReceiveStatus(&hUsbDeviceHS);
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

