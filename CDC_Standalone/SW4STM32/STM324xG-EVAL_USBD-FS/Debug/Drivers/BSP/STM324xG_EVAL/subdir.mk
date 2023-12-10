################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
D:/CUBE_NANDO_HAL/CUBE_NANDO_HAL/CDC_Standalone/Drivers/BSP/STM324xG_EVAL/stm324xg_eval.c \
D:/CUBE_NANDO_HAL/CUBE_NANDO_HAL/CDC_Standalone/Drivers/BSP/STM324xG_EVAL/stm324xg_eval_io.c 

OBJS += \
./Drivers/BSP/STM324xG_EVAL/stm324xg_eval.o \
./Drivers/BSP/STM324xG_EVAL/stm324xg_eval_io.o 

C_DEPS += \
./Drivers/BSP/STM324xG_EVAL/stm324xg_eval.d \
./Drivers/BSP/STM324xG_EVAL/stm324xg_eval_io.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/STM324xG_EVAL/stm324xg_eval.o: D:/CUBE_NANDO_HAL/CUBE_NANDO_HAL/CDC_Standalone/Drivers/BSP/STM324xG_EVAL/stm324xg_eval.c Drivers/BSP/STM324xG_EVAL/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32F407xx -DUSE_STM324xG_EVAL -DUSE_USB_FS -c -I../../../Inc -I../../../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../../../Drivers/STM32F4xx_HAL_Driver/Inc -I../../../Drivers/BSP/STM324xG_EVAL -I../../../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../../../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../../../Drivers/CMSIS/Include -Os -ffunction-sections -Wall -Wno-unused-variable -Wno-pointer-sign -Wno-main -Wno-format -Wno-address -Wno-unused-but-set-variable -Wno-strict-aliasing -Wno-parentheses -Wno-missing-braces -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Drivers/BSP/STM324xG_EVAL/stm324xg_eval_io.o: D:/CUBE_NANDO_HAL/CUBE_NANDO_HAL/CDC_Standalone/Drivers/BSP/STM324xG_EVAL/stm324xg_eval_io.c Drivers/BSP/STM324xG_EVAL/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32F407xx -DUSE_STM324xG_EVAL -DUSE_USB_FS -c -I../../../Inc -I../../../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../../../Drivers/STM32F4xx_HAL_Driver/Inc -I../../../Drivers/BSP/STM324xG_EVAL -I../../../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../../../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../../../Drivers/CMSIS/Include -Os -ffunction-sections -Wall -Wno-unused-variable -Wno-pointer-sign -Wno-main -Wno-format -Wno-address -Wno-unused-but-set-variable -Wno-strict-aliasing -Wno-parentheses -Wno-missing-braces -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-STM324xG_EVAL

clean-Drivers-2f-BSP-2f-STM324xG_EVAL:
	-$(RM) ./Drivers/BSP/STM324xG_EVAL/stm324xg_eval.cyclo ./Drivers/BSP/STM324xG_EVAL/stm324xg_eval.d ./Drivers/BSP/STM324xG_EVAL/stm324xg_eval.o ./Drivers/BSP/STM324xG_EVAL/stm324xg_eval.su ./Drivers/BSP/STM324xG_EVAL/stm324xg_eval_io.cyclo ./Drivers/BSP/STM324xG_EVAL/stm324xg_eval_io.d ./Drivers/BSP/STM324xG_EVAL/stm324xg_eval_io.o ./Drivers/BSP/STM324xG_EVAL/stm324xg_eval_io.su

.PHONY: clean-Drivers-2f-BSP-2f-STM324xG_EVAL

