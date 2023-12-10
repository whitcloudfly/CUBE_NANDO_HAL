################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
D:/CUBE_NANDO_HAL/CUBE_NANDO_HAL/CDC_Standalone/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Src/usbd_cdc.c 

OBJS += \
./Middlewares/STM32_USBD_Library/Class/CDC/usbd_cdc.o 

C_DEPS += \
./Middlewares/STM32_USBD_Library/Class/CDC/usbd_cdc.d 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/STM32_USBD_Library/Class/CDC/usbd_cdc.o: D:/CUBE_NANDO_HAL/CUBE_NANDO_HAL/CDC_Standalone/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Src/usbd_cdc.c Middlewares/STM32_USBD_Library/Class/CDC/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32F407xx -DUSE_STM324xG_EVAL -DUSE_USB_FS -c -I../../../Inc -I../../../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../../../Drivers/STM32F4xx_HAL_Driver/Inc -I../../../Drivers/BSP/STM324xG_EVAL -I../../../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../../../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../../../Drivers/CMSIS/Include -Os -ffunction-sections -Wall -Wno-unused-variable -Wno-pointer-sign -Wno-main -Wno-format -Wno-address -Wno-unused-but-set-variable -Wno-strict-aliasing -Wno-parentheses -Wno-missing-braces -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Middlewares-2f-STM32_USBD_Library-2f-Class-2f-CDC

clean-Middlewares-2f-STM32_USBD_Library-2f-Class-2f-CDC:
	-$(RM) ./Middlewares/STM32_USBD_Library/Class/CDC/usbd_cdc.cyclo ./Middlewares/STM32_USBD_Library/Class/CDC/usbd_cdc.d ./Middlewares/STM32_USBD_Library/Class/CDC/usbd_cdc.o ./Middlewares/STM32_USBD_Library/Class/CDC/usbd_cdc.su

.PHONY: clean-Middlewares-2f-STM32_USBD_Library-2f-Class-2f-CDC

