# HawkHealth GROUP repo -- STM32F767 (Renode + board), arm-none-eabi-gcc.
#
# This build AUTO-DISCOVERS every .c under src/. Each subsystem owner just drops their
# stub files into src/<subsystem>/ and they get compiled -- no edits to this file needed.
# The only hand-maintained list is the FreeRTOS kernel sources (they never change).
CC      := arm-none-eabi-gcc
SIZE    := arm-none-eabi-size
KERNEL  := third_party/FreeRTOS-Kernel
PORT    := $(KERNEL)/portable/GCC/ARM_CM7/r0p1

CPU     := -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard
CFLAGS  := $(CPU) -ffreestanding -O2 -Wall -Wextra -g3 \
           -ffunction-sections -fdata-sections \
           -Iinclude -I$(KERNEL)/include -I$(PORT)
LDFLAGS := $(CPU) -nostartfiles -T src/startup/stm32f767.ld -Wl,--gc-sections

TARGET  := hawkhealth.elf

# Every .c under src/ (your stubs, the platform layer, startup, hooks, app wiring).
APP_SRC    := $(shell find src -name '*.c')
KERNEL_SRC := $(KERNEL)/tasks.c $(KERNEL)/list.c $(KERNEL)/queue.c \
              $(PORT)/port.c $(KERNEL)/portable/MemMang/heap_4.c
SRC        := $(APP_SRC) $(KERNEL_SRC)

.PHONY: all clean
all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) $(SRC) -o $@
	$(SIZE) $@

clean:
	rm -f *.elf
