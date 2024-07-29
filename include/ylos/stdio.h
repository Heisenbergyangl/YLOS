#ifndef YLOS_STDIO_H 
#define YLOS_STDIO_H 

#include <ylos/stdarg.h>

int vsprintf(char *buf, const char *fmt, va_list args);
int sprintf(char *buf, const char *fmt, ...);
int printf(const char *fmt, ...);

#endif 
