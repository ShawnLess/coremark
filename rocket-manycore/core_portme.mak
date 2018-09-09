# Copyright 2018 Embedded Microprocessor Benchmark Consortium (EEMBC)
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# 
# Original Author: Shay Gal-on

############################################
#  Porting Variables.
#
# Please use the question mark notation and stop overriding my environment variables
ROCKET_PATH ?= $(shell readlink -m ../celerity/bsg_riscv/)
ROCKET_BENCH_PATH ?= $(shell readlink -m ../celerity/bsg_rocket/common/benchmark)

MANYCORE_COREMARK ?=$(shell readlink -m ../celerity/bsg_manycore/software/spmd/coremark)

MANYCORE_IMAGE_NAME = manycore_coremark
MANYCORE_IMAGE_C_FILE=$(MANYCORE_IMAGE_NAME).vec.c
MANYCORE_IMAGE_64BIN_FILE=$(MANYCORE_IMAGE_NAME).riscv64

RV_TOOL_PATH=$(ROCKET_PATH)/riscv-install/bin/
RV_BENCH_PATH=$(ROCKET_PATH)/rocket-chip/riscv-tools/riscv-tests/benchmarks/
RV_COMMON_PATH=$(RV_BENCH_PATH)/common/
RV_LINK_SCRIPT=$(RV_COMMON_PATH)/test.ld

RV_PREFIX=riscv64-unknown-elf
RV_CC=$(RV_TOOL_PATH)/$(RV_PREFIX)-gcc
RV_LD=$(RV_TOOL_PATH)/$(RV_PREFIX)-ld
RV_AS=$(RV_TOOL_PATH)/$(RV_PREFIX)-as
RV_HEX=$(RV_TOOL_PATH)/elf2hex 
RV_CFLAGS=-static -std=gnu99 -O2 -ffast-math -fno-common -fno-builtin-printf -DPREALLOCATE=0 -DHOST_DEBUG=0 \

#############################################
#File : core_portme.mak

# Flag : OUTFLAG
#	Use this flag to define how to to get an executable (e.g -o)
OUTFLAG= -o
ifneq ($(HOST_RUN),1)
CC 		= $(RV_CC)
LD		= $(RV_CC)
AS		= $(RV_AS)
else
CC 		= gcc
LD		= gcc
AS		= as
endif
# Flag : CFLAGS
#	Use this flag to define compiler options. Note, you can add compiler options from the command line using XCFLAGS="other flags"
ifneq ($(HOST_RUN),1)
PORT_CFLAGS = -g  $(RV_CFLAGS) 
else
PORT_CFLAGS = -O0 -g -DHOST_RUN=1 
endif

FLAGS_STR = "$(PORT_CFLAGS) $(XCFLAGS) $(XLFLAGS) $(LFLAGS_END)"
CFLAGS = $(PORT_CFLAGS) -I$(PORT_DIR) -I. -I.. -DFLAGS_STR=\"$(FLAGS_STR)\" -I$(ROCKET_BENCH_PATH)/bsg_rocket_manycore_common/
#Flag : LFLAGS_END
#	Define any libraries needed for linking or other flags that should come at the end of the link line (e.g. linker scripts). 
#	Note : On certain platforms, the default clock_gettime implementation is supported but requires linking of librt.
SEPARATE_COMPILE=1
# Flag : SEPARATE_COMPILE
# You must also define below how to create an object file, and how to link.
OBJOUT 	= -o

ifneq ($(HOST_RUN),1)
LFLAGS 	= $(RV_BENCH_PATH)/bsg_rocket_rocc.o $(RV_BENCH_PATH)/syscalls.o -nostartfiles -ffast-math -fno-builtin-printf -lc -lgcc -lm  -T $(RV_LINK_SCRIPT) -L $(RV_BENCH_PATH) -Wl,-Map,link.map,--just-symbols=$(PORT_DIR)/$(MANYCORE_IMAGE_64BIN_FILE)
else
LFLAGS  = -lc -lgcc
endif

ASFLAGS = 
OFLAG 	= -o
COUT 	= -c

LFLAGS_END = 
# Flag : PORT_SRCS
# 	Port specific source files can be added here
#	You may also need cvt.c if the fcvt functions are not provided as intrinsics by your compiler!
PORT_SRCS = $(PORT_DIR)/core_portme.c \
            $(PORT_DIR)/ee_printf.c \
            $(PORT_DIR)/rocket-manycore.c \
	    $(PORT_DIR)/manycore_coremark.vec.c

PORT_OBJS = $(PORT_SRCS:.c=.o)

vpath %.c $(PORT_DIR)
vpath %.s $(PORT_DIR)

# Flag : LOAD
#	For a simple port, we assume self hosted compile and run, no load needed.

# Flag : RUN
#	For a simple port, we assume self hosted compile and run, simple invocation of the executable

LOAD = echo "Please set LOAD to the process of loading the executable to the flash"
RUN = echo "Please set LOAD to the process of running the executable (e.g. via jtag, or board reset)"

OEXT = .o
EXE = .bin

$(OPATH)$(PORT_DIR)/%$(OEXT) : %.c
	$(CC) $(CFLAGS) $(XCFLAGS) $(COUT) $< $(OBJOUT) $@

$(OPATH)%$(OEXT) : %.c
	$(CC) $(CFLAGS) $(XCFLAGS) $(COUT) $< $(OBJOUT) $@

$(OPATH)$(PORT_DIR)/%$(OEXT) : %.s
	$(AS) $(ASFLAGS) $< $(OBJOUT) $@

# Target : port_pre% and port_post%
# For the purpose of this simple port, no pre or post steps needed.

.PHONY : port_prebuild port_postbuild port_prerun port_postrun port_preload port_postload
port_pre% port_post% : 

port_prebuild :
	make -C $(ROCKET_PATH)/rocket-chip/riscv-tools/riscv-tests/benchmarks/ crt.o syscalls.o bsg_rocket_rocc.o
	make -C $(MANYCORE_COREMARK) clean
	make -C $(MANYCORE_COREMARK) main.riscv
	make -C $(MANYCORE_COREMARK) main.vec.c 
	cp      $(MANYCORE_COREMARK)/main.vec.c  $(PORT_DIR)/$(MANYCORE_IMAGE_C_FILE)
	make -C $(MANYCORE_COREMARK) main.riscv64
	cp $(MANYCORE_COREMARK)/main.riscv64 $(PORT_DIR)/$(MANYCORE_IMAGE_64BIN_FILE) 
	make -C $(MANYCORE_COREMARK) clean

ifneq ($(HOST_RUN),1)
port_postbuild:
	$(RV_HEX) 16 32768 coremark.bin > coremark.riscv.hex
	cp coremark.riscv.hex $(ROCKET_BENCH_PATH)/
endif
# FLAG : OPATH
# Path to the output folder. Default -c current folder.
OPATH = ./
MKDIR = mkdir -p

PORT_CLEAN=$(PORT_OBJS) $(PORT_DIR)/$(MANYCORE_IMAGE_C_FILE) $(PORT_DIR)/$(MANYCORE_IMAGE_64BIN_FILE)
