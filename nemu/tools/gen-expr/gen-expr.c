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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>

// this should be enough
static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\\n\", result); "
"  return 0; "
"}";

static size_t pos = 0;           // buf position 
static int depth = 0;            
static int need_nonzero_rhs = 0; // nozero

//choose
static inline uint32_t choose(uint32_t n){
  return (uint32_t)(rand() % (int)n);
}

// write to buf safety
static void append(const char *fmt, ...) {
  if (pos >= sizeof(buf) - 1) return; // buf no space
  va_list ap;
  va_start(ap, fmt); //ap -> fmt+1
  size_t space = sizeof(buf) - 1 - pos;
  int w = vsnprintf(buf + pos, space, fmt, ap); //lenth
  va_end(ap);
  if (w < 0) return;
  if ((size_t)w > space) w = (int)space;
  pos += (size_t)w;
}

// gen()
static inline void gen(char c) {
  if (pos + 1 >= sizeof(buf)) return;
  buf[pos++] = c;
  buf[pos] = '\0';
}

// insert blank
static void gen_blank(void) {
  int k = (int)choose(3);
  while (k--) gen(' ');
}

// gen_num 0~100
static void gen_num(void) {
  unsigned x = (unsigned)choose(100);
  append("%u", x);
}

// gen_rand_op
static void gen_rand_op(void) {
  static const char ops[] = "+-*/";
  char op = ops[choose(4)];
  gen(op);
  //if (op == '/') need_nonzero_rhs = 1;
}

static void gen_rand_expr() {
  // if (need_nonzero_rhs) {                 
  //   unsigned x = 1u + (unsigned)choose(99);
  //   append("%u", x);
  //   need_nonzero_rhs = 0;
  //   return;
  // }

  const int MAX_DEPTH = 8;
  if (depth > MAX_DEPTH || pos > sizeof(buf) - 64) {
    gen_num();
    return;
  }

  switch (choose(3)) {
    case 0:
      gen_num();
      break;
    case 1:
      depth++;
      gen('(');gen_blank();gen_rand_expr();gen_blank();gen(')');
      depth--;
      break;
    default:
      depth++;
      gen_rand_expr();gen_blank();gen_rand_op();gen_blank();gen_rand_expr();
      depth--;
      break;
  }
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);

  int loop = 1; //run times
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }

  int i;
  for (i = 0; i < loop; i ++) {
    pos=0;buf[0]='\0';need_nonzero_rhs=0;
    gen_rand_expr();

    sprintf(code_buf, code_format, buf);
    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc -O0 -w -fsanitize=integer-divide-by-zero -fsanitize-undefined-trap-on-error /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;
    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);
    unsigned result=0;
    
    ret = fscanf(fp, "%u", &result);
    pclose(fp);
    if (ret != 1) {
      i--;
      //fprintf(stderr, "[READ-FAIL] cannot parse output, expr=\"%s\"\n", buf);
      continue;
    }

    printf("%u %s\n", result, buf);
  }

  return 0;
}
