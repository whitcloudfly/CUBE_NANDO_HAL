/*********************************************************************************************************
*
* File                : spi_nor_flash.c
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

#include "spi_nor_flash.h"
//#include "spi.h"
#include "gpio.h"
#include "main.h"
#include <stdio.h>
#include <string.h>
#include <stm32f4xx.h>
//#include <stm32f4xx_hal_spi.h>

/**SPI1 GPIO Configuration
  PA6     ------> SPI1_MISO
  PA7     ------> SPI1_MOSI
  PB3     ------> SPI1_SCK
  PB4     ------> SPI1_CS
*/

#define SPI_FLASH_MISO_PIN SPI1_MISO_Pin
#define SPI_FLASH_MOSI_PIN SPI1_MOSI_Pin
#define SPI_FLASH_SCK_PIN SPI1_SCK_Pin
#define SPI_FLASH_CS_PIN SPI1_CS_Pin

#define FLASH_DUMMY_BYTE 0xA5

/* 第一地址周期 */
#define ADDR_1st_CYCLE(ADDR) (uint8_t)((ADDR)& 0xFF)
/* 第二地址周期 */
#define ADDR_2nd_CYCLE(ADDR) (uint8_t)(((ADDR)& 0xFF00) >> 8)
/* 第三地址周期 */
#define ADDR_3rd_CYCLE(ADDR) (uint8_t)(((ADDR)& 0xFF0000) >> 16)
/* 第四地址周期 */
#define ADDR_4th_CYCLE(ADDR) (uint8_t)(((ADDR)& 0xFF000000) >> 24)

#define UNDEFINED_CMD 0xFF

#define _SPI_NAND_OP_READ_ID                    0x9F    /* 读取制造商ID和设备ID */
#define _SPI_NAND_ADDR_MANUFACTURE_ID           0x00   /* 制造商ID的地址 */
#define _SPI_NAND_ADDR_DEVICE_ID                0x01    /* 设备ID的地址 */

extern SPI_HandleTypeDef hspi1;

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

// 初始化SPI Flash的GPIO引脚
static void spi_flash_gpio_init()
{
	  SPI_HandleTypeDef* spiHandle;

	  GPIO_InitTypeDef GPIO_InitStruct = {0};

	    /* SPI1 clock enable */
	    __HAL_RCC_SPI1_CLK_ENABLE();

	    __HAL_RCC_GPIOA_CLK_ENABLE();
	    __HAL_RCC_GPIOB_CLK_ENABLE();
	    /**SPI1 GPIO Configuration
	    PA4     ------> SPI1_NSS
	    PA6     ------> SPI1_MISO
	    PA7     ------> SPI1_MOSI
	    PB3     ------> SPI1_SCK
	    */
	    /*Configure SPI1_SCK pin : PtPin */
	    GPIO_InitStruct.Pin = SPI1_SCK_Pin;
	    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	    GPIO_InitStruct.Pull = GPIO_NOPULL;
	    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
	    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
	    HAL_GPIO_Init(SPI1_SCK_GPIO_Port, &GPIO_InitStruct);

	    /*Configure SPI1_MOSI pin : PtPin */
	    GPIO_InitStruct.Pin = SPI1_MOSI_Pin;
	    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	    GPIO_InitStruct.Pull = GPIO_NOPULL;
	    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
	    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
	    HAL_GPIO_Init(SPI1_MOSI_GPIO_Port, &GPIO_InitStruct);

	    /*Configure SPI1_MISO pin : PtPin */
	    GPIO_InitStruct.Pin = SPI1_MISO_Pin;
	    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	    GPIO_InitStruct.Pull = GPIO_PULLUP;
	    HAL_GPIO_Init(SPI1_MISO_GPIO_Port, &GPIO_InitStruct);

	    /*Configure SPI1_CS pin : PtPin */
	    GPIO_InitStruct.Pin = SPI1_CS_Pin;
	    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	    GPIO_InitStruct.Pull = GPIO_NOPULL;
	    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	    HAL_GPIO_Init(SPI1_CS_GPIO_Port, &GPIO_InitStruct);
}

// 取消初始化SPI Flash的GPIO引脚
static void spi_flash_gpio_uninit()
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	SPI_HandleTypeDef* spiHandle;
	  /* USER CODE BEGIN SPI1_MspDeInit 0 */

	  /* USER CODE END SPI1_MspDeInit 0 */
	    /* Peripheral clock disable */
	    __HAL_RCC_SPI1_CLK_DISABLE();

	    /*Configure GPIO pin : PtPin */
	    GPIO_InitStruct.Pin = SPI1_SCK_Pin;
	    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	    GPIO_InitStruct.Pull = GPIO_NOPULL;
	    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
	    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
	    HAL_GPIO_Init(SPI1_SCK_GPIO_Port, &GPIO_InitStruct);

	    /*Configure GPIO pin : PtPin */
	    GPIO_InitStruct.Pin = SPI1_MOSI_Pin;
	    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	    GPIO_InitStruct.Pull = GPIO_NOPULL;
	    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
	    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
	    HAL_GPIO_Init(SPI1_MOSI_GPIO_Port, &GPIO_InitStruct);

	    /*Configure GPIO pin : PtPin */
	    GPIO_InitStruct.Pin = SPI1_MISO_Pin;
	    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	    GPIO_InitStruct.Pull = GPIO_PULLUP;
	    HAL_GPIO_Init(SPI1_MISO_GPIO_Port, &GPIO_InitStruct);

	    /*Configure GPIO pin : PtPin */
	    GPIO_InitStruct.Pin = SPI1_CS_Pin;
	    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	    GPIO_InitStruct.Pull = GPIO_NOPULL;
	    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
	    HAL_GPIO_Init(SPI1_CS_GPIO_Port, &GPIO_InitStruct);
}

// 选中SPI Flash芯片
static inline void spi_flash_select_chip()
{
	HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI_FLASH_CS_PIN, GPIO_PIN_RESET);
}

// 取消选中SPI Flash芯片
static inline void spi_flash_deselect_chip()
{
	HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI_FLASH_CS_PIN, GPIO_PIN_SET);
}

// 获取SPI时钟预分频器的值
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

/**
  * @brief  Initializes the SPIx peripheral according to the specified
  *         parameters in the SPI_InitStruct.
  * @param  SPIx: where x can be 1, 2 or 3 to select the SPI peripheral.
  * @param  SPI_InitStruct: pointer to a SPI_InitTypeDef structure that
  *         contains the configuration information for the specified SPI peripheral.
  * @retval None
  */
void SPI_Init(SPI_HandleTypeDef* hspi, SPI_InitTypeDef* SPI_InitStruct)
{
    /* Check the parameters */

    /* Check the SPI parameters */
	assert_param(IS_SPI_ALL_INSTANCE(hspi->Instance));
	assert_param(IS_SPI_MODE(hspi->Init.Mode));
	assert_param(IS_SPI_DIRECTION(hspi->Init.Direction));
	assert_param(IS_SPI_DATASIZE(hspi->Init.DataSize));
	assert_param(IS_SPI_NSS(hspi->Init.NSS));
	assert_param(IS_SPI_BAUDRATE_PRESCALER(hspi->Init.BaudRatePrescaler));
	assert_param(IS_SPI_FIRST_BIT(hspi->Init.FirstBit));
	assert_param(IS_SPI_TIMODE(hspi->Init.TIMode));

    /* Configure SPIx: direction, NSS management, first transmitted bit, BaudRate prescaler
       master/slave mode, CPOL and CPHA */
    hspi->Init.Direction = SPI_InitStruct->Direction;
    hspi->Init.Mode = SPI_InitStruct->Mode;
    hspi->Init.DataSize = SPI_InitStruct->DataSize;
    hspi->Init.CLKPolarity = SPI_InitStruct->CLKPolarity;
    hspi->Init.CLKPhase = SPI_InitStruct->CLKPhase;
    hspi->Init.NSS = SPI_InitStruct->NSS;
    hspi->Init.BaudRatePrescaler = SPI_InitStruct->BaudRatePrescaler;
    hspi->Init.FirstBit = SPI_InitStruct->FirstBit;
    hspi->Init.TIMode = SPI_InitStruct->TIMode;

    /* Initialize the SPI peripheral */
    if (HAL_SPI_Init(hspi) != HAL_OK)
    {
        Error_Handler();
    }
}

// 初始化SPI Flash
static int spi_flash_init(void *conf, uint32_t conf_size)
{
    SPI_InitTypeDef spi_init;

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
    hspi1.Init.CLKPolarity = SPI_POLARITY_HIGH;
    hspi1.Init.CLKPhase = SPI_PHASE_2EDGE;
    hspi1.Init.NSS = SPI_NSS_SOFT;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi1.Init.CRCPolynomial = 10;
    SPI_Init(&hspi1, &spi_init);
//    MX_SPI1_Init(&hspi1);

    /* 使能SPI */
//    SPI_Cmd(SPI1, ENABLE);
    __HAL_SPI_ENABLE(&hspi1);

    return 0;
}

// 取消初始化SPI Flash
static void spi_flash_uninit()
{
//    spi_flash_gpio_uninit(&hspi1);
	HAL_SPI_MspDeInit(&hspi1);
    /* 禁用SPI */
    __HAL_SPI_DISABLE(&hspi1);
}

HAL_StatusTypeDef HAL_SPI_Transmit(SPI_HandleTypeDef *hspi, uint8_t *pData, uint16_t Size, uint32_t Timeout);
HAL_StatusTypeDef HAL_SPI_Receive(SPI_HandleTypeDef *hspi, uint8_t *pData, uint16_t Size, uint32_t Timeout);
HAL_StatusTypeDef HAL_SPI_TransmitReceive(SPI_HandleTypeDef *hspi, uint8_t *pTxData, uint8_t *pRxData, uint16_t Size,
                                          uint32_t Timeout);

/**
 * @brief    SPI发�?�指定长度的数据
 * @param    buf  —�?? 发�?�数据缓冲区首地�?
 * @param    size —�?? 要发送数据的字节�?
 * @retval   成功返回HAL_OK
 */
static HAL_StatusTypeDef SPI_Transmit(uint8_t* send_buf, uint16_t size)
{
    return HAL_SPI_Transmit(&hspi1, send_buf, size, 100);
}

/**
 * @brief   SPI接收指定长度的数�?
 * @param   buf  —�?? 接收数据缓冲区首地址
 * @param   size —�?? 要接收数据的字节�?
 * @retval  成功返回HAL_OK
 */
static HAL_StatusTypeDef SPI_Receive(uint8_t* recv_buf, uint16_t size)
{
   return HAL_SPI_Receive(&hspi1, recv_buf, size, 100);
}


// 发送一个字节到SPI Flash并返回接收到的字节
static uint8_t spi_flash_send_byte(uint8_t byte)
{
    uint8_t rx_byte;

    spi_flash_select_chip();
    // 等待TXE标志位设置，表示发送缓冲区为空
    while (HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY);

    // 发送一个字节
    if (HAL_OK == HAL_SPI_Transmit(&hspi1, &byte, 1, HAL_MAX_DELAY))
    {
        // 等待RXNE标志位设置，表示接收缓冲区非空
        while (HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY);

        // 接收一个字节
        if (HAL_OK == HAL_SPI_Receive(&hspi1, &rx_byte, 1, HAL_MAX_DELAY))
        {
            return rx_byte;
        }
    }

    spi_flash_deselect_chip();

    return 0; // 发送或接收出现问题，返回0或者根据需求返回其他错误值
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
    uint32_t flash_status = FLASH_STATUS_READY;

    spi_flash_select_chip();

    spi_flash_send_byte(spi_conf.status_cmd);

    status = spi_flash_read_byte();

    if (spi_conf.busy_state == 1 && (status & (1 << spi_conf.busy_bit)))
        flash_status = FLASH_STATUS_BUSY;
    else if (spi_conf.busy_state == 0 && !(status & (1 << spi_conf.busy_bit)))
        flash_status = FLASH_STATUS_BUSY;

    spi_flash_deselect_chip();

    return flash_status;
}

// 获取SPI Flash的状态，等待操作完成或超时
static uint32_t spi_flash_get_status()
{
    uint32_t status, timeout = 0x1000000;

    status = spi_flash_read_status();

    /* 等待操作完成或超时 */
    while (status == FLASH_STATUS_BUSY && timeout)
    {
        status = spi_flash_read_status();
        timeout --;
    }

    if (!timeout)
        status = FLASH_STATUS_TIMEOUT;

    return status;
}

// 读取SPI Flash的ID
static void spi_flash_read_id(chip_id_t *chip_id)
{
    spi_flash_select_chip();

    spi_flash_send_byte(spi_conf.read_id_cmd);

    chip_id->maker_id = spi_flash_read_byte();
    chip_id->device_id = spi_flash_read_byte();
    chip_id->third_id = spi_flash_read_byte();
    chip_id->fourth_id = spi_flash_read_byte();
    chip_id->fifth_id = spi_flash_read_byte();
    chip_id->sixth_id = spi_flash_read_byte();

    spi_flash_deselect_chip();
}

// 启用SPI Flash的写使能
static void spi_flash_write_enable()
{
    if (spi_conf.write_en_cmd == UNDEFINED_CMD)
        return;

    spi_flash_select_chip();
    spi_flash_send_byte(spi_conf.write_en_cmd);
    spi_flash_deselect_chip();
}

// 异步写入SPI Flash的一页数据
static void spi_flash_write_page_async(uint8_t *buf, uint32_t page, uint32_t page_size)
{
    uint32_t i;

    spi_flash_write_enable();

    spi_flash_select_chip();

    spi_flash_send_byte(spi_conf.write_cmd);

    page = page << spi_conf.page_offset;

    spi_flash_send_byte(ADDR_3rd_CYCLE(page));
    spi_flash_send_byte(ADDR_2nd_CYCLE(page));
    spi_flash_send_byte(ADDR_1st_CYCLE(page));

    for (i = 0; i < page_size; i++)
        spi_flash_send_byte(buf[i]);

    spi_flash_deselect_chip();
}

// 从指定地址读取数据到缓冲区
static uint32_t spi_flash_read_data(uint8_t *buf, uint32_t page, uint32_t page_offset, uint32_t data_size)
{
    uint32_t i, addr = (page << spi_conf.page_offset) + page_offset;

    spi_flash_select_chip();

    spi_flash_send_byte(spi_conf.read_cmd);

    spi_flash_send_byte(ADDR_3rd_CYCLE(addr));
    spi_flash_send_byte(ADDR_2nd_CYCLE(addr));
    spi_flash_send_byte(ADDR_1st_CYCLE(addr));

    /* AT45DB要求在地址后写入虚拟字节 */
    spi_flash_send_byte(FLASH_DUMMY_BYTE);

    for (i = 0; i < data_size; i++)
        buf[i] = spi_flash_read_byte();

    spi_flash_deselect_chip();

    return FLASH_STATUS_READY;
}

// 从指定页读取数据到缓冲区
static uint32_t spi_flash_read_page(uint8_t *buf, uint32_t page, uint32_t page_size)
{
    return spi_flash_read_data(buf, page, 0, page_size);
}

// 从指定页的偏移量读取备用数据到缓冲区
static uint32_t spi_flash_read_spare_data(uint8_t *buf, uint32_t page, uint32_t offset, uint32_t data_size)
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