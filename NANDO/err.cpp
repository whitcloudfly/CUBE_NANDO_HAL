/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "err.h"

enum
{
    NP_ERR_INTERNAL       = -1,
    NP_ERR_ADDR_EXCEEDED  = -100,
    NP_ERR_ADDR_INVALID   = -101,
    NP_ERR_ADDR_NOT_ALIGN = -102,
    NP_ERR_NAND_WR        = -103,
    NP_ERR_NAND_RD        = -104,
    NP_ERR_NAND_ERASE     = -105,
    NP_ERR_CHIP_NOT_CONF  = -106,
    NP_ERR_CMD_DATA_SIZE  = -107,
    NP_ERR_CMD_INVALID    = -108,
    NP_ERR_BUF_OVERFLOW   = -109,
    NP_ERR_LEN_NOT_ALIGN  = -110,
    NP_ERR_LEN_EXCEEDED   = -111,
    NP_ERR_LEN_INVALID    = -112,
    NP_ERR_BBT_OVERFLOW   = -113,
};

typedef struct
{
    long int code;
    const char *str;
} code_str_t;

static code_str_t err[] =
{
    { NP_ERR_INTERNAL, "内部错误" },
    { NP_ERR_ADDR_EXCEEDED, "操作地址超出芯片大小" },
    { NP_ERR_ADDR_INVALID, "操作地址无效" },
    { NP_ERR_ADDR_NOT_ALIGN,
        "操作地址未与页面/块大小对齐" },
    { NP_ERR_NAND_WR, "写入芯片失败" },
    { NP_ERR_NAND_RD, "读取芯片失败" },
    { NP_ERR_NAND_ERASE, "无法擦除芯片" },
    { NP_ERR_CHIP_NOT_CONF,
        "编程器未配置芯片参数" },
    { NP_ERR_CMD_DATA_SIZE, "命令中的数据大小错误" },
    { NP_ERR_CMD_INVALID, "缓冲区溢出无效命令" },
    { NP_ERR_BUF_OVERFLOW, "缓冲区溢出" },
    { NP_ERR_LEN_NOT_ALIGN, "数据长度未与页面对齐" },
    { NP_ERR_LEN_EXCEEDED, "数据长度超过芯片大小" },
    { NP_ERR_LEN_INVALID, "数据长度错误" },
    { NP_ERR_BBT_OVERFLOW, "坏块表溢出。可能芯片的一些引脚没有连接到编程器" },
};

const char *errCode2str(long int code)
{
    for (unsigned int i = 0; i < sizeof(err) / sizeof (err[0]); i++)
    {
        if (err[i].code == code)
            return err[i].str;
    }

    return "未知错误";
}
