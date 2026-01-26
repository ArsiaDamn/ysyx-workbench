.PHONY: all sim wave clean
TOP ?= top
VSRCS := $(wildcard vsrc/*.v)
CSRCS := $(wildcard csrc/*.cpp)
all:	sim
	@echo "Write this Makefile by your self."

sim:
	@if [ -z "$(VSRCS)" ]; then echo "no Verilog sources found in vsrc/"; exit 1; fi
	@if [ -z "$(CSRCS)" ]; then echo "no C/C++ sources found in csrc/"; exit 1; fi
	verilator --trace -Wall --cc $(VSRCS) --top-module $(TOP) --exe $(CSRCS)
	$(MAKE) -C obj_dir -f V$(TOP).mk V$(TOP)
	./obj_dir/V$(TOP)
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@echo "verilator is working..."
	
wave:
	@if [ -z "$(VSRCS)" ]; then echo "no Verilog sources found in vsrc/"; exit 1; fi
	@if [ -z "$(CSRCS)" ]; then echo "no C/C++ sources found in csrc/"; exit 1; fi
	verilator -Wall --trace --cc $(VSRCS) --top-module $(TOP) --exe $(CSRCS)
	$(MAKE) -C obj_dir -f V$(TOP).mk V$(TOP)
	./obj_dir/V$(TOP)
	@if [ -f wave.vcd ]; then \
		if command -v gtkwave >/dev/null 2>&1; then gtkwave wave.vcd & else echo "gtkwave not found. install: sudo apt-get install -y gtkwave"; fi; \
	else \
		echo "wave.vcd not found. ensure your C++ testbench enables tracing."; \
	fi	


clean:
	rm -rf obj_dir wave.vcd wave.fst

#//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TOPNAME ?= $(TOP)
INC_PATH ?= $(NVBOARD_HOME)/usr/include
# 约束文件（默认 constr/top.nxdc；按实际工程可改）
NXDC_FILES ?= constr/top.nxdc

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
NV_VSRCS = $(shell find $(abspath ./vsrc) -name "*.v")
NV_CSRCS = $(shell find $(abspath ./csrc) -name "*.c" -or -name "*.cc" -or -name "*.cpp")
NV_CSRCS += $(SRC_AUTO_BIND)

# NVBoard rules
include $(NVBOARD_HOME)/scripts/nvboard.mk

# flags for verilator
INCFLAGS := $(addprefix -I, $(INC_PATH))
CXXFLAGS += $(INCFLAGS) -DTOP_NAME=\"V$(TOPNAME)\"
LDFLAGS  += -lSDL2 -lSDL2_image -lSDL2_ttf


$(BIN): $(NV_VSRCS) $(NV_CSRCS) $(NVBOARD_ARCHIVE) | $(BUILD_DIR)
	@rm -rf $(OBJ_DIR)
	$(VERILATOR) $(VERILATOR_CFLAGS) $(NV_VSRCS) \
		--top-module $(TOPNAME) \
		$(addprefix -CFLAGS , $(CXXFLAGS)) \
		-CFLAGS -DUSE_NVBOARD \
		$(addprefix -LDFLAGS , $(NVBOARD_ARCHIVE) $(LDFLAGS)) \
		--Mdir $(OBJ_DIR) --exe -o $(abspath $(BIN)) $(NV_CSRCS)
	$(MAKE) -C $(OBJ_DIR) -f V$(TOPNAME).mk

# 交互仿真入口：make run
run: $(BIN)
	@$(BIN)

clean_nv:
	rm -rf $(BUILD_DIR)

clean: clean_nv

.PHONY: run clean_nv
#//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
include ../Makefile

