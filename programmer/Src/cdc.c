/*
 * cdc.c
 *
 *  Created on: Sep 14, 2023
 *      Author: wjunh
 */

#include "cdc.h"
#include "log.h"                        // 日志库头文件
#include "nand_programmer.h"            // NAND编程器头文件
#include "usb_device.h"
#include "usbd_cdc_if.h" // 包含USB CDC接口文件

#define SEND_TIMEOUT 0x1000000  // 发送超时时间（您可以根据需要进行调整）

// CDC发送缓冲区和发送状态
static uint8_t cdc_tx_buffer[64]; // 这里的大小可以根据你的需求进行调整
static uint8_t cdc_tx_busy = 0;

static int cdc_send(uint8_t *data, uint32_t len)
{
    uint32_t timeout = SEND_TIMEOUT;

    // 检查是否有先前的CDC传输正在进行
    while (cdc_tx_busy && --timeout);

    if (!timeout)
    {
        ERROR_PRINT("等待先前的CDC TX完成超时\r\n");
        return -1;
    }

    // 将数据复制到CDC发送缓冲区
    memcpy(cdc_tx_buffer, data, len);
    cdc_tx_busy = 1;

    // 使用USB VCP发送数据
    if (CDC_Transmit_HS(cdc_tx_buffer, len) != USBD_OK)
    {
        ERROR_PRINT("发送数据失败\r\n");
        cdc_tx_busy = 0;
        return -1;
    }

    return 0;
}

static int cdc_send_ready()
{
    // 检查CDC是否准备好发送
    return !cdc_tx_busy;
}

static uint32_t cdc_peek(uint8_t **data)
{
    // 这里不会查看USB数据，可以不实现
    return 0;
}

static void cdc_consume()
{
    // 这里不需要消耗USB数据，可以不实现
}

static np_comm_cb_t cdc_comm_cb =
{
    .send = cdc_send,
    .send_ready = cdc_send_ready,
    .peek = cdc_peek,
    .consume = cdc_consume,
};

void cdc_init()
{
    // 注册CDC通信回调函数
    np_comm_register(&cdc_comm_cb);

    // 初始化CDC发送状态
    cdc_tx_busy = 0;
}

// 在USB数据接收回调函数中处理接收到的数据
void CDC_ReceiveCallback(uint8_t* Buf, uint32_t Len)
{
    // 在这里处理接收到的数据
    // 你可以调用应用程序特定的函数来处理数据
    // 例如，将数据传递给应用程序处理函数
    app_handle_received_data(Buf, Len);
}

