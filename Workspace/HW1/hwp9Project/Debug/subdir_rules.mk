################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
UARTfuncs.obj: E:/Todo/SE423_Mechatronics/hwp9/UARTfuncs.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: MSP430 Compiler'
	"D:/Code Composer Studio/ccsv8/tools/compiler/ti-cgt-msp430_18.1.4.LTS/bin/cl430" -vmsp -O0 --include_path="D:/Code Composer Studio/ccsv8/ccs_base/msp430/include" --include_path="D:/Code Composer Studio/ccsv8/tools/compiler/ti-cgt-msp430_18.1.4.LTS/include" --define=__MSP430G2553__ -g --printf_support=nofloat --diag_warning=225 --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

user_hwp9.obj: E:/Todo/SE423_Mechatronics/hwp9/user_hwp9.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: MSP430 Compiler'
	"D:/Code Composer Studio/ccsv8/tools/compiler/ti-cgt-msp430_18.1.4.LTS/bin/cl430" -vmsp -O0 --include_path="D:/Code Composer Studio/ccsv8/ccs_base/msp430/include" --include_path="D:/Code Composer Studio/ccsv8/tools/compiler/ti-cgt-msp430_18.1.4.LTS/include" --define=__MSP430G2553__ -g --printf_support=nofloat --diag_warning=225 --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


