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
enum {
  TK_NOTYPE = 256, TK_EQ,TK_DEC,TK_HEX,TK_REG,TK_NEG,
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
  {"==", TK_EQ},        // equal
  {"\\(",'('},          // (
  {"\\)",')'},          // )
  {"\\+", '+'},         // +
  {"-", '-'},           // -
  {"\\*", '*'},         // *
  {"/", '/'},           // /
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

  nr_token = 0;

  while (e[position] != '\0') {
    

    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;
        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
        int ttype = rules[i].token_type;
        
        if (ttype == TK_NOTYPE){break;}

        assert(nr_token < (int)(sizeof(tokens)/sizeof(tokens[0])));  //yichu test
        tokens[nr_token].type = ttype;

        if(ttype==TK_DEC||ttype==TK_HEX||ttype==TK_REG){
          int copy_len = substr_len;
          if(copy_len>(int)sizeof(tokens[nr_token].str)-1){
            copy_len = (int)sizeof(tokens[nr_token].str)-1;
          }
          memcpy(tokens[nr_token].str,substr_start,copy_len);
          tokens[nr_token].str[copy_len] = '\0';
        }
        else {
          tokens[nr_token].str[0] = '\0';
        }

        nr_token++;

/*        switch (rules[i].token_type) {
          default: TODO();
        }
*/
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }
/*  for(int j=0;j<nr_token;j++)
  {printf("token[%d]:type=%d,str=%s\n",j,tokens[j].type,tokens[j].str);}
*/
  return true;
}
// 666666666666666666666666666666666666666666666666666666666666666666666
/* 选做：把一元 '-' 标记成 TK_NEG*/
static void mark_unary_minus(void) {
  for (int i = 0; i < nr_token; i++) {
    if (tokens[i].type == '-') {
      if (i == 0) {
        tokens[i].type = TK_NEG;
      } else {
        int prev = tokens[i - 1].type;
        // 前一个不是“右值结尾”，那当前 '-' 是一元负号
        if (prev == '(' || prev == '+' || prev == '-' || prev == '*' || prev == '/' || prev == TK_EQ) {
          tokens[i].type = TK_NEG;
        }
      }
    }
  }
}

/* 判断 [p..q] 是否被“一对外层括号”完整包住,括号不匹配时会把 *ok 置为 false*/
static bool check_parentheses(int p, int q, bool *ok) {
  if (p > q) { *ok = false; return false; }
  if (tokens[p].type != '(' || tokens[q].type != ')') return false;

  int depth = 0;
  for (int i = p; i <= q; i++) {
    if (tokens[i].type == '(') depth++;
    else if (tokens[i].type == ')') {
      depth--;
      if (depth < 0) { *ok = false; return false; }
      // 如果在中途（i < q）深度回到 0，说明外层括号提前闭合，不是整段包住
      if (depth == 0 && i < q) return false;
    }
  }
  if (depth != 0) { *ok = false; return false; }

  return true;
}

/* 运算符优先级（数字越小优先级越低） */
static int precedence(int type) {
  switch (type) {
    case TK_EQ: return 0;           // 最低：==
    case '+':
    case '-':   return 1;           // 加减
    case '*':
    case '/':   return 2;           // 乘除
    default:    return 100;         // 非双目运算
  }
}

/* 在 [p..q] 内寻找“主导运算符”的下标：
 * 找不到返回 -1，并在错误情况时将 *ok=false
*/
static int dominant_op(int p, int q, bool *ok) {
  int best = -1;
  int best_prec = 100;
  int depth = 0;

  for (int i = p; i <= q; i++) {
    int t = tokens[i].type;

    if (t == '(') { depth++; continue; }
    if (t == ')') {
      depth--;
      if (depth < 0) { *ok = false; return -1; }
      continue;
    }
    if (depth > 0) continue; // 括号内忽略

    // 只考虑双目运算符
    if (t == TK_EQ || t == '+' || t == '-' || t == '*' || t == '/') {
      int prec = precedence(t);
      if (prec < best_prec) {
        best_prec = prec;
        best = i;           // 更低优先级，直接更新
      }
      // 同级不更新，保持最左（实现左结合）
    }
  }
  return best;
}

/* 把 tokens[p..q] 计算出一个值。ok=false 表示出错。 */
static word_t eval(int p, int q, bool *ok) {
  if (p > q) { *ok = false; return 0; }

  // 1) 单个 token：必须是数字或寄存器
  if (p == q) {
    int t = tokens[p].type;
    if (t == TK_DEC) {
      return (word_t)strtoul(tokens[p].str, NULL, 10);
    } 
    else if (t == TK_HEX) {
      return (word_t)strtoul(tokens[p].str, NULL, 16);
    } 
    else if (t == TK_REG) {
      // 跳过前缀 '$'，询问寄存器值
      const char *name = tokens[p].str + 1;
      bool succ = false;
      word_t val = isa_reg_str2val(name, &succ);

      //////////////////////1111111111111111111111111111111111111111111111

      if (!succ &&(strcmp(name,"pc")==0)){
        succ = true;
        val = cpu.pc;
      }
      ///////////////////////1111111111111111111111111111111111111111111111

      if (!succ) {
        printf("Unknown register: %s\n", tokens[p].str);
        *ok = false;
        return 0;
      }
      return val;
    } 
    else {
      // 单个 token 但不是可直接求值的类型
      *ok = false;
      return 0;
    }
  }

  // 2) 如果整段被外层括号包住，去掉一层括号
  if (check_parentheses(p, q, ok)) {
    if (!*ok) return 0;
    return eval(p + 1, q - 1, ok);
  }
  if (!*ok) return 0;

  // 3) 一元负号（选做）：如果起点就是 TK_NEG，求右边并取负
  if (tokens[p].type == TK_NEG) {
    word_t rhs = eval(p + 1, q, ok);
    if (!*ok) return 0;
    // 这里用 (word_t)(-(int64_t)rhs) 可以得到与 ISA 字长一致的二进制补码
    int64_t s = -(int64_t)rhs;
    return (word_t)s;
  }

  // 4) 找到顶层主导运算符
  int op = dominant_op(p, q, ok);
  if (!*ok) return 0;
  if (op < 0) { *ok = false; return 0; }

  // 5) 递归计算左右两边
  word_t lhs = eval(p,     op - 1, ok);
  if (!*ok) return 0;
  word_t rhs = eval(op + 1, q,     ok);
  if (!*ok) return 0;

  // 6) 执行该运算
  switch (tokens[op].type) {
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

// 6666666666666666666666666666666666666666666666666666666666666666666666
word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  mark_unary_minus();
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
