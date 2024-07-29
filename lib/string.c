#include <ylos/string.h>

//==================================
//strcpy函数是src的字符串拷贝到dest中
/*
eg: 1.
    char arr[10] = { 0 };
    const char* p = "abcdef";
    strcpy(arr, p);
    printf("%s\n", arr);
    结果：arr = abcdef

eg: 2.
	char arr[10] = "xxxxxxxxx";
	const char* p = "abcdef";
	strcpy(arr, p);
	printf("%s\n", arr);
	return 0;
    结果：arr = abcdef
*/
char *strcpy(char *dest, const char *src)
{
    char *ptr = dest;
    while (true)
    {
        //先取*在++
        *ptr++ = *src;
        if (*src++ == EOS)
            return dest;
    }
}

//==================================
//strncpy函数是src的字符串count个字符拷贝到dest中
/*
eg: 1.
    char s1[] = "aaaaaaaaaaaaaaaaaaaaaa";
	char s2[] = "helloworld";
	printf("%s\n", strncpy(s1, s2, 5));
	return 0;
    结果：helloaaaaaaaaaaaaaaaaa
*/
char *strncpy(char *dest, const char *src, size_t count)
{
    char *ptr = dest;
    size_t nr = 0;
    for (; nr < count; nr++)
    {
        *ptr++ = *src;
        if (*src++ == EOS)
            return dest;
    }
    dest[count - 1] = EOS;
    return dest;
}

//==================================
//strcat函数是在dest字符串后面追加src字符串，也就是将两个字符串拼接起来。
char *strcat(char *dest, const char *src)
{
    char *ptr = dest;
    while (*ptr != EOS)
    {
        ptr++;
    }
    while (true)
    {
        *ptr++ = *src;
        if (*src++ == EOS)
        {
            return dest;
        }
    }
}

//==================================
//strnlen获取字符串str中实际字符个数，不包括结尾的'\0'；如果实际个数 <= maxlen，则返回实际个数，否则返回第二个参数。
size_t strnlen(const char *str, size_t maxlen)
{
    char *ptr = (char *)str;
    while (*ptr != EOS && maxlen--)
    {
        ptr++;
    }
    return ptr - str;
}

//==================================
//strlen获取字符串长度
size_t strlen(const char *str)
{
    char *ptr = (char *)str;
    while (*ptr != EOS)
    {
        ptr++;
    }
    return ptr - str;
}

//==================================
//strcmp函数字符串比较。顾名思义就是用来比较字符串大小的函数。
//当前者大于后者返回1，前者等于后者返回0，前者小于后者返回-1
int strcmp(const char *lhs, const char *rhs)
{
    while (*lhs == *rhs && *lhs != EOS && *rhs != EOS)
    {
        lhs++;
        rhs++;
    }
    return *lhs < *rhs ? -1 : *lhs > *rhs;
}

//==================================
//strchr() 用于查找字符串中的一个字符，并返回该字符在字符串中第一次出现的位置
//如果在字符串 str 中找到字符 c，则函数返回指向该字符的指针，如果未找到该字符则返回 NULL
char *strchr(const char *str, int ch)
{
    char *ptr = (char *)str;
    while (true)
    {
        if (*ptr == ch)
        {
            return ptr;
        }
        if (*ptr++ == EOS)
        {
            return NULL;
        }
    }
}

//==================================
//strrchr在参数 str 所指向的字符串中搜索最后一次出现字符 c
//该函数返回 str 中最后一次出现字符 c 的位置。如果未找到该值，则函数返回一个空指针
char *strrchr(const char *str, int ch)
{
    char *last = NULL;
    char *ptr = (char *)str;
    while (true)
    {
        if (*ptr == ch)
        {
            last = ptr;
        }
        if (*ptr++ == EOS)
        {
            return last;
        }
    }
}

#define SEPARATOR1 '/'                                       // 目录分隔符 1
#define SEPARATOR2 '\\'                                      // 目录分隔符 2
#define IS_SEPARATOR(c) (c == SEPARATOR1 || c == SEPARATOR2) // 字符是否位目录分隔符

// 获取第一个分隔符
char *strsep(const char *str)
{
    char *ptr = (char *)str;
    while (true)
    {
        if (IS_SEPARATOR(*ptr))
        {
            return ptr;
        }
        if (*ptr++ == EOS)
        {
            return NULL;
        }
    }
}

// 获取最后一个分隔符
char *strrsep(const char *str)
{
    char *last = NULL;
    char *ptr = (char *)str;
    while (true)
    {
        if (IS_SEPARATOR(*ptr))
        {
            last = ptr;
        }
        if (*ptr++ == EOS)
        {
            return last;
        }
    }
}

//==================================
//memcmp 内存比较函数
//逐字节地比较从lhs和rhs开始的count个字节，直至比出
//lhs>rhs返回值>0，lhs<rhs返回值<0，完全相等返回值0
int memcmp(const void *lhs, const void *rhs, size_t count)
{
    char *lptr = (char *)lhs;
    char *rptr = (char *)rhs;
    while ((count > 0) && *lptr == *rptr)
    {
        lptr++;
        rptr++;
        count--;
    }
    if (count == 0)
        return 0;
    return *lptr < *rptr ? -1 : *lptr > *rptr;
}

//==================================
void *memset(void *dest, int ch, size_t count)
{
    char *ptr = dest;
    while (count--)
    {
        *ptr++ = ch;
    }
    return dest;
}

//==================================
void *memcpy(void *dest, const void *src, size_t count)
{
    char *ptr = dest;
    while (count--)
    {
        *ptr++ = *((char *)(src++));
    }
    return dest;
}

//==================================
void *memchr(const void *str, int ch, size_t count)
{
    char *ptr = (char *)str;
    while (count--)
    {
        if (*ptr == ch)
        {
            return (void *)ptr;
        }
        ptr++;
    }
}