################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
config_AIC23.obj: ../config_AIC23.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/ti/ccsv6/tools/compiler/c6000_7.4.14/bin/cl6x" -mv6700 --abi=coffabi -g --include_path="C:/ti/ccsv6/tools/compiler/c6000_7.4.14/include" --include_path="D:/Workspace/DSP_ADPCM/my_dsp_project/Debug" --include_path="C:/ti/bios_5_42_01_09/packages/ti/bios/include" --include_path="C:/ti/bios_5_42_01_09/packages/ti/rtdx/include/c6000" --include_path="C:/ti/C6xCSL/include" --include_path="C:/ti/DSPLIB/c6700/dsplib/include" --include_path="C:/ti/DSK6713/c6000/dsk6713/include" --define=CHIP_6713 --display_error_number --diag_warning=225 --diag_wrap=off --preproc_with_compile --preproc_dependency="config_AIC23.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

config_BSPLink.obj: ../config_BSPLink.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/ti/ccsv6/tools/compiler/c6000_7.4.14/bin/cl6x" -mv6700 --abi=coffabi -g --include_path="C:/ti/ccsv6/tools/compiler/c6000_7.4.14/include" --include_path="D:/Workspace/DSP_ADPCM/my_dsp_project/Debug" --include_path="C:/ti/bios_5_42_01_09/packages/ti/bios/include" --include_path="C:/ti/bios_5_42_01_09/packages/ti/rtdx/include/c6000" --include_path="C:/ti/C6xCSL/include" --include_path="C:/ti/DSPLIB/c6700/dsplib/include" --include_path="C:/ti/DSK6713/c6000/dsk6713/include" --define=CHIP_6713 --display_error_number --diag_warning=225 --diag_wrap=off --preproc_with_compile --preproc_dependency="config_BSPLink.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

my_dsp_projectcfg.cmd: ../my_dsp_project.tcf
	@echo 'Building file: $<'
	@echo 'Invoking: TConf'
	"C:/ti/bios_5_42_01_09/xdctools/tconf" -b -Dconfig.importPath="C:/ti/bios_5_42_01_09/packages;" "$<"
	@echo 'Finished building: $<'
	@echo ' '

my_dsp_projectcfg.s??: | my_dsp_projectcfg.cmd
my_dsp_projectcfg_c.c: | my_dsp_projectcfg.cmd
my_dsp_projectcfg.h: | my_dsp_projectcfg.cmd
my_dsp_projectcfg.h??: | my_dsp_projectcfg.cmd
my_dsp_project.cdb: | my_dsp_projectcfg.cmd

my_dsp_projectcfg.obj: ./my_dsp_projectcfg.s?? $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/ti/ccsv6/tools/compiler/c6000_7.4.14/bin/cl6x" -mv6700 --abi=coffabi -g --include_path="C:/ti/ccsv6/tools/compiler/c6000_7.4.14/include" --include_path="D:/Workspace/DSP_ADPCM/my_dsp_project/Debug" --include_path="C:/ti/bios_5_42_01_09/packages/ti/bios/include" --include_path="C:/ti/bios_5_42_01_09/packages/ti/rtdx/include/c6000" --include_path="C:/ti/C6xCSL/include" --include_path="C:/ti/DSPLIB/c6700/dsplib/include" --include_path="C:/ti/DSK6713/c6000/dsk6713/include" --define=CHIP_6713 --display_error_number --diag_warning=225 --diag_wrap=off --preproc_with_compile --preproc_dependency="my_dsp_projectcfg.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

my_dsp_projectcfg_c.obj: ./my_dsp_projectcfg_c.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/ti/ccsv6/tools/compiler/c6000_7.4.14/bin/cl6x" -mv6700 --abi=coffabi -g --include_path="C:/ti/ccsv6/tools/compiler/c6000_7.4.14/include" --include_path="D:/Workspace/DSP_ADPCM/my_dsp_project/Debug" --include_path="C:/ti/bios_5_42_01_09/packages/ti/bios/include" --include_path="C:/ti/bios_5_42_01_09/packages/ti/rtdx/include/c6000" --include_path="C:/ti/C6xCSL/include" --include_path="C:/ti/DSPLIB/c6700/dsplib/include" --include_path="C:/ti/DSK6713/c6000/dsk6713/include" --define=CHIP_6713 --display_error_number --diag_warning=225 --diag_wrap=off --preproc_with_compile --preproc_dependency="my_dsp_projectcfg_c.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

skeleton.obj: ../skeleton.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/ti/ccsv6/tools/compiler/c6000_7.4.14/bin/cl6x" -mv6700 --abi=coffabi -g --include_path="C:/ti/ccsv6/tools/compiler/c6000_7.4.14/include" --include_path="D:/Workspace/DSP_ADPCM/my_dsp_project/Debug" --include_path="C:/ti/bios_5_42_01_09/packages/ti/bios/include" --include_path="C:/ti/bios_5_42_01_09/packages/ti/rtdx/include/c6000" --include_path="C:/ti/C6xCSL/include" --include_path="C:/ti/DSPLIB/c6700/dsplib/include" --include_path="C:/ti/DSK6713/c6000/dsk6713/include" --define=CHIP_6713 --display_error_number --diag_warning=225 --diag_wrap=off --preproc_with_compile --preproc_dependency="skeleton.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


