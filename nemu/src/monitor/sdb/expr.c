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

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <cpu/cpu.h>
#include "memory/vaddr.h"

enum {
  TK_NOTYPE = 256, 
  TK_EQ,   // ==
  TK_NEQ,  // !=
  TK_DEC,  // 10
  TK_HEX,  // 16
  TK_NEG,  // -
  TK_DEREF,// *p
  TK_NOT,  // !=
  TK_AND,  // &&
  TK_OR,   // ||
  TK_LT,   // <
  TK_LE,   // <=
  TK_GT,   // >
  TK_GE,   // >=
  TK_MOD,  // %
  TK_REG,  // 

  /* TODO: Add more token types */
};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"==", TK_EQ},        // =
  {"!=", TK_NEQ},       // !=
  {"<=", TK_LE},        // <=
  {">=", TK_GE},        // >=
  {"&&", TK_AND},       // && and
  {"\\|\\|", TK_OR},    // || or

  {"\\(",'('},          // (
  {"\\)",')'},          // )
  {"\\+", '+'},         // +
  {"-", '-'},           // -
  {"\\*", '*'},         // *
  {"/", '/'},           // /

  {"%", TK_MOD},        // %
  {"<", TK_LT},         // <
  {">", TK_GT},         // >
  {"!", TK_NOT},        // ! not

  {"0[xX][0-9a-fA-F]+", TK_HEX},         // 16
  {"[0-9]+", TK_DEC},                    // 10
  {"\\$[A-Za-z0-9]+", TK_REG},           // reg
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[1024] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0; //已识别token数

  while (e[position] != '\0') {
    
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position; //匹配的地址
        int substr_len = pmatch.rm_eo;     //匹配的长度

        // Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
        //     i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;
        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
        int ttype = rules[i].token_type;
        if (ttype == TK_NOTYPE){break;}

        assert(nr_token < (int)(sizeof(tokens)/sizeof(tokens[0])));  //yichu test
        tokens[nr_token].type = ttype; //识别到的类型

        if(ttype==TK_DEC||ttype==TK_HEX||ttype==TK_REG){
          if(substr_len>(int)sizeof(tokens[nr_token].str)-1){
            substr_len = (int)sizeof(tokens[nr_token].str)-1;
          }
          memcpy(tokens[nr_token].str,substr_start,substr_len);
          tokens[nr_token].str[substr_len] = '\0';
        }
        else {
          tokens[nr_token].str[0] = '\0';
        }
        nr_token++;
//      switch (rules[i].token_type) {default: TODO();}
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }
  return true;
  // for(int j=0;j<nr_token;j++)
  // {printf("token[%d]:type=%d,str=%s\n",j,tokens[j].type,tokens[j].str);}

}



// 一元-,TK_NEG
static void mark_unary_minus(void) {
  for (int i = 0; i < nr_token; i++) {
    if (tokens[i].type == '-') {
      if (i == 0) {
        tokens[i].type = TK_NEG;
      } else {
        int prev = tokens[i - 1].type;
        // 前不是右值结尾，'-'负号
        if (prev == '(' || prev == '+' || prev == '-' || prev == '*' ||
            prev == '/' || prev == TK_MOD || prev == TK_EQ || prev == TK_NEQ ||
            prev == TK_AND || prev == TK_OR ||
            prev == TK_LT || prev == TK_LE || prev == TK_GT || prev == TK_GE ||
            prev == TK_NOT || prev == TK_DEREF) {
          tokens[i].type = TK_NEG;
        }
      }
    }
  }  
}

// 一元*,TK_DEREF 
static void mark_deref(void) {
  for (int i = 0; i < nr_token; i++) {
    if (tokens[i].type == '*') {
      if (i == 0) {
        tokens[i].type = TK_DEREF;
      } else {
        int prev = tokens[i - 1].type;
        // 前不是右值结尾，'*'解引用
        if (!(prev == TK_DEC || prev == TK_HEX || prev == TK_REG || prev == ')')) {
          tokens[i].type = TK_DEREF;
        }
      }
    }
  }
}

// ( [p,q] )
static bool check_parentheses(int p, int q, bool *ok) {
  if (p > q) { *ok = false; return false; }
  if (tokens[p].type != '(' || tokens[q].type != ')') return false;

  int depth = 0;
  for (int i = p; i <= q; i++) {
    if (tokens[i].type == '(') depth++;
    else if (tokens[i].type == ')') {
      depth--;
      if (depth < 0) { *ok = false; return false; }
      // i<q，外括闭合
      if (depth == 0 && i < q) return false;
    }
  }
  if (depth != 0) { *ok = false; return false; }

  return true;
}

// 优先级,越小越低
static int precedence(int type) {
  switch (type) {
    case TK_OR:                                         return 0; // ||
    case TK_AND:                                        return 1; // &&
    case TK_EQ: case TK_NEQ:                            return 2; // == !=
    case TK_LT: case TK_LE: case TK_GT: case TK_GE:     return 3; // < <= > >=
    case '+': case '-':                                 return 4; // + -
    case '*': case '/': case TK_MOD:                    return 5; // * / %
    default:                                            return 99;
  }
}

// 主导运算符：最低优先级左结合
static int dominant_op(int p, int q, bool *ok) {
  int best = -1;        //目前主导算符位置
  int best_prec = 100;  //目前主导算符优先级
  int depth = 0;

  for (int i = p; i <= q; i++) {
    int t = tokens[i].type;

    if (t == '(') { depth++; continue; }
    if (t == ')') {
      depth--;
      if (depth < 0) { *ok = false; return -1; }
      continue;
    }
    if (depth > 0) continue;

    // 双运算符       
    if (t == TK_EQ || t == '+' || t == '-' || t == '*' || t == '/') {
      int prec = precedence(t);
      if (prec <= best_prec) {
        best_prec = prec;
        best = i;  
      }
    }
  }
  return best;
}

// 算tokens[p,q]
static word_t eval(int p, int q, bool *ok) {
  if (p > q) { *ok = false; return 0; }

  // 单token,数字或寄存器
  if (p == q) {
    int t = tokens[p].type;
    if (t == TK_DEC) {
      return (word_t)strtoul(tokens[p].str, NULL, 10);
    } 
    else if (t == TK_HEX) {
      return (word_t)strtoul(tokens[p].str, NULL, 16);
    } 
    else if (t == TK_REG) {
      const char *name = tokens[p].str + 1;  //$
      bool succ = false;
      word_t val = isa_reg_str2val(name, &succ);
      if (!succ) {
        printf("Unknown register: %s\n", tokens[p].str);
        *ok = false;
        return 0;
      }
      return val;
    } 
    else {
      *ok = false;
      return 0;
    }
  }

  // 去外层括号
  if (check_parentheses(p, q, ok)) {
    if (!*ok) return 0;
    return eval(p + 1, q - 1, ok);
  }
  if (!*ok) return 0;

  // TK_NEG，求右取负
  if (tokens[p].type == TK_NEG) {
    word_t rhs = eval(p + 1, q, ok);
    if (!*ok) return 0;
    int32_t s = -(int32_t)rhs;            
    return (word_t)(uint32_t)s;         
  }

  // 取非！
  if (tokens[p].type == TK_NOT) {
    word_t rhs = eval(p + 1, q, ok);
    if (!*ok) return 0;
    uint32_t r = ((int32_t)rhs == 0) ? 1u : 0u;
    return (word_t)r;
  }

  // 指针解
  if (tokens[p].type == TK_DEREF) {
    word_t addr = eval(p + 1, q, ok);
    if (!*ok) return 0;
    word_t val = vaddr_read((vaddr_t)addr, 4);
    return val;
  }

  // 找主导运算符
  int op = dominant_op(p, q, ok);
  if (!*ok) return 0;
  if (op < 0) { *ok = false; return 0; }

  // 递归计算左右两边
  word_t lhs = eval(p, op - 1, ok);
  if (!*ok) return 0;
  word_t rhs = eval(op + 1, q, ok);
  if (!*ok) return 0;

  //运算version1
/*  switch (tokens[op].type) {
    case TK_EQ: return (lhs == rhs) ? 1 : 0;
    case '+':   return lhs + rhs;
    case '-':   return lhs - rhs;
    case '*':   return lhs * rhs;
    case '/':
      if (rhs == 0) {
        printf("Division by zero\n");
        *ok = false;
        return 0;
      }
      return lhs / rhs;
    default:
      *ok = false;
      return 0;
  }
}
*/
//运算version2
  int32_t lhs_s = (int32_t)lhs;   int32_t rhs_s = (int32_t)rhs;
  switch (tokens[op].type) {
    case TK_OR: {
      uint32_t r = (lhs_s != 0 || rhs_s != 0) ? 1u : 0u;
      return (word_t)r;
    }
    case TK_AND: {
      uint32_t r = (lhs_s != 0 && rhs_s != 0) ? 1u : 0u;
      return (word_t)r;
    }
    case TK_EQ:  { uint32_t r = (lhs_s == rhs_s) ? 1u : 0u; return (word_t)r; }
    case TK_NEQ: { uint32_t r = (lhs_s != rhs_s) ? 1u : 0u; return (word_t)r; }

    case TK_LT:  { uint32_t r = (lhs_s <  rhs_s) ? 1u : 0u; return (word_t)r; }
    case TK_LE:  { uint32_t r = (lhs_s <= rhs_s) ? 1u : 0u; return (word_t)r; }
    case TK_GT:  { uint32_t r = (lhs_s >  rhs_s) ? 1u : 0u; return (word_t)r; }
    case TK_GE:  { uint32_t r = (lhs_s >= rhs_s) ? 1u : 0u; return (word_t)r; }

    case '+': {
      int64_t s = (int64_t)lhs_s + (int64_t)rhs_s;
      return (word_t)(uint32_t)(int32_t)s;  //截32位
    }
    case '-': {
      int64_t s = (int64_t)lhs_s - (int64_t)rhs_s;
      return (word_t)(uint32_t)(int32_t)s;
    } 
    case '*': {
      int64_t s = (int64_t)lhs_s * (int64_t)rhs_s;
      return (word_t)(uint32_t)(int32_t)s;
    }
    case '/': {
      if (rhs_s == 0) { printf("Division by zero\n"); *ok = false; return 0; }
      int32_t s = lhs_s / rhs_s;      
      return (word_t)(uint32_t)s;
    }
    case TK_MOD: {
      if (rhs_s == 0) { printf("Modulo by zero\n"); *ok = false; return 0; }
      int32_t s = lhs_s % rhs_s;      
      return (word_t)(uint32_t)s;
    }
    default:
      *ok = false; return 0;
  }
}


// 6666666666666666666666666666666666666666666666666666666666666666666666
word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  mark_unary_minus();
  mark_deref();

  bool ok = true;
  word_t val = eval(0,nr_token-1,&ok);
  *success = ok;
  return ok?val:0;

/* 
  //TODO: Insert codes to evaluate the expression. 
  TODO();
*/
  return 0;
}
