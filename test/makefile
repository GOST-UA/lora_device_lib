DIR_ROOT := ..
DIR_CMOCKA := $(DIR_ROOT)/vendor/cmocka
DIR_BUILD := build
DIR_BIN := bin

CC := gcc

VPATH += $(DIR_ROOT)/src
VPATH += $(DIR_CMOCKA)/src

INCLUDES += -I$(DIR_ROOT)/include
INCLUDES += -I$(DIR_CMOCKA)/include
INCLUDES += -I.

CMOCKA_DEFINES += -DHAVE_STRINGS_H
CMOCKA_DEFINES += -DHAVE_SIGNAL_H
CMOCKA_DEFINES += -DHAVE_INTTYPES_H
CMOCKA_DEFINES += -DHAVE_MALLOC_H

DEBUG_DEFINES += -D'LORA_DEBUG_INCLUDE="debug_include.h"'

CFLAGS := -O0 -Wall -Werror -g -fprofile-arcs -ftest-coverage $(INCLUDES) $(CMOCKA_DEFINES) $(DEBUG_DEFINES)
LDFLAGS := -fprofile-arcs -g

SRC := $(notdir $(wildcard $(DIR_ROOT)/src/*.c))
SRC_CMOCKA := $(notdir $(wildcard $(DIR_CMOCKA)/src/*.c))

OBJ := $(SRC:.c=.o)
OBJ_CMOCKA := $(SRC_CMOCKA:.c=.o)

TESTS := $(basename $(wildcard tc_*.c))

LINE := ================================================================

.PHONY: clean build_and_run coverage

build_and_run: 
	@ for tc in $(addprefix $(DIR_BIN)/, $(TESTS)); do \
		echo $(LINE); \
		echo ""; \
		make clean $$tc \
		&& echo "" \
		&& ./$$tc; \
		echo ""; \
	done

$(DIR_BUILD)/%.o: %.c
	@ echo building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

coverage:
	@ echo $(LINE)
	@ echo
	@ echo Test Coverage:
	@ echo
	@ make clean > /dev/null
	@ for tc in $(addprefix $(DIR_BIN)/, $(TESTS)); do \
		make $$tc > /dev/null 2>&1 \
		&& ./$$tc > /dev/null 2>&1; \
	done
	@ gcov --no-output --object-directory $(DIR_BUILD) $(OBJ)

clean:
	rm -f $(DIR_BUILD)/*

$(DIR_BIN)/tc_aes: $(addprefix $(DIR_BUILD)/, tc_aes.o lora_aes.o $(OBJ_CMOCKA))
	@ echo linking $@
	@ $(CC) $(LDFLAGS) $^ -o $@

$(DIR_BIN)/tc_cmac: $(addprefix $(DIR_BUILD)/, tc_cmac.o lora_cmac.o lora_aes.o $(OBJ_CMOCKA))
	@ echo linking $@
	@ $(CC) $(LDFLAGS) $^ -o $@

$(DIR_BIN)/tc_frame: $(addprefix $(DIR_BUILD)/, tc_frame.o lora_frame.o mock_lora_cmac.o mock_lora_aes.o $(OBJ_CMOCKA))
	@ echo linking $@
	@ $(CC) $(LDFLAGS) $^ -o $@

$(DIR_BIN)/tc_frame_with_encryption: $(addprefix $(DIR_BUILD)/, tc_frame_with_encryption.o lora_frame.o lora_cmac.o lora_aes.o $(OBJ_CMOCKA))
	@ echo linking $@
	@ $(CC) $(LDFLAGS) $^ -o $@

$(DIR_BIN)/tc_radio_sx1272: $(addprefix $(DIR_BUILD)/, tc_radio_sx1272.o lora_radio_sx1272.o mock_board.o mock_lora_system.o $(OBJ_CMOCKA))
	@ echo linking $@
	@ $(CC) $(LDFLAGS) $^ -o $@

$(DIR_BIN)/tc_event: $(addprefix $(DIR_BUILD)/, tc_event.o lora_event.o mock_system_time.o $(OBJ_CMOCKA))
	@ echo linking $@
	@ $(CC) $(LDFLAGS) $^ -o $@

$(DIR_BIN)/tc_mac: $(addprefix $(DIR_BUILD)/, tc_mac.o lora_mac.o mock_lora_mac_commands.o lora_event.o lora_region.o mock_lora_aes.o mock_lora_cmac.o mock_lora_system.o mock_lora_radio.o lora_frame.o mock_system_time.o $(OBJ_CMOCKA))
	@ echo linking $@
	@ $(CC) $(LDFLAGS) $^ -o $@

$(DIR_BIN)/tc_mac_commands: $(addprefix $(DIR_BUILD)/, tc_mac_commands.o lora_mac_commands.o lora_stream.o $(OBJ_CMOCKA))
	@ echo linking $@
	@ $(CC) $(LDFLAGS) $^ -o $@

$(DIR_BIN)/tc_region: $(addprefix $(DIR_BUILD)/, tc_region.o lora_region.o $(OBJ_CMOCKA))
	@ echo linking $@
	@ $(CC) $(LDFLAGS) $^ -o $@

$(DIR_BIN)/tc_integration: $(addprefix $(DIR_BUILD)/, tc_integration.o mock_lora_system.o mock_system_time.o $(OBJ) $(OBJ_CMOCKA))
	@ echo linking $@
	@ $(CC) $(LDFLAGS) $^ -o $@








