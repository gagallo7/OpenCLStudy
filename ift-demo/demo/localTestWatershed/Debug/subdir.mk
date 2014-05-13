################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../watershed.c 

OBJS += \
./watershed.o 

C_DEPS += \
./watershed.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: NVCC Compiler'
	/usr/local/cuda-5.5/bin/nvcc -I/home/ggallo/Developing/git/OpenCLStudy/ift-demo/include -G -g -O0 -gencode arch=compute_30,code=sm_30 -odir "" -M -o "$(@:%.o=%.d)" "$<"
	/usr/local/cuda-5.5/bin/nvcc -I/home/ggallo/Developing/git/OpenCLStudy/ift-demo/include -G -g -O0 --compile  -x c -o  "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


