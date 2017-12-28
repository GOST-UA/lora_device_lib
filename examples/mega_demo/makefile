DIR_ROOT := ../..
DIR_BUILD := build
DIR_BIN := bin

CC := avr-gcc

VPATH += $(DIR_ROOT)/src
VPATH += .

INCLUDES += -I$(DIR_ROOT)/include
INCLUDES += -I.

SRC := $(notdir $(wildcard $(DIR_ROOT)/src/*.c)) mega_demo.c system.c store.c
OBJ := $(SRC:.c=.o)

CFLAGS += -mmcu=atmega328p
CFLAGS += -Os -Wall -std=gnu99
CFLAGS += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums 
CFLAGS += $(INCLUDES)

CFLAGS += -DF_CPU=16000000U

# only include code useful to a device
CFLAGS += -DLORA_DEVICE	

# avr-gcc specific optimisations
CFLAGS += -DLORA_AVR

mega_demo.elf: $(addprefix $(DIR_BUILD)/, $(OBJ))
	@ echo building $@
	@ $(CC) $(CFLAGS) -Wl,-Map=$@.map,--cref $^ -o $@

$(DIR_BUILD)/%.o: %.c
	@ echo building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

clean:
	@ echo cleaning up objects
	@ rm -f $(DIR_BUILD)/*
