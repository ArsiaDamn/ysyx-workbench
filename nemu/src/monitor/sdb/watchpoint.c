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

#include "sdb.h"

#ifdef CONFIG_WATCHPOINT

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#define NR_WP 32
//#define WP_EXPR_MAX 256

/*
  //结构体已在 sdb.h 中定义
  typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  char expr[WP_EXPR_MAX];   // 保存输入的表达式
  uint32_t last_val;        // 上一次值
  // TODO: Add more members if necessary 

} WP;
*/

WP *head = NULL;
static WP wp_pool[NR_WP];
static WP *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
    wp_pool[i].expr[0] = '\0';
    wp_pool[i].last_val = 0;
  }
  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
// 取空表插入head
WP* new_wp() {
  if (free_ == NULL) {
    assert(0 && "No free watchpoint available, increase NR_WP if needed.");
  }
  WP *wp = free_;
  free_ = free_->next;
  wp->next = head;
  wp->expr[0] = '\0';
  wp->last_val = 0;
  head = wp;
  return wp;
}

// head释放到free_
void free_wp(WP *wp) {
  if (wp == NULL) return;
  // 从head删掉
  if (head == wp) {
    head = wp->next;
  } else {
    WP *prev = head;
    while (prev && prev->next != wp) prev = prev->next;
    assert(prev && "Attempt to free a watchpoint not in active list");
    if (prev) prev->next = wp->next;
  }

  // 放回free头
  wp->next = free_;
  free_ = wp;
}

// 遍历接口（info w）
WP* wp_head() {
  return head;
}

// NO. d命令
WP* find_wp(int no) {
  for (WP *p=head;p;p=p->next) {
    if (p->NO == no) return p;
  }
  return NULL;
}

// info w
void list_watchpoints() {
  if (!head) {
    printf("No watchpoints.\n");
    return;
  }
  printf("Num Expr                               Value(dec/hex)\n");
  for (WP *p = head; p; p = p->next) {
    printf("%3d %-32s %10u (0x%08x)\n",
           p->NO,p->expr,
           p->last_val, p->last_val);
  }
}

// 1触发
int check_watchpoints() {
  int trig = 0;
  for (WP *p = head; p; p = p->next) {
    if (p->expr[0] == '\0') continue; // 没有效
    bool ok = true;
    word_t val = expr(p->expr, &ok);
    if (!ok) {
      // 解析失败
      printf("Watchpoint %d: expr parse error: %s\n", p->NO, p->expr);
      continue;
    }
    if (val != p->last_val) {
      printf("Watchpoint %d triggered:\n", p->NO);
      printf("  expr: %s\n", p->expr);
      printf("  old: %u (0x%08x)\n", p->last_val, p->last_val);
      printf("  new: %u (0x%08x)\n", (unsigned)val, (unsigned)val);
      p->last_val = (uint32_t)val;
      trig = 1;
    }
  }
  return trig;
}

#else  // CONFIG_WATCHPOINT

WP *head = NULL;
void init_wp_pool(void) {}
WP* new_wp(void) { return NULL; }
void free_wp(WP *wp) { (void)wp; }
WP* find_wp(int no) { (void)no; return NULL; }
void list_watchpoints(void) { puts("Watchpoints disabled (CONFIG_WATCHPOINT=n)."); }
int  check_watchpoints(void) { return 0; }

#endif // CONFIG_WATCHPOINT