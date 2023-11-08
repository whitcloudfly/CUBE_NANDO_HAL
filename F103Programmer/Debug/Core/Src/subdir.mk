################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/cdc.c \
../Core/Src/cdc_endp.c \
../Core/Src/cdc_hwcfg.c \
../Core/Src/clock.c \
../Core/Src/flash.c \
../Core/Src/fsmc.c \
../Core/Src/fsmc_nand.c \
../Core/Src/gpio.c \
../Core/Src/jtag.c \
../Core/Src/led.c \
../Core/Src/main.c \
../Core/Src/nand_bad_block.c \
../Core/Src/nand_programmer.c \
../Core/Src/rtc.c \
../Core/Src/spi.c \
../Core/Src/spi_nand_flash.c \
../Core/Src/spi_nor_flash.c \
../Core/Src/stm32f1xx_hal_msp.c \
../Core/Src/stm32f1xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32f1xx.c \
../Core/Src/usart.c 

OBJS += \
./Core/Src/cdc.o \
./Core/Src/cdc_endp.o \
./Core/Src/cdc_hwcfg.o \
./Core/Src/clock.o \
./Core/Src/flash.o \
./Core/Src/fsmc.o \
./Core/Src/fsmc_nand.o \
./Core/Src/gpio.o \
./Core/Src/jtag.o \
./Core/Src/led.o \
./Core/Src/main.o \
./Core/Src/nand_bad_block.o \
./Core/Src/nand_programmer.o \
./Core/Src/rtc.o \
./Core/Src/spi.o \
./Core/Src/spi_nand_flash.o \
./Core/Src/spi_nor_flash.o \
./Core/Src/stm32f1xx_hal_msp.o \
./Core/Src/stm32f1xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32f1xx.o \
./Core/Src/usart.o 

C_DEPS += \
./Core/Src/cdc.d \
./Core/Src/cdc_endp.d \
./Core/Src/cdc_hwcfg.d \
./Core/Src/clock.d \
./Core/Src/flash.d \
./Core/Src/fsmc.d \
./Core/Src/fsmc_nand.d \
./Core/Src/gpio.d \
./Core/Src/jtag.d \
./Core/Src/led.d \
./Core/Src/main.d \
./Core/Src/nand_bad_block.d \
./Core/Src/nand_programmer.d \
./Core/Src/rtc.d \
./Core/Src/spi.d \
./Core/Src/spi_nand_flash.d \
./Core/Src/spi_nor_flash.d \
./Core/Src/stm32f1xx_hal_msp.d \
./Core/Src/stm32f1xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32f1xx.d \
./Core/Src/usart.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xE -c -I../Core/Inc -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/cdc.cyclo ./Core/Src/cdc.d ./Core/Src/cdc.o ./Core/Src/cdc.su ./Core/Src/cdc_endp.cyclo ./Core/Src/cdc_endp.d ./Core/Src/cdc_endp.o ./Core/Src/cdc_endp.su ./Core/Src/cdc_hwcfg.cyclo ./Core/Src/cdc_hwcfg.d ./Core/Src/cdc_hwcfg.o ./Core/Src/cdc_hwcfg.su ./Core/Src/clock.cyclo ./Core/Src/clock.d ./Core/Src/clock.o ./Core/Src/clock.su ./Core/Src/flash.cyclo ./Core/Src/flash.d ./Core/Src/flash.o ./Core/Src/flash.su ./Core/Src/fsmc.cyclo ./Core/Src/fsmc.d ./Core/Src/fsmc.o ./Core/Src/fsmc.su ./Core/Src/fsmc_nand.cyclo ./Core/Src/fsmc_nand.d ./Core/Src/fsmc_nand.o ./Core/Src/fsmc_nand.su ./Core/Src/gpio.cyclo ./Core/Src/gpio.d ./Core/Src/gpio.o ./Core/Src/gpio.su ./Core/Src/jtag.cyclo ./Core/Src/jtag.d ./Core/Src/jtag.o ./Core/Src/jtag.su ./Core/Src/led.cyclo ./Core/Src/led.d ./Core/Src/led.o ./Core/Src/led.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/nand_bad_block.cyclo ./Core/Src/nand_bad_block.d ./Core/Src/nand_bad_block.o ./Core/Src/nand_bad_block.su ./Core/Src/nand_programmer.cyclo ./Core/Src/nand_programmer.d ./Core/Src/nand_programmer.o ./Core/Src/nand_programmer.su ./Core/Src/rtc.cyclo ./Core/Src/rtc.d ./Core/Src/rtc.o ./Core/Src/rtc.su ./Core/Src/spi.cyclo ./Core/Src/spi.d ./Core/Src/spi.o ./Core/Src/spi.su ./Core/Src/spi_nand_flash.cyclo ./Core/Src/spi_nand_flash.d ./Core/Src/spi_nand_flash.o ./Core/Src/spi_nand_flash.su ./Core/Src/spi_nor_flash.cyclo ./Core/Src/spi_nor_flash.d ./Core/Src/spi_nor_flash.o ./Core/Src/spi_nor_flash.su ./Core/Src/stm32f1xx_hal_msp.cyclo ./Core/Src/stm32f1xx_hal_msp.d ./Core/Src/stm32f1xx_hal_msp.o ./Core/Src/stm32f1xx_hal_msp.su ./Core/Src/stm32f1xx_it.cyclo ./Core/Src/stm32f1xx_it.d ./Core/Src/stm32f1xx_it.o ./Core/Src/stm32f1xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32f1xx.cyclo ./Core/Src/system_stm32f1xx.d ./Core/Src/system_stm32f1xx.o ./Core/Src/system_stm32f1xx.su ./Core/Src/usart.cyclo ./Core/Src/usart.d ./Core/Src/usart.o ./Core/Src/usart.su

.PHONY: clean-Core-2f-Src

