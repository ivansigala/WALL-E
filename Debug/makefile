################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include utilities/str/subdir.mk
-include utilities/debug_console_lite/subdir.mk
-include utilities/subdir.mk
-include startup/subdir.mk
-include source/subdir.mk
-include drivers/subdir.mk
-include device/subdir.mk
-include component/uart/subdir.mk
-include board/subdir.mk
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := ADC_FOR_Joysticks_lpadc_interrupt_cm33_core0
BUILD_ARTIFACT_EXTENSION := axf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all:
	+@$(MAKE) --no-print-directory main-build && $(MAKE) --no-print-directory post-build

# Main-build Target
main-build: ADC_FOR_Joysticks_lpadc_interrupt_cm33_core0.axf

# Tool invocations
ADC_FOR_Joysticks_lpadc_interrupt_cm33_core0.axf: $(OBJS) $(USER_OBJS) makefile $(OPTIONAL_TOOL_DEPS)
	@echo 'Building target: $@'
	@echo 'Invoking: MCU Linker'
	arm-none-eabi-gcc -nostdlib -Xlinker -no-warn-rwx-segments -Xlinker -Map="ADC_FOR_Joysticks_lpadc_interrupt_cm33_core0.map" -Xlinker --gc-sections -Xlinker -print-memory-usage -Xlinker --sort-section=alignment -Xlinker --cref -mcpu=cortex-m33 -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -T ADC_FOR_Joysticks_lpadc_interrupt_cm33_core0_Debug.ld -o "ADC_FOR_Joysticks_lpadc_interrupt_cm33_core0.axf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) ADC_FOR_Joysticks_lpadc_interrupt_cm33_core0.axf
	-@echo ' '

post-build:
	-@echo 'Performing post-build steps'
	-arm-none-eabi-size "ADC_FOR_Joysticks_lpadc_interrupt_cm33_core0.axf"; # arm-none-eabi-objcopy -v -O binary "ADC_FOR_Joysticks_lpadc_interrupt_cm33_core0.axf" "ADC_FOR_Joysticks_lpadc_interrupt_cm33_core0.bin" ; # checksum -p MCXN947 -d "ADC_FOR_Joysticks_lpadc_interrupt_cm33_core0.bin";
	-@echo ' '

.PHONY: all clean dependents main-build post-build

-include ../makefile.targets
