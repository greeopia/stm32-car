#################################################################
# AUTO GENERATE BY unify_builder v3.11.1.0
#################################################################

# Usage:
#  - Build:	make prebuild && make COMPILER_DIR=<dir path>
#  - Clean:	make clean

# Targets Dependences Chain:
#  prebuild
#  all -> postbuild -> bin -> elf -> *.o

# Use 'make V=1' to see the full commands
ifdef V
	Q = 
else
	Q = @
endif

COMPILER_DIR ?= /D/keil_v5/ARM/ARMCC
ifneq ($(COMPILER_DIR),)
	_PATH_TMP:=$(COMPILER_DIR)/bin:$(PATH)
	export PATH=$(_PATH_TMP)
endif

EXE?=
CC=$(COMPILER_DIR)/bin/armcc$(EXE)
AS=$(COMPILER_DIR)/bin/armasm$(EXE)
CXX=$(COMPILER_DIR)/bin/armcc$(EXE)
LD=$(COMPILER_DIR)/bin/armlink$(EXE)
AR=$(COMPILER_DIR)/bin/armar$(EXE)

###########################
# targets
###########################

COLOR_END=\033[0m
COLOR_ERR=\033[31;1m
COLOR_WRN=\033[33;1m
COLOR_SUC=\033[32;1m
COLOR_INF=\033[34;1m

.PHONY : all postbuild bin elf prebuild clean

all: postbuild
	@echo ==========
	@printf "$(COLOR_SUC)ALL DONE.$(COLOR_END)\n"
	@echo ==========

clean:
	-rm -fv ./build/Target_1/*.elf ./build/Target_1/*.axf ./build/Target_1/*.out
	-rm -fv ./build/Target_1/*.hex ./build/Target_1/*.bin ./build/Target_1/*.s19
	-rm -rfv ./build/Target_1/.obj

prebuild:
	@printf "$(COLOR_INF)prebuild ...$(COLOR_END)\n"
	@echo nothing to prebuild.

./build/Target_1/.obj/Library:
	$(Q)mkdir -p $@
-include ./build/Target_1/.obj/Library/misc.d
./build/Target_1/.obj/Library/misc.o: ./Library/misc.c Makefile | ./build/Target_1/.obj/Library
	@echo compiling $< ...
	$(Q)$(CC) -c --apcs=interwork -I. -IStart -IUser -ILibrary -I.cmsis/include -IRTE/_Target_1 -D"USE_STDPERIPH_DRIVER" -D"ARM_MATH_CM3" -D"STM32F10X_MD" --cpu Cortex-M3 --li --c99 -D__MICROLIB -O2 --split_sections --gnu --no-multibyte-chars -g -o ./build/Target_1/.obj/Library/misc.o --no_depend_system_headers --depend ./build/Target_1/.obj/Library/misc.d ./Library/misc.c

-include ./build/Target_1/.obj/Library/stm32f10x_adc.d
./build/Target_1/.obj/Library/stm32f10x_adc.o: ./Library/stm32f10x_adc.c Makefile | ./build/Target_1/.obj/Library
	@echo compiling $< ...
	$(Q)$(CC) -c --apcs=interwork -I. -IStart -IUser -ILibrary -I.cmsis/include -IRTE/_Target_1 -D"USE_STDPERIPH_DRIVER" -D"ARM_MATH_CM3" -D"STM32F10X_MD" --cpu Cortex-M3 --li --c99 -D__MICROLIB -O2 --split_sections --gnu --no-multibyte-chars -g -o ./build/Target_1/.obj/Library/stm32f10x_adc.o --no_depend_system_headers --depend ./build/Target_1/.obj/Library/stm32f10x_adc.d ./Library/stm32f10x_adc.c

-include ./build/Target_1/.obj/Library/stm32f10x_bkp.d
./build/Target_1/.obj/Library/stm32f10x_bkp.o: ./Library/stm32f10x_bkp.c Makefile | ./build/Target_1/.obj/Library
	@echo compiling $< ...
	$(Q)$(CC) -c --apcs=interwork -I. -IStart -IUser -ILibrary -I.cmsis/include -IRTE/_Target_1 -D"USE_STDPERIPH_DRIVER" -D"ARM_MATH_CM3" -D"STM32F10X_MD" --cpu Cortex-M3 --li --c99 -D__MICROLIB -O2 --split_sections --gnu --no-multibyte-chars -g -o ./build/Target_1/.obj/Library/stm32f10x_bkp.o --no_depend_system_headers --depend ./build/Target_1/.obj/Library/stm32f10x_bkp.d ./Library/stm32f10x_bkp.c

-include ./build/Target_1/.obj/Library/stm32f10x_can.d
./build/Target_1/.obj/Library/stm32f10x_can.o: ./Library/stm32f10x_can.c Makefile | ./build/Target_1/.obj/Library
	@echo compiling $< ...
	$(Q)$(CC) -c --apcs=interwork -I. -IStart -IUser -ILibrary -I.cmsis/include -IRTE/_Target_1 -D"USE_STDPERIPH_DRIVER" -D"ARM_MATH_CM3" -D"STM32F10X_MD" --cpu Cortex-M3 --li --c99 -D__MICROLIB -O2 --split_sections --gnu --no-multibyte-chars -g -o ./build/Target_1/.obj/Library/stm32f10x_can.o --no_depend_system_headers --depend ./build/Target_1/.obj/Library/stm32f10x_can.d ./Library/stm32f10x_can.c

-include ./build/Target_1/.obj/Library/stm32f10x_cec.d
./build/Target_1/.obj/Library/stm32f10x_cec.o: ./Library/stm32f10x_cec.c Makefile | ./build/Target_1/.obj/Library
	@echo compiling $< ...
	$(Q)$(CC) -c --apcs=interwork -I. -IStart -IUser -ILibrary -I.cmsis/include -IRTE/_Target_1 -D"USE_STDPERIPH_DRIVER" -D"ARM_MATH_CM3" -D"STM32F10X_MD" --cpu Cortex-M3 --li --c99 -D__MICROLIB -O2 --split_sections --gnu --no-multibyte-chars -g -o ./build/Target_1/.obj/Library/stm32f10x_cec.o --no_depend_system_headers --depend ./build/Target_1/.obj/Library/stm32f10x_cec.d ./Library/stm32f10x_cec.c

-include ./build/Target_1/.obj/Library/stm32f10x_crc.d
./build/Target_1/.obj/Library/stm32f10x_crc.o: ./Library/stm32f10x_crc.c Makefile | ./build/Target_1/.obj/Library
	@echo compiling $< ...
	$(Q)$(CC) -c --apcs=interwork -I. -IStart -IUser -ILibrary -I.cmsis/include -IRTE/_Target_1 -D"USE_STDPERIPH_DRIVER" -D"ARM_MATH_CM3" -D"STM32F10X_MD" --cpu Cortex-M3 --li --c99 -D__MICROLIB -O2 --split_sections --gnu --no-multibyte-chars -g -o ./build/Target_1/.obj/Library/stm32f10x_crc.o --no_depend_system_headers --depend ./build/Target_1/.obj/Library/stm32f10x_crc.d ./Library/stm32f10x_crc.c

-include ./build/Target_1/.obj/Library/stm32f10x_dac.d
./build/Target_1/.obj/Library/stm32f10x_dac.o: ./Library/stm32f10x_dac.c Makefile | ./build/Target_1/.obj/Library
	@echo compiling $< ...
	$(Q)$(CC) -c --apcs=interwork -I. -IStart -IUser -ILibrary -I.cmsis/include -IRTE/_Target_1 -D"USE_STDPERIPH_DRIVER" -D"ARM_MATH_CM3" -D"STM32F10X_MD" --cpu Cortex-M3 --li --c99 -D__MICROLIB -O2 --split_sections --gnu --no-multibyte-chars -g -o ./build/Target_1/.obj/Library/stm32f10x_dac.o --no_depend_system_headers --depend ./build/Target_1/.obj/Library/stm32f10x_dac.d ./Library/stm32f10x_dac.c

-include ./build/Target_1/.obj/Library/stm32f10x_dbgmcu.d
./build/Target_1/.obj/Library/stm32f10x_dbgmcu.o: ./Library/stm32f10x_dbgmcu.c Makefile | ./build/Target_1/.obj/Library
	@echo compiling $< ...
	$(Q)$(CC) -c --apcs=interwork -I. -IStart -IUser -ILibrary -I.cmsis/include -IRTE/_Target_1 -D"USE_STDPERIPH_DRIVER" -D"ARM_MATH_CM3" -D"STM32F10X_MD" --cpu Cortex-M3 --li --c99 -D__MICROLIB -O2 --split_sections --gnu --no-multibyte-chars -g -o ./build/Target_1/.obj/Library/stm32f10x_dbgmcu.o --no_depend_system_headers --depend ./build/Target_1/.obj/Library/stm32f10x_dbgmcu.d ./Library/stm32f10x_dbgmcu.c

-include ./build/Target_1/.obj/Library/stm32f10x_dma.d
./build/Target_1/.obj/Library/stm32f10x_dma.o: ./Library/stm32f10x_dma.c Makefile | ./build/Target_1/.obj/Library
	@echo compiling $< ...
	$(Q)$(CC) -c --apcs=interwork -I. -IStart -IUser -ILibrary -I.cmsis/include -IRTE/_Target_1 -D"USE_STDPERIPH_DRIVER" -D"ARM_MATH_CM3" -D"STM32F10X_MD" --cpu Cortex-M3 --li --c99 -D__MICROLIB -O2 --split_sections --gnu --no-multibyte-chars -g -o ./build/Target_1/.obj/Library/stm32f10x_dma.o --no_depend_system_headers --depend ./build/Target_1/.obj/Library/stm32f10x_dma.d ./Library/stm32f10x_dma.c

-include ./build/Target_1/.obj/Library/stm32f10x_exti.d
./build/Target_1/.obj/Library/stm32f10x_exti.o: ./Library/stm32f10x_exti.c Makefile | ./build/Target_1/.obj/Library
	@echo compiling $< ...
	$(Q)$(CC) -c --apcs=interwork -I. -IStart -IUser -ILibrary -I.cmsis/include -IRTE/_Target_1 -D"USE_STDPERIPH_DRIVER" -D"ARM_MATH_CM3" -D"STM32F10X_MD" --cpu Cortex-M3 --li --c99 -D__MICROLIB -O2 --split_sections --gnu --no-multibyte-chars -g -o ./build/Target_1/.obj/Library/stm32f10x_exti.o --no_depend_system_headers --depend ./build/Target_1/.obj/Library/stm32f10x_exti.d ./Library/stm32f10x_exti.c

-include ./build/Target_1/.obj/Library/stm32f10x_flash.d
./build/Target_1/.obj/Library/stm32f10x_flash.o: ./Library/stm32f10x_flash.c Makefile | ./build/Target_1/.obj/Library
	@echo compiling $< ...
	$(Q)$(CC) -c --apcs=interwork -I. -IStart -IUser -ILibrary -I.cmsis/include -IRTE/_Target_1 -D"USE_STDPERIPH_DRIVER" -D"ARM_MATH_CM3" -D"STM32F10X_MD" --cpu Cortex-M3 --li --c99 -D__MICROLIB -O2 --split_sections --gnu --no-multibyte-chars -g -o ./build/Target_1/.obj/Library/stm32f10x_flash.o --no_depend_system_headers --depend ./build/Target_1/.obj/Library/stm32f10x_flash.d ./Library/stm32f10x_flash.c

-include ./build/Target_1/.obj/Library/stm32f10x_fsmc.d
./build/Target_1/.obj/Library/stm32f10x_fsmc.o: ./Library/stm32f10x_fsmc.c Makefile | ./build/Target_1/.obj/Library
	@echo compiling $< ...
	$(Q)$(CC) -c --apcs=interwork -I. -IStart -IUser -ILibrary -I.cmsis/include -IRTE/_Target_1 -D"USE_STDPERIPH_DRIVER" -D"ARM_MATH_CM3" -D"STM32F10X_MD" --cpu Cortex-M3 --li --c99 -D__MICROLIB -O2 --split_sections --gnu --no-multibyte-chars -g -o ./build/Target_1/.obj/Library/stm32f10x_fsmc.o --no_depend_system_headers --depend ./build/Target_1/.obj/Library/stm32f10x_fsmc.d ./Library/stm32f10x_fsmc.c

-include ./build/Target_1/.obj/Library/stm32f10x_gpio.d
./build/Target_1/.obj/Library/stm32f10x_gpio.o: ./Library/stm32f10x_gpio.c Makefile | ./build/Target_1/.obj/Library
	@echo compiling $< ...
	$(Q)$(CC) -c --apcs=interwork -I. -IStart -IUser -ILibrary -I.cmsis/include -IRTE/_Target_1 -D"USE_STDPERIPH_DRIVER" -D"ARM_MATH_CM3" -D"STM32F10X_MD" --cpu Cortex-M3 --li --c99 -D__MICROLIB -O2 --split_sections --gnu --no-multibyte-chars -g -o ./build/Target_1/.obj/Library/stm32f10x_gpio.o --no_depend_system_headers --depend ./build/Target_1/.obj/Library/stm32f10x_gpio.d ./Library/stm32f10x_gpio.c

-include ./build/Target_1/.obj/Library/stm32f10x_i2c.d
./build/Target_1/.obj/Library/stm32f10x_i2c.o: ./Library/stm32f10x_i2c.c Makefile | ./build/Target_1/.obj/Library
	@echo compiling $< ...
	$(Q)$(CC) -c --apcs=interwork -I. -IStart -IUser -ILibrary -I.cmsis/include -IRTE/_Target_1 -D"USE_STDPERIPH_DRIVER" -D"ARM_MATH_CM3" -D"STM32F10X_MD" --cpu Cortex-M3 --li --c99 -D__MICROLIB -O2 --split_sections --gnu --no-multibyte-chars -g -o ./build/Target_1/.obj/Library/stm32f10x_i2c.o --no_depend_system_headers --depend ./build/Target_1/.obj/Library/stm32f10x_i2c.d ./Library/stm32f10x_i2c.c

-include ./build/Target_1/.obj/Library/stm32f10x_iwdg.d
./build/Target_1/.obj/Library/stm32f10x_iwdg.o: ./Library/stm32f10x_iwdg.c Makefile | ./build/Target_1/.obj/Library
	@echo compiling $< ...
	$(Q)$(CC) -c --apcs=interwork -I. -IStart -IUser -ILibrary -I.cmsis/include -IRTE/_Target_1 -D"USE_STDPERIPH_DRIVER" -D"ARM_MATH_CM3" -D"STM32F10X_MD" --cpu Cortex-M3 --li --c99 -D__MICROLIB -O2 --split_sections --gnu --no-multibyte-chars -g -o ./build/Target_1/.obj/Library/stm32f10x_iwdg.o --no_depend_system_headers --depend ./build/Target_1/.obj/Library/stm32f10x_iwdg.d ./Library/stm32f10x_iwdg.c

-include ./build/Target_1/.obj/Library/stm32f10x_pwr.d
./build/Target_1/.obj/Library/stm32f10x_pwr.o: ./Library/stm32f10x_pwr.c Makefile | ./build/Target_1/.obj/Library
	@echo compiling $< ...
	$(Q)$(CC) -c --apcs=interwork -I. -IStart -IUser -ILibrary -I.cmsis/include -IRTE/_Target_1 -D"USE_STDPERIPH_DRIVER" -D"ARM_MATH_CM3" -D"STM32F10X_MD" --cpu Cortex-M3 --li --c99 -D__MICROLIB -O2 --split_sections --gnu --no-multibyte-chars -g -o ./build/Target_1/.obj/Library/stm32f10x_pwr.o --no_depend_system_headers --depend ./build/Target_1/.obj/Library/stm32f10x_pwr.d ./Library/stm32f10x_pwr.c

-include ./build/Target_1/.obj/Library/stm32f10x_rcc.d
./build/Target_1/.obj/Library/stm32f10x_rcc.o: ./Library/stm32f10x_rcc.c Makefile | ./build/Target_1/.obj/Library
	@echo compiling $< ...
	$(Q)$(CC) -c --apcs=interwork -I. -IStart -IUser -ILibrary -I.cmsis/include -IRTE/_Target_1 -D"USE_STDPERIPH_DRIVER" -D"ARM_MATH_CM3" -D"STM32F10X_MD" --cpu Cortex-M3 --li --c99 -D__MICROLIB -O2 --split_sections --gnu --no-multibyte-chars -g -o ./build/Target_1/.obj/Library/stm32f10x_rcc.o --no_depend_system_headers --depend ./build/Target_1/.obj/Library/stm32f10x_rcc.d ./Library/stm32f10x_rcc.c

-include ./build/Target_1/.obj/Library/stm32f10x_rtc.d
./build/Target_1/.obj/Library/stm32f10x_rtc.o: ./Library/stm32f10x_rtc.c Makefile | ./build/Target_1/.obj/Library
	@echo compiling $< ...
	$(Q)$(CC) -c --apcs=interwork -I. -IStart -IUser -ILibrary -I.cmsis/include -IRTE/_Target_1 -D"USE_STDPERIPH_DRIVER" -D"ARM_MATH_CM3" -D"STM32F10X_MD" --cpu Cortex-M3 --li --c99 -D__MICROLIB -O2 --split_sections --gnu --no-multibyte-chars -g -o ./build/Target_1/.obj/Library/stm32f10x_rtc.o --no_depend_system_headers --depend ./build/Target_1/.obj/Library/stm32f10x_rtc.d ./Library/stm32f10x_rtc.c

-include ./build/Target_1/.obj/Library/stm32f10x_sdio.d
./build/Target_1/.obj/Library/stm32f10x_sdio.o: ./Library/stm32f10x_sdio.c Makefile | ./build/Target_1/.obj/Library
	@echo compiling $< ...
	$(Q)$(CC) -c --apcs=interwork -I. -IStart -IUser -ILibrary -I.cmsis/include -IRTE/_Target_1 -D"USE_STDPERIPH_DRIVER" -D"ARM_MATH_CM3" -D"STM32F10X_MD" --cpu Cortex-M3 --li --c99 -D__MICROLIB -O2 --split_sections --gnu --no-multibyte-chars -g -o ./build/Target_1/.obj/Library/stm32f10x_sdio.o --no_depend_system_headers --depend ./build/Target_1/.obj/Library/stm32f10x_sdio.d ./Library/stm32f10x_sdio.c

-include ./build/Target_1/.obj/Library/stm32f10x_spi.d
./build/Target_1/.obj/Library/stm32f10x_spi.o: ./Library/stm32f10x_spi.c Makefile | ./build/Target_1/.obj/Library
	@echo compiling $< ...
	$(Q)$(CC) -c --apcs=interwork -I. -IStart -IUser -ILibrary -I.cmsis/include -IRTE/_Target_1 -D"USE_STDPERIPH_DRIVER" -D"ARM_MATH_CM3" -D"STM32F10X_MD" --cpu Cortex-M3 --li --c99 -D__MICROLIB -O2 --split_sections --gnu --no-multibyte-chars -g -o ./build/Target_1/.obj/Library/stm32f10x_spi.o --no_depend_system_headers --depend ./build/Target_1/.obj/Library/stm32f10x_spi.d ./Library/stm32f10x_spi.c

-include ./build/Target_1/.obj/Library/stm32f10x_tim.d
./build/Target_1/.obj/Library/stm32f10x_tim.o: ./Library/stm32f10x_tim.c Makefile | ./build/Target_1/.obj/Library
	@echo compiling $< ...
	$(Q)$(CC) -c --apcs=interwork -I. -IStart -IUser -ILibrary -I.cmsis/include -IRTE/_Target_1 -D"USE_STDPERIPH_DRIVER" -D"ARM_MATH_CM3" -D"STM32F10X_MD" --cpu Cortex-M3 --li --c99 -D__MICROLIB -O2 --split_sections --gnu --no-multibyte-chars -g -o ./build/Target_1/.obj/Library/stm32f10x_tim.o --no_depend_system_headers --depend ./build/Target_1/.obj/Library/stm32f10x_tim.d ./Library/stm32f10x_tim.c

-include ./build/Target_1/.obj/Library/stm32f10x_usart.d
./build/Target_1/.obj/Library/stm32f10x_usart.o: ./Library/stm32f10x_usart.c Makefile | ./build/Target_1/.obj/Library
	@echo compiling $< ...
	$(Q)$(CC) -c --apcs=interwork -I. -IStart -IUser -ILibrary -I.cmsis/include -IRTE/_Target_1 -D"USE_STDPERIPH_DRIVER" -D"ARM_MATH_CM3" -D"STM32F10X_MD" --cpu Cortex-M3 --li --c99 -D__MICROLIB -O2 --split_sections --gnu --no-multibyte-chars -g -o ./build/Target_1/.obj/Library/stm32f10x_usart.o --no_depend_system_headers --depend ./build/Target_1/.obj/Library/stm32f10x_usart.d ./Library/stm32f10x_usart.c

-include ./build/Target_1/.obj/Library/stm32f10x_wwdg.d
./build/Target_1/.obj/Library/stm32f10x_wwdg.o: ./Library/stm32f10x_wwdg.c Makefile | ./build/Target_1/.obj/Library
	@echo compiling $< ...
	$(Q)$(CC) -c --apcs=interwork -I. -IStart -IUser -ILibrary -I.cmsis/include -IRTE/_Target_1 -D"USE_STDPERIPH_DRIVER" -D"ARM_MATH_CM3" -D"STM32F10X_MD" --cpu Cortex-M3 --li --c99 -D__MICROLIB -O2 --split_sections --gnu --no-multibyte-chars -g -o ./build/Target_1/.obj/Library/stm32f10x_wwdg.o --no_depend_system_headers --depend ./build/Target_1/.obj/Library/stm32f10x_wwdg.d ./Library/stm32f10x_wwdg.c

./build/Target_1/.obj/Start:
	$(Q)mkdir -p $@
-include ./build/Target_1/.obj/Start/core_cm3.d
./build/Target_1/.obj/Start/core_cm3.o: ./Start/core_cm3.c Makefile | ./build/Target_1/.obj/Start
	@echo compiling $< ...
	$(Q)$(CC) -c --apcs=interwork -I. -IStart -IUser -ILibrary -I.cmsis/include -IRTE/_Target_1 -D"USE_STDPERIPH_DRIVER" -D"ARM_MATH_CM3" -D"STM32F10X_MD" --cpu Cortex-M3 --li --c99 -D__MICROLIB -O2 --split_sections --gnu --no-multibyte-chars -g -o ./build/Target_1/.obj/Start/core_cm3.o --no_depend_system_headers --depend ./build/Target_1/.obj/Start/core_cm3.d ./Start/core_cm3.c

-include ./build/Target_1/.obj/Start/startup_stm32f10x_md.d
./build/Target_1/.obj/Start/startup_stm32f10x_md.o: ./Start/startup_stm32f10x_md.s Makefile | ./build/Target_1/.obj/Start
	@echo assembling $< ...
	$(Q)$(AS) --apcs=interwork -I. -IStart -IUser -ILibrary -I.cmsis/include -IRTE/_Target_1 --cpu Cortex-M3 --li --pd "__MICROLIB SETA 1" -g -o ./build/Target_1/.obj/Start/startup_stm32f10x_md.o --depend ./build/Target_1/.obj/Start/startup_stm32f10x_md.d ./Start/startup_stm32f10x_md.s

-include ./build/Target_1/.obj/Start/system_stm32f10x.d
./build/Target_1/.obj/Start/system_stm32f10x.o: ./Start/system_stm32f10x.c Makefile | ./build/Target_1/.obj/Start
	@echo compiling $< ...
	$(Q)$(CC) -c --apcs=interwork -I. -IStart -IUser -ILibrary -I.cmsis/include -IRTE/_Target_1 -D"USE_STDPERIPH_DRIVER" -D"ARM_MATH_CM3" -D"STM32F10X_MD" --cpu Cortex-M3 --li --c99 -D__MICROLIB -O2 --split_sections --gnu --no-multibyte-chars -g -o ./build/Target_1/.obj/Start/system_stm32f10x.o --no_depend_system_headers --depend ./build/Target_1/.obj/Start/system_stm32f10x.d ./Start/system_stm32f10x.c

./build/Target_1/.obj/User:
	$(Q)mkdir -p $@
-include ./build/Target_1/.obj/User/BlueTooth.d
./build/Target_1/.obj/User/BlueTooth.o: ./User/BlueTooth.c Makefile | ./build/Target_1/.obj/User
	@echo compiling $< ...
	$(Q)$(CC) -c --apcs=interwork -I. -IStart -IUser -ILibrary -I.cmsis/include -IRTE/_Target_1 -D"USE_STDPERIPH_DRIVER" -D"ARM_MATH_CM3" -D"STM32F10X_MD" --cpu Cortex-M3 --li --c99 -D__MICROLIB -O2 --split_sections --gnu --no-multibyte-chars -g -o ./build/Target_1/.obj/User/BlueTooth.o --no_depend_system_headers --depend ./build/Target_1/.obj/User/BlueTooth.d ./User/BlueTooth.c

-include ./build/Target_1/.obj/User/Delay.d
./build/Target_1/.obj/User/Delay.o: ./User/Delay.c Makefile | ./build/Target_1/.obj/User
	@echo compiling $< ...
	$(Q)$(CC) -c --apcs=interwork -I. -IStart -IUser -ILibrary -I.cmsis/include -IRTE/_Target_1 -D"USE_STDPERIPH_DRIVER" -D"ARM_MATH_CM3" -D"STM32F10X_MD" --cpu Cortex-M3 --li --c99 -D__MICROLIB -O2 --split_sections --gnu --no-multibyte-chars -g -o ./build/Target_1/.obj/User/Delay.o --no_depend_system_headers --depend ./build/Target_1/.obj/User/Delay.d ./User/Delay.c

-include ./build/Target_1/.obj/User/IMU.d
./build/Target_1/.obj/User/IMU.o: ./User/IMU.c Makefile | ./build/Target_1/.obj/User
	@echo compiling $< ...
	$(Q)$(CC) -c --apcs=interwork -I. -IStart -IUser -ILibrary -I.cmsis/include -IRTE/_Target_1 -D"USE_STDPERIPH_DRIVER" -D"ARM_MATH_CM3" -D"STM32F10X_MD" --cpu Cortex-M3 --li --c99 -D__MICROLIB -O2 --split_sections --gnu --no-multibyte-chars -g -o ./build/Target_1/.obj/User/IMU.o --no_depend_system_headers --depend ./build/Target_1/.obj/User/IMU.d ./User/IMU.c

-include ./build/Target_1/.obj/User/IR.d
./build/Target_1/.obj/User/IR.o: ./User/IR.c Makefile | ./build/Target_1/.obj/User
	@echo compiling $< ...
	$(Q)$(CC) -c --apcs=interwork -I. -IStart -IUser -ILibrary -I.cmsis/include -IRTE/_Target_1 -D"USE_STDPERIPH_DRIVER" -D"ARM_MATH_CM3" -D"STM32F10X_MD" --cpu Cortex-M3 --li --c99 -D__MICROLIB -O2 --split_sections --gnu --no-multibyte-chars -g -o ./build/Target_1/.obj/User/IR.o --no_depend_system_headers --depend ./build/Target_1/.obj/User/IR.d ./User/IR.c

-include ./build/Target_1/.obj/User/OLED.d
./build/Target_1/.obj/User/OLED.o: ./User/OLED.c Makefile | ./build/Target_1/.obj/User
	@echo compiling $< ...
	$(Q)$(CC) -c --apcs=interwork -I. -IStart -IUser -ILibrary -I.cmsis/include -IRTE/_Target_1 -D"USE_STDPERIPH_DRIVER" -D"ARM_MATH_CM3" -D"STM32F10X_MD" --cpu Cortex-M3 --li --c99 -D__MICROLIB -O2 --split_sections --gnu --no-multibyte-chars -g -o ./build/Target_1/.obj/User/OLED.o --no_depend_system_headers --depend ./build/Target_1/.obj/User/OLED.d ./User/OLED.c

-include ./build/Target_1/.obj/User/PID.d
./build/Target_1/.obj/User/PID.o: ./User/PID.c Makefile | ./build/Target_1/.obj/User
	@echo compiling $< ...
	$(Q)$(CC) -c --apcs=interwork -I. -IStart -IUser -ILibrary -I.cmsis/include -IRTE/_Target_1 -D"USE_STDPERIPH_DRIVER" -D"ARM_MATH_CM3" -D"STM32F10X_MD" --cpu Cortex-M3 --li --c99 -D__MICROLIB -O2 --split_sections --gnu --no-multibyte-chars -g -o ./build/Target_1/.obj/User/PID.o --no_depend_system_headers --depend ./build/Target_1/.obj/User/PID.d ./User/PID.c

-include ./build/Target_1/.obj/User/SpeedDecision.d
./build/Target_1/.obj/User/SpeedDecision.o: ./User/SpeedDecision.c Makefile | ./build/Target_1/.obj/User
	@echo compiling $< ...
	$(Q)$(CC) -c --apcs=interwork -I. -IStart -IUser -ILibrary -I.cmsis/include -IRTE/_Target_1 -D"USE_STDPERIPH_DRIVER" -D"ARM_MATH_CM3" -D"STM32F10X_MD" --cpu Cortex-M3 --li --c99 -D__MICROLIB -O2 --split_sections --gnu --no-multibyte-chars -g -o ./build/Target_1/.obj/User/SpeedDecision.o --no_depend_system_headers --depend ./build/Target_1/.obj/User/SpeedDecision.d ./User/SpeedDecision.c

-include ./build/Target_1/.obj/User/SysTick.d
./build/Target_1/.obj/User/SysTick.o: ./User/SysTick.c Makefile | ./build/Target_1/.obj/User
	@echo compiling $< ...
	$(Q)$(CC) -c --apcs=interwork -I. -IStart -IUser -ILibrary -I.cmsis/include -IRTE/_Target_1 -D"USE_STDPERIPH_DRIVER" -D"ARM_MATH_CM3" -D"STM32F10X_MD" --cpu Cortex-M3 --li --c99 -D__MICROLIB -O2 --split_sections --gnu --no-multibyte-chars -g -o ./build/Target_1/.obj/User/SysTick.o --no_depend_system_headers --depend ./build/Target_1/.obj/User/SysTick.d ./User/SysTick.c

-include ./build/Target_1/.obj/User/Tasks.d
./build/Target_1/.obj/User/Tasks.o: ./User/Tasks.c Makefile | ./build/Target_1/.obj/User
	@echo compiling $< ...
	$(Q)$(CC) -c --apcs=interwork -I. -IStart -IUser -ILibrary -I.cmsis/include -IRTE/_Target_1 -D"USE_STDPERIPH_DRIVER" -D"ARM_MATH_CM3" -D"STM32F10X_MD" --cpu Cortex-M3 --li --c99 -D__MICROLIB -O2 --split_sections --gnu --no-multibyte-chars -g -o ./build/Target_1/.obj/User/Tasks.o --no_depend_system_headers --depend ./build/Target_1/.obj/User/Tasks.d ./User/Tasks.c

-include ./build/Target_1/.obj/User/filter.d
./build/Target_1/.obj/User/filter.o: ./User/filter.c Makefile | ./build/Target_1/.obj/User
	@echo compiling $< ...
	$(Q)$(CC) -c --apcs=interwork -I. -IStart -IUser -ILibrary -I.cmsis/include -IRTE/_Target_1 -D"USE_STDPERIPH_DRIVER" -D"ARM_MATH_CM3" -D"STM32F10X_MD" --cpu Cortex-M3 --li --c99 -D__MICROLIB -O2 --split_sections --gnu --no-multibyte-chars -g -o ./build/Target_1/.obj/User/filter.o --no_depend_system_headers --depend ./build/Target_1/.obj/User/filter.d ./User/filter.c

-include ./build/Target_1/.obj/User/main.d
./build/Target_1/.obj/User/main.o: ./User/main.c Makefile | ./build/Target_1/.obj/User
	@echo compiling $< ...
	$(Q)$(CC) -c --apcs=interwork -I. -IStart -IUser -ILibrary -I.cmsis/include -IRTE/_Target_1 -D"USE_STDPERIPH_DRIVER" -D"ARM_MATH_CM3" -D"STM32F10X_MD" --cpu Cortex-M3 --li --c99 -D__MICROLIB -O2 --split_sections --gnu --no-multibyte-chars -g -o ./build/Target_1/.obj/User/main.o --no_depend_system_headers --depend ./build/Target_1/.obj/User/main.d ./User/main.c

-include ./build/Target_1/.obj/User/motor.d
./build/Target_1/.obj/User/motor.o: ./User/motor.c Makefile | ./build/Target_1/.obj/User
	@echo compiling $< ...
	$(Q)$(CC) -c --apcs=interwork -I. -IStart -IUser -ILibrary -I.cmsis/include -IRTE/_Target_1 -D"USE_STDPERIPH_DRIVER" -D"ARM_MATH_CM3" -D"STM32F10X_MD" --cpu Cortex-M3 --li --c99 -D__MICROLIB -O2 --split_sections --gnu --no-multibyte-chars -g -o ./build/Target_1/.obj/User/motor.o --no_depend_system_headers --depend ./build/Target_1/.obj/User/motor.d ./User/motor.c

-include ./build/Target_1/.obj/User/stm32f10x_it.d
./build/Target_1/.obj/User/stm32f10x_it.o: ./User/stm32f10x_it.c Makefile | ./build/Target_1/.obj/User
	@echo compiling $< ...
	$(Q)$(CC) -c --apcs=interwork -I. -IStart -IUser -ILibrary -I.cmsis/include -IRTE/_Target_1 -D"USE_STDPERIPH_DRIVER" -D"ARM_MATH_CM3" -D"STM32F10X_MD" --cpu Cortex-M3 --li --c99 -D__MICROLIB -O2 --split_sections --gnu --no-multibyte-chars -g -o ./build/Target_1/.obj/User/stm32f10x_it.o --no_depend_system_headers --depend ./build/Target_1/.obj/User/stm32f10x_it.d ./User/stm32f10x_it.c

-include ./build/Target_1/.obj/User/vofa.d
./build/Target_1/.obj/User/vofa.o: ./User/vofa.c Makefile | ./build/Target_1/.obj/User
	@echo compiling $< ...
	$(Q)$(CC) -c --apcs=interwork -I. -IStart -IUser -ILibrary -I.cmsis/include -IRTE/_Target_1 -D"USE_STDPERIPH_DRIVER" -D"ARM_MATH_CM3" -D"STM32F10X_MD" --cpu Cortex-M3 --li --c99 -D__MICROLIB -O2 --split_sections --gnu --no-multibyte-chars -g -o ./build/Target_1/.obj/User/vofa.o --no_depend_system_headers --depend ./build/Target_1/.obj/User/vofa.d ./User/vofa.c

./build/Target_1/.obj/cJSON:
	$(Q)mkdir -p $@
-include ./build/Target_1/.obj/cJSON/cJSON.d
./build/Target_1/.obj/cJSON/cJSON.o: ./cJSON/cJSON.c Makefile | ./build/Target_1/.obj/cJSON
	@echo compiling $< ...
	$(Q)$(CC) -c --apcs=interwork -I. -IStart -IUser -ILibrary -I.cmsis/include -IRTE/_Target_1 -D"USE_STDPERIPH_DRIVER" -D"ARM_MATH_CM3" -D"STM32F10X_MD" --cpu Cortex-M3 --li --c99 -D__MICROLIB -O2 --split_sections --gnu --no-multibyte-chars -g -o ./build/Target_1/.obj/cJSON/cJSON.o --no_depend_system_headers --depend ./build/Target_1/.obj/cJSON/cJSON.d ./cJSON/cJSON.c

-include ./build/Target_1/.obj/cJSON/cJSON_Utils.d
./build/Target_1/.obj/cJSON/cJSON_Utils.o: ./cJSON/cJSON_Utils.c Makefile | ./build/Target_1/.obj/cJSON
	@echo compiling $< ...
	$(Q)$(CC) -c --apcs=interwork -I. -IStart -IUser -ILibrary -I.cmsis/include -IRTE/_Target_1 -D"USE_STDPERIPH_DRIVER" -D"ARM_MATH_CM3" -D"STM32F10X_MD" --cpu Cortex-M3 --li --c99 -D__MICROLIB -O2 --split_sections --gnu --no-multibyte-chars -g -o ./build/Target_1/.obj/cJSON/cJSON_Utils.o --no_depend_system_headers --depend ./build/Target_1/.obj/cJSON/cJSON_Utils.d ./cJSON/cJSON_Utils.c

./build/Target_1/.obj/kalman_filter:
	$(Q)mkdir -p $@
-include ./build/Target_1/.obj/kalman_filter/kalman_filter.d
./build/Target_1/.obj/kalman_filter/kalman_filter.o: ./kalman_filter/kalman_filter.c Makefile | ./build/Target_1/.obj/kalman_filter
	@echo compiling $< ...
	$(Q)$(CC) -c --apcs=interwork -I. -IStart -IUser -ILibrary -I.cmsis/include -IRTE/_Target_1 -D"USE_STDPERIPH_DRIVER" -D"ARM_MATH_CM3" -D"STM32F10X_MD" --cpu Cortex-M3 --li --c99 -D__MICROLIB -O2 --split_sections --gnu --no-multibyte-chars -g -o ./build/Target_1/.obj/kalman_filter/kalman_filter.o --no_depend_system_headers --depend ./build/Target_1/.obj/kalman_filter/kalman_filter.d ./kalman_filter/kalman_filter.c

objs = ./build/Target_1/.obj/Library/misc.o ./build/Target_1/.obj/Library/stm32f10x_adc.o ./build/Target_1/.obj/Library/stm32f10x_bkp.o ./build/Target_1/.obj/Library/stm32f10x_can.o ./build/Target_1/.obj/Library/stm32f10x_cec.o ./build/Target_1/.obj/Library/stm32f10x_crc.o ./build/Target_1/.obj/Library/stm32f10x_dac.o ./build/Target_1/.obj/Library/stm32f10x_dbgmcu.o ./build/Target_1/.obj/Library/stm32f10x_dma.o ./build/Target_1/.obj/Library/stm32f10x_exti.o ./build/Target_1/.obj/Library/stm32f10x_flash.o ./build/Target_1/.obj/Library/stm32f10x_fsmc.o ./build/Target_1/.obj/Library/stm32f10x_gpio.o ./build/Target_1/.obj/Library/stm32f10x_i2c.o ./build/Target_1/.obj/Library/stm32f10x_iwdg.o ./build/Target_1/.obj/Library/stm32f10x_pwr.o ./build/Target_1/.obj/Library/stm32f10x_rcc.o ./build/Target_1/.obj/Library/stm32f10x_rtc.o ./build/Target_1/.obj/Library/stm32f10x_sdio.o ./build/Target_1/.obj/Library/stm32f10x_spi.o ./build/Target_1/.obj/Library/stm32f10x_tim.o ./build/Target_1/.obj/Library/stm32f10x_usart.o ./build/Target_1/.obj/Library/stm32f10x_wwdg.o ./build/Target_1/.obj/Start/core_cm3.o ./build/Target_1/.obj/Start/startup_stm32f10x_md.o ./build/Target_1/.obj/Start/system_stm32f10x.o ./build/Target_1/.obj/User/BlueTooth.o ./build/Target_1/.obj/User/Delay.o ./build/Target_1/.obj/User/IMU.o ./build/Target_1/.obj/User/IR.o ./build/Target_1/.obj/User/OLED.o ./build/Target_1/.obj/User/PID.o ./build/Target_1/.obj/User/SpeedDecision.o ./build/Target_1/.obj/User/SysTick.o ./build/Target_1/.obj/User/Tasks.o ./build/Target_1/.obj/User/filter.o ./build/Target_1/.obj/User/main.o ./build/Target_1/.obj/User/motor.o ./build/Target_1/.obj/User/stm32f10x_it.o ./build/Target_1/.obj/User/vofa.o ./build/Target_1/.obj/cJSON/cJSON.o ./build/Target_1/.obj/cJSON/cJSON_Utils.o ./build/Target_1/.obj/kalman_filter/kalman_filter.o
elf: $(objs) Makefile
	@printf "$(COLOR_INF)linking ./build/Target_1/project.axf ...$(COLOR_END)\n"
	$(LD) --cpu Cortex-M3 --library_type=microlib --scatter build/Target_1/project.sct --strict --summary_stderr --info summarysizes --map --xref --callgraph --symbols --info sizes --info totals --info unused --info veneers --list ./build/Target_1/project.map ./build/Target_1/.obj/Library/misc.o ./build/Target_1/.obj/Library/stm32f10x_adc.o ./build/Target_1/.obj/Library/stm32f10x_bkp.o ./build/Target_1/.obj/Library/stm32f10x_can.o ./build/Target_1/.obj/Library/stm32f10x_cec.o ./build/Target_1/.obj/Library/stm32f10x_crc.o ./build/Target_1/.obj/Library/stm32f10x_dac.o ./build/Target_1/.obj/Library/stm32f10x_dbgmcu.o ./build/Target_1/.obj/Library/stm32f10x_dma.o ./build/Target_1/.obj/Library/stm32f10x_exti.o ./build/Target_1/.obj/Library/stm32f10x_flash.o ./build/Target_1/.obj/Library/stm32f10x_fsmc.o ./build/Target_1/.obj/Library/stm32f10x_gpio.o ./build/Target_1/.obj/Library/stm32f10x_i2c.o ./build/Target_1/.obj/Library/stm32f10x_iwdg.o ./build/Target_1/.obj/Library/stm32f10x_pwr.o ./build/Target_1/.obj/Library/stm32f10x_rcc.o ./build/Target_1/.obj/Library/stm32f10x_rtc.o ./build/Target_1/.obj/Library/stm32f10x_sdio.o ./build/Target_1/.obj/Library/stm32f10x_spi.o ./build/Target_1/.obj/Library/stm32f10x_tim.o ./build/Target_1/.obj/Library/stm32f10x_usart.o ./build/Target_1/.obj/Library/stm32f10x_wwdg.o ./build/Target_1/.obj/Start/core_cm3.o ./build/Target_1/.obj/Start/startup_stm32f10x_md.o ./build/Target_1/.obj/Start/system_stm32f10x.o ./build/Target_1/.obj/User/BlueTooth.o ./build/Target_1/.obj/User/Delay.o ./build/Target_1/.obj/User/IMU.o ./build/Target_1/.obj/User/IR.o ./build/Target_1/.obj/User/OLED.o ./build/Target_1/.obj/User/PID.o ./build/Target_1/.obj/User/SpeedDecision.o ./build/Target_1/.obj/User/SysTick.o ./build/Target_1/.obj/User/Tasks.o ./build/Target_1/.obj/User/filter.o ./build/Target_1/.obj/User/main.o ./build/Target_1/.obj/User/motor.o ./build/Target_1/.obj/User/stm32f10x_it.o ./build/Target_1/.obj/User/vofa.o ./build/Target_1/.obj/cJSON/cJSON.o ./build/Target_1/.obj/cJSON/cJSON_Utils.o ./build/Target_1/.obj/kalman_filter/kalman_filter.o -o ./build/Target_1/project.axf

bin: elf Makefile
	@printf "$(COLOR_INF)output binary files ...$(COLOR_END)\n"
	$(COMPILER_DIR)/bin/fromelf$(EXE) --i32combined --output ./build/Target_1/project.hex ./build/Target_1/project.axf
	$(COMPILER_DIR)/bin/fromelf$(EXE) --m32combined --output ./build/Target_1/project.s19 ./build/Target_1/project.axf

postbuild: bin
	@printf "$(COLOR_INF)postbuild ...$(COLOR_END)\n"
	@echo nothing to postbuild.

