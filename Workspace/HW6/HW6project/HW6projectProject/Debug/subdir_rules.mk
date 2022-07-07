################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
UARTfuncs.obj: E:/Todo/SE423_Mechatronics/Workspace/HW6/HW6project/UARTfuncs.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: MSP430 Compiler'
	"D:/CodeComposerStudio/ccsv8/tools/compiler/ti-cgt-msp430_18.1.4.LTS/bin/cl430" -vmsp -O0 --include_path="D:/CodeComposerStudio/ccsv8/ccs_base/msp430/include" --include_path="D:/CodeComposerStudio/ccsv8/tools/compiler/ti-cgt-msp430_18.1.4.LTS/include" --define=__MSP430G2553__ -g --printf_support=nofloat --diag_warning=225 --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

user_HW6project.obj: E:/Todo/SE423_Mechatronics/Workspace/HW6/HW6project/user_HW6project.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: MSP430 Compiler'
	"D:/CodeComposerStudio/ccsv8/tools/compiler/ti-cgt-msp430_18.1.4.LTS/bin/cl430" -vmsp -O0 --include_path="D:/CodeComposerStudio/ccsv8/ccs_base/msp430/include" --include_path="D:/CodeComposerStudio/ccsv8/tools/compiler/ti-cgt-msp430_18.1.4.LTS/include" --define=__MSP430G2553__ -g --printf_support=nofloat --diag_warning=225 --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


