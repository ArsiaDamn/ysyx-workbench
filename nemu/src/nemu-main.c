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

#include <common.h>
#include <stdio.h>
#include <string.h>


void init_monitor(int, char *[]);
void am_init_monitor();
void engine_start();
int is_exit_status_bad();

word_t expr(char *e,bool *success);

int main(int argc, char *argv[]) {
  /* Initialize the monitor. */
#ifdef CONFIG_TARGET_AM
  am_init_monitor();
#else
  init_monitor(argc, argv);
#endif



/*
///////////////////////////////////////////////////////////////////////
  if (argc > 1) {
    const char *path = argv[1];
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
      printf("Cannot open input file: %s\n", path);
      return 1;
    }
    char line[1 << 16];   // 足够长
    unsigned total = 0, fail = 0;

    while (fgets(line, sizeof(line), fp)) {
      // 跳过空行
      char *p = line;
      while (*p == ' ' || *p == '\t') p++;
      if (*p == '\0' || *p == '\n') continue;

      // 解析“期望结果 + 空格 + 表达式”
      unsigned expected = 0;
      int off = 0;
      if (sscanf(p, "%u %n", &expected, &off) != 1) {
        // 行格式不对，跳过
        printf("[WARN] Bad line: %s", line);
        continue;
      }
      char *expr_str = p + off;
      // 去掉行尾换行
      expr_str[strcspn(expr_str, "\r\n")] = '\0';

      bool ok = true;
      word_t got = expr(expr_str, &ok);
      if (!ok) {
        printf("[FAIL] expr() failed: %s\n", expr_str);
        fail++;
      } else if (got != (word_t)expected) {
        printf("[FAIL] expect=%u got=%u | %s\n", expected, (unsigned)got, expr_str);
        fail++;
      }
        total++;
    }
    fclose(fp);
    printf("[SUMMARY] %u cases, %u failed\n", total, fail);
    return fail ? 1 : 0;
  }


/////////////////////////////////////////////////////////////////////////////////////////
*/
  /* Start engine. */
  engine_start();

  return is_exit_status_bad();
}
