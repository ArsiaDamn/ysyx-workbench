#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

//TODO: there are some details to consider:
//目前只支持 %s、%d、%%，缺少：
//宽度修饰符：%10s, %5d
//精度修饰符：%.2f
//标志：%+d, %04d
//长度修饰符：%ld, %lld
//其他类型：%x, %o, %c, %u, %f, %p 等
//同时，buf[1024]没有做溢出处理
int printf(const char *fmt, ...) {
  char buf[1024];
  va_list ap;
  va_start(ap, fmt);
  int ret = vsprintf(buf, fmt, ap);
  va_end(ap);
  for (int i = 0; buf[i]; i++) {putch(buf[i]);}
  return ret;
  //panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  char *p = out;
  for (const char *s = fmt; *s; s++) {
    if (*s != '%') {*p++ = *s; continue;} 
    s++;
    if (*s == '\0') break;
    if (*s == '%') {*p++ = '%'; continue;}
    if (*s == 's') {
      const char *str = va_arg(ap, const char *);
      //NULL pointer is not handled
      while (*str) {*p++ = *str++;}
      continue;
    }
    if (*s == 'd') {
      int x = va_arg(ap, int);
      if (x < 0) {x = -x; *p++ = '-';}
      char buf[32];
      int i = 0;
      while (x > 0) {buf[i++] = '0' + x % 10; x /= 10;} 
      while (i > 0) {*p++ = buf[--i];}
      continue;
    }
    *p++ = '%';
    *p++ = *s;
  }
  *p = '\0';
  return p - out;
  //panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int ret = vsprintf(out, fmt, ap);
  va_end(ap);
  return ret;
  //panic("Not implemented");
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int ret = vsnprintf(out, n, fmt, ap);
  va_end(ap);
  return ret;
  //panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  char *p = out;
  for (const char *s = fmt; *s; s++) {
    if (*s != '%') {*p++ = *s; continue;}
    s++;
    if (*s == '\0') break;
    if (*s == '%') {
      if (p - out < n - 1) {*p++ = '%';}
      continue;
    }
    if (*s == 's') {
      const char *str = va_arg(ap, const char *);
      //NULL pointer is not handled
      while (*str) {
        if (p - out < n - 1) {*p++ = *str;}
        str++;
      }
      continue;
    }
    if (*s == 'd') {
      int x = va_arg(ap, int);
      if (x < 0) {x = -x; *p++ = '-';}
      char buf[32];
      int i = 0;
      while (x > 0) {buf[i++] = '0' + x % 10; x /= 10;} 
      while (i > 0) {
        if (p - out < n - 1) {*p++ = buf[--i];}
      }
      continue;
    }
    if (p - out < n - 1) {*p++ = '%';}
    if (p - out < n - 1) {*p++ = *s;}
  }
  if (p - out < n) {*p = '\0';}
  return p - out;
  //panic("Not implemented");
}

#endif
