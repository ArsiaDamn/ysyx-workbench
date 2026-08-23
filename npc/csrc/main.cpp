#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>  
#include "Vtop.h"
#include "verilated.h"
#include "verilated_vcd_c.h"
#include <stdint.h>

#ifdef USE_NVBOARD
#include <nvboard.h>
void nvboard_bind_all_pins(Vtop* top);
#endif


// DPI-C: Trap
static bool ebreak = false;
extern "C" void npc_trap() {
    printf("[NPC] HIT GOOD TRAP: ebreak.\n");
    ebreak = true;
}
static const uint32_t PMEM_SIZE = 64 * 1024;
static uint8_t pmem[PMEM_SIZE];

static inline uint32_t pmem_read32(uint32_t addr) {
  if (addr >= PMEM_SIZE) return 0;
  return (uint32_t)pmem[addr] |
         ((uint32_t)pmem[addr + 1] << 8) |
         ((uint32_t)pmem[addr + 2] << 16) |
         ((uint32_t)pmem[addr + 3] << 24);
}

static inline void pmem_write32(uint32_t addr, uint32_t data, uint8_t wmask) {
  if (addr >= PMEM_SIZE) return;
  if (wmask & 0x1) pmem[addr]     = data & 0xff;
  if (wmask & 0x2) pmem[addr + 1] = (data >> 8) & 0xff;
  if (wmask & 0x4) pmem[addr + 2] = (data >> 16) & 0xff;
  if (wmask & 0x8) pmem[addr + 3] = (data >> 24) & 0xff;
}

// DPI-C: RTL的通用访存接口 (lw/sw)
extern "C" int pmem_read(int raddr) {
  uint32_t addr = (uint32_t)raddr & ~0x3u;
  return pmem_read32(addr);
}

extern "C" void pmem_write(int waddr, int wdata, char wmask) {
  uint32_t addr = (uint32_t)waddr & ~0x3u;
  pmem_write32(addr, wdata, (uint8_t)wmask);
}

static void init_pmem() {
  printf("No image provided. Using default init_pmem().\n");
// ====================== 最终核对版 · 完整测试集 ======================
// 初始PC=0x00000000 | 完全匹配官方反汇编 | 格式统一
// ======================================================================
pmem_write32(0x00000000, 0x07b06b93u, 0xF);  // ori  x23,x0,123        // x23=0x0000007B
pmem_write32(0x00000004, 0x67806c13u, 0xF);  // ori  x24,x0,0x678      // x24=0x00000678
pmem_write32(0x00000008, 0x00806093u, 0xF);  // ori  x1,x0,8           // x1=0x00000008
pmem_write32(0x0000000c, 0x00c06113u, 0xF);  // ori  x2,x0,12          // x2=0x0000000C
pmem_write32(0x00000010, 0x00006193u, 0xF);  // ori  x3,x0,0           // x3=0x00000000
pmem_write32(0x00000014, 0x001105b3u, 0xF);  // add  x11,x2,x1         // x11 = x2+x1 = 0x00000014
pmem_write32(0x00000018, 0x40110633u, 0xF);  // sub  x12,x2,x1         // x12 = x2-x1 = 0x00000004
pmem_write32(0x0000001c, 0x00110693u, 0xF);  // addi x13,x2,1          // x13 = x2+1 = 0x0000000D
pmem_write32(0x00000020, 0x00316733u, 0xF);  // or   x14,x2,x3         // x14 = x2|x3 = 0x0000000C
pmem_write32(0x00000024, 0x0020f7b3u, 0xF);  // and  x15,x1,x2         // x15 = x1&x2 = 0x00000008
pmem_write32(0x00000028, 0x001149b3u, 0xF);  // xor  x19,x2,x1         // x19 = x2^x1 = 0x00000004
pmem_write32(0x0000002c, 0x00406213u, 0xF);  // ori  x4,x0,4           // x4=0x00000004
pmem_write32(0x00000030, 0x004113b3u, 0xF);  // sll  x7,x2,x4          // x7 = x2<<4 = 0x000000C0
pmem_write32(0x00000034, 0x08006793u, 0xF);  // ori  x15,x0,128        // x15=0x00000080
pmem_write32(0x00000038, 0x0047d333u, 0xF);  // srl  x6,x15,x4         // x6 = x15>>4 = 0x00000008
pmem_write32(0x0000003c, 0x4047d2b3u, 0xF);  // sra  x5,x15,x4         // x5 = x15>>>4= 0x00000008
pmem_write32(0x00000040, 0x00406213u, 0xF);  // ori  x4,x0,4           // x4=0x00000004
pmem_write32(0x00000044, 0xfe40ae23u, 0xF);  // sw   x4,-4(x1)         // mem[0x00000004] = 0x00000004
pmem_write32(0x00000048, 0x00202023u, 0xF);  // sw   x2,0(x0)          // mem[0x00000000] = 0x0000000C
pmem_write32(0x0000004c, 0x00302223u, 0xF);  // sw   x3,4(x0)          // mem[0x00000004] = 0x00000000
pmem_write32(0x00000050, 0xff80a283u, 0xF);  // lw   x5,-8(x1)         // x5 = mem[0x00000000] = 0x0000000C
pmem_write32(0x00000054, 0x004112b3u, 0xF);  // sll  x5,x2,x4          // x5 = 0x000000C0
pmem_write32(0x00000058, 0x00110193u, 0xF);  // addi x3,x2,1           // x3 = x2+1 = 0x0000000D
pmem_write32(0x0000005c, 0x0001e133u, 0xF);  // or   x2,x3,x0          // x2 = x3|x0 = 0x0000000D
pmem_write32(0x00000060, 0xfe519ce3u, 0xF);  // bne  x3,x5,_addi       // pc=0x00000058 ,x3=0xC0 == x5=0xC0 → PC=0x64

pmem_write32(0x00000064, 0x04c00e93u, 0xF);  // addi x29, x0, 76         // x29 = 0x0000004C
pmem_write32(0x00000068, 0x0ab00d93u, 0xF);  // addi x27, x0, 171        // x27 = 0x000000AB
pmem_write32(0x0000006C, 0x01bea223u, 0xF);  // sw   x27, 4(x29)         // mem[x29+4] = x27  (即 mem[0x50] = 0xAB)
pmem_write32(0x00000070, 0x0100006fu, 0xF);  // jal  x0, 16              // rd=x0(不存)，PC 跳至 0x70+16 = 0x00000080
pmem_write32(0x00000074, 0x0000e013u, 0xF);  // ori  x0, x1, 0           // NOP (跳过)
pmem_write32(0x00000078, 0x0000e013u, 0xF);  // ori  x0, x1, 0           // NOP (跳过)
pmem_write32(0x0000007C, 0x0000e013u, 0xF);  // ori  x0, x1, 0           // NOP (跳过)
pmem_write32(0x00000080, 0x004eae03u, 0xF);  // lw   x28, 4(x29)         // x28 = mem[x29+4] (即 x28 = mem[0x50] = 0xAB)
pmem_write32(0x00000084, 0x01cd8863u, 0xF);  // beq  x27, x28, 16        // 若 x27==x28，PC 跳至 0x84+16 = 0x00000094
pmem_write32(0x00000088, 0x0000e013u, 0xF);  // ori  x0, x1, 0           // NOP (跳过)
pmem_write32(0x0000008C, 0x0000e013u, 0xF);  // ori  x0, x1, 0           // NOP (跳过)
pmem_write32(0x00000090, 0x0000e013u, 0xF);  // ori  x0, x1, 0           // NOP (跳过)
pmem_write32(0x00000094, 0x004eaf03u, 0xF);  // lw   x30, 4(x29)         // x30 = mem[x29+4] (即 x30 = mem[0x50] = 0xAB)
pmem_write32(0x00000098, 0x00100073u, 0xF);  // ebreak
}
// load.hex 
static void load_image(const char *img_file) {
  FILE *fp = fopen(img_file, "r");
  if (fp == NULL) {
    printf("Can not open '%s'. Fallback to init_pmem().\n", img_file);
    init_pmem();
    return;
  }

  char line[512];
  uint32_t max_byte_addr = 0;

  while (fgets(line, sizeof(line), fp)) {
    if (strncmp(line, "v3.0", 4) == 0) continue;

    char *colon = strchr(line, ':');
    if (!colon) continue;

    *colon = '\0';
    uint32_t word_addr = 0;
    if (sscanf(line, "%x", &word_addr) != 1) continue;

    uint32_t byte_addr = word_addr * 4;
    char *token = strtok(colon + 1, " \t\r\n");
    
    while (token != NULL) {
      uint32_t inst;
      if (sscanf(token, "%x", &inst) == 1) {
        if (byte_addr < PMEM_SIZE) {
          pmem_write32(byte_addr, inst, 0xF);
          if (byte_addr > max_byte_addr) max_byte_addr = byte_addr;
        }
        byte_addr += 4;
      }
      token = strtok(NULL, " \t\r\n");
    }
  }

  fclose(fp);
  printf("Loaded %s successfully, max memory address: 0x%08x\n", img_file, max_byte_addr + 4);
}


int main(int argc, char** argv) {
  auto* contextp = new VerilatedContext;
  contextp->commandArgs(argc, argv);

  Vtop* top = new Vtop{contextp};

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

  if (argc > 1) {
    load_image(argv[1]);
  } else {
    init_pmem();
  }

  top->rst_n = 0;
  top->clk   = 0;

  const uint64_t max_steps = 20000; // 放大步数以防跑更长的文件超时
  uint64_t steps = 0;

  auto eval_one_half_cycle = [&]() {
    //top->inst = pmem_read32(top->pc);

    // if (top->mem_valid) {
    //   if (top->mem_wen) pmem_write32(top->mem_addr, top->mem_wdata, top->mem_wmask);
    //   else top->mem_rdata = pmem_read32(top->mem_addr);
    // } else {
    //   top->mem_rdata = 0;
    // }

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

  while (!contextp->gotFinish() && steps < max_steps && !ebreak) {
#ifdef USE_NVBOARD
    nvboard_update();
#endif
    eval_one_half_cycle();
    steps++;
  }

  if (ebreak) {
      printf("Simulation ended gracefully by ebreak (nemutrap).\n");
  } else if (steps >= max_steps) {
      printf("Simulation ended by step timeout.\n");
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