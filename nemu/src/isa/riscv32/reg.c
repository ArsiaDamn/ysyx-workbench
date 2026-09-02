/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>
#include "local-include/reg.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

const char *regs[] = {
  "zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

void isa_reg_display() {
	for (int i = 0; i < 32; i++) {
    		printf("%-3s 0x%08x\n", reg_name(i), cpu.gpr[i]);
  	}
  printf("pc  0x%08x\n", cpu.pc);
}

/// xi
static int xi(const char *s) {
  if (strcmp(s, "fp") == 0) return 8;   // s0
  if (strcmp(s, "x0") == 0) return 0;   // zero
  if (s[0] == 'x' && isdigit((unsigned char)s[1])) {
    int idx = 0;
    for (int i = 1; s[i]; i++) {
      if (!isdigit((unsigned char)s[i])) return -1;
      idx = idx * 10 + (s[i] - '0');
    }
    if (idx < 32) return idx;
  }
  return -1;
}

///卡了半天发现原来框架代码还没写完，濠神我爱你一万年///
word_t isa_reg_str2val(const char *s, bool *success) {
  //ABI
  for (int i = 0; i < 32; i++) {
    if (strcmp(s, regs[i]) == 0) {
      if (success) *success = true;
      return (word_t)cpu.gpr[i];
    } 
  }
  // x[i]
  int idx = xi(s);
  if (idx >= 0) {
    if (success) *success = true;
    return (word_t)cpu.gpr[idx];
  }
  //pc
  if (strcmp(s, "pc") == 0) {
    if (success) *success = true;
    return (word_t)cpu.pc;
  }
  if (success) *success = false;
  return 0;
}
