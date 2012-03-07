################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../leddisplay.c \
../main.c \
Z:/Projekt/Libs/texas/radiocc2500/radio.c 

CMD_SRCS += \
../lnk_msp430f2274.cmd 

OBJS += \
./leddisplay.obj \
./main.obj \
./radio.obj 

C_DEPS += \
./leddisplay.pp \
./main.pp \
./radio.pp 

OBJS_QUOTED += \
"./leddisplay.obj" \
"./main.obj" \
"./radio.obj" 


# Each subdirectory must supply rules for building sources it contributes
leddisplay.obj: ../leddisplay.c $(GEN_SRCS) $(GEN_OPTS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler v3.1'
	"C:/Program Files/Texas Instruments/CC Essentials v3.1/tools/compiler/MSP430/bin/cl430" $(GEN_OPTS_QUOTED) --fp_reassoc=off --include_path="C:/Program Files/Texas Instruments/CC Essentials v3.1/tools/compiler/MSP430/include" --sat_reassoc=off --symdebug:dwarf --plain_char=unsigned --silicon_version=msp --printf_support=full --preproc_with_compile --preproc_dependency="leddisplay.pp" $(subst #,$(wildcard $(subst $(SPACE),\$(SPACE),$<)),"#")
	@echo 'Finished building: $<'
	@echo ' '

main.obj: ../main.c $(GEN_SRCS) $(GEN_OPTS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler v3.1'
	"C:/Program Files/Texas Instruments/CC Essentials v3.1/tools/compiler/MSP430/bin/cl430" $(GEN_OPTS_QUOTED) --fp_reassoc=off --include_path="C:/Program Files/Texas Instruments/CC Essentials v3.1/tools/compiler/MSP430/include" --sat_reassoc=off --symdebug:dwarf --plain_char=unsigned --silicon_version=msp --printf_support=full --preproc_with_compile --preproc_dependency="main.pp" $(subst #,$(wildcard $(subst $(SPACE),\$(SPACE),$<)),"#")
	@echo 'Finished building: $<'
	@echo ' '

radio.obj: Z:/Projekt/Libs/texas/radiocc2500/radio.c $(GEN_SRCS) $(GEN_OPTS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler v3.1'
	"C:/Program Files/Texas Instruments/CC Essentials v3.1/tools/compiler/MSP430/bin/cl430" $(GEN_OPTS_QUOTED) --fp_reassoc=off --include_path="C:/Program Files/Texas Instruments/CC Essentials v3.1/tools/compiler/MSP430/include" --sat_reassoc=off --symdebug:dwarf --plain_char=unsigned --silicon_version=msp --printf_support=full --preproc_with_compile --preproc_dependency="radio.pp" $(subst #,$(wildcard $(subst $(SPACE),\$(SPACE),$<)),"#")
	@echo 'Finished building: $<'
	@echo ' '


