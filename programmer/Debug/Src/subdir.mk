################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/cdc.c \
../Src/cdc_endp.c \
../Src/cdc_hwcfg.c \
../Src/clock.c \
../Src/flash.c \
../Src/fsmc.c \
../Src/fsmc_nand.c \
../Src/gpio.c \
../Src/jtag.c \
../Src/led.c \
../Src/main.c \
../Src/nand_bad_block.c \
../Src/nand_programmer.c \
../Src/rtc.c \
../Src/spi.c \
../Src/spi_nand_flash.c \
../Src/spi_nor_flash.c \
../Src/stm32f4xx_hal_msp.c \
../Src/stm32f4xx_it.c \
../Src/syscalls.c \
../Src/sysmem.c \
../Src/system_stm32f4xx.c \
../Src/usart.c \
../Src/usb_device.c \
../Src/usbd_cdc_if.c \
../Src/usbd_conf.c \
../Src/usbd_desc.c \
../Src/usbd_init.c 

OBJS += \
./Src/cdc.o \
./Src/cdc_endp.o \
./Src/cdc_hwcfg.o \
./Src/clock.o \
./Src/flash.o \
./Src/fsmc.o \
./Src/fsmc_nand.o \
./Src/gpio.o \
./Src/jtag.o \
./Src/led.o \
./Src/main.o \
./Src/nand_bad_block.o \
./Src/nand_programmer.o \
./Src/rtc.o \
./Src/spi.o \
./Src/spi_nand_flash.o \
./Src/spi_nor_flash.o \
./Src/stm32f4xx_hal_msp.o \
./Src/stm32f4xx_it.o \
./Src/syscalls.o \
./Src/sysmem.o \
./Src/system_stm32f4xx.o \
./Src/usart.o \
./Src/usb_device.o \
./Src/usbd_cdc_if.o \
./Src/usbd_conf.o \
./Src/usbd_desc.o \
./Src/usbd_init.o 

C_DEPS += \
./Src/cdc.d \
./Src/cdc_endp.d \
./Src/cdc_hwcfg.d \
./Src/clock.d \
./Src/flash.d \
./Src/fsmc.d \
./Src/fsmc_nand.d \
./Src/gpio.d \
./Src/jtag.d \
./Src/led.d \
./Src/main.d \
./Src/nand_bad_block.d \
./Src/nand_programmer.d \
./Src/rtc.d \
./Src/spi.d \
./Src/spi_nand_flash.d \
./Src/spi_nor_flash.d \
./Src/stm32f4xx_hal_msp.d \
./Src/stm32f4xx_it.d \
./Src/syscalls.d \
./Src/sysmem.d \
./Src/system_stm32f4xx.d \
./Src/usart.d \
./Src/usb_device.d \
./Src/usbd_cdc_if.d \
./Src/usbd_conf.d \
./Src/usbd_desc.d \
./Src/usbd_init.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o Src/%.su Src/%.cyclo: ../Src/%.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Src

clean-Src:
	-$(RM) ./Src/cdc.cyclo ./Src/cdc.d ./Src/cdc.o ./Src/cdc.su ./Src/cdc_endp.cyclo ./Src/cdc_endp.d ./Src/cdc_endp.o ./Src/cdc_endp.su ./Src/cdc_hwcfg.cyclo ./Src/cdc_hwcfg.d ./Src/cdc_hwcfg.o ./Src/cdc_hwcfg.su ./Src/clock.cyclo ./Src/clock.d ./Src/clock.o ./Src/clock.su ./Src/flash.cyclo ./Src/flash.d ./Src/flash.o ./Src/flash.su ./Src/fsmc.cyclo ./Src/fsmc.d ./Src/fsmc.o ./Src/fsmc.su ./Src/fsmc_nand.cyclo ./Src/fsmc_nand.d ./Src/fsmc_nand.o ./Src/fsmc_nand.su ./Src/gpio.cyclo ./Src/gpio.d ./Src/gpio.o ./Src/gpio.su ./Src/jtag.cyclo ./Src/jtag.d ./Src/jtag.o ./Src/jtag.su ./Src/led.cyclo ./Src/led.d ./Src/led.o ./Src/led.su ./Src/main.cyclo ./Src/main.d ./Src/main.o ./Src/main.su ./Src/nand_bad_block.cyclo ./Src/nand_bad_block.d ./Src/nand_bad_block.o ./Src/nand_bad_block.su ./Src/nand_programmer.cyclo ./Src/nand_programmer.d ./Src/nand_programmer.o ./Src/nand_programmer.su ./Src/rtc.cyclo ./Src/rtc.d ./Src/rtc.o ./Src/rtc.su ./Src/spi.cyclo ./Src/spi.d ./Src/spi.o ./Src/spi.su ./Src/spi_nand_flash.cyclo ./Src/spi_nand_flash.d ./Src/spi_nand_flash.o ./Src/spi_nand_flash.su ./Src/spi_nor_flash.cyclo ./Src/spi_nor_flash.d ./Src/spi_nor_flash.o ./Src/spi_nor_flash.su ./Src/stm32f4xx_hal_msp.cyclo ./Src/stm32f4xx_hal_msp.d ./Src/stm32f4xx_hal_msp.o ./Src/stm32f4xx_hal_msp.su ./Src/stm32f4xx_it.cyclo ./Src/stm32f4xx_it.d ./Src/stm32f4xx_it.o ./Src/stm32f4xx_it.su ./Src/syscalls.cyclo ./Src/syscalls.d ./Src/syscalls.o ./Src/syscalls.su ./Src/sysmem.cyclo ./Src/sysmem.d ./Src/sysmem.o ./Src/sysmem.su ./Src/system_stm32f4xx.cyclo ./Src/system_stm32f4xx.d ./Src/system_stm32f4xx.o ./Src/system_stm32f4xx.su ./Src/usart.cyclo ./Src/usart.d ./Src/usart.o ./Src/usart.su ./Src/usb_device.cyclo ./Src/usb_device.d ./Src/usb_device.o ./Src/usb_device.su ./Src/usbd_cdc_if.cyclo ./Src/usbd_cdc_if.d ./Src/usbd_cdc_if.o ./Src/usbd_cdc_if.su ./Src/usbd_conf.cyclo ./Src/usbd_conf.d ./Src/usbd_conf.o ./Src/usbd_conf.su ./Src/usbd_desc.cyclo ./Src/usbd_desc.d ./Src/usbd_desc.o ./Src/usbd_desc.su ./Src/usbd_init.cyclo ./Src/usbd_init.d ./Src/usbd_init.o ./Src/usbd_init.su

.PHONY: clean-Src

