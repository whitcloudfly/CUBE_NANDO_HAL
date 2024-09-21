/*********************************************************************************************************
*
* File                : spi_nor.c
* Hardware Environment:
* Build Environment   : STM32CUBEIDE  Version: 1.13.2
* Version             : V1.0
* By                  :
*
*                                  (c) Copyright 2005-2011, WaveShare
*                                       http://www.waveshare.net
*                                          All Rights Reserved
*
*********************************************************************************************************/

#include "spi_nor.h"
#include "main.h"
#include "spi.h"
#include "gpio.h"
#include "log.h"
#include <stm32f4xx.h>
#include <stdio.h>
/**SPI1 GPIO Configuration
  PA6     ------> SPI1_MISO
  PA7     ------> SPI1_MOSI
  PB3     ------> SPI1_SCK
  PA15    ------> SPI1_CS
*/
#define SPI_FLASH_MISO_PIN  SPI1_MISO_Pin
#define SPI_FLASH_MOSI_PIN  SPI1_MOSI_Pin
#define SPI_FLASH_SCK_PIN   SPI1_SCK_Pin
#define SPI_FLASH_CS_PIN    SPI1_CS_Pin

#define FLASH_DUMMY_BYTE 0xA5

#define FLASH_READY 0
#define FLASH_BUSY  1
#define FLASH_TIMEOUT 2

#define SPI_TIMEOUT 0x1000000

/* 第一地址周期 */
#define ADDR_1st_CYCLE(ADDR) (uint8_t)((ADDR)& 0xFF)
/* 第二地址周期 */
#define ADDR_2nd_CYCLE(ADDR) (uint8_t)(((ADDR)& 0xFF00) >> 8)
/* 第三地址周期 */
#define ADDR_3rd_CYCLE(ADDR) (uint8_t)(((ADDR)& 0xFF0000) >> 16)
/* 第四地址周期 */
#define ADDR_4th_CYCLE(ADDR) (uint8_t)(((ADDR)& 0xFF000000) >> 24)

#define UNDEFINED_CMD 0xFF

typedef struct __attribute__((__packed__))
{
    uint8_t page_offset;    // 页偏移量
    uint8_t read_cmd;       // 读取指令
    uint8_t read_id_cmd;    // 读取ID指令
    uint8_t write_cmd;      // 写入指令
    uint8_t write_en_cmd;   // 写使能指令
    uint8_t erase_cmd;      // 擦除指令
    uint8_t status_cmd;     // 状态指令
    uint8_t busy_bit;       // 忙状态位
    uint8_t busy_state;     // 忙状态值
    uint32_t freq;          // SPI频率
} spi_conf_t;

static spi_conf_t spi_conf;
extern SPI_HandleTypeDef hspi1;

// 初始化SPI Flash的GPIO引脚
static void spi_flash_gpio_init()
{
	    GPIO_InitTypeDef GPIO_InitStruct = {0};

	    /* 使能SPI外设时钟 */
	    /* SPI1 clock enable */
	    __HAL_RCC_SPI1_CLK_ENABLE();

	    __HAL_RCC_GPIOA_CLK_ENABLE();
	    __HAL_RCC_GPIOB_CLK_ENABLE();
	    /*SPI1 GPIO Configuration*/
	    /* 配置SPI SCK引脚 */
	    GPIO_InitStruct.Pin = SPI_FLASH_SCK_PIN;
	    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
	    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	    /* 配置SPI MISO引脚 */
	    GPIO_InitStruct.Pin = SPI_FLASH_MOSI_PIN;
	    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	    GPIO_InitStruct.Pull = GPIO_NOPULL;
	    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
	    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	    /* 配置SPI MOSI引脚 */
	    GPIO_InitStruct.Pin = SPI_FLASH_MISO_PIN;
	    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	    GPIO_InitStruct.Pull = GPIO_NOPULL;
	    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
	    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	    /* 配置SPI CS引脚 */
	    GPIO_InitStruct.Pin = SPI_FLASH_CS_PIN;
	    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	    GPIO_InitStruct.Pull = GPIO_PULLUP;
	    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	    /* SPI1 interrupt Init */
	    HAL_NVIC_SetPriority(SPI1_IRQn, 0, 0);
	    HAL_NVIC_EnableIRQ(SPI1_IRQn);
}

// 取消初始化SPI Flash的GPIO引脚
static void spi_flash_gpio_uninit()
{
    __HAL_RCC_SPI1_CLK_DISABLE();

    /**SPI1 GPIO Configuration
    PA6     ------> SPI1_MISO
    PA7     ------> SPI1_MOSI
    PB3     ------> SPI1_SCK
    PA15    ------> SPI1_CS
    */
    HAL_GPIO_DeInit(GPIOA, SPI1_MISO_Pin|SPI1_MOSI_Pin);

    HAL_GPIO_DeInit(SPI1_SCK_GPIO_Port, SPI1_SCK_Pin);

    HAL_GPIO_DeInit(SPI1_CS_GPIO_Port, SPI_FLASH_CS_PIN);

    /* SPI1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(SPI1_IRQn);

}

static inline void spi_flash_select_chip()
{
   	HAL_GPIO_WritePin(GPIOA, SPI_FLASH_CS_PIN, GPIO_PIN_RESET);
}

static inline void spi_flash_deselect_chip()
{
  	HAL_GPIO_WritePin(GPIOA, SPI_FLASH_CS_PIN, GPIO_PIN_SET);
}

static uint16_t spi_flash_get_baud_rate_prescaler(uint32_t spi_freq_khz)
{
    uint32_t system_clock_khz = SystemCoreClock / 1000;

    if (spi_freq_khz >= system_clock_khz / 2)
         return SPI_BAUDRATEPRESCALER_2;
    else if (spi_freq_khz >= system_clock_khz / 4)
         return SPI_BAUDRATEPRESCALER_4;
    else if (spi_freq_khz >= system_clock_khz / 8)
         return SPI_BAUDRATEPRESCALER_8;
    else if (spi_freq_khz >= system_clock_khz / 16)
         return SPI_BAUDRATEPRESCALER_16;
    else if (spi_freq_khz >= system_clock_khz / 32)
         return SPI_BAUDRATEPRESCALER_32;
    else if (spi_freq_khz >= system_clock_khz / 64)
          return SPI_BAUDRATEPRESCALER_64;
    else if (spi_freq_khz >= system_clock_khz / 128)
          return SPI_BAUDRATEPRESCALER_128;
    else
          return SPI_BAUDRATEPRESCALER_256;
}

// 初始化SPI Flash
static int spi_flash_init(void *conf, uint32_t conf_size)
{
    if (conf_size < sizeof(spi_conf_t))
        return -1;
    spi_conf = *(spi_conf_t *)conf;

    spi_flash_gpio_init();

    spi_flash_deselect_chip();

    /* 配置SPI */
    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi1.Init.NSS = SPI_NSS_SOFT;
    hspi1.Init.BaudRatePrescaler =
    		spi_flash_get_baud_rate_prescaler(spi_conf.freq);
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi1.Init.CRCPolynomial = 10;
    if (HAL_SPI_Init(&hspi1) != HAL_OK)
    {
      Error_Handler();
    }

    /* Enable SPI */
    __HAL_SPI_ENABLE(&hspi1);

    FLASH_Enable4ByteAddr();

     return 0;
}

// 取消初始化SPI Flash
static void spi_flash_uninit()
{
    spi_flash_gpio_uninit(&hspi1);

    /* 禁用SPI */
    __HAL_SPI_DISABLE(&hspi1);
}

// 发送一个字节到SPI Flash并返回接收到的字节
static uint8_t spi_flash_send_byte(uint8_t byte)
{
/*	uint8_t RxData;
 	HAL_SPI_TransmitReceive_IT(&hspi1, &byte, &RxData, 1);
 	return RxData;*/
	uint8_t 						r_data = 0;
	HAL_StatusTypeDef 				status;
	if((status = HAL_SPI_TransmitReceive(&hspi1, &byte, &r_data, 1, SPI_TIMEOUT)) != HAL_OK )
	{
		if (status == HAL_ERROR)
	    {
		    printf("SPI transmission error!\n");
	    }
		else if (status == HAL_TIMEOUT)
		{
			 printf("SPI transmission timeout!\n");
		}
		else if (status == HAL_BUSY)
		{
			 printf("SPI is busy!\n");
	    }
		return 0;
	}
	return r_data;
}

// 从SPI Flash中读取一个字节
static inline uint8_t spi_flash_read_byte()
{
    return spi_flash_send_byte(FLASH_DUMMY_BYTE);
}

// 读取SPI Flash的状态寄存器值
static uint32_t spi_flash_read_status()
{
    uint8_t status;
    uint32_t flash_status = FLASH_READY;

    spi_flash_select_chip();

    spi_flash_send_byte(spi_conf.status_cmd);

    status = spi_flash_read_byte();

    if (spi_conf.busy_state == 1 && (status & (1 << spi_conf.busy_bit)))
        flash_status = FLASH_BUSY;
    else if (spi_conf.busy_state == 0 && !(status & (1 << spi_conf.busy_bit)))
        flash_status = FLASH_BUSY;

    spi_flash_deselect_chip();

    return flash_status;
}

// 获取SPI Flash的状态，等待操作完成或超时
static uint32_t spi_flash_get_status()
{
    uint32_t status, timeout = 0x1000000;

    status = spi_flash_read_status();

    /* Wait for an operation to complete or a TIMEOUT to occur */
    while (status == FLASH_BUSY && timeout)
    {
        status = spi_flash_read_status();
        timeout --;
    }

    if (!timeout)
        status = FLASH_TIMEOUT;

    return status;
}

// 读取SPI Flash的ID
static void spi_flash_read_id(chip_id_t *chip_id)
{
    spi_flash_select_chip();

    spi_flash_send_byte(spi_conf.read_id_cmd);
    spi_flash_send_byte(FLASH_DUMMY_BYTE);
    spi_flash_send_byte(FLASH_DUMMY_BYTE);
    spi_flash_send_byte(0x00);
    chip_id->maker_id  = spi_flash_read_byte();  // 读取厂商ID
    chip_id->device_id = spi_flash_read_byte();  // 读取设备ID
    chip_id->third_id  = spi_flash_read_byte();  // 读取第三个ID
    chip_id->fourth_id = spi_flash_read_byte();  // 读取第四个ID
    chip_id->fifth_id  = spi_flash_read_byte();
    chip_id->sixth_id  = spi_flash_read_byte();

    spi_flash_deselect_chip();
}

/**
 * @brief       发送地址
 *   @note      根据芯片型号的不同, 发送24ibt / 32bit地址
 * @param       address : 要发送的地址
 * @retval      无
 */
static void norflash_send_address(uint32_t address)
{
	spi_flash_send_byte((uint8_t)((address)>>24)); /* 发送 bit31 ~ bit24 地址 */
	spi_flash_send_byte((uint8_t)((address)>>16));     /* 发送 bit23 ~ bit16 地址 */
	spi_flash_send_byte((uint8_t)((address)>>8));      /* 发送 bit15 ~ bit8  地址 */
	spi_flash_send_byte((uint8_t)address);             /* 发送 bit7  ~ bit0  地址 */
}

/**
 * @brief       读取状态寄存器，一共有3个状态寄存器
 *   @note      状态寄存器1：
 *              BIT7  6   5   4   3   2   1   0
 *              SPR   RV  TB BP2 BP1 BP0 WEL BUSY
 *              SPR:默认0,状态寄存器保护位,配合WP使用
 *              TB,BP2,BP1,BP0:FLASH区域写保护设置
 *              WEL:写使能锁定
 *              BUSY:忙标记位(1,忙;0,空闲)
 *              默认:0x00
 *
 *              状态寄存器2：
 *              BIT7  6   5   4   3   2   1   0
 *              SUS   CMP LB3 LB2 LB1 (R) QE  SRP1
 *
 *              状态寄存器3：
 *              BIT7      6    5    4   3   2   1   0
 *              HOLD/RST  DRV1 DRV0 (R) (R) WPS ADP ADS
 *
 * @param       regno: 状态寄存器号，范:1~3
 * @retval      状态寄存器值
 */
uint8_t norflash_read_sr(uint8_t regno)
{
    uint8_t byte = 0, command = 0;

    switch (regno)
    {
        case 1:
            command = 0x05;  /* 读状态寄存器1指令 */
            break;

        case 2:
            command = 0x35;  /* 读状态寄存器2指令 */
            break;

        case 3:
            command = 0x15;  /* 读状态寄存器3指令 */
            break;

        default:
            command = 0x05;
            break;
    }

    spi_flash_select_chip();
    spi_flash_send_byte(command);      /* 发送读寄存器命令 */
    byte = spi_flash_send_byte(0Xff);  /* 读取一个字节 */
    spi_flash_deselect_chip();

    return byte;
}

/**
 * @brief       写状态寄存器
 *   @note      寄存器说明见norflash_read_sr函数说明
 * @param       regno: 状态寄存器号，范:1~3
 * @param       sr   : 要写入状态寄存器的值
 * @retval      无
 */
void norflash_write_sr(uint8_t regno, uint8_t sr)
{
    uint8_t command = 0;

    switch (regno)
    {
        case 1:
            command = 0x01;  /* 写状态寄存器1指令 */
            break;

        case 2:
            command = 0x31;  /* 写状态寄存器2指令 */
            break;

        case 3:
            command = 0x11;  /* 写状态寄存器3指令 */
            break;

        default:
            command = 0x01;
            break;
    }

    spi_flash_select_chip();
    spi_flash_send_byte(command);  /* 发送读寄存器命令 */
    spi_flash_send_byte(sr);       /* 写入一个字节 */
    spi_flash_deselect_chip();
}

// 启用SPI Flash的写使能
void spi_flash_write_enable()
{
    if (spi_conf.write_en_cmd == UNDEFINED_CMD)
        return;

    spi_flash_select_chip();
    spi_flash_send_byte(spi_conf.write_en_cmd);
    spi_flash_deselect_chip();
}

void FLASH_Enable4ByteAddr()
{
	uint8_t temp;
	    temp = norflash_read_sr(3);         /* 读取状态寄存器3，判断地址模式 */

	    if ((temp & 0X01) == 0)             /* 如果不是4字节地址模式,则进入4字节地址模式 */
	    {
	    	spi_flash_write_enable();        /* 写使能 */
	        temp |= 1 << 1;                 /* ADP=1, 上电4位地址模式 */
	        norflash_write_sr(3, temp);     /* 写SR3 */

	        spi_flash_select_chip();
	        spi_flash_send_byte(0xB7);    /* 使能4字节地址指令 */
	        spi_flash_deselect_chip();
	    }
}

// 异步写入SPI Flash的一页数据
static void spi_flash_write_page_async(uint8_t *buf, uint32_t page,
		uint32_t page_size)
{
    uint32_t i;

    spi_flash_write_enable();

    spi_flash_select_chip();

    spi_flash_send_byte(spi_conf.write_cmd);

    page = page << spi_conf.page_offset;

    spi_flash_send_byte(ADDR_4th_CYCLE(page));
    spi_flash_send_byte(ADDR_3rd_CYCLE(page));
    spi_flash_send_byte(ADDR_2nd_CYCLE(page));
    spi_flash_send_byte(ADDR_1st_CYCLE(page));

    for (i = 0; i < page_size; i++)
        spi_flash_send_byte(buf[i]);

    spi_flash_deselect_chip();
}

// 从指定地址读取数据到缓冲区
static uint32_t spi_flash_read_data(uint8_t *buf, uint32_t page,
		uint32_t page_offset, uint32_t data_size)
{
    uint32_t i, addr = (page << spi_conf.page_offset) + page_offset;

    spi_flash_select_chip();

    spi_flash_send_byte(spi_conf.read_cmd);

    spi_flash_send_byte(ADDR_4th_CYCLE(addr));
    spi_flash_send_byte(ADDR_3rd_CYCLE(addr));
    spi_flash_send_byte(ADDR_2nd_CYCLE(addr));
    spi_flash_send_byte(ADDR_1st_CYCLE(addr));

    /* AT45DB要求在地址后写入虚拟字节 */
    spi_flash_send_byte(FLASH_DUMMY_BYTE);

    for (i = 0; i < data_size; i++)
        buf[i] = spi_flash_read_byte();

    spi_flash_deselect_chip();

    return FLASH_READY;
}

// 从指定页读取数据到缓冲区
static uint32_t spi_flash_read_page(uint8_t *buf, uint32_t page,
		uint32_t page_size)
{
    return spi_flash_read_data(buf, page, 0, page_size);
}

// 从指定页的偏移量读取备用数据到缓冲区
static uint32_t spi_flash_read_spare_data(uint8_t *buf, uint32_t page,
		uint32_t offset, uint32_t data_size)
{
    return FLASH_STATUS_INVALID_CMD;
}

// 擦除指定块
static uint32_t spi_flash_erase_block(uint32_t page)
{
    uint32_t addr = page << spi_conf.page_offset;

    spi_flash_write_enable();

    spi_flash_select_chip();

    spi_flash_send_byte(spi_conf.erase_cmd);

    spi_flash_send_byte(ADDR_4th_CYCLE(addr));
    spi_flash_send_byte(ADDR_3rd_CYCLE(addr));
    spi_flash_send_byte(ADDR_2nd_CYCLE(addr));
    spi_flash_send_byte(ADDR_1st_CYCLE(addr));

    spi_flash_deselect_chip();

    return spi_flash_get_status();
}

// 检查是否支持坏块管理
static inline bool spi_flash_is_bb_supported()
{
    return false;
}

// SPI NOR Flash的硬件抽象层
flash_hal_t hal_spi_nor =
{
    .init = spi_flash_init,
    .uninit = spi_flash_uninit,
    .read_id = spi_flash_read_id,
    .erase_block = spi_flash_erase_block,
    .read_page = spi_flash_read_page,
    .read_spare_data = spi_flash_read_spare_data, 
    .write_page_async = spi_flash_write_page_async,
    .read_status = spi_flash_read_status,
    .is_bb_supported = spi_flash_is_bb_supported
};
