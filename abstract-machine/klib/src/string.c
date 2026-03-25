#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  const char *p = s;
  while (*p) p++;
  return (size_t)(p - s);
  //panic("Not implemented");
}

char *strcpy(char *dst, const char *src) {
  char *p = dst;
  while (*src) {*p++ = *src++;}
  *p = '\0';
  return dst;
  //panic("Not implemented");
}

char *strncpy(char *dst, const char *src, size_t n) {
  size_t i;
  for (i = 0; i < n && src[i]; i++) {dst[i] = src[i];}
  for (i = 0; i < n; i++) {dst[i] = '\0';}
  return dst;
  //panic("Not implemented");
}

char *strcat(char *dst, const char *src) {
  char *p = dst;
  while (*dst) dst++;
  while ((*dst++ = *src++) != '\0') { }
  return p;
  //panic("Not implemented");
}

int strcmp(const char *s1, const char *s2) {
  while (*s1 && *s2 && (*s1 == *s2)) {s1++; s2++;}
  return (int)(unsigned char)*s1 - (int)(unsigned char)*s2;
  //panic("Not implemented");
}

int strncmp(const char *s1, const char *s2, size_t n) {
  for (size_t i = 0; i < n; i++) {
    if (s1[i] != s2[i]) {return (int)(unsigned char)s1[i] - (int)(unsigned char)s2[i];}
    if (s1[i] == '\0')  {return 0;}
  }
  return 0;  // n = 0
  //panic("Not implemented");
}

void *memset(void *s, int c, size_t n) {
  unsigned char *p = s;
  for (size_t i = 0; i < n; i++) {p[i] = (unsigned char)c;}
  return s;
  //panic("Not implemented");
}

void *memmove(void *dst, const void *src, size_t n) {
  //panic("Not implemented");
  unsigned char *p = (unsigned char *)dst;
  const unsigned char *s = (const unsigned char *)src;
  if (p < s)      {for (size_t i = 0; i < n; i++) {p[i] = s[i];} } 
  else if (p > s) {for (size_t i = n; i > 0; i--) {p[i - 1] = s[i - 1];}}
  return dst;
}

void *memcpy(void *out, const void *in, size_t n) {
  unsigned char *p = (unsigned char *)out;
  const unsigned char *s = (const unsigned char *)in;
  for (size_t i = 0; i < n; i++) {p[i] = s[i];}
  return out;
  //panic("Not implemented");
}

int memcmp(const void *s1, const void *s2, size_t n) {
  const unsigned char *p1 = (const unsigned char *)s1;
  const unsigned char *p2 = (const unsigned char *)s2;
  for (size_t i = 0; i < n; i++) {
    if (p1[i] != p2[i]) {return (int)p1[i] - (int)p2[i];}
  }
  return 0;
  //panic("Not implemented");
}

#endif
