#include <ylos/stdarg.h>
#include <ylos/console.h>
#include <ylos/stdio.h>

extern int32 console_write();

static char buf[1024];

int printk(const char *fmt, ...)
{
    va_list args;
    int i;

    va_start(args, fmt);

    i = vsprintf(buf, fmt, args);

    va_end(args);

    console_write(NULL, buf, i);

    return i;
}