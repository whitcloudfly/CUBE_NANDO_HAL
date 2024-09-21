/* 版权所有 (C) 2023 NANDO作者
 * 本程序是自由软件；您可以重新分发它和/或修改
 * 根据GNU通用公共许可证第3版的条款。
 */

#include "spi_nand.h"
#include "main.h"
#include "spi.h"
#include "gpio.h"
#include "log.h"
#include <stm32f4xx.h>
#include <stdio.h>

/* SPI NAND命令集 */
#define _OP_GET_FEATURE                0x0F    /* 获取特性 */
#define _OP_SET_FEATURE                0x1F    /* 设置特性 */
#define _OP_PAGE_READ                  0x13    /* 将页数据加载到SPI NAND芯片的缓存中 */
#define _OP_READ_FROM_CACHE_SINGLE     0x03    /* 从SPI NAND芯片的缓存中读取数据，单速度 */
#define _OP_READ_FROM_CACHE_DUAL       0x3B    /* 从SPI NAND芯片的缓存中读取数据，双速度 */
#define _OP_READ_FROM_CACHE_QUAD       0x6B    /* 从SPI NAND芯片的缓存中读取数据，四速度 */
#define _OP_WRITE_ENABLE               0x06    /* 启用将数据写入SPI NAND芯片 */
#define _OP_WRITE_DISABLE              0x04    /* 复位写使能锁存器（WEL） */
#define _OP_PROGRAM_LOAD_SINGLE        0x02    /* 将数据写入带有缓存复位的SPI NAND芯片的缓存，单速度 */
#define _OP_PROGRAM_LOAD_QUAD          0x32    /* 将数据写入带有缓存复位的SPI NAND芯片的缓存，四速度 */
#define _OP_PROGRAM_LOAD_RAMDOM_SINGLE 0x84    /* 将数据写入SPI NAND芯片的缓存，单速度 */
#define _OP_PROGRAM_LOAD_RAMDON_QUAD   0x34    /* 将数据写入SPI NAND芯片的缓存，四速度 */
#define _OP_PROGRAM_EXECUTE            0x10    /* 将缓存中的数据写入SPI NAND芯片 */
#define _OP_READ_ID                    0x9F    /* 读取制造商ID和设备ID */
#define _OP_BLOCK_ERASE                0xD8    /* 擦除块 */
#define _OP_RESET                      0xFF    /* 复位 */
#define _OP_DIE_SELECT                 0xC2    /* 选择Die */
/* SPI NAND命令集的寄存器地址 */
#define _ADDR_ECC                      0x90    /* ECC配置的地址 */
#define _ADDR_PROTECT                  0xA0    /* 保护的地址 */
#define _ADDR_FEATURE                  0xB0    /* 特性的地址 */
#define _ADDR_STATUS                   0xC0    /* 状态的地址 */
#define _ADDR_FEATURE_4                0xD0    /* 状态4的地址 */
#define _ADDR_STATUS_5                 0xE0    /* 状态5的地址 */
#define _ADDR_MANUFACTURE_ID           0x00   /* 制造商ID的地址 */
#define _ADDR_DEVICE_ID                0x01    /* 设备ID的地址 */
/* SPI NAND命令集的寄存器地址的值 */
#define _VAL_DISABLE_PROTECTION        0x0     /* 禁用写保护的值 */
#define _VAL_ENABLE_PROTECTION         0x38    /* 启用写保护的值 */
#define _VAL_OIP                       0x1     /* OIP = 操作正在进行中 */
#define _VAL_ERASE_FAIL                0x4     /* 擦除失败 */
#define _VAL_PROGRAM_FAIL              0x8     /* 编程失败 */

#define SPI_FLASH_MISO_PIN SPI1_MISO_Pin
#define SPI_FLASH_MOSI_PIN SPI1_MOSI_Pin
#define SPI_FLASH_SCK_PIN SPI1_SCK_Pin
#define SPI_FLASH_CS_PIN SPI1_CS_Pin

#define FLASH_DUMMY_BYTE 0xFF
/* 第一寻址周期 */
#define ADDR_1st_CYCLE(ADDR) (uint8_t)((ADDR)& 0xFF)
/* 第二寻址周期 */
#define ADDR_2nd_CYCLE(ADDR) (uint8_t)(((ADDR)& 0xFF00) >> 8)
/* 第三寻址周期 */
#define ADDR_3rd_CYCLE(ADDR) (uint8_t)(((ADDR)& 0xFF0000) >> 16)
/* 第四寻址周期 */
#define ADDR_4th_CYCLE(ADDR) (uint8_t)(((ADDR)& 0xFF000000) >> 24)

#define UNDEFINED_CMD 0xFF

static void spi_flash_chip_init(void);

typedef struct __attribute__((__packed__))
{
    uint32_t spare_offset;
    uint8_t mode_data;
    uint8_t unlock_data;
    uint8_t ecc_err_bits_mask;
    uint8_t ecc_err_bits_state;
    uint8_t read_dummy_prepend;
    uint8_t plane_select_have;
    uint8_t die_select_type;
    uint32_t freq;
} spi_conf_t;

static spi_conf_t spi_conf;
extern SPI_HandleTypeDef hspi1;

enum
{
    FLASH_OP_EMPTY = 0,
    FLASH_OP_ERASE = 1,
    FLASH_OP_WRITE = 2,
    FLASH_OP_READ  = 3,
    FLASH_OP_SPARE = 4,
};

static uint32_t flash_last_operation = FLASH_OP_EMPTY;
static uint32_t current_die = 0;

static void spi_flash_gpio_init()
{
//	HAL_SPI_MspInit(&hspi1);
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
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* 配置SPI MISO引脚 */
    GPIO_InitStruct.Pin = SPI_FLASH_MOSI_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* 配置SPI MOSI引脚 */
    GPIO_InitStruct.Pin = SPI_FLASH_MISO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* 配置SPI CS引脚 */
    GPIO_InitStruct.Pin = SPI_FLASH_CS_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* SPI1 interrupt Init */
    HAL_NVIC_SetPriority(SPI1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(SPI1_IRQn);
}

static void spi_flash_gpio_uninit()
{
//	HAL_SPI_MspDeInit(&hspi1);

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
	HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI_FLASH_CS_PIN, GPIO_PIN_RESET);
}

static inline void spi_flash_deselect_chip()
{
	HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI_FLASH_CS_PIN, GPIO_PIN_SET);
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

static int spi_flash_init(void *conf, uint32_t conf_size)
{
    if (conf_size < sizeof(spi_conf_t))
        return -1; 
    spi_conf = *(spi_conf_t *)conf;

    spi_flash_gpio_init();  // 初始化SPI Flash的GPIO引脚

    spi_flash_deselect_chip();  // 取消片选信号，SPI Flash不被选中

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

    /* 使能SPI */
    __HAL_SPI_ENABLE(&hspi1);

    spi_flash_chip_init();  // 初始化SPI Flash芯片

    return 0;
}

static void spi_flash_uninit()
{
    spi_flash_gpio_uninit();  // 反初始化SPI Flash的GPIO引脚

    __HAL_SPI_DISABLE(&hspi1);
}

static uint8_t spi_flash_send_byte(uint8_t byte)
{
  uint32_t timeout = 0x1000000;
  uint8_t rx_byte = 0;

//  HAL_SPI_TransmitReceive(&hspi1, &byte, &rx_byte, 1, timeout);
	HAL_StatusTypeDef 				status;
	if((status = HAL_SPI_TransmitReceive(&hspi1, &byte, &rx_byte, 1, timeout)) != HAL_OK )
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

  return rx_byte;
}

static inline uint8_t spi_flash_read_byte()
{
    return spi_flash_send_byte(FLASH_DUMMY_BYTE);
}

static void spi_flash_set_feature(uint8_t addr, uint8_t data)
{
    spi_flash_select_chip();  // 选中SPI Flash
    spi_flash_send_byte(_OP_SET_FEATURE);  // 发送设置特征的操作码
    spi_flash_send_byte(addr);  // 发送地址
    spi_flash_send_byte(data);  // 发送数据
    spi_flash_deselect_chip();  // 取消片选信号，SPI Flash不被选中
}

static void spi_flash_get_feature(uint8_t addr, uint8_t *data)
{
    spi_flash_select_chip();  // 选中SPI Flash
    spi_flash_send_byte(_OP_GET_FEATURE);  // 发送获取特征的操作码
    spi_flash_send_byte(addr);  // 发送地址
    *data = spi_flash_read_byte();  // 读取一个字节的数据
    spi_flash_deselect_chip();  // 取消片选信号，SPI Flash不被选中
}

static uint32_t spi_flash_read_status()
{
    uint32_t timeout = 0x1000000;  // 超时时间
    uint8_t status;

    do {
        spi_flash_get_feature(_ADDR_STATUS, &status);  // 获取状态
    } while((status & _VAL_OIP) && timeout);

    if (!timeout)
        return FLASH_STATUS_TIMEOUT;

    switch(flash_last_operation){
        case FLASH_OP_ERASE:
            if(status & _VAL_ERASE_FAIL)
                return FLASH_STATUS_ERROR;
            break;
        case FLASH_OP_WRITE:
            if(status & _VAL_PROGRAM_FAIL)
                return FLASH_STATUS_ERROR;
            break;
        case FLASH_OP_READ:
            if((status & spi_conf.ecc_err_bits_mask) == spi_conf.ecc_err_bits_state)
                return FLASH_STATUS_ERROR;
            break;
        case FLASH_OP_SPARE:
        case FLASH_OP_EMPTY:
        default:
            break;
    }
    return FLASH_STATUS_READY;
}

// 选择 SPI Flash 的芯片
static void spi_flash_select_die_cmd(uint32_t die)
{
    switch(spi_conf.die_select_type) {
    case 1: {
        spi_flash_select_chip();  // 选择 Flash 芯片
        spi_flash_send_byte(_OP_DIE_SELECT);  // 发送选择芯片的指令
        spi_flash_send_byte(die);  // 发送芯片编号
        spi_flash_deselect_chip();  // 取消选择芯片
        break;
    }
    case 2: {
        uint8_t feature;
        spi_flash_get_feature(_ADDR_FEATURE_4, &feature);  // 读取 Flash 的特征值
        if(die == 0) {
            feature &= ~(0x40);  // 清除特征值中的某位
        } else {
            feature |= 0x40;  // 设置特征值中的某位
        }
        spi_flash_set_feature(_ADDR_FEATURE_4, feature);  // 设置 Flash 的特征值
        break;
    }
    default:
        break;
    }
}

// 选择 SPI Flash 的 die（芯片）（用于多芯片的情况）
static void spi_flash_select_die(uint32_t page)
{
    uint32_t die = 0;
    if(spi_conf.die_select_type) {
        if(!spi_conf.plane_select_have)
            die = ((page >> 16) & 0xff);  // 计算芯片编号
        else
            die = ((page >> 17) & 0xff);  // 计算芯片编号
        if (current_die != die) {
            current_die = die;
            spi_flash_select_die_cmd(die);  // 选择芯片
        }
    }
}

// 读取 SPI Flash 的 ID（厂商ID和设备ID）
static void spi_flash_read_id(chip_id_t *chip_id)
{
    spi_flash_select_chip();  // 选择 Flash 芯片

    spi_flash_send_byte(_OP_READ_ID);  // 发送读取 ID 的指令
    spi_flash_send_byte(_ADDR_MANUFACTURE_ID);  // 发送读取厂商ID的指令

    chip_id->maker_id = spi_flash_read_byte();  // 读取厂商ID
    chip_id->device_id = spi_flash_read_byte();  // 读取设备ID
    chip_id->third_id = spi_flash_read_byte();  // 读取第三个ID
    chip_id->fourth_id = spi_flash_read_byte();  // 读取第四个ID
    chip_id->fifth_id = spi_flash_read_byte();  // 读取第五个ID
    chip_id->sixth_id = spi_flash_read_byte();  // 读取第六个ID

    spi_flash_deselect_chip();  // 取消选择芯片
}

// SPI Flash 初始化
static void spi_flash_chip_init(void)
{
    if(spi_conf.die_select_type) {
        spi_flash_select_die_cmd(0);  // 选择第一个芯片
        if(spi_conf.mode_data != UNDEFINED_CMD)
            spi_flash_set_feature(_ADDR_FEATURE, spi_conf.mode_data);  // 设置 Flash 的特征值
        if(spi_conf.unlock_data != UNDEFINED_CMD)
            spi_flash_set_feature(_ADDR_PROTECT, spi_conf.unlock_data);  // 设置 Flash 的保护值
        spi_flash_select_die_cmd(1);  // 选择第二个芯片
    }
    if(spi_conf.mode_data != UNDEFINED_CMD)
        spi_flash_set_feature(_ADDR_FEATURE, spi_conf.mode_data);  // 设置 Flash 的特征值
    if(spi_conf.unlock_data != UNDEFINED_CMD)
        spi_flash_set_feature(_ADDR_PROTECT, spi_conf.unlock_data);  // 设置 Flash 的保护值
}

// 启用 Flash 的写入使能
static void spi_flash_write_enable()
{
    spi_flash_select_chip();  // 选择 Flash 芯片
    spi_flash_send_byte(_OP_WRITE_ENABLE);  // 发送写使能指令
    spi_flash_deselect_chip();  // 取消选择芯片
}

// 向 Flash 写入数据
static void spi_flash_program_load(uint8_t *buf, uint32_t page_size, uint32_t page)
{
    uint32_t i;
    uint32_t addr = 0;
    spi_flash_select_chip();  // 选择 Flash 芯片

    spi_flash_send_byte(_OP_PROGRAM_LOAD_SINGLE);  // 发送写入数据的指令

    if(spi_conf.plane_select_have) {
        if((page >> 6)& (0x1))
            spi_flash_send_byte(ADDR_2nd_CYCLE(addr) | (0x10));  // 发送地址的第二个字节
        else
            spi_flash_send_byte(ADDR_2nd_CYCLE(addr) & (0xef));  // 发送地址的第二个字节
    } else {
        spi_flash_send_byte(ADDR_2nd_CYCLE(addr));  // 发送地址的第二个字节
    }

    spi_flash_send_byte(ADDR_1st_CYCLE(addr));  // 发送地址的第一个字节

    for (i = 0; i < page_size; i++)
        spi_flash_send_byte(buf[i]);  // 逐字节写入数据

    spi_flash_deselect_chip();  // 取消选择芯片
}

// 异步写入页面数据到闪存
static void spi_flash_write_page_async(uint8_t *buf, uint32_t page, uint32_t page_size)
{
    spi_flash_select_die(page);  // 选择闪存芯片

    spi_flash_write_enable();  // 使能写入

    spi_flash_select_chip();  // 选择闪存芯片

    spi_flash_program_load(buf, page_size, page);  // 加载数据到写入缓冲区

//    spi_flash_write_enable();  // 使能写入

    spi_flash_select_chip();  // 选择闪存芯片
    spi_flash_send_byte(_OP_PROGRAM_EXECUTE);  // 发送写入执行命令
    flash_last_operation = FLASH_OP_WRITE;  // 记录上一次操作为写入
    spi_flash_send_byte(ADDR_3rd_CYCLE(page));  // 发送地址的第三个周期
    spi_flash_send_byte(ADDR_2nd_CYCLE(page));  // 发送地址的第二个周期
    spi_flash_send_byte(ADDR_1st_CYCLE(page));  // 发送地址的第一个周期
    spi_flash_deselect_chip();  // 取消选择闪存芯片
    // spi_flash_wait_operation_end();  // 等待操作结束

    // spi_flash_write_disable();  // 禁用写入
}

// 将页面数据加载到缓存中
static uint32_t spi_flash_load_page_into_cache(uint32_t page)
{
    spi_flash_select_die(page);  // 选择闪存芯片

    spi_flash_select_chip();  // 选择闪存芯片
    spi_flash_send_byte(_OP_PAGE_READ);  // 发送页面读取命令
    flash_last_operation = FLASH_OP_READ;  // 记录上一次操作为读取
    spi_flash_send_byte(ADDR_3rd_CYCLE(page));  // 发送地址的第三个周期
    spi_flash_send_byte(ADDR_2nd_CYCLE(page));  // 发送地址的第二个周期
    spi_flash_send_byte(ADDR_1st_CYCLE(page));  // 发送地址的第一个周期
    spi_flash_deselect_chip();  // 取消选择闪存芯片

    return spi_flash_read_status();  // 读取状态寄存器的值
}

// 读取页面数据
static uint32_t spi_flash_read_page(uint8_t *buf, uint32_t page, uint32_t data_size)
{
    uint32_t status = spi_flash_load_page_into_cache(page);  // 将页面数据加载到缓存中
    uint32_t data_offset = 0;

    spi_flash_select_chip();  // 选择闪存芯片
    spi_flash_send_byte(_OP_READ_FROM_CACHE_SINGLE);  // 发送从缓存中读取数据的命令

    if (spi_conf.read_dummy_prepend)
        spi_flash_send_byte(FLASH_DUMMY_BYTE);  // 如果有前导字节，发送一个虚拟字节

    if (spi_conf.plane_select_have) {
        if ((page >> 6) & (0x1))
            spi_flash_send_byte(ADDR_2nd_CYCLE(data_offset) | (0x10));  // 发送地址的第二个周期
        else
            spi_flash_send_byte(ADDR_2nd_CYCLE(data_offset) & (0xef));  // 发送地址的第二个周期
    } else {
        spi_flash_send_byte(ADDR_2nd_CYCLE(data_offset));  // 发送地址的第二个周期
    }

    spi_flash_send_byte(ADDR_1st_CYCLE(data_offset));  // 发送地址的第一个周期

    if (!spi_conf.read_dummy_prepend)
        spi_flash_send_byte(FLASH_DUMMY_BYTE);  // 如果没有前导字节，发送一个虚拟字节

    for (uint32_t i = 0; i < data_size; i++)
        buf[i] = spi_flash_read_byte();  // 读取字节数据到缓冲区

    spi_flash_deselect_chip();  // 取消选择闪存芯片
    return status;  // 返回状态寄存器的值
}

// 读取备用数据
static uint32_t spi_flash_read_spare_data(uint8_t *buf, uint32_t page, uint32_t offset, uint32_t data_size)
{
    uint32_t status;

    spi_flash_select_die(page);  // 选择闪存芯片

    spi_flash_select_chip();  // 选择闪存芯片
    spi_flash_send_byte(_OP_PAGE_READ);  // 发送页面读取命令
    flash_last_operation = FLASH_OP_SPARE;  // 记录上一次操作为备用
    spi_flash_send_byte(ADDR_3rd_CYCLE(page));  // 发送地址的第三个周期
    spi_flash_send_byte(ADDR_2nd_CYCLE(page));  // 发送地址的第二个周期
    spi_flash_send_byte(ADDR_1st_CYCLE(page));  // 发送地址的第一个周期
    spi_flash_deselect_chip();  // 取消选择闪存芯片
    status = spi_flash_read_status();  // 读取状态寄存器的值

    spi_flash_select_chip();  // 选择闪存芯片
    spi_flash_send_byte(_OP_READ_FROM_CACHE_SINGLE);  // 发送从缓存中读取数据的命令

    if (spi_conf.read_dummy_prepend)
        spi_flash_send_byte(FLASH_DUMMY_BYTE);  // 如果有前导字节，发送一个虚拟字节

    offset += spi_conf.spare_offset;
    if (spi_conf.plane_select_have) {
        if ((page >> 6) & (0x1))
            spi_flash_send_byte(ADDR_2nd_CYCLE(offset) | (0x10));  // 发送地址的第二个周期
        else
            spi_flash_send_byte(ADDR_2nd_CYCLE(offset) & (0xef));  // 发送地址的第二个周期
    } else {
        spi_flash_send_byte(ADDR_2nd_CYCLE(offset));  // 发送地址的第二个周期
    }
    spi_flash_send_byte(ADDR_1st_CYCLE(offset));  // 发送地址的第一个周期

    if (!spi_conf.read_dummy_prepend)
        spi_flash_send_byte(FLASH_DUMMY_BYTE);  // 如果没有前导字节，发送一个虚拟字节

    for (uint32_t i = 0; i < data_size; i++)
        buf[i] = spi_flash_read_byte();  // 读取字节数据到缓冲区

    spi_flash_deselect_chip();  // 取消选择闪存芯片
    return status;  // 返回状态寄存器的值
}

// 擦除块
static uint32_t spi_flash_erase_block(uint32_t page)
{
    spi_flash_select_die(page);  // 选择闪存芯片

    spi_flash_write_enable();  // 使能写入

    spi_flash_select_chip();  // 选择闪存芯片

    spi_flash_send_byte(_OP_BLOCK_ERASE);  // 发送块擦除命令
    flash_last_operation = FLASH_OP_ERASE;  // 记录上一次操作为擦除

    spi_flash_send_byte(ADDR_3rd_CYCLE(page));  // 发送地址的第三个周期
    spi_flash_send_byte(ADDR_2nd_CYCLE(page));  // 发送地址的第二个周期
    spi_flash_send_byte(ADDR_1st_CYCLE(page));  // 发送地址的第一个周期

    spi_flash_deselect_chip();  // 取消选择闪存芯片

    return spi_flash_read_status();  // 读取状态寄存器的值
}

// 检查是否支持坏块检测
static inline bool spi_flash_is_bb_supported()
{
    return true;
}

// 定义闪存硬件抽象层结构体
flash_hal_t hal_spi_nand =
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
