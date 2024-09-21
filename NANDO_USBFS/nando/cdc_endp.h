/*
 * cdc.h
 *
 *  Created on: Oct 4, 2023
 *      Author: wjunh
 */

#ifndef INC_CDC_ENDP_H_
#define INC_CDC_ENDP_H_

#include <stdint.h>

#define PACKET_SIZE 64
#define CIRC_BUF_SIZE 34

typedef uint8_t packet_buf_t[PACKET_SIZE];

void CDC_IN_Callback ();
void CDC_OUT_Callback(uint8_t **Buf, uint32_t *Len);
uint32_t USB_Data_Peek(uint8_t **data);
uint32_t USB_Data_Get(uint8_t **data);

#endif /* INC_CDC_ENDP_H_ */
