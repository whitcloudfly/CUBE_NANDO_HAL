################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../nando/cdc.c \
../nando/cdc_endp.c \
../nando/cdc_hwcfg.c \
../nando/clock.c \
../nando/flash.c \
../nando/fsmc_nand.c \
../nando/jtag.c \
../nando/led.c \
../nando/nand_bad_block.c \
../nando/nand_programmer.c \
../nando/spi_nand.c \
../nando/spi_nor.c 

OBJS += \
./nando/cdc.o \
./nando/cdc_endp.o \
./nando/cdc_hwcfg.o \
./nando/clock.o \
./nando/flash.o \
./nando/fsmc_nand.o \
./nando/jtag.o \
./nando/led.o \
./nando/nand_bad_block.o \
./nando/nand_programmer.o \
./nando/spi_nand.o \
./nando/spi_nor.o 

C_DEPS += \
./nando/cdc.d \
./nando/cdc_endp.d \
./nando/cdc_hwcfg.d \
./nando/clock.d \
./nando/flash.d \
./nando/fsmc_nand.d \
./nando/jtag.d \
./nando/led.d \
./nando/nand_bad_block.d \
./nando/nand_programmer.d \
./nando/spi_nand.d \
./nando/spi_nor.d 


# Each subdirectory must supply rules for building sources it contributes
nando/%.o nando/%.su nando/%.cyclo: ../nando/%.c nando/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I"D:/workspace/NANDO_USBFS/nando" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@"  -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-nando

clean-nando:
	-$(RM) ./nando/cdc.cyclo ./nando/cdc.d ./nando/cdc.o ./nando/cdc.su ./nando/cdc_endp.cyclo ./nando/cdc_endp.d ./nando/cdc_endp.o ./nando/cdc_endp.su ./nando/cdc_hwcfg.cyclo ./nando/cdc_hwcfg.d ./nando/cdc_hwcfg.o ./nando/cdc_hwcfg.su ./nando/clock.cyclo ./nando/clock.d ./nando/clock.o ./nando/clock.su ./nando/flash.cyclo ./nando/flash.d ./nando/flash.o ./nando/flash.su ./nando/fsmc_nand.cyclo ./nando/fsmc_nand.d ./nando/fsmc_nand.o ./nando/fsmc_nand.su ./nando/jtag.cyclo ./nando/jtag.d ./nando/jtag.o ./nando/jtag.su ./nando/led.cyclo ./nando/led.d ./nando/led.o ./nando/led.su ./nando/nand_bad_block.cyclo ./nando/nand_bad_block.d ./nando/nand_bad_block.o ./nando/nand_bad_block.su ./nando/nand_programmer.cyclo ./nando/nand_programmer.d ./nando/nand_programmer.o ./nando/nand_programmer.su ./nando/spi_nand.cyclo ./nando/spi_nand.d ./nando/spi_nand.o ./nando/spi_nand.su ./nando/spi_nor.cyclo ./nando/spi_nor.d ./nando/spi_nor.o ./nando/spi_nor.su

.PHONY: clean-nando

