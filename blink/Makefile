PROJECT = blink
BUILD_DIR = bin

SHARED_DIR = ../common
CFILES = blink.c
#CFILES += api.c
#AFILES += api-asm.S

# TODO - you will need to edit these two lines!
DEVICE=stm32f446re
OPENCM3_TARGETS=stm32/f4
OOCD_FILE = board/st_nucleo_f4.cfg

# You shouldn't have to edit anything below here.
VPATH += $(SHARED_DIR)
INCLUDES += $(patsubst %,-I%, . $(SHARED_DIR))
OPENCM3_DIR=../libopencm3

all:

$(OPENCM3_DIR)/mk/genlink-config.mk \
$(OPENCM3_DIR)/mk/genlink-rules.mk:
	git submodule update --init $(OPENCM3_DIR)

include $(OPENCM3_DIR)/mk/genlink-config.mk
include ../rules.mk
include $(OPENCM3_DIR)/mk/genlink-rules.mk
