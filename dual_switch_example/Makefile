## .PHONY: all sim wave clean
#
# TOP ?= top
# V      ?= top.v
# TB     ?= sim_main.cpp
# TB_TRC ?= sim_main_trace.cpp
# OBJ := obj_dir
#
# all: sim
# sim:
#	@if [ ! -f "$(V)" ]; then echo "missing Verilog: $(V)"; exit 1; fi
#	@if [ ! -f "$(TB_TRC)" ]; then echo "missing trace testbench: $(TB_TRC)"; exit 1; fi
#	verilator -Wall --trace --cc $(V) --top-module $(TOP) --exe $(TB_TRC)
#	$(MAKE) -C $(OBJ) -f V$(TOP).mk V$(TOP)
#	./$(OBJ)/V$(TOP)
#	$(call git_commit,"sim RTL") # DO NOT REMOVE THIS LINE!!!
#
# wave:
#	@if [ -f wave.vcd ]; then \
#		if command -v gtkwave >/dev/null 2>&1; then gtkwave wave.vcd & else echo "gtkwave not found. install: sudo apt-get install -y gtkwave"; fi; \
#	else \
#		echo "wave.vcd not found. run 'make sim' first."; \
#	fi
# # 清理
# clean:
#	rm -rf $(OBJ) wave.vcd wave.fst
#	# rm -rf $(OBJ) wave.vcd wave.fst
#	# include ../Makefile
# include ../Makefile
TOPNAME = top
NXDC_FILES = constr/top.nxdc
INC_PATH ?= $(NVBOARD_HOME)/usr/include

VERILATOR := verilator
VERILATOR_CFLAGS += -MMD --build -cc \
	-O3 -x-assign fast -x-initial fast --noassert

BUILD_DIR := ./build
OBJ_DIR := $(BUILD_DIR)/obj_dir
BIN := $(BUILD_DIR)/V$(TOPNAME)

default: $(BIN)

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# constraint -> auto bind cpp
SRC_AUTO_BIND = $(abspath $(BUILD_DIR)/auto_bind.cpp)
$(SRC_AUTO_BIND): $(NXDC_FILES) | $(BUILD_DIR)
	python3 $(NVBOARD_HOME)/scripts/auto_pin_bind.py $< $@

# project sources
VSRCS = $(shell find $(abspath ./vsrc) -name "*.v")
CSRCS = $(shell find $(abspath ./csrc) -name "*.c" -or -name "*.cc" -or -name "*.cpp")
CSRCS += $(SRC_AUTO_BIND)

# NVBoard rules
include $(NVBOARD_HOME)/scripts/nvboard.mk

# verilator flags
INCFLAGS := $(addprefix -I, $(INC_PATH))
CXXFLAGS += $(INCFLAGS) -DTOP_NAME=\"V$(TOPNAME)\"
LDFLAGS  += -lSDL2 -lSDL2_image -lSDL2_ttf

$(BIN): $(VSRCS) $(CSRCS) $(NVBOARD_ARCHIVE) | $(BUILD_DIR)
	@rm -rf $(OBJ_DIR)
	$(VERILATOR) $(VERILATOR_CFLAGS) $(VSRCS) \
		--top-module $(TOPNAME) \
		$(addprefix -CFLAGS , $(CXXFLAGS)) \
		$(addprefix -LDFLAGS , $(NVBOARD_ARCHIVE) $(LDFLAGS)) \
		--Mdir $(OBJ_DIR) --exe -o $(abspath $(BIN)) $(CSRCS)
	$(MAKE) -C $(OBJ_DIR) -f V$(TOPNAME).mk

all: default

run: $(BIN)
	@$(BIN)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: default all clean run