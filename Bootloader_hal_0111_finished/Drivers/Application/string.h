
#ifndef _STRING_H
#define _STRING_H

#include "types.h"

/**
 * This function will set the content of memory to specified value
 *
 * @param s the address of source memory
 * @param c the value shall be set in content
 * @param count the copied length
 *
 * @return the address of source memory
 */
void *memset(void *s, int c, rt_ubase_t count);

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
void *memcpy(void *dst, const void *src, rt_ubase_t count);


/**
 * This function will compare two areas of memory
 *
 * @param cs one area of memory
 * @param ct another area of memory
 * @param count the size of the area
 *
 * @return the result
 */
rt_int32_t memcmp(const void *cs, const void *ct, rt_ubase_t count);


/**
 * This function will move memory content from source address to destination
 * address.
 *
 * @param dest the address of destination memory
 * @param src  the address of source memory
 * @param n the copied length
 *
 * @return the address of destination memory
 */
void *memmove(void *dest, const void *src, rt_ubase_t n);

/**
 * This function will return the length of a string, which terminate will
 * null character.
 *
 * @param s the string
 *
 * @return the length of string
 */
rt_size_t strlen(const char *s);

int strcmp (const char *s1, const char *s2);

unsigned int str2hex(const char* s);

/**
 * @brief  在字符串 s1 中查找字符串 s2 的第一次出现
 * @param  s1: 被查找的目标字符串
 * @param  s2: 要搜索的子字符串
 * @retval 指向 s1 中匹配 s2 处的指针；若未找到则返回 NULL
 */
char *strstr(const char *s1, const char *s2);
#endif

