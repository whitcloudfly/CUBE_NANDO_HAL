/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : FSMC_NAND.c
  * Description        : This file provides code for the configuration
  *                      of the FSMC peripheral.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#include "fsmc_nand.h"
#include "fsmc.h"
#include "log.h"
#include <stm32f1xx.h>


#define CMD_AREA                   (uint32_t)(1<<16)  /* A16 = CLE  high */
#define ADDR_AREA                  (uint32_t)(1<<17)  /* A17 = ALE high */

#define DATA_AREA                  ((uint32_t)0x00000000) 

/* NAND memory status */  
#define NAND_ERROR                 ((uint32_t)0x00000001)
#define NAND_READY                 ((uint32_t)0x00000040)

/* FSMC NAND memory address computation */
/* 1st addressing cycle */
#define ADDR_1st_CYCLE(ADDR)       (uint8_t)((ADDR)& 0xFF)
/* 2nd addressing cycle */
#define ADDR_2nd_CYCLE(ADDR)       (uint8_t)(((ADDR)& 0xFF00) >> 8)
/* 3rd addressing cycle */
#define ADDR_3rd_CYCLE(ADDR)       (uint8_t)(((ADDR)& 0xFF0000) >> 16)
/* 4th addressing cycle */
#define ADDR_4th_CYCLE(ADDR)       (uint8_t)(((ADDR)& 0xFF000000) >> 24)

#define FSMC_Bank_NAND     FSMC_Bank2_3
#define Bank_NAND_ADDR     Bank2_NAND_ADDR 
#define Bank2_NAND_ADDR    ((uint32_t)0x70000000)     
#define ROW_ADDRESS (addr.page + (addr.block + (addr.zone * NAND_ZONE_SIZE)) * \
    NAND_BLOCK_SIZE)

#define UNDEFINED_CMD 0xFF

extern NAND_HandleTypeDef hnand1;

typedef struct __attribute__((__packed__))
{
    uint8_t setup_time;         // FSMC NAND Flash的时序配置参数
    uint8_t wait_setup_time;
    uint8_t hold_setup_time;
    uint8_t hi_z_setup_time;
    uint8_t clr_setup_time;
    uint8_t ar_setup_time;
    uint8_t row_cycles;         // 地址周期数
    uint8_t col_cycles;
    uint8_t read1_cmd;          // 读命令
    uint8_t read2_cmd;
    uint8_t read_spare_cmd;
    uint8_t read_id_cmd;
    uint8_t reset_cmd;          // 复位命令
    uint8_t write1_cmd;         // 写命令
    uint8_t write2_cmd;
    uint8_t erase1_cmd;         // 擦除命令
    uint8_t erase2_cmd;
    uint8_t status_cmd;         // 状态命令
    uint8_t set_features_cmd;
    uint8_t enable_ecc_addr;    // 使能ECC校验的地址
    uint8_t enable_ecc_value;   // 使能ECC校验的值
    uint8_t disable_ecc_value;  // 禁止ECC校验的值
} fsmc_conf_t;

static fsmc_conf_t fsmc_conf;   // FSMC NAND Flash的配置结构体

static uint32_t FSMC_Initialized = 0;

static void nand_gpio_init(void)
{
	  /* USER CODE BEGIN FSMC_MspInit 0 */

	  /* USER CODE END FSMC_MspInit 0 */
	  GPIO_InitTypeDef GPIO_InitStruct = {0};
	  if (FSMC_Initialized) {
	    return;
	  }
	  FSMC_Initialized = 1;

	  /* Peripheral clock enable */
	  __HAL_RCC_FSMC_CLK_ENABLE();

	  /** FSMC GPIO Configuration
	  PE7   ------> FSMC_D4
	  PE8   ------> FSMC_D5
	  PE9   ------> FSMC_D6
	  PE10   ------> FSMC_D7
	  PE11   ------> FSMC_D8
	  PE12   ------> FSMC_D9
	  PE13   ------> FSMC_D10
	  PE14   ------> FSMC_D11
	  PE15   ------> FSMC_D12
	  PD8   ------> FSMC_D13
	  PD9   ------> FSMC_D14
	  PD10   ------> FSMC_D15
	  PD11   ------> FSMC_CLE
	  PD12   ------> FSMC_ALE
	  PD14   ------> FSMC_D0
	  PD15   ------> FSMC_D1
	  PD0   ------> FSMC_D2
	  PD1   ------> FSMC_D3
	  PD4   ------> FSMC_NOE
	  PD5   ------> FSMC_NWE
	  PD6   ------> FSMC_NWAIT
	  PD7   ------> FSMC_NCE2
	  */
	  /* GPIO_InitStruct */
	  GPIO_InitStruct.Pin = GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10
	                          |GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14
	                          |GPIO_PIN_15;
	  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

	  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	  /* GPIO_InitStruct */
	  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11
	                          |GPIO_PIN_12|GPIO_PIN_14|GPIO_PIN_15|GPIO_PIN_0
	                          |GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_7;
	  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

	  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

	  /* GPIO_InitStruct */
	  GPIO_InitStruct.Pin = GPIO_PIN_6;
	  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;

	  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

	  /* Peripheral interrupt init */
	  HAL_NVIC_SetPriority(FSMC_IRQn, 0, 0);
	  HAL_NVIC_EnableIRQ(FSMC_IRQn);
	  /* USER CODE BEGIN FSMC_MspInit 1 */

	  /* USER CODE END FSMC_MspInit 1 */
}


static void nand_fsmc_init()
{
//    FSMC_NAND_InitTypeDef fsmc_init;

//    FSMC_NAND_PCC_TimingTypeDef timing_init;
    FSMC_NAND_PCC_TimingTypeDef ComSpaceTiming = {0};
    FSMC_NAND_PCC_TimingTypeDef AttSpaceTiming = {0};

//    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);
//    __HAL_RCC_FSMC_CLK_ENABLE();

    hnand1.Instance = FSMC_NAND_DEVICE;
//    fsmc_init.FSMC_Bank = FSMC_Bank2_NAND;  // 设置FSMC NAND Flash的相关参数
    hnand1.Init.NandBank = FSMC_NAND_BANK2;
//    fsmc_init.FSMC_Waitfeature = FSMC_Waitfeature_Enable;
    hnand1.Init.Waitfeature = FSMC_NAND_PCC_WAIT_FEATURE_ENABLE;
//    fsmc_init.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_8b;
    hnand1.Init.MemoryDataWidth = FSMC_NAND_PCC_MEM_BUS_WIDTH_8;
//    fsmc_init.FSMC_ECC = FSMC_ECC_Enable;
    hnand1.Init.EccComputation = FSMC_NAND_ECC_DISABLE;
//    fsmc_init.FSMC_ECCPageSize = FSMC_ECCPageSize_2048Bytes;
    hnand1.Init.ECCPageSize = FSMC_NAND_ECC_PAGE_SIZE_256BYTE;
//    fsmc_init.FSMC_TCLRSetupTime = fsmc_conf.clr_setup_time;
    hnand1.Init.TCLRSetupTime = fsmc_conf.clr_setup_time;
//    fsmc_init.FSMC_TARSetupTime = fsmc_conf.ar_setup_time;
    hnand1.Init.TARSetupTime = fsmc_conf.ar_setup_time;
    /* ComSpaceTiming */
//    timing_init.FSMC_SetupTime = fsmc_conf.setup_time;  // 设置时序参数
    ComSpaceTiming.SetupTime = fsmc_conf.setup_time;  // 设置时序参数
//    timing_init.FSMC_WaitSetupTime = fsmc_conf.wait_setup_time;
    ComSpaceTiming.WaitSetupTime = fsmc_conf.wait_setup_time;
//    timing_init.FSMC_HoldSetupTime = fsmc_conf.hold_setup_time;
    ComSpaceTiming.HoldSetupTime = fsmc_conf.hold_setup_time;
//    timing_init.FSMC_HiZSetupTime = fsmc_conf.hi_z_setup_time;
    ComSpaceTiming.HiZSetupTime = fsmc_conf.hi_z_setup_time;
    /* AttSpaceTiming */
    AttSpaceTiming.SetupTime = fsmc_conf.setup_time;
    AttSpaceTiming.WaitSetupTime = fsmc_conf.wait_setup_time;
    AttSpaceTiming.HoldSetupTime = fsmc_conf.hold_setup_time;
    AttSpaceTiming.HiZSetupTime = fsmc_conf.hi_z_setup_time;

    if (HAL_NAND_Init(&hnand1, &ComSpaceTiming, &AttSpaceTiming) != HAL_OK)
    {
      Error_Handler( );
    }  // 初始化FSMC NAND Flash

//    FSMC_NANDCmd(FSMC_Bank_NAND, ENABLE);  // 使能FSMC NAND Flash
    __HAL_RCC_FSMC_CLK_ENABLE();
}

static void nand_print_fsmc_info()
{
    DEBUG_PRINT("Setup time: %d\r\n", fsmc_conf.setup_time);
    DEBUG_PRINT("Wait setup time: %d\r\n", fsmc_conf.wait_setup_time);
    DEBUG_PRINT("Hold setup time: %d\r\n", fsmc_conf.hold_setup_time);
    DEBUG_PRINT("HiZ setup time: %d\r\n", fsmc_conf.hi_z_setup_time);
    DEBUG_PRINT("CLR setup time: %d\r\n", fsmc_conf.clr_setup_time);
    DEBUG_PRINT("AR setup time: %d\r\n", fsmc_conf.ar_setup_time);
    DEBUG_PRINT("Row cycles: %d\r\n", fsmc_conf.row_cycles);
    DEBUG_PRINT("Col. cycles: %d\r\n", fsmc_conf.col_cycles);
    DEBUG_PRINT("Read command 1: %d\r\n", fsmc_conf.read1_cmd);
    DEBUG_PRINT("Read command 2: %d\r\n", fsmc_conf.read2_cmd);
    DEBUG_PRINT("Read spare command: %d\r\n", fsmc_conf.read_spare_cmd);    
    DEBUG_PRINT("Read ID command: %d\r\n", fsmc_conf.read_id_cmd);
    DEBUG_PRINT("Reset command: %d\r\n", fsmc_conf.reset_cmd);
    DEBUG_PRINT("Write 1 command: %d\r\n", fsmc_conf.write1_cmd);
    DEBUG_PRINT("Write 2 command: %d\r\n", fsmc_conf.write2_cmd);
    DEBUG_PRINT("Erase 1 command: %d\r\n", fsmc_conf.erase1_cmd);
    DEBUG_PRINT("Erase 2 command: %d\r\n", fsmc_conf.erase2_cmd);
    DEBUG_PRINT("Status command: %d\r\n", fsmc_conf.status_cmd);
    DEBUG_PRINT("Set feature command: %d\r\n", fsmc_conf.set_features_cmd);
    DEBUG_PRINT("Enable ECC address: %d\r\n", fsmc_conf.enable_ecc_addr);
    DEBUG_PRINT("Enable ECC value: %d\r\n", fsmc_conf.enable_ecc_value);
    DEBUG_PRINT("Disable ECC value: %d\r\n", fsmc_conf.disable_ecc_value);
}

static void nand_reset()
{
    *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = fsmc_conf.reset_cmd;  // 发送复位命令给NAND Flash
}

static int nand_init(void *conf, uint32_t conf_size)
{
    if (conf_size < sizeof(fsmc_conf_t))
        return -1;
   
    fsmc_conf = *(fsmc_conf_t *)conf;  // 从传入的配置结构体中获取配置参数

    nand_gpio_init();  // 初始化GPIO引脚
    nand_fsmc_init();  // 初始化FSMC NAND Flash
    nand_print_fsmc_info();  // 打印FSMC的配置信息
    nand_reset();  // 复位NAND Flash

    return 0;
}

static void nand_uninit()
{
    //TODO
}

static uint32_t nand_read_status()
{
    uint32_t data, status;

    *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = fsmc_conf.status_cmd;  // 发送状态命令给NAND Flash
    data = *(__IO uint8_t *)(Bank_NAND_ADDR);

    if ((data & NAND_ERROR) == NAND_ERROR)
        status = FLASH_STATUS_ERROR;
    else if ((data & NAND_READY) == NAND_READY)
        status = FLASH_STATUS_READY;
    else
        status = FLASH_STATUS_BUSY;

    return status;  // 返回NAND Flash的状态
}

static uint32_t nand_get_status()
{
    uint32_t status, timeout = 0x1000000;

    status = nand_read_status();

    /* 等待NAND操作完成或超时发生 */
    while (status == FLASH_STATUS_BUSY && timeout)
    {
        status = nand_read_status();
        timeout --;
    }

    if (!timeout)
        status = FLASH_STATUS_TIMEOUT;

    return status;  // 返回NAND Flash的最终状态
}

// 从NAND Flash读取ID
static void nand_read_id(chip_id_t *nand_id)
{
    uint32_t data = 0;

    *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = fsmc_conf.read_id_cmd;  // 发送读取ID的命令给NAND Flash
    *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = 0x00;

    /* 从NAND Flash读取ID的序列 */
    data = *(__IO uint32_t *)(Bank_NAND_ADDR | DATA_AREA);
    nand_id->maker_id   = ADDR_1st_CYCLE(data);  // 读取制造商ID
    nand_id->device_id  = ADDR_2nd_CYCLE(data);  // 读取设备ID
    nand_id->third_id   = ADDR_3rd_CYCLE(data);  // 读取第三个ID
    nand_id->fourth_id  = ADDR_4th_CYCLE(data);  // 读取第四个ID

    data = *((__IO uint32_t *)(Bank_NAND_ADDR | DATA_AREA) + 1);
    nand_id->fifth_id   = ADDR_1st_CYCLE(data);  // 读取第五个ID
    nand_id->sixth_id   = ADDR_2nd_CYCLE(data);  // 读取第六个ID
}

// 异步方式写入NAND Flash的页数据
static void nand_write_page_async(uint8_t *buf, uint32_t page, uint32_t page_size)
{
    uint32_t i;

    *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = fsmc_conf.write1_cmd;  // 发送写入命令给NAND Flash

    switch (fsmc_conf.col_cycles)
    {
    case 1:
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = 0x00;
        break;
    case 2:
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = 0x00;
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = 0x00;
        break;
    case 3:
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = 0x00;
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = 0x00;
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = 0x00;
        break;
    case 4:
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = 0x00;
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = 0x00;
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = 0x00;
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = 0x00;
        break;
    default:
        break;
    }

    switch (fsmc_conf.row_cycles)
    {
    case 1:
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(page);
        break;
    case 2:
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(page);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_2nd_CYCLE(page);
        break;
    case 3:
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(page);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_2nd_CYCLE(page);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_3rd_CYCLE(page);
        break;
    case 4:
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(page);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_2nd_CYCLE(page);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_3rd_CYCLE(page);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_4th_CYCLE(page);
        break;
    default:
        break;
    }

    for(i = 0; i < page_size; i++)
        *(__IO uint8_t *)(Bank_NAND_ADDR | DATA_AREA) = buf[i];  // 逐字节写入数据到NAND Flash的数据区

    if (fsmc_conf.write2_cmd != UNDEFINED_CMD)
        *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = fsmc_conf.write2_cmd;  // 发送写入命令2给NAND Flash
}

// 从NAND Flash读取数据
static uint32_t nand_read_data(uint8_t *buf, uint32_t page, uint32_t page_offset, uint32_t data_size)
{
    uint32_t i;

    *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = fsmc_conf.read1_cmd;  // 发送读取命令给NAND Flash

    switch (fsmc_conf.col_cycles)
    {
    case 1:
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(page_offset);
        break;
    case 2:
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(page_offset);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_2nd_CYCLE(page_offset);
        break;
    case 3:
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(page_offset);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_2nd_CYCLE(page_offset);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_3rd_CYCLE(page_offset);
        break;
    case 4:
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(page_offset);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_2nd_CYCLE(page_offset);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_3rd_CYCLE(page_offset);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_4th_CYCLE(page_offset);
    default:
        break;
    }

    switch (fsmc_conf.row_cycles)
    {
    case 1:
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(page);
        break;
    case 2:
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(page);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_2nd_CYCLE(page);
        break;
    case 3:
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(page);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_2nd_CYCLE(page);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_3rd_CYCLE(page);
        break;
    case 4:
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(page);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_2nd_CYCLE(page);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_3rd_CYCLE(page);
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_4th_CYCLE(page);
        break;
    default:
        break;
    }

    if (fsmc_conf.read2_cmd != UNDEFINED_CMD)
        *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = fsmc_conf.read2_cmd;  // 发送读取命令2给NAND Flash

    for (i = 0; i < data_size; i++)
        buf[i] = *(__IO uint8_t *)(Bank_NAND_ADDR | DATA_AREA);  // 逐字节从NAND Flash的数据区读取数据

    return nand_get_status();  // 获取NAND Flash的状态
}

// 从NAND Flash读取数据页
static uint32_t nand_read_page(uint8_t *buf, uint32_t page, uint32_t page_size)
{
    return nand_read_data(buf, page, 0, page_size);
}

// 从NAND Flash读取备用数据
static uint32_t nand_read_spare_data(uint8_t *buf, uint32_t page, uint32_t offset, uint32_t data_size)
{
    uint32_t i;

    if (fsmc_conf.read_spare_cmd == UNDEFINED_CMD)
        return FLASH_STATUS_INVALID_CMD;

    *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = fsmc_conf.read_spare_cmd;  // 发送读取备用数据命令给NAND Flash

    switch (fsmc_conf.col_cycles)
    {
    case 1:
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(offset);  // 发送列地址的第1个周期
        break;
    case 2:
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(offset);  // 发送列地址的第1个周期
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_2nd_CYCLE(offset);  // 发送列地址的第2个周期
        break;
    case 3:
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(offset);  // 发送列地址的第1个周期
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_2nd_CYCLE(offset);  // 发送列地址的第2个周期
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_3rd_CYCLE(offset);  // 发送列地址的第3个周期
        break;
    case 4:
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(offset);  // 发送列地址的第1个周期
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_2nd_CYCLE(offset);  // 发送列地址的第2个周期
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_3rd_CYCLE(offset);  // 发送列地址的第3个周期
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_4th_CYCLE(offset);  // 发送列地址的第4个周期
    default:
        break;
    }

    switch (fsmc_conf.row_cycles)
    {
    case 1:
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(page);  // 发送行地址的第1个周期
        break;
    case 2:
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(page);  // 发送行地址的第1个周期
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_2nd_CYCLE(page);  // 发送行地址的第2个周期
        break;
    case 3:
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(page);  // 发送行地址的第1个周期
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_2nd_CYCLE(page);  // 发送行地址的第2个周期
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_3rd_CYCLE(page);  // 发送行地址的第3个周期
        break;
    case 4:
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(page);  // 发送行地址的第1个周期
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_2nd_CYCLE(page);  // 发送行地址的第2个周期
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_3rd_CYCLE(page);  // 发送行地址的第3个周期
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_4th_CYCLE(page);  // 发送行地址的第4个周期
        break;
    default:
        break;
    }

    for (i = 0; i < data_size; i++)
        buf[i] = *(__IO uint8_t *)(Bank_NAND_ADDR | DATA_AREA);  // 逐字节从NAND Flash的数据区读取数据

    return nand_get_status();  // 获取NAND Flash的状态
}

// 擦除NAND Flash的块
static uint32_t nand_erase_block(uint32_t page)
{
    *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = fsmc_conf.erase1_cmd;  // 发送擦除命令1给NAND Flash

    switch (fsmc_conf.row_cycles)
    {
    case 1:
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(page);  // 发送行地址的第1个周期
        break;
    case 2:
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(page);  // 发送行地址的第1个周期
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_2nd_CYCLE(page);  // 发送行地址的第2个周期
        break;
    case 3:
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(page);  // 发送行地址的第1个周期
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_2nd_CYCLE(page);  // 发送行地址的第2个周期
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_3rd_CYCLE(page);  // 发送行地址的第3个周期
        break;
    case 4:
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(page);  // 发送行地址的第1个周期
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_2nd_CYCLE(page);  // 发送行地址的第2个周期
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_3rd_CYCLE(page);  // 发送行地址的第3个周期
        *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_4th_CYCLE(page);  // 发送行地址的第4个周期
        break;
    default:
        break;
    }

    if (fsmc_conf.erase2_cmd != UNDEFINED_CMD)
        *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = fsmc_conf.erase2_cmd;  // 发送擦除命令2给NAND Flash

    return nand_get_status();  // 获取NAND Flash的状态
}

// 检查是否支持坏块
static inline bool nand_is_bb_supported()
{
    return true;
}

// 启用或禁用硬件ECC
static uint32_t nand_enable_hw_ecc(bool enable)
{
    uint8_t enable_ecc;

    if (fsmc_conf.set_features_cmd == UNDEFINED_CMD)
        return FLASH_STATUS_INVALID_CMD;

    enable_ecc = enable ? fsmc_conf.enable_ecc_value : fsmc_conf.disable_ecc_value;

    *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = fsmc_conf.set_features_cmd;  // 发送设置特性命令给NAND Flash
    *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = fsmc_conf.enable_ecc_addr;  // 发送使能ECC地址
    *(__IO uint8_t *)(Bank_NAND_ADDR | DATA_AREA) = enable_ecc;  // 发送使能ECC的值
    *(__IO uint8_t *)(Bank_NAND_ADDR | DATA_AREA) = 0;
    *(__IO uint8_t *)(Bank_NAND_ADDR | DATA_AREA) = 0;
    *(__IO uint8_t *)(Bank_NAND_ADDR | DATA_AREA) = 0;

    return 0;
}

// FSMC NAND Flash HAL对象
flash_hal_t hal_fsmc =
{
    .init = nand_init,
    .uninit = nand_uninit,
    .read_id = nand_read_id,
    .erase_block = nand_erase_block,
    .read_page = nand_read_page,
    .read_spare_data = nand_read_spare_data,
    .write_page_async = nand_write_page_async,
    .read_status = nand_read_status,
    .is_bb_supported = nand_is_bb_supported,
    .enable_hw_ecc = nand_enable_hw_ecc,
};
