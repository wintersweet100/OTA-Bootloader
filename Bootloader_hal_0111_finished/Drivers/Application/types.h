
#ifndef _TYPES_H
#define _TYPES_H


#include <stddef.h>  // 包含这个头文件，它里面有标准的 NULL 定义

typedef unsigned char  rt_uint8_t ;
typedef unsigned short rt_uint16_t ;
typedef unsigned int   rt_uint32_t ;
typedef  int   rt_int32_t;
typedef unsigned char  rt_bool_t;
typedef unsigned int    rt_ubase_t ;
typedef unsigned long  rt_size_t;

#define RT_FALSE 0

#define RT_NULL ((void *)0)
//#define NULL ((void *)0)

/*-----------------以太网W5500相关的define-start----------------------------------------------------------------*/
#define	MAX_SOCK_NUM		8	/**< Maxmium number of socket  */

typedef char int8;

typedef volatile char vint8;

typedef unsigned char uint8;

typedef volatile unsigned char vuint8;

typedef int int16;

typedef unsigned short uint16;

typedef long int32;

typedef unsigned long uint32;

typedef uint8			u_char;		/**< 8-bit value */
typedef uint8 			SOCKET;
typedef uint16			u_short;	/**< 16-bit value */
typedef uint16			u_int;		/**< 16-bit value */
typedef uint32			u_long;		/**< 32-bit value */

typedef union _un_l2cval 
{
	u_long	lVal;
	u_char	cVal[4];
}un_l2cval;

typedef union _un_i2cval 
{
	u_int	iVal;
	u_char	cVal[2];
}un_i2cval;

//typedef __IO uint32_t  vu32;
//typedef __IO uint16_t vu16;
//typedef __IO uint8_t  vu8;



/*-----------------以太网W5500相关的define-end----------------------------------------------------------------*/

#endif

