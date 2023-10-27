/*
 * spi_nor.c
 *
 *  Created on: Oct 24, 2023
 *      Author: wjunh
 */

#include "spi_nor.h"

 /**********************************************************************************
  * 函数功能: 模块初始化
  */
uint8_t SPI_NOR_Init(void)
{
	SPI_NOR_Reset();
	return W25Qxx_GetStatus();
}


static void	SPI_NOR_Reset(void)
{
	uint8_t cmd[2] = {RESET_ENABLE_CMD,RESET_MEMORY_CMD};

	W25Qxx_Enable();
	/* Send the reset command */
	HAL_SPI_Transmit(&hspi1, cmd, 2, SPI_TIMEOUT_VALUE);
	W25Qxx_Disable();

}

 /**********************************************************************************
  * 函数功能: 获取设备状态
  */
static uint8_t W25Qxx_GetStatus(void)
{
	uint8_t cmd[] = {READ_STATUS_REG1_CMD};
	uint8_t status;

	W25Qxx_Enable();
	/* Send the read status command */
	HAL_SPI_Transmit(&hspi1, cmd, 1, SPI_TIMEOUT_VALUE);
	/* Reception of the data */
	HAL_SPI_Receive(&hspi1,&status, 1, SPI_TIMEOUT_VALUE);
	W25Qxx_Disable();

	/* Check the value of the register */
  if((status & SPI_NOR_FSR_BUSY) != 0)
  {
    return W25Qxx_BUSY;
  }
	else
	{
		return W25Qxx_OK;
	}
}

 /**********************************************************************************
  * 函数功能: 写使能
  */
uint8_t SPI_NOR_WriteEnable(void)
{
	uint8_t cmd[] = {WRITE_ENABLE_CMD};
	uint32_t tickstart = HAL_GetTick();

	/*Select the FLASH: Chip Select low */
	W25Qxx_Enable();
	/* Send the read ID command */
	HAL_SPI_Transmit(&hspi1, cmd, 1, SPI_TIMEOUT_VALUE);
	/*Deselect the FLASH: Chip Select high */
	W25Qxx_Disable();

	/* Wait the end of Flash writing */
	while(W25Qxx_GetStatus() == W25Qxx_BUSY);
	{
		/* Check for the Timeout */
    if((HAL_GetTick() - tickstart) > SPI_TIMEOUT_VALUE)
    {
			return W25Qxx_TIMEOUT;
    }
	}

	return W25Qxx_OK;
}

 /**********************************************************************************
  * 函数功能: 获取设备ID
  */
void SPI_NOR_Read_ID(uint8_t *ID)
{
	uint8_t cmd[4] = {READ_ID_CMD,0x00,0x00,0x00};

	W25Qxx_Enable();
	/* Send the read ID command */
	HAL_SPI_Transmit(&hspi1, cmd, 4, SPI_TIMEOUT_VALUE);
	/* Reception of the data */
	HAL_SPI_Receive(&hspi1,ID, 2, SPI_TIMEOUT_VALUE);
	W25Qxx_Disable();

}

 /**********************************************************************************
  * 函数功能: 读数据
  * 输入参数: 缓存数组指针、读地址、字节数
  */
uint8_t SPI_NOR_Read(uint8_t* pData, uint32_t ReadAddr, uint32_t Size)
{
	uint8_t cmd[4];

	/* Configure the command */
	cmd[0] = READ_CMD;
	cmd[1] = (uint8_t)(ReadAddr >> 16);
	cmd[2] = (uint8_t)(ReadAddr >> 8);
	cmd[3] = (uint8_t)(ReadAddr);

	W25Qxx_Enable();
	/* Send the read ID command */
	HAL_SPI_Transmit(&hspi1, cmd, 4, SPI_TIMEOUT_VALUE);
	/* Reception of the data */
	if (HAL_SPI_Receive(&hspi1, pData,Size,SPI_TIMEOUT_VALUE) != HAL_OK)
  {
    return W25Qxx_ERROR;
  }
	W25Qxx_Disable();
	return W25Qxx_OK;
}

 /**********************************************************************************
  * 函数功能: 写数据
  * 输入参数: 缓存数组指针、写地址、字节数
  */
uint8_t SPI_NOR_Write(uint8_t* pData, uint32_t WriteAddr, uint32_t Size)
{
	uint8_t cmd[4];
	uint32_t end_addr, current_size, current_addr;
	uint32_t tickstart = HAL_GetTick();

	/* Calculation of the size between the write address and the end of the page */
  current_addr = 0;

  while (current_addr <= WriteAddr)
  {
    current_addr += SPI_NOR_PAGE_SIZE;
  }
  current_size = current_addr - WriteAddr;

  /* Check if the size of the data is less than the remaining place in the page */
  if (current_size > Size)
  {
    current_size = Size;
  }

  /* Initialize the adress variables */
  current_addr = WriteAddr;
  end_addr = WriteAddr + Size;

  /* Perform the write page by page */
  do
  {
		/* Configure the command */
		cmd[0] = PAGE_PROG_CMD;
		cmd[1] = (uint8_t)(current_addr >> 16);
		cmd[2] = (uint8_t)(current_addr >> 8);
		cmd[3] = (uint8_t)(current_addr);

		/* Enable write operations */
		SPI_NOR_WriteEnable();

		W25Qxx_Enable();
    /* Send the command */
    if (HAL_SPI_Transmit(&hspi1,cmd, 4, SPI_TIMEOUT_VALUE) != HAL_OK)
    {
      return W25Qxx_ERROR;
    }

    /* Transmission of the data */
    if (HAL_SPI_Transmit(&hspi1, pData,current_size, SPI_TIMEOUT_VALUE) != HAL_OK)
    {
      return W25Qxx_ERROR;
    }

        W25Qxx_Disable();
    	/* Wait the end of Flash writing */
		while(W25Qxx_GetStatus() == W25Qxx_BUSY);
		{
			/* Check for the Timeout */
			if((HAL_GetTick() - tickstart) > SPI_TIMEOUT_VALUE)
			{
				return W25Qxx_TIMEOUT;
			}
		}

    /* Update the address and size variables for next page programming */
    current_addr += current_size;
    pData += current_size;
    current_size = ((current_addr + SPI_NOR_PAGE_SIZE) > end_addr) ? (end_addr - current_addr) : SPI_NOR_PAGE_SIZE;
  } while (current_addr < end_addr);


	return W25Qxx_OK;
}

 /**********************************************************************************
  * 函数功能: 扇区擦除
  * 输入参数: 地址
  */
uint8_t SPI_NOR_Erase_Block(uint32_t Address)
{
	uint8_t cmd[4];
	uint32_t tickstart = HAL_GetTick();
	cmd[0] = SECTOR_ERASE_CMD;
	cmd[1] = (uint8_t)(Address >> 16);
	cmd[2] = (uint8_t)(Address >> 8);
	cmd[3] = (uint8_t)(Address);

	/* Enable write operations */
	SPI_NOR_WriteEnable();

	/*Select the FLASH: Chip Select low */
	W25Qxx_Enable();
	/* Send the read ID command */
	HAL_SPI_Transmit(&hspi1, cmd, 4, SPI_TIMEOUT_VALUE);
	/*Deselect the FLASH: Chip Select high */
	W25Qxx_Disable();

	/* Wait the end of Flash writing */
	while(W25Qxx_GetStatus() == W25Qxx_BUSY);
	{
		/* Check for the Timeout */
    if((HAL_GetTick() - tickstart) > SPI_NOR_SECTOR_ERASE_MAX_TIME)
    {
			return W25Qxx_TIMEOUT;
    }
	}
	return W25Qxx_OK;
}

 /**********************************************************************************
  * 函数功能: 芯片擦除
  */
uint8_t SPI_NOR_Erase_Chip(void)
{
	uint8_t cmd[4];
	uint32_t tickstart = HAL_GetTick();
	cmd[0] = CHIP_ERASE_CMD;

	/* Enable write operations */
	SPI_NOR_WriteEnable();

	/*Select the FLASH: Chip Select low */
	W25Qxx_Enable();
	/* Send the read ID command */
	HAL_SPI_Transmit(&hspi1, cmd, 1, SPI_TIMEOUT_VALUE);
	/*Deselect the FLASH: Chip Select high */
	W25Qxx_Disable();

	/* Wait the end of Flash writing */
	while(W25Qxx_GetStatus() != W25Qxx_BUSY);
	{
		/* Check for the Timeout */
    if((HAL_GetTick() - tickstart) > SPI_NOR_BULK_ERASE_MAX_TIME)
    {
			return W25Qxx_TIMEOUT;
    }
	}
	return W25Qxx_OK;
}

