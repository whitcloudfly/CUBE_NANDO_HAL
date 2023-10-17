/*
 * usbd_init.c
 *
 *  Created on: Sep 15, 2023
 *      Author: wjunh
 */

#include "usbd_init.h"

extern USBD_HandleTypeDef hUsbDeviceFS;
extern PCD_HandleTypeDef hpcd_USB_OTG_FS;
extern uint8_t UserTxBufferFS[APP_TX_DATA_SIZE];

USB_ISR Usb_ISR;

/*******************************************************************************
* 函数  : STM32_USBD_Init
* 描述  : USB初始化函数
* 输入  : Tx_Over: 发送回调函数
		  RxFunction: 接收回调函数
* 输出  : None
* 返回  : None
* 注意  : None
*******************************************************************************/

void STM32_USBD_Init(void Tx_Over(void), void (*RxFunction)(uint8_t *RX_Data,uint32_t *Len))
{

    MX_USB_DEVICE_Init();
	Usb_ISR.USBTXOver = Tx_Over;
	Usb_ISR.USBRXOperation = RxFunction;
}

/*******************************************************************************
* 函数  : USBD_SendBuff
* 描述  : USB发送函数
* 输入  :Buf: 发送数据缓存区
		 Len: 发送数据长度
* 输出  : None
* 返回  : None
* 注意  : None
*******************************************************************************/

void USBD_SendBuff(uint8_t* Buf, uint16_t Len)
{
   CDC_Transmit_FS(Buf,Len);
}


/*******************************************************************************
* 函数  : usb_printf
* 描述  : printf重定义函数
* 输入  : None
* 输出  : None
* 返回  : None
* 注意  : None
*******************************************************************************/
void usb_printf(const char *format, ...)
{
    va_list args;
    uint32_t length;

    va_start(args, format);
    length = vsnprintf((char *)UserTxBufferFS, APP_TX_DATA_SIZE, (char *)format, args);
    va_end(args);
    CDC_Transmit_FS(UserTxBufferFS, length);
}



/*void OTG_FS_IRQHandler(void)
{
  HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);
}
*/
/*******************************************************************************
* 函数  : RecvMemcpy
* 描述  : 接收数据回调函数
* 输入  : None
* 输出  : None
* 返回  : None
* 注意  : None
*******************************************************************************/

void RecvMemcpy(uint8_t *str,uint32_t *Len)
{
   uint8_t *mainstr = (uint8_t *)malloc(sizeof(uint8_t)*(*Len));
   memcpy(mainstr,str,*Len);

   RecvProcess(str,Len);
}

/*******************************************************************************
* 函数  : RecvProcess
* 描述  : 接收数据处理函数
* 输入  : None
* 输出  : None
* 返回  : None
* 注意  : None
*******************************************************************************/
void RecvProcess(uint8_t *str,uint32_t *Len)
{

}
