################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/Neo_6m.c \
../Core/Src/adc1.c \
../Core/Src/bme280.c \
../Core/Src/clock.c \
../Core/Src/gpio.c \
../Core/Src/hc-sr04.c \
../Core/Src/i2c1.c \
../Core/Src/main.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32f4xx.c \
../Core/Src/tim2.c \
../Core/Src/uart2.c 

OBJS += \
./Core/Src/Neo_6m.o \
./Core/Src/adc1.o \
./Core/Src/bme280.o \
./Core/Src/clock.o \
./Core/Src/gpio.o \
./Core/Src/hc-sr04.o \
./Core/Src/i2c1.o \
./Core/Src/main.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32f4xx.o \
./Core/Src/tim2.o \
./Core/Src/uart2.o 

C_DEPS += \
./Core/Src/Neo_6m.d \
./Core/Src/adc1.d \
./Core/Src/bme280.d \
./Core/Src/clock.d \
./Core/Src/gpio.d \
./Core/Src/hc-sr04.d \
./Core/Src/i2c1.d \
./Core/Src/main.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32f4xx.d \
./Core/Src/tim2.d \
./Core/Src/uart2.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F446xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/Neo_6m.cyclo ./Core/Src/Neo_6m.d ./Core/Src/Neo_6m.o ./Core/Src/Neo_6m.su ./Core/Src/adc1.cyclo ./Core/Src/adc1.d ./Core/Src/adc1.o ./Core/Src/adc1.su ./Core/Src/bme280.cyclo ./Core/Src/bme280.d ./Core/Src/bme280.o ./Core/Src/bme280.su ./Core/Src/clock.cyclo ./Core/Src/clock.d ./Core/Src/clock.o ./Core/Src/clock.su ./Core/Src/gpio.cyclo ./Core/Src/gpio.d ./Core/Src/gpio.o ./Core/Src/gpio.su ./Core/Src/hc-sr04.cyclo ./Core/Src/hc-sr04.d ./Core/Src/hc-sr04.o ./Core/Src/hc-sr04.su ./Core/Src/i2c1.cyclo ./Core/Src/i2c1.d ./Core/Src/i2c1.o ./Core/Src/i2c1.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32f4xx.cyclo ./Core/Src/system_stm32f4xx.d ./Core/Src/system_stm32f4xx.o ./Core/Src/system_stm32f4xx.su ./Core/Src/tim2.cyclo ./Core/Src/tim2.d ./Core/Src/tim2.o ./Core/Src/tim2.su ./Core/Src/uart2.cyclo ./Core/Src/uart2.d ./Core/Src/uart2.o ./Core/Src/uart2.su

.PHONY: clean-Core-2f-Src

