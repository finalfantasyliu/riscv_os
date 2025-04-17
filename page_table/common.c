#include "common.h"

void putchar(char ch);
/* 這種表現形式為varadic function，其為可接受任意數量與型別的argument，他的主要聲明結構為
    1. create variable argument list (va_list)
    2. initailized argument list(va_start(list,fixed_arg)), fixed_arg為第一個argument
    3. retrieve arguments va_arg(list, type)，這個要注意的點是，每次使用va_arg都會在你的va_list中取出下一個argument，
       但如果你呼叫的次數多於你的argument數量，會是屬於undefined behaviour
    4. Cleanup the list(va_end )
//此功能為stdarg.h所提供，但也可以使用compiler內建的功能
*/

void printf(const char *fmt, ...)
{
    va_list vargs;
    va_start(vargs, fmt);

    while (*fmt)
    {
        if (*fmt == '%')
        {
            fmt++;
            switch (*fmt)
            {
            case '\0' : {
                putchar('%');
                goto end;
            }
             case '%':
            {
                putchar('%');
                break;
            }
            case 's':
            {
                const char *s=va_arg(vargs,const char *);
                while(*s)
                {
                    putchar(*s);
                    s++;
                }
                break;
            }
            case 'd':
            {
                int value=va_arg(vargs,int);
                if(value<0)
                {
                    putchar('-');
                    value=-value;
                }
                int divisor=1;
                while(value/divisor>9)
                divisor*=10;
                while(divisor>0){
                    //在ascii中數字轉charator為’0‘+數字，而這邊因為要轉成10進位，需要一位一位去轉
                    putchar('0'+value/divisor);
                    value %=divisor;
                    divisor/=10;
                }
                break;
            }
            case 'x':
            {
            int value=va_arg(vargs,int);
            for(int i=7;i>=0;i--)
            {
                //16進位為4bit為一組，而一般32bit系統下，會有8組4bit，因此需右移8次
                int nibble=(value>>(i*4))&0xf;
                //這邊直接用一個char的arrary操作，取得對應的值(第一次看到這寫法)
                putchar("0123456789abcdef"[nibble]);
            }
            }
            }
        }
        else{
            putchar(*fmt);
        }
        fmt++;
    }
    end:
    va_end(vargs);
}
void *memset(void *buf, char c, size_t n)
{
    uint8_t *p=(uint8_t*)buf;
    while(n--)
    {
        *p++=c;
    }
    return buf;
}
void *memcpy(void *dst, const void *src, size_t n){
    uint8_t *d=(uint8_t*)dst;
    uint8_t *s=(uint8_t*)src;
    while(n--)
    {
        *d++=*s++;
    }
    return dst;
}
char* strcpy(char *dst, const char *src)
{
    char *d=dst;
    while(src)
    {
        *d++=*src++;
    }
    *d='\0';
    return dst;
}
int strcmp(const char *s1,const char *s2)
{
    while(*s1&&*s2)
    {
        if(*s1!=*s2)
        {
            break;
        }
        s1++;
        s2++;
    }
    return *(unsigned int *)s1-*(unsigned int *)s2;
}