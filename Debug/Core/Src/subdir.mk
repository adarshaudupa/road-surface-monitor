################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/NEO_6M.c \
../Core/Src/esp32_comms.c \
../Core/Src/hc-sr04.c \
../Core/Src/i2c1.c \
../Core/Src/lsm6ds3.c \
../Core/Src/main.c \
../Core/Src/stm32f4xx_hal_msp.c \
../Core/Src/stm32f4xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32f4xx.c \
../Core/Src/tim5.c \
../Core/Src/uart1.c \
../Core/Src/uart2.c \
../Core/Src/uart3.c 

OBJS += \
./Core/Src/NEO_6M.o \
./Core/Src/esp32_comms.o \
./Core/Src/hc-sr04.o \
./Core/Src/i2c1.o \
./Core/Src/lsm6ds3.o \
./Core/Src/main.o \
./Core/Src/stm32f4xx_hal_msp.o \
./Core/Src/stm32f4xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32f4xx.o \
./Core/Src/tim5.o \
./Core/Src/uart1.o \
./Core/Src/uart2.o \
./Core/Src/uart3.o 

C_DEPS += \
./Core/Src/NEO_6M.d \
./Core/Src/esp32_comms.d \
./Core/Src/hc-sr04.d \
./Core/Src/i2c1.d \
./Core/Src/lsm6ds3.d \
./Core/Src/main.d \
./Core/Src/stm32f4xx_hal_msp.d \
./Core/Src/stm32f4xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32f4xx.d \
./Core/Src/tim5.d \
./Core/Src/uart1.d \
./Core/Src/uart2.d \
./Core/Src/uart3.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F446xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/NEO_6M.cyclo ./Core/Src/NEO_6M.d ./Core/Src/NEO_6M.o ./Core/Src/NEO_6M.su ./Core/Src/esp32_comms.cyclo ./Core/Src/esp32_comms.d ./Core/Src/esp32_comms.o ./Core/Src/esp32_comms.su ./Core/Src/hc-sr04.cyclo ./Core/Src/hc-sr04.d ./Core/Src/hc-sr04.o ./Core/Src/hc-sr04.su ./Core/Src/i2c1.cyclo ./Core/Src/i2c1.d ./Core/Src/i2c1.o ./Core/Src/i2c1.su ./Core/Src/lsm6ds3.cyclo ./Core/Src/lsm6ds3.d ./Core/Src/lsm6ds3.o ./Core/Src/lsm6ds3.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/stm32f4xx_hal_msp.cyclo ./Core/Src/stm32f4xx_hal_msp.d ./Core/Src/stm32f4xx_hal_msp.o ./Core/Src/stm32f4xx_hal_msp.su ./Core/Src/stm32f4xx_it.cyclo ./Core/Src/stm32f4xx_it.d ./Core/Src/stm32f4xx_it.o ./Core/Src/stm32f4xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32f4xx.cyclo ./Core/Src/system_stm32f4xx.d ./Core/Src/system_stm32f4xx.o ./Core/Src/system_stm32f4xx.su ./Core/Src/tim5.cyclo ./Core/Src/tim5.d ./Core/Src/tim5.o ./Core/Src/tim5.su ./Core/Src/uart1.cyclo ./Core/Src/uart1.d ./Core/Src/uart1.o ./Core/Src/uart1.su ./Core/Src/uart2.cyclo ./Core/Src/uart2.d ./Core/Src/uart2.o ./Core/Src/uart2.su ./Core/Src/uart3.cyclo ./Core/Src/uart3.d ./Core/Src/uart3.o ./Core/Src/uart3.su

.PHONY: clean-Core-2f-Src

