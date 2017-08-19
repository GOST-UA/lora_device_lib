DIR_ROOT := ..
DIR_BUILD := build
DIR_BIN := bin

CC := gcc

VPATH += $(DIR_ROOT)/src

INCLUDES += -I$(DIR_ROOT)/include
INCLUDES += -I.

CFLAGS := -Wall -Werror -g $(INCLUDES)
LDFLAGS := -g

SRC := $(notdir $(wildcard $(DIR_ROOT)/src/*.c))

OBJ := $(SRC:.c=.o)

all: $(addprefix $(DIR_BIN)/, print_frames)

$(DIR_BIN)/print_frames:  $(addprefix $(DIR_BUILD)/, print_frames.o print_hex.o lora_frame.o lora_cmac.o lora_aes.o)
	@ echo linking $@
	@ $(CC) $(LDFLAGS) $^ -o $@	
.PHONY: clean 

$(DIR_BUILD)/%.o: %.c
	@ echo building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(DIR_BUILD)/*






