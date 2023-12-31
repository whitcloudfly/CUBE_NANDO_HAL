/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "stm32f4xx.h"
#include "flash.h"
#include "stm32f4xx_hal.h"
#include <string.h>

// 擦除指定页的Flash存储器
int flash_page_erase(uint32_t page_addr)
{
    HAL_StatusTypeDef status;
    FLASH_EraseInitTypeDef erase_init;

    // 解锁Flash存储器
    HAL_FLASH_Unlock();

    // 配置擦除初始化结构体
    erase_init.TypeErase = FLASH_TYPEERASE_SECTORS;
    erase_init.Sector = FLASH_SECTOR_0; // 选择要擦除的扇区，可以根据您的需求更改
    erase_init.NbSectors = 1; // 要擦除的扇区数量
    erase_init.VoltageRange = FLASH_VOLTAGE_RANGE_3; // 选择电压范围，可以根据您的需求更改

    status = HAL_FLASHEx_Erase(&erase_init, NULL); // 擦除指定页

    // 锁定Flash存储器
    HAL_FLASH_Lock();

    return status != HAL_OK ? -1 : 0;
}

// 向Flash存储器写入数据
int flash_write(uint32_t addr, uint8_t *data, uint32_t data_len)
{
    int ret = -1;
    uint32_t data_word;
    uint32_t i;

    // 解锁Flash存储器
    HAL_FLASH_Unlock();

    for (i = 0; i < data_len; i += 4)
    {
        // 从数据缓冲区中读取32位字
        memcpy(&data_word, &data[i], 4);

        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, data_word) != HAL_OK)
            goto Exit;

        addr += 4; // 更新地址，指向下一个32位字
    }

    ret = data_len;
Exit:
    // 锁定Flash存储器
    HAL_FLASH_Lock();

    return ret;
}

// 从Flash存储器读取数据
int flash_read(uint32_t addr, uint8_t *data, uint32_t data_len)
{
    uint32_t i;

    for (i = 0; i < data_len; i++)
    {
        data[i] = *(__IO uint8_t *)(addr + i); // 从Flash存储器中读取字节数据
    }

    return i;
}
