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
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"
#include "memory/vaddr.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <utils.h>


static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  nemu_state.state=NEMU_QUIT;
  return -1;
}

// 单步执行：si [N]，默认 N=1
static int cmd_si(char *args) {
  int n = 1;
  if (args != NULL) {
    // 跳过前导空格
    while (*args == ' ') args++;
    if (*args != '\0') {
      char *end = NULL;
      int v = strtol(args, &end, 10);
      if (end == args) {
        printf("Usage: si [N]\n");
        return 0;
      }
      if (v <= 0) v = 1;
      n = v;
    }
  }
  cpu_exec(n);
  return 0;
}

// 扫描内存：x N EXPR
static int cmd_x(char *args) {
  if (args == NULL) {
    printf("Usage: x N EXPR\n");
    return 0;
  }

  // N EXPR
  char *n_str = strtok(args, " \t");
  char *expr_str = strtok(NULL, " \t");

  if (n_str == NULL || expr_str == NULL) {
    printf("Usage: x N EXPR\n");
    return 0;
  }

  char *end = NULL;
  int n = strtol(n_str, &end, 10);
  if (end == n_str || n <= 0) {
    printf("Invalid N: %s\n", n_str);
    return 0;
  }

  unsigned long long addr = 0;
  if (expr_str[0] == '0' && (expr_str[1] == 'x' || expr_str[1] == 'X')) {
    addr = strtoull(expr_str, &end, 16);
  } else {
    addr = strtoull(expr_str, &end, 10);
  }
  if (end == expr_str) {
    printf("Invalid EXPR ,expect hex or dec %s\n", expr_str);
    return 0;
  }

  // riscv32 每项读4字节，逐项输出
  for (long i = 0; i < n; i++) {
    uint32_t data = vaddr_read((vaddr_t)(addr + i * 4), 4);
    printf("0x%08llx: 0x%08x\n", addr + i * 4, data);
  }
  return 0;
}

//p EXPR 测试expr（）
static int cmd_p(char *args){
  if(args==NULL){
    printf("Udsge: p EXPR\n");
    return 0;
  }
  bool ok = false;
  word_t val = expr(args,&ok);
  if(!ok){
    printf("Bad expression: %s\n", args);
    return 0;
  }
  printf("= 0x%08x (%u)\n", (unsigned)val, (unsigned)val);
  return 0;
}

#ifdef CONFIG_WATCHPOINT
// w EXPR 设置监视点
static int cmd_w(char *args) {
  if (args == NULL) {
    printf("Usage: w EXPR\n");
    return 0;
  }
  while (*args==' '||*args=='\t') args++;
  if (*args=='\0') {
    printf("Usage: w EXPR\n");
    return 0;
  }
  // 申请监视点
  WP *wp = new_wp();
  // 保存表达式
  strncpy(wp->expr, args, sizeof(wp->expr)-1);
  wp->expr[sizeof(wp->expr)-1] = '\0';
  // 求初值
  bool ok = true;
  word_t v = expr(wp->expr, &ok);
  if (!ok) {
    printf("Invalid expression: %s\n", wp->expr);
    free_wp(wp);
    return 0;
  }
  wp->last_val = (uint32_t)v;
  printf("Watchpoint %d set: %s = %u (0x%08x)\n",
         wp->NO, wp->expr, wp->last_val, wp->last_val);
  return 0;
}

// d N 删除监视点 
static int cmd_d(char *args) {
  if (args == NULL) {
    printf("Usage: d N\n");
    return 0;
  }
  while (*args==' '||*args=='\t') args++;
  if (*args=='\0') { printf("Usage: d N\n"); return 0; }

  char *end=NULL;
  int no = strtol(args,&end,10);
  if (end==args || no<0) {
    printf("Bad number: %s\n", args);
    return 0;
  }
  WP *wp = find_wp(no);
  if (!wp) {
    printf("No such watchpoint: %d\n", no);
    return 0;
  }
  free_wp(wp);
  printf("Deleted watchpoint %d\n", no);
  return 0;
}

// info r | w：打印寄存器 | 监视点
static int cmd_info(char *args) {
  if (args == NULL) {
    printf("Plesase type: info r | info w\n");
    return 0;
  }
  while (*args == ' ') args++;
  if (args[0] == 'r' && (args[1] == '\0')) {
    isa_reg_display();
  } 
  else if (args[0] == 'w' && (args[1] == '\0')){
    if (!head) {
      printf("No watchpoints.\n");
    }
    else {
      printf("No.\tReg\t\tValue(dec)\tValue(hex)\n");
      for (WP *p = head; p; p = p->next) {
        printf("%3d\t%s\t\t%u\t0x%08x\n",
               p->NO,p->expr,
               p->last_val, p->last_val);
      }
    }
  }
  else {
    printf("Unknown subcommand for info: %s\n", args);
    printf("Usage: info r | info w\n");
  }
  return 0;
}
#else
//在关闭时提示
static int cmd_w(char *args){(void)args;puts("Watchpoints disabled.");return 0; }
static int cmd_d(char *args){(void)args;puts("Watchpoints disabled.");return 0; }
// info 仅 r 
static int cmd_info(char *args) {
  if (args == NULL) {
    printf("Please type: info r | info w\n");
    return 0;
  }
  while (*args == ' ') args++;
  if (args[0] == 'r' && (args[1] == '\0')) {
    isa_reg_display();
  }
  else if (args[0] == 'w' && (args[1] == '\0')){
    printf("Watchpoints disabled.\n");
  }
  else {
    printf("Unknown subcommand for info: %s\n", args);
    printf("Usage: info r\n");
  }
  return 0; 
}
#endif

//test-expr PATH: 批量读PATH，expr()校验
static int cmd_test_expr(char *args) {
  if (args == NULL) {
    printf("Usage: test-expr PATH\n");
    return 0;
  }
  while (*args == ' ' || *args == '\t') args++;
  if (*args == '\0') {
    printf("Usage: test-expr PATH\n");
    return 0;
  }

  const char *path = args;
  FILE *fp = fopen(path, "r");
  if (fp == NULL) {
    printf("Cannot open input file: %s\n", path);
    return 0;
  }

  char line[1 << 16];
  int total = 0, fail = 0;
  while (fgets(line, sizeof(line), fp)) {
    total++;
    char *p = line;
    while (*p == ' ' || *p == '\t') p++;
    if (*p == '\0' || *p == '\n') continue;

    unsigned expected = 0;
    int off = 0;
    // if (sscanf(p, "%u %n", &expected, &off) != 1) {
    //   printf("[WARN] Bad line: %s", line);
    //   continue;
    // }
    sscanf(p, "%u %n", &expected, &off);
    char *expr_str = p + off;
    expr_str[strcspn(expr_str, "\r\n")] = '\0';

    bool ok = true;
    word_t got = expr(expr_str, &ok);
    if (!ok) {
      printf("[FAIL LINE:%d] expr() parse/eval failed | %s\n", total,expr_str);
      fail++;
    } else if (got != (word_t)expected) {
      printf("[FAIL LINE:%d] expect=%u got=%u | %s\n", total , expected, (unsigned)got, expr_str);
      fail++;
    }
  }
  fclose(fp);
  printf("[SUMMARY] %d cases, %d failed\n", total, fail);
  return 0;
}

static int cmd_help(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);//1111
} cmd_table [] = {
  { "help", "Display information about all supported commands",    cmd_help },
  { "c", "Continue the execution of the program",                  cmd_c },
  { "q", "Exit NEMU",                                              cmd_q },
  { "si",   "Single-step execute N instructions (default 1)",      cmd_si   },
  { "x",    "Scan memory: x N EXPR (EXPR is a hex/dec immediate)", cmd_x    },
  { "p",    "Evaluate expression: p EXPR",                         cmd_p },
  { "test-expr", "Run expressions from file: test-expr PATH",      cmd_test_expr },
  { "w",    "Set a watchpoint: w EXPR",                            cmd_w },                 
  { "d",    "Delete a watchpoint: d N",                            cmd_d },
  #ifdef CONFIG_WATCHPOINT
  { "info", "info r | info w: print registers | watchpoints",      cmd_info },
  #else
  { "info", "print registers",                             cmd_info },  
  #endif
  /* TODO: Add more commands */
};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}
void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }
  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  #ifdef CONFIG_WATCHPOINT
  /* Initialize the watchpoint pool. */
  init_wp_pool();
  #endif
}
