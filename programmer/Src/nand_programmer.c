/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
根据功能和逻辑，可以将该代码分为以下几部分进行分析：

1. 数据结构和全局变量部分：
   - `np_prog_t`结构体：用于存储程序运行时的状态和数据。
   - `boot_config_t`结构体：用于存储引导配置。
   - `cmd_handler`数组：存储命令处理函数的映射关系。
   - `np_comm_cb`变量：存储NP通信回调函数的指针。

2. 命令处理函数部分：
   - `np_cmd_nand_read_id`函数：执行NAND读取ID命令。
   - `np_cmd_nand_erase`函数：执行NAND擦除命令。
   - `np_cmd_nand_read`函数：执行NAND读取命令。
   - `np_cmd_nand_write`函数：执行NAND写入命令。
   - `np_cmd_nand_conf`函数：执行NAND配置命令。
   - `np_cmd_read_bad_blocks`函数：执行读取坏块表命令。
   - `np_cmd_version_get`函数：执行获取版本命令。
   - `np_cmd_active_image_get`函数：执行获取激活镜像命令。
   - `np_cmd_fw_update_start`函数：执行固件更新起始命令。
   - `np_cmd_fw_update_data`函数：执行固件更新数据命令。
   - `np_cmd_fw_update_end`函数：执行固件更新结束命令。
   - `np_cmd_fw_update`函数：根据命令类型调用相应的固件更新函数。
   - `np_cmd_is_valid`函数：检查命令是否有效。

3. 命令处理函数调用和处理部分：
   - `np_cmd_handler`函数：对接收到的命令进行处理，根据命令码调用相应的处理函数。
   - `np_packet_handler`函数：处理接收到的数据包，不断调用命令处理函数进行处理。
   - `np_nand_handler`函数：处理NAND操作，包括处理NAND写入状态和发送错误信息。

4. 初始化和处理函数部分：
   - `np_init`函数：初始化`np_prog_t`结构体。
   - `np_handler`函数：NP的处理函数，调用`np_packet_handler`和`np_nand_handler`函数。

5. 通信回调函数注册和注销部分：
   - `np_comm_register`函数：注册NP通信回调函数。
   - `np_comm_unregister`函数：注销NP通信回调函数。

以上是根据代码的功能和逻辑将代码分成的几个部分。每个部分都有不同的功能和职责，共同实现了整个固件更新模块的功能。
在实际应用中，可以根据需要对各个部分进行修改和扩展，以适应具体的系统要求和需求。*/

#include "nand_programmer.h"
#include "log.h"
#include "version.h"
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdbool.h>
#include <inttypes.h>


#define NP_PACKET_BUF_SIZE 64
#define NP_MAX_PAGE_SIZE 0x21C0 /* 8KB + 448 spare */
#define NP_WRITE_ACK_BYTES 1984
#define NP_NAND_TIMEOUT 0x1000000

#define NP_NAND_GOOD_BLOCK_MARK 0xFF

#define BOOT_CONFIG_ADDR 0x08005000
#define FLASH_START_ADDR 0x08000000
#define FLASH_SIZE 0x40000
#define FLASH_PAGE_SIZE 0x800
#define FLASH_BLOCK_SIZE 0x800

typedef struct
{
    uint8_t *rx_buf;            // 接收缓冲区
    uint32_t rx_buf_len;        // 接收缓冲区长度
    uint64_t addr;              // 地址
    uint64_t len;               // 长度
    uint64_t base_addr;         // 基地址
    uint32_t page_size;         // 页大小
    uint32_t block_size;        // 块大小
    uint64_t total_size;        // 总大小
    int addr_is_set;            // 地址是否设置
    int bb_is_read;             // 是否读取坏块信息
    int chip_is_conf;           // 芯片是否配置
//    np_page_t page;             // 页信息
    uint64_t bytes_written;     // 写入的字节数
    uint64_t bytes_ack;         // 确认写入的字节数
    int skip_bb;                // 跳过坏块
    int nand_wr_in_progress;    // NAND 写入是否进行中
    uint32_t nand_timeout;      // NAND 超时
//    chip_info_t chip_info;      // 芯片信息
    uint8_t active_image;       // 活动镜像
    uint8_t hal;                // HAL 版本
} np_prog_t;

typedef struct
{
    int id;                     // ID
    bool is_chip_cmd;           // 是否为芯片命令
    int (*exec)(np_prog_t *prog);// 执行函数
} np_cmd_handler_t;

static np_comm_cb_t *np_comm_cb;                   // 通信回调函数
static np_prog_t prog;                             // 编程器结构体

// NP 初始化函数
void np_init()
{
    prog.active_image = 0xff;
}

// 数据包处理函数
static void np_packet_handler(np_prog_t *prog)
{
    int ret;

    do
    {
        prog->rx_buf_len = np_comm_cb->peek(&prog->rx_buf);
        if (!prog->rx_buf_len)
            break;

//        ret = np_cmd_handler(prog);

        np_comm_cb->consume();

//        if (ret < 0)
//            np_send_error(-ret);
    }
    while (1);
}

// NP 处理函数
void np_handler()
{
    np_packet_handler(&prog);
//    np_nand_handler(&prog);
}

// 注册 NP 通信回调函数
int np_comm_register(np_comm_cb_t *cb)
{
    np_comm_cb = cb;

    return 0;
}

// 注销 NP 通信回调函数
void np_comm_unregister(np_comm_cb_t *cb)
{
    if (np_comm_cb == cb)
        np_comm_cb = NULL;
}
