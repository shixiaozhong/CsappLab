//自己写的一些phase中的伪代码

// 2.一个大循环判断六个数字都是小于等于6，并且都互不相等。
int i = 0;
while (i++)
{
    a[i]--;
    if (a[i] > 5)
        explode_bomb();
    if (i == 6)
        jmp 401153 for (int j = i + 1; j <= 5; j++) if (a[i] == a[j])
            explode_bomb();
}

// 4. 创建一个指针数组，循环进行选择，按照ai的大小来选择链表节点顺序放置到指针数组当中。

// 数组a表示输入的六个数
// l数组表示节点指针数组
for (int i = 0; i < 6; i++)
{
    ListNode *p = 0x6032d0;
    if (a[i] > 1)
    {
        for (int j = 0; j < a[i]; j++)
            p = p->next;
    }
    l[i] = p;
}

int fun7(int x, Treenode *root)
{
    if (root == NULL)
        return -1;
    if (x >= root->val)
    {
        int res = 0;
        if (x == root->val)
            return res;
        return 2 * fun7(x, root->right) + 1;
    }
    else
    {
        return 2 * fun7(x, root->left);
    }
}
// %edi  %esi  %edx
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
        return 2 * func4(a, y + 1, c) + 1; // 修改的是第二个参数
    }
    return 2 * func4(a, b, y - 1); // 修改的是第三个参数
}