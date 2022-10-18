
// func4函数中的递归求值

#include <stdio.h>

int func4(int a, int b, int c) // a, 0, 14
{
    int x = c - b;
    int y = x >> 31; // 取符号位
    x = y + x;
    x = x >> 1;
    y = x + b * 1;

    if (y <= a)
    {
        int res = 0;
        if (y >= a)
            return res;
        return 2 * func4(a, y + 1, c) + 1;
    }
    return 2 * func4(a, b, y - 1);
}

int main()
{
    for (int i = 0; i <= 14; i++) //在phase_4函数里要保证输入的第一个参数是小于等于14
        if (func4(i, 0, 14) == 0)
            printf("%d\n", i);
    return 0;
}
