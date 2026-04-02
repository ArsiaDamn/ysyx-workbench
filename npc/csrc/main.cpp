#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "Vtop.h"
#include "verilated.h"
#include "verilated_vcd_c.h"
#include <stdint.h>

#ifdef USE_NVBOARD
#include <nvboard.h>
void nvboard_bind_all_pins(Vtop* top);
#endif

// #define WAVE

static const uint32_t PMEM_SIZE = 64 * 1024;
static uint8_t pmem[PMEM_SIZE];

static inline uint32_t pmem_read32(uint32_t addr) {
  return (uint32_t)pmem[addr] |
         ((uint32_t)pmem[addr + 1] << 8) |
         ((uint32_t)pmem[addr + 2] << 16) |
         ((uint32_t)pmem[addr + 3] << 24);
}

static inline void pmem_write32(uint32_t addr, uint32_t data, uint8_t wmask) {
  if (wmask & 0x1) pmem[addr]     = data & 0xff;
  if (wmask & 0x2) pmem[addr + 1] = (data >> 8) & 0xff;
  if (wmask & 0x4) pmem[addr + 2] = (data >> 16) & 0xff;
  if (wmask & 0x8) pmem[addr + 3] = (data >> 24) & 0xff;
}

static void init_pmem() {
  // 最小 addi 程序（示例）
  pmem_write32(0x0, 0x00100093u, 0xF); // addi x1,x0,1
  pmem_write32(0x4, 0x00208113u, 0xF); // addi x2,x1,2
  pmem_write32(0x8, 0x00000013u, 0xF); // nop
  pmem_write32(0xC, 0x00000013u, 0xF); // nop
}

int main(int argc, char** argv) {
  auto* contextp = new VerilatedContext;
  contextp->commandArgs(argc, argv);

  Vtop* top = new Vtop{contextp};     // 先 new top，后 trace

  VerilatedVcdC* m_trace = nullptr;
#ifdef WAVE
  contextp->traceEverOn(true);
  m_trace = new VerilatedVcdC;
  top->trace(m_trace, 99);
  m_trace->open("wave.vcd");
#endif

#ifdef USE_NVBOARD
  nvboard_bind_all_pins(top);
  nvboard_init();
#endif

  init_pmem();

  top->rst_n = 0;
  top->clk   = 0;

  // 仿真步数上限，避免没写 finish 时跑死
  const uint64_t max_steps = 2000;
  uint64_t steps = 0;

  auto eval_one_half_cycle = [&]() {
    // 注入指令（每次 eval 前都按当前 pc 取指）
    top->inst = pmem_read32(top->pc);

    if (top->mem_valid) {
      if (top->mem_wen) pmem_write32(top->mem_addr, top->mem_wdata, top->mem_wmask);
      else top->mem_rdata = pmem_read32(top->mem_addr);
    } else {
      top->mem_rdata = 0;
    }

    top->clk = !top->clk;
    top->eval();
    contextp->timeInc(1);

#ifdef WAVE
    m_trace->dump(contextp->time());
#endif
  };

  // reset
  for (int i = 0; i < 10; i++) eval_one_half_cycle();
  top->rst_n = 1;

  while (!contextp->gotFinish() && steps < max_steps) {
#ifdef USE_NVBOARD
    nvboard_update();
#endif
    eval_one_half_cycle();
    steps++;
  }

#ifdef WAVE
  m_trace->close();
  delete m_trace;
#endif
#ifdef USE_NVBOARD
  nvboard_quit();
#endif
  delete top;
  delete contextp;
  return 0;
}
