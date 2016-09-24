CC ?= gcc
LD ?= ld

CFLAGS := -g -m64 -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -ffreestanding \
	-mcmodel=kernel -Wall -Wextra -Werror -pedantic -std=c99 \
	-fno-omit-frame-pointer \
	-Wframe-larger-than=1024 -Wstack-usage=1024 \
	-Wno-unknown-warning-option $(if $(DEBUG),-DDEBUG)
LFLAGS := -nostdlib -z max-page-size=0x1000

INC := ./inc
SRC := ./src

C_SOURCES := $(wildcard $(SRC)/*.c)
C_OBJECTS := $(C_SOURCES:.c=.o)
C_DEPS := $(C_SOURCES:.c=.d)
S_SOURCES := $(wildcard $(SRC)/*.S)
S_OBJECTS := $(S_SOURCES:.S=.o)
S_DEPS := $(S_SOURCES:.S=.d)

OBJ := $(C_OBJECTS) $(S_OBJECTS)
DEP := $(C_DEPS) $(S_DEPS)

all: kernel

kernel: $(OBJ) kernel.ld
	$(LD) $(LFLAGS) -T kernel.ld -o $@ $(OBJ)

$(S_OBJECTS): %.o: %.S
	$(CC) -D__ASM_FILE__ -I$(INC) -g -MMD -c $< -o $@

$(C_OBJECTS): %.o: %.c
	$(CC) $(CFLAGS) -I$(INC) -g -MMD -c $< -o $@

run: clean kernel
ifeq ($(DEBUG), 1)
	qemu-system-x86_64 -kernel kernel -s -serial stdio &
	sleep 1
	gdb -x gdb_initial_commands kernel
	killall qemu-system-x86_64
else 
	qemu-system-x86_64 -kernel kernel  -serial stdio
endif

-include $(DEP)

.PHONY: clean
clean:
	rm -f kernel $(OBJ) $(DEP)
