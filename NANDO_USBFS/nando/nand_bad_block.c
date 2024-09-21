/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "nand_bad_block.h"
#include <string.h>

#define NAND_BAD_BLOCK_TABLE_SIZE 20

static uint32_t nand_bad_block_table_count; // NAND 坏块表中的块计数
static uint32_t nand_bad_block_table[NAND_BAD_BLOCK_TABLE_SIZE]; // NAND 坏块表

void nand_bad_block_table_init()
{
    memset(nand_bad_block_table, 0, sizeof(nand_bad_block_table)); // 初始化 NAND 坏块表为 0
    nand_bad_block_table_count = 0; // 初始化 NAND 坏块计数为 0
}

int nand_bad_block_table_add(uint32_t page)
{
    if (nand_bad_block_table_count == NAND_BAD_BLOCK_TABLE_SIZE)
        return -1; // 坏块表已满，无法添加

    nand_bad_block_table[nand_bad_block_table_count++] = page; // 在坏块表末尾添加新的块
    return 0; // 添加成功
}

bool nand_bad_block_table_lookup(uint32_t page)
{
    uint32_t i;

    for (i = 0; i < nand_bad_block_table_count; i++)
    {
        if (nand_bad_block_table[i] == page)
            return true; // 在坏块表中找到了对应的块
    }

    return false; // 在坏块表中未找到对应的块
}

void *nand_bad_block_table_iter_alloc(uint32_t *page)
{
    if (!nand_bad_block_table_count)
        return NULL; // 坏块表为空，无法进行迭代

    *page = nand_bad_block_table[0]; // 将坏块表的第一个块赋值给 page

    return &nand_bad_block_table[0]; // 返回坏块表的第一个块的地址作为迭代器
}

void *nand_bad_block_table_iter_next(void *iter, uint32_t *page)
{
    uint32_t *bbt_iter = iter;

    if (!bbt_iter)
       return NULL; // 无效的迭代器，无法进行下一次迭代

    bbt_iter++; // 迭代器指向下一个块

    if (bbt_iter - &nand_bad_block_table[0] >= nand_bad_block_table_count)
        return NULL; // 已达到坏块表的末尾，无法进行下一次迭代

    *page = *bbt_iter; // 将当前迭代的块赋值给 page

    return bbt_iter; // 返回下一次迭代的迭代器
}
