
#include "uart.h"

/*
        APP执行自我复制程序，将自身从flash复制到SRAM中运行
        在汇编中编写获取from、to、length参数，并调用该函数
        from: 源地址	0x08009000
        to: 目标地址	0x20000000
*/
void copy_myself(int *from, int *to, int length) {
    int i;
    for (i = 0; i < length / 4 + 1; i++) {
        to[i] = from[i];
    }
}

void delay(int d) {
    while (d--);
}

int mymain() {
    char c = 'A';

    int (*fp)(char c);

    fp = putchar;

    while (1) {

        fp(c++); // 绝对跳转
        delay(1000000);
        if (c == 'Z')
            c = 'A';
    }

    return 0;
}
