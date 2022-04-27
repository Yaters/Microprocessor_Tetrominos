################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/freertos.c \
../Src/game_logic.c \
../Src/main.c \
../Src/stm32l4xx_hal_msp.c \
../Src/stm32l4xx_hal_timebase_tim.c \
../Src/stm32l4xx_it.c \
../Src/syscalls.c \
../Src/sysmem.c \
../Src/system_stm32l4xx.c \
../Src/tetris.c \
../Src/tetristhemequiet.c 

OBJS += \
./Src/freertos.o \
./Src/game_logic.o \
./Src/main.o \
./Src/stm32l4xx_hal_msp.o \
./Src/stm32l4xx_hal_timebase_tim.o \
./Src/stm32l4xx_it.o \
./Src/syscalls.o \
./Src/sysmem.o \
./Src/system_stm32l4xx.o \
./Src/tetris.o \
./Src/tetristhemequiet.o 

C_DEPS += \
./Src/freertos.d \
./Src/game_logic.d \
./Src/main.d \
./Src/stm32l4xx_hal_msp.d \
./Src/stm32l4xx_hal_timebase_tim.d \
./Src/stm32l4xx_it.d \
./Src/syscalls.d \
./Src/sysmem.d \
./Src/system_stm32l4xx.d \
./Src/tetris.d \
./Src/tetristhemequiet.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o: ../Src/%.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L4S5xx -c -I../Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Src

clean-Src:
	-$(RM) ./Src/freertos.d ./Src/freertos.o ./Src/game_logic.d ./Src/game_logic.o ./Src/main.d ./Src/main.o ./Src/stm32l4xx_hal_msp.d ./Src/stm32l4xx_hal_msp.o ./Src/stm32l4xx_hal_timebase_tim.d ./Src/stm32l4xx_hal_timebase_tim.o ./Src/stm32l4xx_it.d ./Src/stm32l4xx_it.o ./Src/syscalls.d ./Src/syscalls.o ./Src/sysmem.d ./Src/sysmem.o ./Src/system_stm32l4xx.d ./Src/system_stm32l4xx.o ./Src/tetris.d ./Src/tetris.o ./Src/tetristhemequiet.d ./Src/tetristhemequiet.o

.PHONY: clean-Src

