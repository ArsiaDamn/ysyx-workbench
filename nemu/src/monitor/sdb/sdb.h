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

#ifndef __SDB_H__
#define __SDB_H__

#include <common.h>
#include <stdint.h>
#include <stdbool.h>

////////////// add ////////////////
typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  char expr[256];   // 保存输入的表达式
  uint32_t last_val;        // 上一次值
  // TODO: Add more members if necessary 
} WP;

extern WP *head;

void init_wp_pool(void);
WP* new_wp(void);                 
void free_wp(WP *wp);         
WP* find_wp(int no);          
void list_watchpoints(void);      
int  check_watchpoints(void);     
////////////// add ////////////////

word_t expr(char *e, bool *success);
#endif
