#include <cstdio>
#include <cstdlib>
#include <cassert>

#include "verilated.h"
#include "Vtop.h"

#if VM_TRACE
#include "verilated_vcd_c.h"
#endif

#ifdef USE_NVBOARD
#include "nvboard.h"
void nvboard_bind_all_pins(Vtop* top);
#endif

// 仿真时间
static vluint64_t main_time = 0;
double sc_time_stamp() { return main_time; }

//NVBoard
#ifdef USE_NVBOARD
static int run_with_nvboard(int argc, char** argv) {
  printf("NVBoard is working...\n");
  Verilated::commandArgs(argc, argv);

  Vtop dut;
  nvboard_bind_all_pins(&dut);
  nvboard_init();

  while (!Verilated::gotFinish()) {
    nvboard_update();
    dut.eval();
    main_time++;
  }

  nvboard_quit();
  return 0;
}
#endif

//Verilator
static int run_with_verilator(int argc, char** argv) {
  
  Verilated::commandArgs(argc, argv);

  Vtop* top = new Vtop;

#if VM_TRACE
  Verilated::traceEverOn(true);
  VerilatedVcdC* tfp = new VerilatedVcdC;
  top->trace(tfp, 0);
  tfp->open("wave.vcd");
#endif
  // 随机测试（N组）
  const int N = 16;
  for (int i = 0; i < N; ++i) {
    int a = rand() & 1;
    int b = rand() & 1;

    top->a = a;
    top->b = b;
    top->eval();

    int f_hw  = top->f;
    int f_ref = (a ^ b);

    printf("[t=%02llu] a=%d  b=%d  f=%d  (ref=%d)%s\n",
           (unsigned long long)main_time, a, b, f_hw, f_ref,
           (f_hw == f_ref) ? "" : " <-- MISMATCH");
    assert(f_hw == f_ref);

#if VM_TRACE
    tfp->dump(main_time);
#endif
    main_time++;
  }

#if VM_TRACE
  tfp->close();
  delete tfp;
#endif
  delete top;

  printf("Trace finished. Generated wave.vcd\n");
  return 0;
}

//入口：用宏切换两条路径 
int main(int argc, char** argv) {
#ifdef USE_NVBOARD
  return run_with_nvboard(argc, argv);
#else
  return run_with_verilator(argc, argv);
#endif
}