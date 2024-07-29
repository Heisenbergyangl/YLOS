/* (C) Copyright 2022 Steven;
 * @author: Steven kangweibaby@163.com
 * @date: 2022-06-28
 */

#include <ylos/stdarg.h>
#include <ylos/stdio.h>
#include <ylos/syscall.h>

static char buf[1024];

int printf(const char *fmt, ...)
{
    va_list args;
    int i;

    va_start(args, fmt);

    i = vsprintf(buf, fmt, args);

    va_end(args);

    write(STDOUT_FILENO, buf, i);

    return i;
}