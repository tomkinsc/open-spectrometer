################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
AsymmetricPWM-DevInit_F2806x.obj: ../AsymmetricPWM-DevInit_F2806x.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccsv5/tools/compiler/c2000/bin/cl2000" -v28 -mt -ml -g --include_path="C:/ti/ccsv5/tools/compiler/c2000/include" --diag_warning=225 --cla_support=cla0 --float_support=fpu32 --vcu_support=vcu0 --preproc_with_compile --preproc_dependency="AsymmetricPWM-DevInit_F2806x.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

AsymmetricPWM-Main.obj: ../AsymmetricPWM-Main.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccsv5/tools/compiler/c2000/bin/cl2000" -v28 -mt -ml -g --include_path="C:/ti/ccsv5/tools/compiler/c2000/include" --diag_warning=225 --cla_support=cla0 --float_support=fpu32 --vcu_support=vcu0 --preproc_with_compile --preproc_dependency="AsymmetricPWM-Main.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

DSP280x_usDelay.obj: ../DSP280x_usDelay.asm $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccsv5/tools/compiler/c2000/bin/cl2000" -v28 -mt -ml -g --include_path="C:/ti/ccsv5/tools/compiler/c2000/include" --diag_warning=225 --cla_support=cla0 --float_support=fpu32 --vcu_support=vcu0 --preproc_with_compile --preproc_dependency="DSP280x_usDelay.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

F2806x_CodeStartBranch.obj: ../F2806x_CodeStartBranch.asm $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccsv5/tools/compiler/c2000/bin/cl2000" -v28 -mt -ml -g --include_path="C:/ti/ccsv5/tools/compiler/c2000/include" --diag_warning=225 --cla_support=cla0 --float_support=fpu32 --vcu_support=vcu0 --preproc_with_compile --preproc_dependency="F2806x_CodeStartBranch.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

F2806x_GlobalVariableDefs.obj: ../F2806x_GlobalVariableDefs.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccsv5/tools/compiler/c2000/bin/cl2000" -v28 -mt -ml -g --include_path="C:/ti/ccsv5/tools/compiler/c2000/include" --diag_warning=225 --cla_support=cla0 --float_support=fpu32 --vcu_support=vcu0 --preproc_with_compile --preproc_dependency="F2806x_GlobalVariableDefs.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


