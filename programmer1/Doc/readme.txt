STM32 USB设备CDC类增加多个端点

首先肯定是要在配置描述符里增加端点的描述符，修改接口描述符的总数
(usbd_cdc.c)

__ALIGN_BEGIN static uint8_t USBD_CDC_CfgFSDesc[USB_CDC_CONFIG_DESC_SIZ] __ALIGN_END =
{
  /* Configuration Descriptor */
  0x09,                                       /* bLength: Configuration Descriptor size */
  USB_DESC_TYPE_CONFIGURATION,                /* bDescriptorType: Configuration */
  USB_CDC_CONFIG_DESC_SIZ,                    /* wTotalLength:no of returned bytes */
  0x00,
  0x01,                                       /* bNumInterfaces: 2 interface */
  0x01,                                       /* bConfigurationValue: Configuration value */
  0x00,                                       /* iConfiguration: Index of string descriptor describing the configuration */
  0xC0,                                       /* bmAttributes: self powered */
  0x32,                                       /* MaxPower 0 mA */

  /* Data class interface descriptor */
  0x09,                                       /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_INTERFACE,                    /* bDescriptorType: */
  0x00,                                       /* bInterfaceNumber: Number of Interface */
  0x00,                                       /* bAlternateSetting: Alternate setting */
  0x04,                                       /* bNumEndpoints: Four endpoints used */
  0x00,                                       /* bInterfaceClass: Custom */
  0x00,                                       /* bInterfaceSubClass: */
  0x00,                                       /* bInterfaceProtocol: */
  0x00,                                       /* iInterface: */

  /* Endpoint OUT Descriptor */
  0x07,                                       /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,                     /* bDescriptorType: Endpoint */
  EP1_OUT_EP,                                 /* bEndpointAddress */
  0x02,                                       /* bmAttributes: Bulk */
  LOBYTE(CDC_DATA_FS_MAX_PACKET_SIZE),        /* wMaxPacketSize: */
  HIBYTE(CDC_DATA_FS_MAX_PACKET_SIZE),
  0x00,                                       /* bInterval: ignore for Bulk transfer */

  /* Endpoint IN Descriptor */
  0x07,                                       /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,                     /* bDescriptorType: Endpoint */
  EP1_IN_EP,                                  /* bEndpointAddress */
  0x02,                                       /* bmAttributes: Bulk */
  LOBYTE(CDC_DATA_FS_MAX_PACKET_SIZE),        /* wMaxPacketSize: */
  HIBYTE(CDC_DATA_FS_MAX_PACKET_SIZE),
  0x00,                                        /* bInterval: ignore for Bulk transfer */

  /* Endpoint OUT Descriptor */
  0x07,                                       /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,                     /* bDescriptorType: Endpoint */
  EP2_OUT_EP,                                 /* bEndpointAddress */
  0x02,                                       /* bmAttributes: Bulk */
  LOBYTE(CDC_DATA_FS_MAX_PACKET_SIZE),        /* wMaxPacketSize: */
  HIBYTE(CDC_DATA_FS_MAX_PACKET_SIZE),
  0x00,                                       /* bInterval: ignore for Bulk transfer */

  /* Endpoint IN Descriptor */
  0x07,                                       /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,                     /* bDescriptorType: Endpoint */
  EP2_IN_EP,                                  /* bEndpointAddress */
  0x02,                                       /* bmAttributes: Bulk */
  LOBYTE(CDC_DATA_FS_MAX_PACKET_SIZE),        /* wMaxPacketSize: */
  HIBYTE(CDC_DATA_FS_MAX_PACKET_SIZE),
  0x00                                        /* bInterval: ignore for Bulk transfer */
};



/**
  * @brief  USBD_CDC_DataIn
  *         Data sent on non-control IN endpoint
  * @param  pdev: device instance
  * @param  epnum: endpoint number
  * @retval status
  */
static uint8_t USBD_CDC_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  USBD_CDC_HandleTypeDef *hcdc;
  PCD_HandleTypeDef *hpcd = pdev->pData;

  if (pdev->pClassData == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  hcdc = (USBD_CDC_HandleTypeDef *)pdev->pClassData;

  if ((pdev->ep_in[epnum].total_length > 0U) &&
      ((pdev->ep_in[epnum].total_length % hpcd->IN_ep[epnum].maxpacket) == 0U))
  {
    /* Update the packet total length */
    pdev->ep_in[epnum].total_length = 0U;

    /* Send ZLP */
    (void)USBD_LL_Transmit(pdev, epnum, NULL, 0U);
  }
  else
  {
    hcdc->TxState[(epnum & 0x0FU) - 1] = 0U;
    ((USBD_CDC_ItfTypeDef *)pdev->pUserData)->TransmitCplt(hcdc->TxBuffer[(epnum & 0x0FU) - 1],
                     &hcdc->TxLength[(epnum & 0x0FU) - 1], epnum);
  }

  return (uint8_t)USBD_OK;
}


修改CDC句柄数据结构
(usbd_cdc.h)

#define TX_EP_NUM					0x02U
#define EP1_IN_EP					0x81U
#define EP1_OUT_EP					0x01U
#define EP2_IN_EP					0x82U
#define EP2_OUT_EP					0x02U

//主要扩展了Tx的Buffer Length State 
typedef struct
{
  uint32_t data[CDC_DATA_HS_MAX_PACKET_SIZE / 4U];      /* Force 32bits alignment */
  uint8_t  CmdOpCode;
  uint32_t CmdLength;
  uint8_t  *RxBuffer;
  uint8_t  *TxBuffer[TX_EP_NUM];
  uint32_t RxLength;
  uint32_t TxLength[TX_EP_NUM];
  __IO uint32_t TxState[TX_EP_NUM];
  __IO uint32_t RxState;
} USBD_CDC_HandleTypeDef;

修改发送缓冲区
(usbd_cdc_if.c)
/** Data to send over USB CDC are stored in this buffer   */
uint8_t UserTxBufferFS[TX_EP_NUM][APP_TX_DATA_SIZE];

/**
  * @brief  Initializes the CDC media low layer over the FS USB IP
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Init_FS(void)
{
  /* USER CODE BEGIN 3 */
  /* Set Application Buffers */
  USBD_CDC_SetTxBuffer(&hUsbDeviceFS, UserTxBufferFS[0], 0);
  USBD_CDC_SetT2Buffer(&hUsbDeviceFS, UserTxBufferFS[1], 0);
  USBD_CDC_SetRxBuffer(&hUsbDeviceFS, UserRxBufferFS);
  return (USBD_OK);
  /* USER CODE END 3 */
}


在USBD_LL_Init 函数后面添加
(usbd_conf.c)

  HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_FS, 1, 0x40);
  HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_FS, 2, 0x40);