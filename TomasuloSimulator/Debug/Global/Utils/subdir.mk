################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Global/Utils/PrintUtility.c \

OBJS += \
./Global/Utils/PrintUtility.o \

C_DEPS += \
./Global/Utils/PrintUtility.d \


# Each subdirectory must supply rules for building sources it contributes
Global/Utils/%.o: ../Global/Utils/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


