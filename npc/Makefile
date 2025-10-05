.PHONY: all sim wave clean
TOP ?= top
VSRCS := $(wildcard vsrc/*.v)
CSRCS := $(wildcard csrc/*.cpp)
all:	sim
	@echo "Write this Makefile by your self."

sim:
	@if [ -z "$(VSRCS)" ]; then echo "no Verilog sources found in vsrc/"; exit 1; fi
	@if [ -z "$(CSRCS)" ]; then echo "no C/C++ sources found in csrc/"; exit 1; fi
	verilator -Wall --cc $(VSRCS) --top-module $(TOP) --exe $(CSRCS)
	$(MAKE) -C obj_dir -f V$(TOP).mk V$(TOP)
	./obj_dir/V$(TOP)
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@echo "Write this Makefile by your self."
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

include ../Makefile

