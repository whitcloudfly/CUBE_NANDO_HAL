/*
 * cdc_hwcfg.h
 *
 *  Created on: Oct 5, 2023
 *      Author: wjunh
 */

#ifndef INC_CDC_HWCFG_H_
#define INC_CDC_HWCFG_H_

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
#define MASS_MEMORY_START     0x04002000
#define BULK_MAX_PACKET_SIZE  0x00000040
#define LED_ON                0xF0
#define LED_OFF               0xFF

#define         ID1          (0x1FFFF7E8)
#define         ID2          (0x1FFFF7EC)
#define         ID3          (0x1FFFF7F0)

/* Exported functions ------------------------------------------------------- */

uint32_t CDC_Send_DATA (uint8_t *ptrBuffer, uint8_t Send_length);
uint32_t CDC_Receive_DATA(void);
uint32_t CDC_ReceiveDataLen(void);
void CDC_ReceiveDataAck(void);
int CDC_IsPacketSent(void);
uint32_t USB_Data_Peek(uint8_t **data);
uint32_t USB_Data_Get(uint8_t **data);
void USB_DataRx_Sched(void);

#endif /* INC_CDC_HWCFG_H_ */
