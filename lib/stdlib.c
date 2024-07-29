#include <ylos/stdlib.h>

void delay(u32 count)
{
    while (count--)
        ;
}

void hang()
{
    while (true)
        ;
}

/*
BCD 码（Binary-Code Decimal）又称8421码

用4位二进制来表示1位十进制

十进制               BCD码                 二进制
0                    0000                  0000
1                    0001                  0001
2                    0010                  0010
3                    0011                  0011
4                    0100                  0100
5                    0101                  0101
6                    0110                  0110
7                    0111                  0111
8                    1000                  1000
9                    1001                  1001
10                   0001_0000             1010
11                   0001_0001             1011
12                   0001_0010             1100
13                   0001_0011             1101
14                   0001_0100             1110
15                   0001_0101             1111
16                   0001_0110             0001_0000
17                   0001_0111             0001_0001
18                   0001_1000             0001_0010
19                   0001_1001             0001_0011
20                   0010_0000             0001_0100
...                  ...                   ...
*/

/*
eg. value = 0001_0111

 = (0001_0111 & 0xf) + (0001_0111 >> 4) * 10

 = (0111) + (0001) * 10

 = (0111) + (1010)

 = 0b10001

 <==> 17 
*/
u8 bcd_to_bin(u8 value)
{
    return (value & 0xf) + (value >> 4) * 10;
}

u8 bin_to_bcd(u8 value)
{
    return (value / 10) * 0x10 + (value % 10);
}

u32 div_round_up(u32 num, u32 size)
{
    return (num + size -1) / size;
}

int atoi(const char *str)
{
    if (str == NULL)
        return 0;
    int sign = 1;
    int result = 0;
    if (*str == '-')
    {
        sign = -1;
        str++;
    }
    for (; *str; str++)
    {
        result = result * 10 + (*str - '0');
    }
    return result * sign;
}