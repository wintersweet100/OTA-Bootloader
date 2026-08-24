#include "shell.h"

/**
 * This function will set the content of memory to specified value
 *
 * @param s the address of source memory
 * @param c the value shall be set in content
 * @param count the copied length
 *
 * @return the address of source memory
 */
void *memset(void *s, int c, rt_ubase_t count)
{
    char *xs = (char *)s;

    while (count--)
        *xs++ = c;

    return s;
}

/**
 * This function will copy memory content from source address to destination
 * address.
 *
 * @param dst the address of destination memory
 * @param src  the address of source memory
 * @param count the copied length
 *
 * @return the address of destination memory
 */
void *memcpy(void *dst, const void *src, rt_ubase_t count)
{
    char *tmp = (char *)dst, *s = (char *)src;
    rt_ubase_t len;

    if (tmp <= s || tmp > (s + count))
    {
        while (count--)
            *tmp ++ = *s ++;
    }
    else
    {
        for (len = count; len > 0; len --)
            tmp[len - 1] = s[len - 1];
    }

    return dst;
	
}

/**
 * This function will compare two areas of memory
 *
 * @param cs one area of memory
 * @param ct another area of memory
 * @param count the size of the area
 *
 * @return the result
 */
rt_int32_t memcmp(const void *cs, const void *ct, rt_ubase_t count)
{
    const unsigned char *su1, *su2;
    int res = 0;

    for (su1 = (const unsigned char *)cs, su2 = (const unsigned char *)ct; 0 < count; ++su1, ++su2, count--)
        if ((res = *su1 - *su2) != 0)
            break;

    return res;
}

/**
 * This function will move memory content from source address to destination
 * address.
 *内存移动函数，功能类似标准库的 memmove()
 *将 n 字节的数据从源地址 src 复制到目标地址 dest，并处理内存重叠的情况。
 * @param dest the address of destination memory
 * @param src  the address of source memory
 * @param n the copied length
 *
 * @return the address of destination memory
 */
void *memmove(void *dest, const void *src, rt_ubase_t n)
{
    char *tmp = (char *)dest, *s = (char *)src;

    if (s < tmp && tmp < s + n)
    {
        tmp += n;
        s += n;

        while (n--)
            *(--tmp) = *(--s);
    }
    else
    {
        while (n--)
            *tmp++ = *s++;
    }

    return dest;
}

/**
 * This function will return the length of a string, which terminate will
 * null character.
 *
 * @param s the string
 *
 * @return the length of string
 */
rt_size_t strlen(const char *s)
{
    const char *sc;

    for (sc = s; *sc != '\0'; ++sc) /* nothing */
        ;

    return sc - s;
}

int strcmp (const char *s1, const char *s2)
{
    while (*s1 && *s1 == *s2)
        s1++, s2++;
    return (*s1 - *s2);
}

unsigned int str2hex(const char* s)
{
	unsigned int sum=0;
    unsigned char c;
    unsigned int val;
	while ( *s == ' '  ||  *s == '\t') s++;

    if (*s == '0')s++;
    if (*s == 'x')s++;
    if (*s == 'X')s++;    

    c = *s;
	while (c)
	{
        if (c >= '0' && c <= '9')
            val = c - '0';
        else if (c >= 'a' && c <= 'z')
            val = c - 'a' + 10;
        else if (c >= 'A' && c <= 'Z')
            val = c - 'A' + 10;
		sum = sum * 16 + val;
		++s;
        c = *s;
	}
	return sum;
}

/**
 * @brief  在字符串 s1 中查找字符串 s2 的第一次出现
 * @param  s1: 被查找的目标字符串
 * @param  s2: 要搜索的子字符串
 * @retval 指向 s1 中匹配 s2 处的指针；若未找到则返回 NULL
 */
char *strstr(const char *s1, const char *s2)
{
    const char *p = s1;
    const char *q = s2;
    const char *res = NULL;

    // 如果子串为空，按标准定义返回原串
    if (!*s2) return (char *)s1;

    while (*p)
    {
        // 发现第一个字符匹配
        if (*p == *q)
        {
            res = p; // 记录当前起始位置
            // 开始比对后续字符
            while (*p && *q && *p == *q)
            {
                p++;
                q++;
            }
            // 如果 q 走到了末尾，说明完全匹配
            if (!*q)
            {
                return (char *)res;
            }
            // 如果没匹配完，回退指针，继续从下一个位置找
            p = res + 1;
            q = s2;
        }
        else
        {
            p++;
        }
    }
    return NULL;
}
