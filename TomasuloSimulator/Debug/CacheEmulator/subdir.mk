################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../CacheEmulator/EmulateCache.c 

OBJS += \
./CacheEmulator/EmulateCache.o 

C_DEPS += \
./CacheEmulator/EmulateCache.d 


# Each subdirectory must supply rules for building sources it contributes
CacheEmulator/%.o: ../CacheEmulator/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


