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
// 取空闲链表，插入 head，返回指针
WP* new_wp() {
  if (free_ == NULL) {
    // 池用完了assert
    assert(0 && "No free watchpoint available, increase NR_WP if needed.");
  }
  WP *wp = free_;
  free_ = free_->next;

  // 初始化节点
  wp->next = head;
  wp->expr[0] = '\0';
  wp->last_val = 0;

  head = wp;
  return wp;
}

// 释放监视点到free链
void free_wp(WP *wp) {
  if (wp == NULL) return;
  // 从 head 链删掉
  if (head == wp) {
    head = wp->next;
  } else {
    WP *prev = head;
    while (prev && prev->next != wp) prev = prev->next;
    //防止wp不在head链
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

// NO 查找（d 命令）
WP* find_wp(int no) {
  for (WP *p=head;p;p=p->next) {
    if (p->NO == no) return p;
  }
  return NULL;
}

// 列出所有活动监视点
void list_watchpoints() {
  if (!head) {
    printf("No watchpoints.\n");
    return;
  }
  printf("Num Expr                               Value(dec/hex)\n");
  for (WP *p = head; p; p = p->next) {
    printf("%3d %-32s %10u (0x%08x)\n",
           p->NO,
           p->expr[0] ? p->expr : "(unset)",
           p->last_val, p->last_val);
  }
}

// 监视点检查（ cpu_exec 调用）
// 1 触发，0无变化
int check_watchpoints() {
  int trig = 0;
  for (WP *p = head; p; p = p->next) {
    if (p->expr[0] == '\0') continue; // 没有有效表达式
    bool ok = true;
    word_t val = expr(p->expr, &ok);
    if (!ok) {
      // 表达式解析失败
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
      // 不立即 return，可显示多个变化；
    }
  }
  return trig;
}
