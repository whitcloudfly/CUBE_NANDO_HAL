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
#include "nand_bad_block.h"
#include "fsmc_nand.h"
#include "fsmc.h"
#include "chip_info.h"
#include "chip.h"
#include "led.h"
#include "log.h"
#include "version.h"
#include "flash.h"
#include "spi_nor_flash.h"
#include "spi_nand_flash.h"
//#include "spi.h"
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

#define BOOT_CONFIG_ADDR 0x08003800
#define FLASH_START_ADDR 0x08000000
#define FLASH_SIZE 0x40000
#define FLASH_PAGE_SIZE 0x800
#define FLASH_BLOCK_SIZE 0x800

// 定义枚举类型 np_cmd_code_t，表示 NAND 命令的代码
typedef enum
{
    NP_CMD_NAND_READ_ID     = 0x00, // 读取 NAND ID
    NP_CMD_NAND_ERASE       = 0x01, // 擦除 NAND
    NP_CMD_NAND_READ        = 0x02, // 读取 NAND
    NP_CMD_NAND_WRITE_S     = 0x03, // 写入 NAND（单页写入）
    NP_CMD_NAND_WRITE_D     = 0x04, // 写入 NAND（双页写入）
    NP_CMD_NAND_WRITE_E     = 0x05, // 写入 NAND（扩展页写入）
    NP_CMD_NAND_CONF        = 0x06, // 配置 NAND
    NP_CMD_NAND_READ_BB     = 0x07, // 读取坏块表
    NP_CMD_VERSION_GET      = 0x08, // 获取版本信息
    NP_CMD_ACTIVE_IMAGE_GET = 0x09, // 获取活动镜像信息
    NP_CMD_FW_UPDATE_S      = 0x0a, // 启动固件更新（单页写入）
    NP_CMD_FW_UPDATE_D      = 0x0b, // 启动固件更新（双页写入）
    NP_CMD_FW_UPDATE_E      = 0x0c, // 启动固件更新（扩展页写入）
    NP_CMD_NAND_LAST        = 0x0d, // NAND 命令的最后一个代码
} np_cmd_code_t;

// 错误代码
enum
{
    NP_ERR_INTERNAL       = -1,    // 内部错误
    NP_ERR_ADDR_EXCEEDED  = -100,  // 地址超过限制
    NP_ERR_ADDR_INVALID   = -101,  // 无效的地址
    NP_ERR_ADDR_NOT_ALIGN = -102,  // 地址未对齐
    NP_ERR_NAND_WR        = -103,  // NAND 写入错误
    NP_ERR_NAND_RD        = -104,  // NAND 读取错误
    NP_ERR_NAND_ERASE     = -105,  // NAND 擦除错误
    NP_ERR_CHIP_NOT_CONF  = -106,  // 芯片未配置
    NP_ERR_CMD_DATA_SIZE  = -107,  // 无效的命令数据大小
    NP_ERR_CMD_INVALID    = -108,  // 无效的命令
    NP_ERR_BUF_OVERFLOW   = -109,  // 缓冲区溢出
    NP_ERR_LEN_NOT_ALIGN  = -110,  // 长度未对齐
    NP_ERR_LEN_EXCEEDED   = -111,  // 长度超过限制
    NP_ERR_LEN_INVALID    = -112,  // 无效的长度
    NP_ERR_BBT_OVERFLOW   = -113,  // 坏块表溢出
};

// 结构体 np_cmd_t，以紧凑方式定义，表示一个通用命令结构，包含一个命令代码
typedef struct __attribute__((__packed__))
{
    np_cmd_code_t code; // 命令代码
} np_cmd_t;

// 结构体 np_cmd_flags_t，以紧凑方式定义，表示命令标志，包括 skip_bb、inc_spare 和 enable_hw_ecc
typedef struct __attribute__((__packed__))
{
    uint8_t skip_bb : 1;        // 是否跳过坏块
    uint8_t inc_spare : 1;      // 是否包含备用区域
    uint8_t enable_hw_ecc: 1;   // 是否启用硬件 ECC
} np_cmd_flags_t;

// 结构体 np_erase_cmd_t，以紧凑方式定义，表示擦除命令结构，包含一个命令、地址、长度和标志
typedef struct __attribute__((__packed__))
{
    np_cmd_t cmd;           // 命令
    uint64_t addr;          // 地址
    uint64_t len;           // 长度
    np_cmd_flags_t flags;   // 标志
} np_erase_cmd_t;

// 结构体 np_write_start_cmd_t，以紧凑方式定义，表示写入开始命令结构，包含一个命令、地址、长度和标志
typedef struct __attribute__((__packed__))
{
    np_cmd_t cmd;           // 命令
    uint64_t addr;          // 地址
    uint64_t len;           // 长度
    np_cmd_flags_t flags;   // 标志
} np_write_start_cmd_t;

typedef struct __attribute__((__packed__))
{
    np_cmd_t cmd;       // 命令
    uint8_t len;        // 数据长度
    uint8_t data[];     // 数据
} np_write_data_cmd_t;

typedef struct __attribute__((__packed__))
{
    np_cmd_t cmd;       // 命令
} np_write_end_cmd_t;

typedef struct __attribute__((__packed__))
{
    np_cmd_t cmd;           // 命令
    uint64_t addr;          // 地址
    uint64_t len;           // 长度
    np_cmd_flags_t flags;   // 标志
} np_read_cmd_t;

typedef struct __attribute__((__packed__))
{
    np_cmd_t cmd;           // 命令
    uint8_t hal;            // HAL 版本
    uint32_t page_size;     // 页大小
    uint32_t block_size;    // 块大小
    uint64_t total_size;    // 总大小
    uint32_t spare_size;    // 备用区域大小
    uint8_t bb_mark_off;    // 坏块标记偏移
    uint8_t hal_conf[];     // HAL 配置
} np_conf_cmd_t;

enum
{
    NP_RESP_DATA   = 0x00,   // 数据响应
    NP_RESP_STATUS = 0x01,   // 状态响应
};

typedef struct __attribute__((__packed__))
{
    uint8_t code;           // 响应代码
    uint8_t info;           // 响应信息
    uint8_t data[];         // 响应数据
} np_resp_t;

enum
{
    NP_STATUS_OK        = 0x00,   // 响应状态：正常
    NP_STATUS_ERROR     = 0x01,   // 响应状态：错误
    NP_STATUS_BB        = 0x02,   // 响应状态：坏块
    NP_STATUS_WRITE_ACK = 0x03,   // 响应状态：写入确认
    NP_STATUS_BB_SKIP   = 0x04,   // 响应状态：跳过坏块
    NP_STATUS_PROGRESS  = 0x05,   // 响应状态：进度
};

typedef struct __attribute__((__packed__))
{
    np_resp_t header;       // 响应头部
    chip_id_t nand_id;      // NAND ID
} np_resp_id_t;

typedef struct __attribute__((__packed__))
{
    np_resp_t header;       // 响应头部
    uint64_t addr;          // 坏块地址
    uint32_t size;          // 坏块大小
} np_resp_bad_block_t;

typedef struct __attribute__((__packed__))
{
    np_resp_t header;       // 响应头部
    uint64_t bytes_ack;     // 确认写入的字节数
} np_resp_write_ack_t;

typedef struct __attribute__((__packed__))
{
    np_resp_t header;       // 响应头部
    uint8_t err_code;       // 错误代码
} np_resp_err_t;

typedef struct __attribute__((__packed__))
{
    np_resp_t header;       // 响应头部
    uint64_t progress;      // 进度
} np_resp_progress_t;

typedef struct __attribute__((__packed__))
{
    uint8_t major;          // 主版本号
    uint8_t minor;          // 次版本号
    uint16_t build;         // 构建号
} version_t;

typedef struct __attribute__((__packed__))
{
    np_resp_t header;           // 响应头部
    version_t version;          // 版本信息
} np_resp_version_t;

typedef struct __attribute__((__packed__))
{
    np_resp_t header;           // 响应头部
    uint8_t active_image;       // 活动镜像
} np_resp_active_image_t;

typedef struct
{
    uint32_t addr;              // 地址
    int is_valid;               // 是否有效
} np_prog_addr_t;

typedef struct
{
    uint8_t buf[NP_MAX_PAGE_SIZE];  // 缓冲区
    uint32_t page;              // 页数
    uint32_t offset;            // 偏移量
} np_page_t;

typedef struct __attribute__((__packed__))
{
    uint8_t active_image;       // 活动镜像
} boot_config_t;

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
    np_page_t page;             // 页信息
    uint64_t bytes_written;     // 写入的字节数
    uint64_t bytes_ack;         // 确认写入的字节数
    int skip_bb;                // 跳过坏块
    int nand_wr_in_progress;    // NAND 写入是否进行中
    uint32_t nand_timeout;      // NAND 超时
    chip_info_t chip_info;      // 芯片信息
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

static flash_hal_t *hal[] = { &hal_fsmc, &hal_spi_nor, &hal_spi_nand };  // HAL 驱动数组

uint8_t np_packet_send_buf[NP_PACKET_BUF_SIZE];    // 发送数据包缓冲区

static int np_send_ok_status()
{
    np_resp_t status = { NP_RESP_STATUS, NP_STATUS_OK };       // 正常状态响应
    size_t len = sizeof(status);

    if (np_comm_cb)
        np_comm_cb->send((uint8_t *)&status, len);             // 发送响应数据

    return 0;
}

static int np_send_error(uint8_t err_code)
{
    np_resp_t status = { NP_RESP_STATUS, NP_STATUS_ERROR };    // 错误状态响应
    np_resp_err_t err_status = { status, err_code };           // 错误状态信息
    size_t len = sizeof(err_status);

    if (np_comm_cb)
        np_comm_cb->send((uint8_t *)&err_status, len);         // 发送错误响应数据

    return 0;
}

// 发送坏块信息函数，参数为块地址、大小和是否跳过标志
static int np_send_bad_block_info(uint64_t addr, uint32_t size, bool is_skipped)
{
    // 根据是否跳过标志确定info的值
    uint8_t info = is_skipped ? NP_STATUS_BB_SKIP : NP_STATUS_BB;

    // 构造响应头部
    np_resp_t resp_header = { NP_RESP_STATUS, info };

    // 构造坏块信息
    np_resp_bad_block_t bad_block = { resp_header, addr, size };

    // 调用回调函数发送坏块信息，如果发送成功返回-1
    if (np_comm_cb->send((uint8_t *)&bad_block, sizeof(bad_block)))
        return -1;

    return 0;
}

// 发送进度信息函数，参数为进度值
static int np_send_progress(uint64_t progress)
{
    // 构造响应头部
    np_resp_t resp_header = { NP_RESP_STATUS, NP_STATUS_PROGRESS };

    // 构造进度信息
    np_resp_progress_t resp_progress = { resp_header, progress };

    // 调用回调函数发送进度信息，如果发送成功返回-1
    if (np_comm_cb->send((uint8_t *)&resp_progress, sizeof(resp_progress)))
        return -1;

    return 0;
}

// 内部函数，用于执行NAND读取ID的命令
static int _np_cmd_nand_read_id(np_prog_t *prog)
{
    np_resp_id_t resp;
    size_t resp_len = sizeof(resp);

    DEBUG_PRINT("Read ID command\r\n");

    // 设置响应头部的code和info字段
    resp.header.code = NP_RESP_DATA;
    resp.header.info = resp_len - sizeof(resp.header);

    // 调用硬件抽象层的读取ID函数
    hal[prog->hal]->read_id(&resp.nand_id);

    // 如果设置了通信回调函数，则发送响应数据
    if (np_comm_cb)
        np_comm_cb->send((uint8_t *)&resp, resp_len);

    DEBUG_PRINT("Chip ID: 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\r\n",
        resp.nand_id.maker_id, resp.nand_id.device_id, resp.nand_id.third_id,
        resp.nand_id.fourth_id, resp.nand_id.fifth_id, resp.nand_id.sixth_id);

    return 0;
}

// 执行NAND读取ID的命令
static int np_cmd_nand_read_id(np_prog_t *prog)
{
    int ret;

    led_rd_set(true);
    ret = _np_cmd_nand_read_id(prog);
    led_rd_set(false);

    return ret;
}

// 从页中读取坏块信息，参数为编程器、块号、页号和坏块标志指针
static int np_read_bad_block_info_from_page(np_prog_t *prog, uint32_t block,
    uint32_t page, bool *is_bad)
{
    uint32_t status;
    uint64_t addr = block * prog->chip_info.block_size;
    uint8_t *bb_mark = &prog->page.buf[prog->chip_info.page_size +
        prog->chip_info.bb_mark_off];

    // 从页的备用区域读取坏块标志
    status = hal[prog->hal]->read_spare_data(bb_mark, page,
        prog->chip_info.bb_mark_off, 1);

    // 如果读取命令无效，则尝试读取整个页的数据
    if (status == FLASH_STATUS_INVALID_CMD)
    {
        status = hal[prog->hal]->read_page(prog->page.buf, page,
            prog->chip_info.page_size + prog->chip_info.spare_size);
    }

    switch (status)
    {
    case FLASH_STATUS_READY:
        break;
    case FLASH_STATUS_ERROR:
        ERROR_PRINT("NAND read bad block info error at 0x%" PRIx64 "\r\n",
            addr);
        return NP_ERR_NAND_RD;
    case FLASH_STATUS_TIMEOUT:
        ERROR_PRINT("NAND read timeout at 0x%" PRIx64 "\r\n", addr);
        return NP_ERR_NAND_RD;
    default:
        ERROR_PRINT("Unknown NAND status\r\n");
        return NP_ERR_NAND_RD;
    }

    // 判断坏块标志是否为NP_NAND_GOOD_BLOCK_MARK
    *is_bad = prog->page.buf[prog->chip_info.page_size +
        prog->chip_info.bb_mark_off] != NP_NAND_GOOD_BLOCK_MARK;

    return 0;
}

// 内部函数，执行读取坏块命令，并可选择发送进度信息
static int _np_cmd_read_bad_blocks(np_prog_t *prog, bool send_progress)
{
    int ret;
    bool is_bad;
    uint32_t block, block_num, page_num, page;

    // 检查是否支持坏块检测
    if (!hal[prog->hal]->is_bb_supported())
        goto Exit;

    block_num = prog->chip_info.total_size / prog->chip_info.block_size;
    page_num = prog->chip_info.block_size / prog->chip_info.page_size;

    /* Bad block - not 0xFF value in the first or second page in the block at
     * some offset in the page spare area
     */

    // 遍历所有块
    for (block = 0; block < block_num; block++)
    {
        page = block * page_num;

        // 如果需要发送进度信息，则调用np_send_progress函数发送
        if (send_progress)
            np_send_progress(page);

        // 从第一个页和第二个页读取坏块信息
        if ((ret = np_read_bad_block_info_from_page(prog, block, page,
            &is_bad)))
        {
            return ret;
        }

        if (!is_bad && (ret = np_read_bad_block_info_from_page(prog, block,
            page + 1, &is_bad)))
        {
            return ret;
        }

        // 如果是坏块，则将该页添加到坏块表中
        if (is_bad && nand_bad_block_table_add(page))
            return NP_ERR_BBT_OVERFLOW;
    }

Exit:
    prog->bb_is_read = 1;

    return 0;
}

// NAND擦除函数
static int np_nand_erase(np_prog_t *prog, uint32_t page)
{
    uint32_t status;
    uint64_t addr = page * prog->chip_info.page_size;
    
    DEBUG_PRINT("NAND擦除地址：0x%" PRIx64 "\r\n", addr);

    // 调用硬件抽象层的块擦除函数
    status = hal[prog->hal]->erase_block(page);
    switch (status)
    {
    case FLASH_STATUS_READY:
        break;
    case FLASH_STATUS_ERROR:
        // 如果擦除命令失败，发送坏块信息
        if (np_send_bad_block_info(addr, prog->chip_info.block_size, false))
            return -1;
        break;
    case FLASH_STATUS_TIMEOUT:
        ERROR_PRINT("NAND擦除超时，地址：0x%" PRIx64 "\r\n", addr);
        break;
    default:
        ERROR_PRINT("未知的NAND状态\r\n");
        return -1;
    }

    return 0;
}

// 执行NAND擦除命令的内部函数
static int _np_cmd_nand_erase(np_prog_t *prog)
{
    int ret;
    uint64_t addr, len, total_size, total_len;
    uint32_t page, pages, pages_in_block, page_size, block_size;
    np_erase_cmd_t *erase_cmd;
    bool skip_bb, inc_spare, is_bad = false;

    // 检查缓冲区长度是否正确
    if (prog->rx_buf_len < sizeof(np_erase_cmd_t))
    {
        ERROR_PRINT("擦除命令的缓冲区长度错误：%lu\r\n",
            prog->rx_buf_len);
        return NP_ERR_LEN_INVALID;
    }
    erase_cmd = (np_erase_cmd_t *)prog->rx_buf;
    total_len = len = erase_cmd->len;
    addr = erase_cmd->addr;
    skip_bb = erase_cmd->flags.skip_bb;
    inc_spare = erase_cmd->flags.inc_spare;

    DEBUG_PRINT("擦除地址：0x%" PRIx64 "，长度：0x%" PRIx64 "字节\r\n", addr,
        len);

    pages_in_block = prog->chip_info.block_size / prog->chip_info.page_size;

    if (inc_spare)
    {
        pages = prog->chip_info.total_size / prog->chip_info.page_size;
        page_size = prog->chip_info.page_size + prog->chip_info.spare_size;
        block_size = pages_in_block * page_size;
        total_size = (uint64_t)pages * page_size;
    }
    else
    {
        page_size = prog->chip_info.page_size;
        block_size = prog->chip_info.block_size;
        total_size = prog->chip_info.total_size;
    }

    // 如果设置了跳过坏块标志，并且坏块表未读取，则调用_np_cmd_read_bad_blocks函数进行坏块表读取
    if (skip_bb && !prog->bb_is_read && (ret = _np_cmd_read_bad_blocks(prog, false)))
    {
        return ret;
    }

    // 检查地址是否对齐到块大小
    if (addr % block_size)
    {
        ERROR_PRINT("地址0x%" PRIx64 "未对齐到块大小0x%lx\r\n", addr, block_size);
        return NP_ERR_ADDR_NOT_ALIGN;
    }

    // 检查长度是否为零
    if (!len)
    {
        ERROR_PRINT("长度为零\r\n");
        return NP_ERR_LEN_INVALID;
    }

    // 检查长度是否对齐到块大小
    if (len % block_size)
    {
        ERROR_PRINT("长度0x%" PRIx64 "未对齐到块大小0x%lx\r\n", len, block_size);
        return NP_ERR_LEN_NOT_ALIGN;
    }

    // 检查擦除地址是否超出芯片大小
    if (addr + len > total_size)
    {
        ERROR_PRINT("擦除地址超出范围：0x%" PRIx64 "+0x%" PRIx64 "大于芯片大小0x%" PRIx64 "\r\n", addr, len, total_size);
        return NP_ERR_ADDR_EXCEEDED;
    }

    page = addr / page_size;

    while (len)
    {
        if (addr >= total_size)
        {
            ERROR_PRINT("擦除地址0x%" PRIx64 "超出范围：0x%" PRIx64 "\r\n", addr, total_size);
            return NP_ERR_ADDR_EXCEEDED;
        }

        // 如果设置了跳过坏块标志，并且当前块为坏块，则跳过当前块，并发送坏块信息
        if (skip_bb && (is_bad = nand_bad_block_table_lookup(page)))
        {
            DEBUG_PRINT("跳过坏块，地址：0x%" PRIx64 "\r\n", addr);
            if (np_send_bad_block_info(addr, block_size, true))
                return -1;
        }

        // 如果不是坏块，则执行擦除操作
        if (!is_bad && np_nand_erase(prog, page))
            return NP_ERR_NAND_ERASE;

        addr += block_size;
        page += pages_in_block;
        /* 在部分擦除时不计算坏块 */
        if (!is_bad || (is_bad && erase_cmd->len == total_size))
            len -= block_size;

        // 发送进度信息
        np_send_progress(total_len - len);
    }

    return np_send_ok_status();
}

// 执行NAND擦除命令的函数
static int np_cmd_nand_erase(np_prog_t *prog)
{
    int ret;

    // 设置写入指示灯
    led_wr_set(true);
    // 调用内部的_np_cmd_nand_erase函数执行擦除命令
    ret = _np_cmd_nand_erase(prog);
    // 关闭写入指示灯
    led_wr_set(false);

    return ret;
}

// 发送写入确认的函数
static int np_send_write_ack(uint64_t bytes_ack)
{
    np_resp_t resp_header = { NP_RESP_STATUS, NP_STATUS_WRITE_ACK };
    np_resp_write_ack_t write_ack = { resp_header, bytes_ack };

    // 发送写入确认响应
    if (np_comm_cb->send((uint8_t *)&write_ack, sizeof(write_ack)))
        return -1;

    return 0;
}

// 执行NAND写入开始命令的函数
static int np_cmd_nand_write_start(np_prog_t *prog)
{
    int ret;
    uint64_t addr, len;
    uint32_t pages, pages_in_block;
    np_write_start_cmd_t *write_start_cmd;

    // 检查缓冲区长度是否正确
    if (prog->rx_buf_len < sizeof(np_write_start_cmd_t))
    {
        ERROR_PRINT("写入开始命令的缓冲区长度错误：%lu\r\n",
            prog->rx_buf_len);
        return NP_ERR_LEN_INVALID;
    }

    write_start_cmd = (np_write_start_cmd_t *)prog->rx_buf;

    // 如果硬件支持硬件ECC，启用硬件ECC
    if (hal[prog->hal]->enable_hw_ecc)
        hal[prog->hal]->enable_hw_ecc(write_start_cmd->flags.enable_hw_ecc);

    addr = write_start_cmd->addr;
    len = write_start_cmd->len;

    DEBUG_PRINT("写入地址：0x%" PRIx64 "，长度：0x%" PRIx64 "字节\r\n",
        addr, len);

    if (write_start_cmd->flags.inc_spare)
    {
        pages = prog->chip_info.total_size / prog->chip_info.page_size;
        pages_in_block = prog->chip_info.block_size /
            prog->chip_info.page_size;
        prog->page_size = prog->chip_info.page_size +
            prog->chip_info.spare_size;
        prog->block_size = pages_in_block * prog->page_size;
        prog->total_size = (uint64_t)pages * prog->page_size;
    }
    else
    {
        prog->page_size = prog->chip_info.page_size;
        prog->block_size = prog->chip_info.block_size;
        prog->total_size = prog->chip_info.total_size;
    }

    // 检查写入地址是否超出芯片大小
    if (addr + len > prog->total_size)
    {
        ERROR_PRINT("写入地址0x%" PRIx64 "+0x%" PRIx64
            "超出芯片大小0x%" PRIx64 "\r\n", addr, len,
            prog->total_size);
        return NP_ERR_ADDR_EXCEEDED;
    }

    // 检查地址是否对齐到页大小
    if (addr % prog->page_size)
    {
        ERROR_PRINT("地址0x%" PRIx64 "未对齐到页大小0x%lx\r\n", addr, prog->page_size);
        return NP_ERR_ADDR_NOT_ALIGN;
    }

    // 检查长度是否为零
    if (!len)
    {
        ERROR_PRINT("长度为零\r\n");
        return NP_ERR_LEN_INVALID;
    }

    // 检查长度是否对齐到页大小
    if (len % prog->page_size)
    {
        ERROR_PRINT("长度0x%" PRIx64 "未对齐到页大小0x%lx\r\n", len, prog->page_size);
        return NP_ERR_LEN_NOT_ALIGN;
    }

    // 设置跳过坏块标志，并且读取坏块信息
    prog->skip_bb = write_start_cmd->flags.skip_bb;
    if (prog->skip_bb && !prog->bb_is_read &&
        (ret = _np_cmd_read_bad_blocks(prog, false)))
    {
        return ret;
    }

    // 检查页大小是否超过缓冲区大小
    if (prog->page_size > sizeof(prog->page.buf))
    {
        ERROR_PRINT("页大小0x%lx超过缓冲区大小0x%x\r\n", prog->page_size, sizeof(prog->page.buf));
        return NP_ERR_BUF_OVERFLOW;
    }

    // 设置写入相关的参数
    prog->addr = addr;
    prog->len = len;
    prog->addr_is_set = 1;

    prog->page.page = addr / prog->page_size;
    prog->page.offset = 0;

    prog->bytes_written = 0;
    prog->bytes_ack = 0;

    // 发送OK状态响应
    return np_send_ok_status();
}

// 处理NAND状态的函数
static int np_nand_handle_status(np_prog_t *prog)
{
    switch (hal[prog->hal]->read_status())
    {
    case FLASH_STATUS_ERROR:
        // 如果状态为错误，发送坏块信息
        if (np_send_bad_block_info(prog->addr, prog->block_size, false))
            return -1;
        /* 继续执行下面的代码 */
    case FLASH_STATUS_READY:
        // 如果状态为就绪，表示操作完成，将标志位和超时计数器重置
        prog->nand_wr_in_progress = 0;
        prog->nand_timeout = 0;
        break;
    case FLASH_STATUS_BUSY:
        // 如果状态为繁忙，增加超时计数器，并检查是否超时
        if (++prog->nand_timeout == NP_NAND_TIMEOUT)
        {
            ERROR_PRINT("NAND写入超时，地址：0x%" PRIx64 "\r\n", prog->addr);
            prog->nand_wr_in_progress = 0;
            prog->nand_timeout = 0;
            return -1;
        }
        break;
    default:
        ERROR_PRINT("未知的NAND状态\r\n");
        prog->nand_wr_in_progress = 0;
        prog->nand_timeout = 0;
        return -1;
    }

    return 0;
}

// 执行NAND写入的函数
static int np_nand_write(np_prog_t *prog)
{   
    // 如果上一次的NAND写入还在进行中，等待其完成
    if (prog->nand_wr_in_progress)
    {
        DEBUG_PRINT("等待上一次NAND写入完成\r\n");
        do
        {
            if (np_nand_handle_status(prog))
                return -1;
        }
        while (prog->nand_wr_in_progress);
    }

    DEBUG_PRINT("NAND写入，地址：0x%" PRIx64 "，长度：%lu字节\r\n", prog->addr,
        prog->page_size);

    // 调用硬件抽象层函数执行异步写入操作
    hal[prog->hal]->write_page_async(prog->page.buf, prog->page.page,
        prog->page_size);

    prog->nand_wr_in_progress = 1;

    return 0;
}

// 执行NAND写入数据命令的函数
static int np_cmd_nand_write_data(np_prog_t *prog)
{
    uint32_t write_len, bytes_left, len;
    np_write_data_cmd_t *write_data_cmd;

    // 检查缓冲区长度是否正确
    if (prog->rx_buf_len < sizeof(np_write_data_cmd_t))
    {
        ERROR_PRINT("写入数据命令的缓冲区长度错误：%lu\r\n",
            prog->rx_buf_len);
        return NP_ERR_LEN_INVALID;
    }

    write_data_cmd = (np_write_data_cmd_t *)prog->rx_buf;
    len = write_data_cmd->len;
    if (len + sizeof(np_write_data_cmd_t) > NP_PACKET_BUF_SIZE)
    {
        ERROR_PRINT("数据大小错误：0x%lx\r\n", len);
        return NP_ERR_CMD_DATA_SIZE;
    }

    if (len + sizeof(np_write_data_cmd_t) != prog->rx_buf_len)
    {
        ERROR_PRINT("缓冲区长度0x%lx大于命令长度0x%lx\r\n",
            prog->rx_buf_len, len + sizeof(np_write_data_cmd_t));
        return NP_ERR_CMD_DATA_SIZE;
    }

    // 检查写入地址是否已设置
    if (!prog->addr_is_set)
    {
        ERROR_PRINT("写入地址未设置\r\n");
        return NP_ERR_ADDR_INVALID;
    }

    // 计算本次写入的长度
    if (prog->page.offset + len > prog->page_size)
        write_len = prog->page_size - prog->page.offset;
    else
        write_len = len;

    // 将数据拷贝到页缓冲区
    memcpy(prog->page.buf + prog->page.offset, write_data_cmd->data, write_len);
    prog->page.offset += write_len;

    // 如果页缓冲区已满，进行页写入操作
    if (prog->page.offset == prog->page_size)
    {
        // 如果启用了跳过坏块，并且当前页是坏块，跳过该坏块
        while (prog->skip_bb && nand_bad_block_table_lookup(prog->page.page))
        {
            DEBUG_PRINT("跳过坏块，地址：0x%" PRIx64 "\r\n", prog->addr);
            if (np_send_bad_block_info(prog->addr, prog->block_size, true))
                return -1;

            prog->addr += prog->block_size;
            prog->page.page += prog->block_size / prog->page_size;
        }

        // 检查写入地址是否超过芯片大小
        if (prog->addr >= prog->total_size)
        {
            ERROR_PRINT("写入地址0x%" PRIx64 "超过芯片大小0x%" PRIx64 "\r\n", prog->addr,
                prog->total_size);
            return NP_ERR_ADDR_EXCEEDED;
        }

        // 执行NAND写入操作
        if (np_nand_write(prog))
            return NP_ERR_NAND_WR;

        // 更新地址和页偏移
        prog->addr += prog->page_size;
        prog->page.page++;
        prog->page.offset = 0;
    }

    // 处理剩余的数据
    bytes_left = len - write_len;
    if (bytes_left)
    {
        memcpy(prog->page.buf, write_data_cmd->data + write_len, bytes_left);
        prog->page.offset += bytes_left;
    }

    // 更新已写入和已确认的字节数
    prog->bytes_written += len;
    if (prog->bytes_written - prog->bytes_ack >= prog->page_size ||
        prog->bytes_written == prog->len)
    {
        // 发送写入确认响应
        if (np_send_write_ack(prog->bytes_written))
            return -1;
        prog->bytes_ack = prog->bytes_written;
    }

    // 检查实际写入的数据长度是否超过指定的长度
    if (prog->bytes_written > prog->len)
    {
        ERROR_PRINT("实际写入数据长度0x%" PRIx64 "超过0x%" PRIx64 "\r\n", prog->bytes_written, prog->len);
        return NP_ERR_LEN_EXCEEDED;
    }

    return 0;
}

// 结束NAND写入命令的函数
static int np_cmd_nand_write_end(np_prog_t *prog)
{
    // 清除地址已设置的标志位
    prog->addr_is_set = 0;

    // 检查是否有未写入的数据
    if (prog->page.offset)
    {
        ERROR_PRINT("未写入长度为0x%lx的数据\r\n",
            prog->page.offset);
        return NP_ERR_NAND_WR;
    }

    // 发送操作完成的响应
    return np_send_ok_status();
}

// 执行NAND写入命令的函数
static int np_cmd_nand_write(np_prog_t *prog)
{
    np_cmd_t *cmd = (np_cmd_t *)prog->rx_buf;
    int ret = 0;

    // 根据命令类型执行相应的操作
    switch (cmd->code)
    {
    case NP_CMD_NAND_WRITE_S:
        led_wr_set(true);
        ret = np_cmd_nand_write_start(prog);
        break;
    case NP_CMD_NAND_WRITE_D:
        ret = np_cmd_nand_write_data(prog);
        break;
    case NP_CMD_NAND_WRITE_E:
        ret = np_cmd_nand_write_end(prog);
        led_wr_set(false);
        break;
    default:
        break;
    }

    // 如果操作失败，关闭写入指示灯
    if (ret < 0)
        led_wr_set(false);

    return ret;
}

// 执行NAND读取操作的函数
static int np_nand_read(uint64_t addr, np_page_t *page, uint32_t page_size,
    uint32_t block_size, np_prog_t *prog)
{
    uint32_t status;

    // 调用硬件抽象层函数执行页面读取操作
    status = hal[prog->hal]->read_page(page->buf, page->page, page_size);
    switch (status)
    {
    case FLASH_STATUS_READY:
        break;
    case FLASH_STATUS_ERROR:
        // 如果读取状态为错误，发送坏块信息
        if (np_send_bad_block_info(addr, block_size, false))
            return -1;
        break;
    case FLASH_STATUS_TIMEOUT:
        ERROR_PRINT("NAND读取超时，地址：0x%" PRIx64 "\r\n", addr);
        break;
    default:
        ERROR_PRINT("未知的NAND状态\r\n");
        return -1;
    }

    return 0;
}

// 执行NAND读取命令的函数
static int _np_cmd_nand_read(np_prog_t *prog)
{
    int ret;
    static np_page_t page;
    np_read_cmd_t *read_cmd;
    bool skip_bb, inc_spare;
    uint64_t addr, len, total_size;
    uint32_t send_len, block_size, page_size, pages,
        pages_in_block;
    uint32_t resp_header_size = offsetof(np_resp_t, data);
    uint32_t tx_data_len = sizeof(np_packet_send_buf) - resp_header_size;
    np_resp_t *resp = (np_resp_t *)np_packet_send_buf;

    // 检查接收缓冲区长度是否正确
    if (prog->rx_buf_len < sizeof(np_read_cmd_t))
    {
        ERROR_PRINT("读取命令缓冲区长度错误 %lu\r\n",
            prog->rx_buf_len);
        return NP_ERR_LEN_INVALID;
    }

    read_cmd = (np_read_cmd_t *)prog->rx_buf;
    addr = read_cmd->addr;
    len = read_cmd->len;
    skip_bb = read_cmd->flags.skip_bb;
    inc_spare = read_cmd->flags.inc_spare;

    DEBUG_PRINT("读取地址 0x%" PRIx64 " 长度为 0x%" PRIx64 " 的数据命令\r\n", addr,
        len);

    if (inc_spare)
    {
        pages = prog->chip_info.total_size / prog->chip_info.page_size;
        pages_in_block = prog->chip_info.block_size /
            prog->chip_info.page_size;
        page_size = prog->chip_info.page_size + prog->chip_info.spare_size;
        block_size = pages_in_block * page_size;
        total_size = (uint64_t)pages * page_size;
    }
    else
    {
        page_size = prog->chip_info.page_size;
        block_size = prog->chip_info.block_size;
        total_size = prog->chip_info.total_size;
    }

    if (addr + len > total_size)
    {
        ERROR_PRINT("读取地址 0x%" PRIx64 "+0x%" PRIx64
            " 超出芯片大小 0x%" PRIx64 "\r\n", addr, len, total_size);
        return NP_ERR_ADDR_EXCEEDED;
    }

    if (addr % page_size)
    {
        ERROR_PRINT("读取地址 0x%" PRIx64
            " 不对齐于页面大小 0x%lx\r\n", addr, page_size);
        return NP_ERR_ADDR_NOT_ALIGN;
    }

    if (!len)
    {
        ERROR_PRINT("长度为0\r\n");
        return NP_ERR_LEN_INVALID;
    }

    if (len % page_size)
    {
        ERROR_PRINT("读取长度 0x%" PRIx64
            " 不对齐于页面大小 0x%lx\r\n", len, page_size);
        return NP_ERR_LEN_NOT_ALIGN;
    }

    if (skip_bb && !prog->bb_is_read && (ret = _np_cmd_read_bad_blocks(prog,
        false)))
    {
        return ret;
    }

    page.page = addr / page_size;
    page.offset = 0;

    resp->code = NP_RESP_DATA;

    while (len)
    {
        if (addr >= total_size)
        {
            ERROR_PRINT("读取地址 0x%" PRIx64
                " 超出芯片大小 0x%" PRIx64 "\r\n", addr, total_size);
            return NP_ERR_ADDR_EXCEEDED;
        }

        if (skip_bb && nand_bad_block_table_lookup(page.page))
        {
            DEBUG_PRINT("跳过坏块地址 0x%" PRIx64 "\r\n", addr);
            if (np_send_bad_block_info(addr, block_size, true))
                return -1;

            /* 在部分读取时不计算坏块 */
            if (read_cmd->len == total_size)
                len -= block_size;
            addr += block_size;
            page.page += block_size / page_size;
            continue;
        }

        if (np_nand_read(addr, &page, page_size, block_size, prog))
            return NP_ERR_NAND_RD;

        while (page.offset < page_size && len)
        {
            if (page_size - page.offset >= tx_data_len)
                send_len = tx_data_len;
            else
                send_len = page_size - page.offset;

            if (send_len > len)
                send_len = len;

            memcpy(resp->data, page.buf + page.offset, send_len);

            while (!np_comm_cb->send_ready());

            resp->info = send_len;
            if (np_comm_cb->send(np_packet_send_buf,
                resp_header_size + send_len))
            {
                return -1;
            }

            page.offset += send_len;
            len -= send_len;
        }

        addr += page_size;
        page.offset = 0;
        page.page++;
    }

    return 0;
}

// 执行NAND读取命令的函数
static int np_cmd_nand_read(np_prog_t *prog)
{
    int ret;

    led_rd_set(true); // 设置读取指示灯为亮
    ret = _np_cmd_nand_read(prog); // 执行NAND读取命令
    led_rd_set(false); // 设置读取指示灯为灭

    return ret;
}

// 填充芯片信息
static void np_fill_chip_info(np_conf_cmd_t *conf_cmd, np_prog_t *prog)
{
    prog->chip_info.page_size = conf_cmd->page_size;
    prog->chip_info.block_size = conf_cmd->block_size;
    prog->chip_info.total_size = conf_cmd->total_size;
    prog->chip_info.spare_size = conf_cmd->spare_size;
    prog->chip_info.bb_mark_off = conf_cmd->bb_mark_off;
    prog->chip_is_conf = 1;
}

// 打印芯片信息
static void np_print_chip_info(np_prog_t *prog)
{
    DEBUG_PRINT("页面大小: %lu\r\n", prog->chip_info.page_size);
    DEBUG_PRINT("块大小: %lu\r\n", prog->chip_info.block_size);
    DEBUG_PRINT("总大小: 0x%" PRIx64 "\r\n", prog->chip_info.total_size);
    DEBUG_PRINT("备用区大小: %lu\r\n", prog->chip_info.spare_size);
    DEBUG_PRINT("坏块标记偏移量: %d\r\n", prog->chip_info.bb_mark_off);
}

// 执行NAND配置命令
static int np_cmd_nand_conf(np_prog_t *prog)
{
    np_conf_cmd_t *conf_cmd;

    DEBUG_PRINT("芯片配置命令\r\n");

    // 检查接收缓冲区长度是否正确
    if (prog->rx_buf_len < sizeof(np_conf_cmd_t))
    {
        ERROR_PRINT("配置命令缓冲区长度错误 %lu\r\n",
            prog->rx_buf_len);
        return NP_ERR_LEN_INVALID;
    }

    conf_cmd = (np_conf_cmd_t *)prog->rx_buf;

    np_fill_chip_info(conf_cmd, prog); // 填充芯片信息
    np_print_chip_info(prog); // 打印芯片信息

    prog->hal = conf_cmd->hal;
    if (hal[prog->hal]->init(conf_cmd->hal_conf,
        prog->rx_buf_len - sizeof(np_conf_cmd_t)))
    {
        ERROR_PRINT("HAL配置命令缓冲区长度错误 %lu\r\n",
            prog->rx_buf_len);
        return NP_ERR_LEN_INVALID;
    }

    nand_bad_block_table_init(); // 初始化坏块表
    prog->bb_is_read = 0;

    return np_send_ok_status();
}

// 发送坏块信息
static int np_send_bad_blocks(np_prog_t *prog)
{
    uint32_t page;
    void *bb_iter;

    // 遍历坏块表，并发送坏块信息
    for (bb_iter = nand_bad_block_table_iter_alloc(&page); bb_iter;
        bb_iter = nand_bad_block_table_iter_next(bb_iter, &page))
    {
        if (np_send_bad_block_info(page * prog->chip_info.page_size,
            prog->chip_info.block_size, false))
        {
            return -1;
        }
    }

    return 0;
}

// 执行读取坏块命令
int np_cmd_read_bad_blocks(np_prog_t *prog)
{
    int ret;

    led_rd_set(true); // 设置读取指示灯为亮
    nand_bad_block_table_init(); // 初始化坏块表
    ret = _np_cmd_read_bad_blocks(prog, true); // 执行读取坏块命令
    led_rd_set(false); // 设置读取指示灯为灭

    if (ret || (ret = np_send_bad_blocks(prog))) // 发送坏块信息
        return ret;

    return np_send_ok_status(); // 发送成功状态
}

// 获取版本号命令
int np_cmd_version_get(np_prog_t *prog)
{
    np_resp_version_t resp;
    size_t resp_len = sizeof(resp);

    DEBUG_PRINT("读取版本号命令\r\n");

    resp.header.code = NP_RESP_DATA;
    resp.header.info = resp_len - sizeof(resp.header);
    resp.version.major = SW_VERSION_MAJOR;
    resp.version.minor = SW_VERSION_MINOR;
    resp.version.build = SW_VERSION_BUILD;

    if (np_comm_cb)
        np_comm_cb->send((uint8_t *)&resp, resp_len);

    return 0;
}

// 读取引导配置
static int np_boot_config_read(boot_config_t *config)
{
    if (flash_read(BOOT_CONFIG_ADDR, (uint8_t *)config, sizeof(boot_config_t))
        < 0)
    {
        return -1;
    }
    
    return 0;
}

// 写入引导配置
static int np_boot_config_write(boot_config_t *config)
{
    if (flash_page_erase(BOOT_CONFIG_ADDR) < 0)
        return -1;

    if (flash_write(BOOT_CONFIG_ADDR, (uint8_t *)config, sizeof(boot_config_t))
        < 0)
    {
        return -1;
    }

    return 0;
}

// 获取活动镜像命令
static int np_cmd_active_image_get(np_prog_t *prog)
{
    boot_config_t boot_config;
    np_resp_active_image_t resp;
    size_t resp_len = sizeof(resp);

    DEBUG_PRINT("获取活动镜像命令\r\n");

    if (prog->active_image == 0xff)
    {
        if (np_boot_config_read(&boot_config))
            return NP_ERR_INTERNAL;
        prog->active_image = boot_config.active_image;
    }

    resp.header.code = NP_RESP_DATA;
    resp.header.info = resp_len - sizeof(resp.header);
    resp.active_image = prog->active_image;

    if (np_comm_cb)
        np_comm_cb->send((uint8_t *)&resp, resp_len);

    return 0;
}

// 开始固件更新命令
static int np_cmd_fw_update_start(np_prog_t *prog)
{
    uint64_t addr, len;
    np_write_start_cmd_t *write_start_cmd;

    if (prog->rx_buf_len < sizeof(np_write_start_cmd_t))
    {
        ERROR_PRINT("写入开始命令的缓冲区长度错误 %lu\r\n",
            prog->rx_buf_len);
        return NP_ERR_LEN_INVALID;
    }

    write_start_cmd = (np_write_start_cmd_t *)prog->rx_buf;
    addr = write_start_cmd->addr;
    len = write_start_cmd->len;

    DEBUG_PRINT("写入命令 0x%" PRIx64 " 地址 0x%" PRIx64 " 字节\r\n", addr,
        len);

    prog->base_addr = FLASH_START_ADDR;
    prog->page_size = FLASH_PAGE_SIZE;
    prog->block_size = FLASH_BLOCK_SIZE;
    prog->total_size = FLASH_SIZE;

    if (addr + len > prog->base_addr + prog->total_size)
    {
        ERROR_PRINT("写入地址 0x%" PRIx64 "+0x%" PRIx64
            " 超过闪存大小 0x%" PRIx64 "\r\n", addr, len,
            prog->base_addr + prog->total_size);
        return NP_ERR_ADDR_EXCEEDED;
    }

    if (addr % prog->page_size)
    {
        ERROR_PRINT("地址 0x%" PRIx64
            " 未对齐到页大小 0x%lx\r\n", addr, prog->page_size);
        return NP_ERR_ADDR_NOT_ALIGN;
    }

    if (!len)
    {
        ERROR_PRINT("长度为0\r\n");
        return NP_ERR_LEN_INVALID;
    }

    if (len % prog->page_size)
    {
        ERROR_PRINT("长度 0x%" PRIx64
            " 未对齐到页大小 0x%lx\r\n", len, prog->page_size);
        return NP_ERR_LEN_NOT_ALIGN;
    }

    prog->addr = addr;
    prog->len = len;
    prog->addr_is_set = 1;

    prog->page.page = addr / prog->page_size;
    prog->page.offset = 0;

    prog->bytes_written = 0;
    prog->bytes_ack = 0;

    return np_send_ok_status();
}

// 写入固件数据命令
static int np_cmd_fw_update_data(np_prog_t *prog)
{
    uint32_t write_len;
    uint64_t bytes_left, len;
    np_write_data_cmd_t *write_data_cmd;

    if (prog->rx_buf_len < sizeof(np_write_data_cmd_t))
    {
        ERROR_PRINT("写入数据命令的缓冲区长度错误 %lu\r\n",
            prog->rx_buf_len);
        return NP_ERR_LEN_INVALID;
    }

    write_data_cmd = (np_write_data_cmd_t *)prog->rx_buf;
    len = write_data_cmd->len;
    if (len + sizeof(np_write_data_cmd_t) > NP_PACKET_BUF_SIZE)
    {
        ERROR_PRINT("数据大小错误 0x%" PRIx64 "\r\n", len);
        return NP_ERR_CMD_DATA_SIZE;
    }

    if (len + sizeof(np_write_data_cmd_t) != prog->rx_buf_len)
    {
        ERROR_PRINT("缓冲区长度 0x%lx 大于命令长度 0x%" PRIx64 "\r\n",
            prog->rx_buf_len, len + sizeof(np_write_data_cmd_t));
        return NP_ERR_CMD_DATA_SIZE;
    }

    if (!prog->addr_is_set)
    {
        ERROR_PRINT("写入地址未设置\r\n");
        return NP_ERR_ADDR_INVALID;
    }

    if (prog->page.offset + len > prog->page_size)
        write_len = prog->page_size - prog->page.offset;
    else
        write_len = len;

    memcpy(prog->page.buf + prog->page.offset, write_data_cmd->data, write_len);
    prog->page.offset += write_len;

    if (prog->page.offset == prog->page_size)
    {
        if (prog->addr >= prog->base_addr + prog->total_size)
        {
            ERROR_PRINT("写入地址 0x%" PRIx64
                " 超过闪存大小 0x%" PRIx64 "\r\n",
                prog->addr, prog->base_addr + prog->total_size);
            return NP_ERR_ADDR_EXCEEDED;
        }

        if (flash_page_erase((uint32_t)prog->addr) < 0)
            return NP_ERR_INTERNAL;

        if (flash_write((uint32_t)prog->addr, prog->page.buf,
            prog->page_size) < 0)
        {
            return NP_ERR_INTERNAL;
        }

        prog->addr += prog->page_size;
        prog->page.page++;
        prog->page.offset = 0;
    }

    bytes_left = len - write_len;
    if (bytes_left)
    {
        memcpy(prog->page.buf, write_data_cmd->data + write_len, bytes_left);
        prog->page.offset += bytes_left;
    }

    prog->bytes_written += len;
    if (prog->bytes_written - prog->bytes_ack >= prog->page_size ||
        prog->bytes_written == prog->len)
    {
        if (np_send_write_ack(prog->bytes_written))
            return -1;
        prog->bytes_ack = prog->bytes_written;
    }

    if (prog->bytes_written > prog->len)
    {
        ERROR_PRINT("实际写入数据长度 0x%" PRIx64
            " 超过 0x%" PRIx64 "\r\n", prog->bytes_written, prog->len);
        return NP_ERR_LEN_EXCEEDED;
    }

    return 0;
}


// 固件更新结束命令
static int np_cmd_fw_update_end(np_prog_t *prog)
{
    boot_config_t boot_config;

    prog->addr_is_set = 0;

    if (prog->page.offset)
    {
        ERROR_PRINT("未写入长度为 0x%lx 的数据\r\n",
            prog->page.offset);
        return NP_ERR_NAND_WR;
    }

    if (np_boot_config_read(&boot_config))
        return NP_ERR_INTERNAL;

    if (prog->active_image == 0xff)
        prog->active_image = boot_config.active_image;
    boot_config.active_image = prog->active_image ? 0 : 1;
    if (np_boot_config_write(&boot_config))
        return NP_ERR_INTERNAL;

    return np_send_ok_status();
}

// 固件更新命令
static int np_cmd_fw_update(np_prog_t *prog)
{
    np_cmd_t *cmd = (np_cmd_t *)prog->rx_buf;
    int ret = 0;

    switch (cmd->code)
    {
    case NP_CMD_FW_UPDATE_S:
        led_wr_set(true);
        ret = np_cmd_fw_update_start(prog);
        break;
    case NP_CMD_FW_UPDATE_D:
        ret = np_cmd_fw_update_data(prog);
        break;
    case NP_CMD_FW_UPDATE_E:
        ret = np_cmd_fw_update_end(prog);
        led_wr_set(false);
        break;
    default:
        break;
    }

    if (ret < 0)
        led_wr_set(false);

    return ret;
}

// 命令处理函数的定义
static np_cmd_handler_t cmd_handler[] =
{
    { NP_CMD_NAND_READ_ID, 1, np_cmd_nand_read_id },             // NAND 读取 ID 命令
    { NP_CMD_NAND_ERASE, 1, np_cmd_nand_erase },                 // NAND 擦除命令
    { NP_CMD_NAND_READ, 1, np_cmd_nand_read },                   // NAND 读取命令
    { NP_CMD_NAND_WRITE_S, 1, np_cmd_nand_write },               // NAND 写入起始命令
    { NP_CMD_NAND_WRITE_D, 1, np_cmd_nand_write },               // NAND 写入数据命令
    { NP_CMD_NAND_WRITE_E, 1, np_cmd_nand_write },               // NAND 写入结束命令
    { NP_CMD_NAND_CONF, 0, np_cmd_nand_conf },                   // NAND 配置命令
    { NP_CMD_NAND_READ_BB, 1, np_cmd_read_bad_blocks },          // 读取坏块表命令
    { NP_CMD_VERSION_GET, 0, np_cmd_version_get },               // 获取版本命令
    { NP_CMD_ACTIVE_IMAGE_GET, 0, np_cmd_active_image_get },     // 获取激活镜像命令
    { NP_CMD_FW_UPDATE_S, 0, np_cmd_fw_update },                 // 固件更新起始命令
    { NP_CMD_FW_UPDATE_D, 0, np_cmd_fw_update },                 // 固件更新数据命令
    { NP_CMD_FW_UPDATE_E, 0, np_cmd_fw_update },                 // 固件更新结束命令
};

// 检查命令是否有效
static bool np_cmd_is_valid(np_cmd_code_t code)
{
    return code >= 0 && code < NP_CMD_NAND_LAST;
}

// 命令处理函数
static int np_cmd_handler(np_prog_t *prog)
{
    np_cmd_t *cmd;

    if (prog->rx_buf_len < sizeof(np_cmd_t))
    {
        ERROR_PRINT("命令长度错误：%lu\r\n",
            prog->rx_buf_len);
        return NP_ERR_LEN_INVALID;
    }
    cmd = (np_cmd_t *)prog->rx_buf;

    if (!np_cmd_is_valid(cmd->code))
    {
        ERROR_PRINT("无效的命令码：%d\r\n", cmd->code);
        return NP_ERR_CMD_INVALID;
    }

    if (!prog->chip_is_conf && cmd_handler[cmd->code].is_chip_cmd)
    {
        ERROR_PRINT("芯片未配置\r\n");
        return NP_ERR_CHIP_NOT_CONF;
    }

    return cmd_handler[cmd->code].exec(prog);
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

        ret = np_cmd_handler(prog);

        np_comm_cb->consume();

        if (ret < 0)
            np_send_error(-ret);
    }
    while (1);
}

// NAND 处理函数
static void np_nand_handler(np_prog_t *prog)
{
    if (prog->nand_wr_in_progress)
    {
        if (np_nand_handle_status(prog))
            np_send_error(NP_ERR_NAND_WR);
    }
}

// NP 初始化函数
void np_init()
{
    prog.active_image = 0xff;
}

// NP 处理函数
void np_handler()
{
    np_packet_handler(&prog);
    np_nand_handler(&prog);
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

