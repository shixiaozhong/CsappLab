[TOC]

#### 前言

这个是csapp所有Lab中的第二个，叫做Bomb Lab，是一个二进制的拆炸弹实验，这个实验还是比较有意思的，通过阅读汇编代码来输入正确的答案来拆除炸弹，做这个实验的话，只需要将csapp的第三章看完就可以做了，基本具备阅读汇编代码的能力。如果你一点都看不懂实验中的汇编代码，可能你得多看看书了。这个完整的Lab做下来，看汇编基本就和c语言一样丝滑了。

这是csapp官方网页的Lab链接[(CS:APP Lab)](http://csapp.cs.cmu.edu/3e/labs.html)，下载Self-Study Handout就可以了，再转移到自己的Linux环境下就可以，这个实验环境只需要有gcc，gdb工具，可以用`objdump -d`命令得到反汇编代码就可以了。

还需要一点gdb的知识，[Enscript Output (cmu.edu)](http://csapp.cs.cmu.edu/2e/docs/gdbnotes-x86-64.pdf)这是课程官方提供的gdb使用文档，我这里提供一个x命令的使用方式[(gdb x 命令详解)](https://blog.csdn.net/renlonggg/article/details/73550306)，其实只需要x命令就可以了。

<img src="http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221016095055125.png" alt="image-20221016095055125" style="zoom: 67%;" />

首先解压压缩包到自己的目录下就可以，建议新建一个目录，单独放着。解压完后会得到三个文件。bomb.c是源文件，bomb是生成的可执行文件，最后测试输入是否正确就是通过执行bomb。README文件不用管。

![image-20221016095657859](http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221016095657859.png)

1. 首先通过`objdump -d bomb > bomb.s`，执行反汇编命令，顺便将得到的反汇编代码重定向到bomb.s文件中。所以bomb.s文件中就是存储的bomb的反汇编代码。

    ![image-20221016100124326](http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221016100124326.png)

再创建一个`res.txt`文件储存输入的答案，因为有很多phase，每次测试都重复输入是很麻烦的，查看bomb.c文件以后发现是支持命令行参数的，所以使用res.txt是很方便的。

2. 通过你环境下的阅读工具查看反汇编代码，例如vim,emacs啥的，直接`vim bomb.s `就可以，没有vim可以去下载一个。

    <img src="http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221016113021690.png" alt="image-20221016113021690" style="zoom: 67%;" />

3. 查看bomb.c文件，就知道需要干嘛了，一共有六个phase，全部解除就成功了。

    <img src="http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221016113004790.png" alt="image-20221016113004790" style="zoom: 67%;" />

---

#### phase_1

<img src="http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221016113055934.png" alt="image-20221016113055934" style="zoom:67%;" />

现在开始拆除phase_1。首先打开bomb.s文件，因为没有各个phase的源文件，既然要想拆除炸弹，只能自己看汇编来看函数的内部实现。找到phase_1，看就完了。

```cpp
0000000000400ee0 <phase_1>:
400ee0:	48 83 ec 08          	sub    $0x8,%rsp			// 开辟栈空间，就是phase_1的第一个参数readline
400ee4:	be 00 24 40 00       	mov    $0x402400,%esi				// 0x402400作为第一个参数传入函数
400ee9:	e8 4a 04 00 00       	callq  401338 <strings_not_equal>	// 判断字符串是否相等	
400eee:	85 c0                	test   %eax,%eax					
400ef0:	74 05                	je     400ef7 <phase_1+0x17>		// 相等直接跳转到400ef7
400ef2:	e8 43 05 00 00       	callq  40143a <explode_bomb>		// 否则炸弹爆炸
    
400ef7:	48 83 c4 08          	add    $0x8,%rsp
400efb:	c3                   	retq   
```

这题还是很简单的，可以通过gdb查看0x402400存储的是什么字符串，然后输入相同的就可以。在res.txt中加上，再测试对不对。

![image-20221016102210975](http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221016102210975.png)

![image-20221016102447281](http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221016102447281.png)

使用`./bomb res.txt`命令执行，有以下输出就是正确的，可以在bomb.c中可以看到每一个phase通过后的祝福语。

![image-20221016102608005](http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221016102608005.png)

否则随便一个thank you，就会引发炸弹爆炸

![image-20221016102743080](http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221016102743080.png)

---

#### phase_2

![image-20221016113120931](http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221016113120931.png)

```cpp
0000000000400efc <phase_2>:
400efc:	55                   	push   %rbp
400efd:	53                   	push   %rbx						// 压入两个被调用者保存寄存器
400efe:	48 83 ec 28          	sub    $0x28,%rsp			// 开辟站栈帧空间，后面看出创建了一个数组
400f02:	48 89 e6             	mov    %rsp,%rsi				// 栈顶指针作为第一个参数
400f05:	e8 52 05 00 00       	callq  40145c <read_six_numbers>// 从readline中读入六个数字
400f0a:	83 3c 24 01          	cmpl   $0x1,(%rsp)			// 如果读入第一个元素a[0]等于1，直接跳转到400f30
400f0e:	74 20                	je     400f30 <phase_2+0x34>// 否则炸弹爆炸
400f10:	e8 25 05 00 00       	callq  40143a <explode_bomb>
400f15:	eb 19                	jmp    400f30 <phase_2+0x34>// 无条件跳转到400f30
    
// 循环：判断a[i] == 2 * a[i - 1]，否则炸弹爆炸 
400f17:	8b 43 fc             	mov    -0x4(%rbx),%eax		// %eax保存a[0]
400f1a:	01 c0                	add    %eax,%eax			// %eax*2
400f1c:	39 03                	cmp    %eax,(%rbx)			// 判断%eax是不是等于a[1]
400f1e:	74 05                	je     400f25 <phase_2+0x29>// 相等跳转到400f25
400f20:	e8 15 05 00 00       	callq  40143a <explode_bomb>// 不相等炸弹直接爆炸
    
400f25:	48 83 c3 04          	add    $0x4,%rbx			// 指向a[1]的指针先后走，指向a[2]
400f29:	48 39 eb             	cmp    %rbp,%rbx			// 判断有没有走到a[6],没有继续循环
400f2c:	75 e9                	jne    400f17 <phase_2+0x1b>
    
400f2e:	eb 0c                	jmp    400f3c <phase_2+0x40>

400f30:	48 8d 5c 24 04       	lea    0x4(%rsp),%rbx			// 取a[1]的地址,放入%rbx
400f35:	48 8d 6c 24 18       	lea    0x18(%rsp),%rbp			// 取a[6]的地址，放入%rbp
400f3a:	eb db                	jmp    400f17 <phase_2+0x1b>	// 无条件跳转到400f17
    
400f3c:	48 83 c4 28          	add    $0x28,%rsp
400f40:	5b                   	pop    %rbx
400f41:	5d                   	pop    %rbp
400f42:	c3                   	retq   
```

第一个元素保证必须是1，且满足`a[i] = a[i] * 2`，所以输入的六个数为`1 2 4 8 16 32`。

![image-20221016104328973](http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221016104328973.png)

---

#### phase_3

![image-20221016113136419](http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221016113136419.png)

```cpp
0000000000400f43 <phase_3>:
400f43:	48 83 ec 18          	sub    $0x18,%rsp				// 开辟栈帧空间
400f47:	48 8d 4c 24 0c       	lea    0xc(%rsp),%rcx			// 传入地址作为第一个参数
400f4c:	48 8d 54 24 08       	lea    0x8(%rsp),%rdx			// 传入地址作为第二个参数
400f51:	be cf 25 40 00       	mov    $0x4025cf,%esi// 作为第一个参数传入sscanf函数，可以通过gdb查看"%d %d"
400f56:	b8 00 00 00 00       	mov    $0x0,%eax	 // %eax存储返回值，先置为0		
400f5b:	e8 90 fc ff ff       	callq  400bf0 <__isoc99_sscanf@plt>//从readline中读入两个整型
400f60:	83 f8 01             	cmp    $0x1,%eax				   // 判断返回值，如果大于1 
400f63:	7f 05                	jg     400f6a <phase_3+0x27>		// 跳转到400f6a
400f65:	e8 d0 04 00 00       	callq  40143a <explode_bomb>		// 否则炸弹爆炸
    
400f6a:	83 7c 24 08 07       	cmpl   $0x7,0x8(%rsp)				// 0x8(%rsp)存储的即使输入的第一个值
400f6f:	77 3c                	ja     400fad <phase_3+0x6a>		// 大于7跳转到400fad
400f71:	8b 44 24 08          	mov    0x8(%rsp),%eax				// %eax保存输入的第一个值
400f75:	ff 24 c5 70 24 40 00 	jmpq   *0x402470(,%rax,8)	//无条件跳转到...，由输入的第一个值决定 
    														//gdb查看0x402470->0x00400f7c,即下面一条指令地址

// 可以看出这是一个switch语句，将值保存到%eax， 且执行完全部跳转到400fbe									
400f7c:	b8 cf 00 00 00       	mov    $0xcf,%eax		
400f81:	eb 3b                	jmp    400fbe <phase_3+0x7b>
400f83:	b8 c3 02 00 00       	mov    $0x2c3,%eax			
400f88:	eb 34                	jmp    400fbe <phase_3+0x7b>
400f8a:	b8 00 01 00 00       	mov    $0x100,%eax
400f8f:	eb 2d                	jmp    400fbe <phase_3+0x7b>	
400f91:	b8 85 01 00 00       	mov    $0x185,%eax				
400f96:	eb 26                	jmp    400fbe <phase_3+0x7b>
400f98:	b8 ce 00 00 00       	mov    $0xce,%eax
400f9d:	eb 1f                	jmp    400fbe <phase_3+0x7b>
400f9f:	b8 aa 02 00 00       	mov    $0x2aa,%eax
400fa4:	eb 18                	jmp    400fbe <phase_3+0x7b>
400fa6:	b8 47 01 00 00       	mov    $0x147,%eax
400fab:	eb 11                	jmp    400fbe <phase_3+0x7b>
    	
400fad:	e8 88 04 00 00       	callq  40143a <explode_bomb>	// 炸弹爆炸，所以输入的第一个值一定小于等于7
400fb2:	b8 00 00 00 00       	mov    $0x0,%eax
400fb7:	eb 05                	jmp    400fbe <phase_3+0x7b>
400fb9:	b8 37 01 00 00       	mov    $0x137,%eax
    
400fbe:	3b 44 24 0c          	cmp    0xc(%rsp),%eax			//比较输入的第二个值是不是等于%eax
400fc2:	74 05                	je     400fc9 <phase_3+0x86>	// 相等直接跳转，否则炸弹爆炸
400fc4:	e8 71 04 00 00       	callq  40143a <explode_bomb>
400fc9:	48 83 c4 18          	add    $0x18,%rsp
400fcd:	c3                   	retq   
```

![image-20221016154305043](http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221016154305043.png)

上面汇编代码的意思就是输入两个数，第一个数作为索引，执行switch，得到的一个数必须要与第二个输入的数相等。

所以这题的答案很多，有7个，随便输入一个就可以，假设输入的第一个数为0，则第二个必须为0xcf，即为207，所以这题的一个答案为`0 207`。

![image-20221016112920204](http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221016112920204.png)

---

#### phase_4

![image-20221016113151065](http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221016113151065.png)

```cpp
000000000040100c <phase_4>:
40100c:	48 83 ec 18          	sub    $0x18,%rsp					// 开辟栈帧空间
401010:	48 8d 4c 24 0c       	lea    0xc(%rsp),%rcx				// 做第二个参数
401015:	48 8d 54 24 08       	lea    0x8(%rsp),%rdx				// 做第一个参数
40101a:	be cf 25 40 00       	mov    $0x4025cf,%esi				// gdb查看是"%d %d"，所以输入的是两个整型
40101f:	b8 00 00 00 00       	mov    $0x0,%eax
401024:	e8 c7 fb ff ff       	callq  400bf0 <__isoc99_sscanf@plt>	// 从readline中读入数据
401029:	83 f8 02             	cmp    $0x2,%eax					// 读入数据个数小于等于2，直接炸弹爆炸
40102c:	75 07                	jne    401035 <phase_4+0x29>
40102e:	83 7c 24 08 0e       	cmpl   $0xe,0x8(%rsp)				// 判断第一个值是不是小于等于14
401033:	76 05                	jbe    40103a <phase_4+0x2e>		// 如果是直接跳转到40103a,否则炸弹爆炸
401035:	e8 00 04 00 00       	callq  40143a <explode_bomb>
40103a:	ba 0e 00 00 00       	mov    $0xe,%edx					// 第三个参数14
40103f:	be 00 00 00 00       	mov    $0x0,%esi					// 第二个参数0
401044:	8b 7c 24 08          	mov    0x8(%rsp),%edi				// 第一个值作为第一个参数
401048:	e8 81 ff ff ff       	callq  400fce <func4>				// 跳转到fun4函数中
40104d:	85 c0                	test   %eax,%eax					// 判断func4的返回值是不是不等于0
40104f:	75 07                	jne    401058 <phase_4+0x4c>		// 不等于0直接炸弹爆炸
401051:	83 7c 24 0c 00       	cmpl   $0x0,0xc(%rsp)				// 判断第二个值是不是等于0，不是炸弹爆炸
401056:	74 05                	je     40105d <phase_4+0x51>		// 所以输入的第二个值为0

401058:	e8 dd 03 00 00       	callq  40143a <explode_bomb>
40105d:	48 83 c4 18          	add    $0x18,%rsp
401061:	c3                   	retq
```

上面这段汇编代码的意思是输入两个数字，第二个必须为0，第一个作为参数去调用func4，返回值必须为0，只有满足条件的输入才是正确的。

```cpp
0000000000400fce <func4>:
400fce:	48 83 ec 08          	sub    $0x8,%rsp			// 开辟栈帧空间
400fd2:	89 d0                	mov    %edx,%eax			// 
400fd4:	29 f0                	sub    %esi,%eax			// %eax保存14-0，还是14
400fd6:	89 c1                	mov    %eax,%ecx			// %ecx保存14
400fd8:	c1 e9 1f             	shr    $0x1f,%ecx			// 14逻辑右移31位，目的在于取符号位
400fdb:	01 c8                	add    %ecx,%eax			// 加上符号位，%eax存储的还是14
400fdd:	d1 f8                	sar    %eax					// %eax右移一位，得到7
400fdf:	8d 0c 30             	lea    (%rax,%rsi,1),%ecx	// %ecx保存%eax，值为7
    
400fe2:	39 f9                	cmp    %edi,%ecx			//比较输入的第一个值和7的大小
400fe4:	7e 0c                	jle    400ff2 <func4+0x24>	// 小于等于直接跳转到400f2
    
400fe6:	8d 51 ff             	lea    -0x1(%rcx),%edx		// %edx改为%rcx -1，注意修改的是第三个参数
400fe9:	e8 e0 ff ff ff       	callq  400fce <func4>		// 递归调用
400fee:	01 c0                	add    %eax,%eax			// %eax*2
400ff0:	eb 15                	jmp    401007 <func4+0x39>	// 直接return %eax
    
400ff2:	b8 00 00 00 00       	mov    $0x0,%eax			// %eax赋值为0
400ff7:	39 f9                	cmp    %edi,%ecx			// 判断%ecx和第一个值的大小
400ff9:	7d 0c                	jge    401007 <func4+0x39>	// 如果大于等于，直接return 0    
400ffb:	8d 71 01             	lea    0x1(%rcx),%esi		// 否则%esi变为%rcx+1， 注意修改的是第二个参数
400ffe:	e8 cb ff ff ff       	callq  400fce <func4>		// 递归
    
401003:	8d 44 00 01          	lea    0x1(%rax,%rax,1),%eax // return %rax * 2 + 1
401007:	48 83 c4 08          	add    $0x8,%rsp
40100b:	c3                   	retq
```

看了fun4的代码可以发现这是一个递归程序，但是可以卡掉不用去看递归程序，就是当输入的第一个值为7时，恰好可以卡掉递归，所以本题的答案就是`7 0`。

但是本着学习的目的，还是得研究一下这个递归，下面给出一份伪代码提供参考。

<img src="http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221016145140373.png" alt="image-20221016145140373" style="zoom: 50%;" />

既然要返回值为0，可以写一个简单的测试代码得到结果。

<img src="http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221016145359097.png" alt="image-20221016145359097" style="zoom: 67%;" />

![image-20221016145531210](http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221016145531210.png)

所以本题一共有四个答案，`0 0`, `1 0`, `3 0`, `7 0`。随便选一个就可以。

![image-20221016145736964](http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221016145736964.png)

---

#### phase_5

![image-20221016113202820](http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221016113202820.png)

```cpp
0000000000401062 <phase_5>:
401062:	53                   	push   %rbx
401063:	48 83 ec 20          	sub    $0x20,%rsp		// 开辟栈帧空间，后面可以看出是开了一个局部的字符数组s
401067:	48 89 fb             	mov    %rdi,%rbx				// 存储第一个参数readline中的字符串首地址
40106a:	64 48 8b 04 25 28 00 	mov    %fs:0x28,%rax			// 设置金丝雀值，防止缓冲区溢出
401071:	00 00 
401073:	48 89 44 24 18       	mov    %rax,0x18(%rsp)			// 将金丝雀值放到指定的位置
401078:	31 c0                	xor    %eax,%eax				// 将%eax置为0
    
40107a:	e8 9c 02 00 00       	callq  40131b <string_length>	// 求出readline中的字符串长度
40107f:	83 f8 06             	cmp    $0x6,%eax				// 判断长度是否等于6
401082:	74 4e                	je     4010d2 <phase_5+0x70>	// 相等直接跳转到4010d2，否则炸弹爆炸
401084:	e8 b1 03 00 00       	callq  40143a <explode_bomb>
401089:	eb 47                	jmp    4010d2 <phase_5+0x70>
// 循环：  
40108b:	0f b6 0c 03          	movzbl (%rbx,%rax,1),%ecx	// 将第一个字符的ascall码进行符号扩展保存到%ecx中，
40108f:	88 0c 24             	mov    %cl,(%rsp)				// 将第一个字符的低四位拷贝到s[0]
401092:	48 8b 14 24          	mov    (%rsp),%rdx				// %rdx保存s[0]
401096:	83 e2 0f             	and    $0xf,%edx				// 与上0xf, %edx得到的就是s[0]的低八位
401099:	0f b6 92 b0 24 40 00 	movzbl 0x4024b0(%rdx),%edx//gdb查看0x4024b0，是一个字符数组，%rdx作为索引取访问字符数组，%edx得到的就是访问的到的字符，实际存储的是ascall码值
4010a0:	88 54 04 10          	mov    %dl,0x10(%rsp,%rax,1)  	// s[16 + i] = 字符的低四位
4010a4:	48 83 c0 01          	add    $0x1,%rax				// 计数器i++
4010a8:	48 83 f8 06          	cmp    $0x6,%rax				// 判断是不是小于等于6，是就继续循环
4010ac:	75 dd                	jne    40108b <phase_5+0x29>
    
4010ae:	c6 44 24 16 00       	movb   $0x0,0x16(%rsp)			// s[22]置为0
4010b3:	be 5e 24 40 00       	mov    $0x40245e,%esi			// gdb查看0x40245e,是字符串"flyers"
4010b8:	48 8d 7c 24 10       	lea    0x10(%rsp),%rdi			// s[16]作为首地址
4010bd:	e8 76 02 00 00       	callq  401338 <strings_not_equal>
4010c2:	85 c0                	test   %eax,%eax				// 判断字符串相不相等
4010c4:	74 13                	je     4010d9 <phase_5+0x77>	// 不相等炸弹爆炸， 相等跳转到4010d9
4010c6:	e8 6f 03 00 00       	callq  40143a <explode_bomb>
4010cb:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
4010d0:	eb 07                	jmp    4010d9 <phase_5+0x77>
    
4010d2:	b8 00 00 00 00       	mov    $0x0,%eax				// %eax置0，作计数器
4010d7:	eb b2                	jmp    40108b <phase_5+0x29>	// 直接跳转到40108b
    
4010d9:	48 8b 44 24 18       	mov    0x18(%rsp),%rax			// 判断金丝雀值有没有发生改变
4010de:	64 48 33 04 25 28 00 	xor    %fs:0x28,%rax
4010e5:	00 00 
4010e7:	74 05                	je     4010ee <phase_5+0x8c>	// 没改变直接跳转释放栈空间
4010e9:	e8 42 fa ff ff       	callq  400b30 <__stack_chk_fail@plt>//否则调用__stack_chk_fail函数
4010ee:	48 83 c4 20          	add    $0x20,%rsp
4010f2:	5b                   	pop    %rbx
4010f3:	c3                   	retq   
```

![image-20221016153013063](http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221016153013063.png)

这个phase比上面的稍微难一点，主要是要看懂是字符数组的索引。这一段汇编代码的意思就是输入一个长度为6的字符串，依照这写字符串的ascall码的低四位作为索引去指定的字符数组中查找到指定的字符，组成新字符串，然后判断是否和指定的相同，相同就是成功，否则失败。最终需要得到的字符是`"flyers"`。

![image-20221016153352478](http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221016153352478.png)

所以输入的答案可以为`IONEFG`。

![image-20221016154037575](http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221016154037575.png)

---

#### phase_6

![image-20221016113214896](http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221016113214896.png)

个人感觉phase_6应该是所有phase里难度最大的，因为汇编代码有亿点点长，完整看完可能都不知道在干嘛，所以我先将整体代码的逻辑罗列出来，可以更好的去理解汇编代码。

代码可以简单分为几个部分：

1. 读入六个数字存放到数组当中。

2. 一个大循环判断六个数字都是小于等于6，并且都互不相等。
3. 一个循环实现ai = 7 - ai,将输入的所有数字都转换为7 - ai。
4. 创建一个指针数组，循环进行选择，按照ai的大小来选择链表节点顺序放置到指针数组当中。
5. 将数组中的所有节点重新连接起来，形成一个新的链表。
6. 判断新链表是不是降序排列的。如果不是炸弹爆炸。

**1.读入六个数字存放到数组当中。**

```cpp
4010f4:	41 56                	push   %r14
4010f6:	41 55                	push   %r13
4010f8:	41 54                	push   %r12							//压入一堆被调用者保存寄存器
4010fa:	55                   	push   %rbp
4010fb:	53                   	push   %rbx
4010fc:	48 83 ec 50          	sub    $0x50,%rsp 					// 开辟栈空间
401100:	49 89 e5             	mov    %rsp,%r13 					// %r13保存栈顶指针
401103:	48 89 e6             	mov    %rsp,%rsi 					// 栈顶指针作为第一个参数传入输入函数
401106:	e8 51 03 00 00       	callq  40145c <read_six_numbers> 	// 输入函数读入六个数字
```

**2.一个大循环判断六个数字都是小于等于6，并且都互不相等**。

```cpp
40110b:	49 89 e6             	mov    %rsp,%r14 			// %r14保存栈顶指针rsp
40110e:	41 bc 00 00 00 00    	mov    $0x0,%r12d 			// %12d值为0

// 循环：保证输入的元素的值必须是<=6的，否则炸弹爆炸
401114:	4c 89 ed             	mov    %r13,%rbp 				// %rbp保存栈顶指针rsp
401117:	41 8b 45 00          	mov    0x0(%r13),%eax 			// %eax保存ai
40111b:	83 e8 01             	sub    $0x1,%eax 				// ai--
40111e:	83 f8 05             	cmp    $0x5,%eax 			// 如果%eax - 1 <= 5 就跳转到地址为401128的指令处
401121:	76 05                	jbe    401128 <phase_6+0x34>
401123:	e8 12 03 00 00       	callq  40143a <explode_bomb>
401128:	41 83 c4 01          	add    $0x1,%r12d 				// %r12d值为1，做计数器
40112c:	41 83 fc 06          	cmp    $0x6,%r12d 				
401130:	74 21                	je     401153 <phase_6+0x5f> 	// 如果%r12d值是等于6的，则跳转到401153处
401132:	44 89 e3             	mov    %r12d,%ebx				// %ebx保存%r12d

//循环： 判断输入的六个数是不是互不相等，否则炸弹爆炸
401135:	48 63 c3             	movslq %ebx,%rax				// 将%ebx的值以符号扩展的形式保存到%rax中
401138:	8b 04 84             	mov    (%rsp,%rax,4),%eax		// %eax保存下一个位置的值
40113b:	39 45 00             	cmp    %eax,0x0(%rbp)		
40113e:	75 05                	jne    401145 <phase_6+0x51>	// 判断%rbp是否与%eax的值相等，否则炸弹爆炸
401140:	e8 f5 02 00 00       	callq  40143a <explode_bomb>
401145:	83 c3 01             	add    $0x1,%ebx				//计数器%ebx++
401148:	83 fb 05             	cmp    $0x5,%ebx				// 小于等于5继续循环
40114b:	7e e8                	jle    401135 <phase_6+0x41>
    
40114d:	49 83 c5 04          	add    $0x4,%r13				// %r13 + 4，
401151:	eb c1                	jmp    401114 <phase_6+0x20>  	// 证明是一个大循环
```

提供一份伪代码供参考。

<img src="http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221012170930115.png" alt="image-20221012170930115" style="zoom:67%;" />

**3.一个循环实现ai = 7 - ai,将输入的所有数字都转换为7 - ai**。

下面的代码代表是循环执行`a[i] = 7 - a[i]`,将输入的所有元素都进行替换。

```cpp
401153:	48 8d 74 24 18       	lea    0x18(%rsp),%rsi		// %rsi保存rsp+0x18，即指向a6后面4个字节的地址
401158:	4c 89 f0             	mov    %r14,%rax     		// %rax保存rsp，即指向a1
40115b:	b9 07 00 00 00       	mov    $0x7,%ecx			// %ecx保存常数7
    
//循环：计算ai = 7 - ai
401160:	89 ca                	mov    %ecx,%edx			//edx保存7
401162:	2b 10                	sub    (%rax),%edx			//edx保存7-ai
401164:	89 10                	mov    %edx,(%rax)			//ai = 7 - ai
401166:	48 83 c0 04          	add    $0x4,%rax			//rax向后走4个字节，指向ai+1
40116a:	48 39 f0             	cmp    %rsi,%rax    		//未遍历完所有的值，跳转到401160继续执行
40116d:	75 f1                	jne    401160 <phase_6+0x6c>
    
40116f:	be 00 00 00 00       	mov    $0x0,%esi 			// %esi保存0
401174:	eb 21                	jmp    401197 <phase_6+0xa3> //无条件跳转到401197
```

**4.创建一个指针数组，循环进行选择，按照ai的大小来选择链表节点顺序放置到指针数组当中**。

```cpp
401197:	8b 0c 34             	mov    (%rsp,%rsi,1),%ecx 		// %ecx保存 ai
40119a:	83 f9 01             	cmp    $0x1,%ecx 				// 如果ai<=1，跳转到401183
40119d:	7e e4                	jle    401183 <phase_6+0x8f>
```

如果ai <= 1，就去换下一个值来进行选择。

```cpp
//如果 a1 <= 1:
//循环：
401183:	ba d0 32 60 00       	mov    $0x6032d0,%edx			// %edx保存地址0x6032d0
401188:	48 89 54 74 20       	mov    %rdx,0x20(%rsp,%rsi,2)	// 即m[rsp + 20 + 2 * %rsi]储存0x6032d0
40118d:	48 83 c6 04          	add    $0x4,%rsi				// %rsi加4, %rsi是计数器					
401191:	48 83 fe 18          	cmp    $0x18,%rsi				// 判断%rsi是不是等于0x18
401195:	74 14                	je     4011ab <phase_6+0xb7>	// %rsi等于0x18直接跳转到4011ab
401197:	8b 0c 34             	mov    (%rsp,%rsi,1),%ecx 		// %ecx保存ai
40119a:	83 f9 01             	cmp    $0x1,%ecx 				// 如果ai<=1，跳转到401183
40119d:	7e e4                	jle    401183 <phase_6+0x8f>	//
```

ai <= 1的循环结束,换了一个新的值以后，接着执行ai > 1的指令。

用gdb查看0x6032d0的地址，可以看出来存储的是一个链表，每个节点的大小为16个字节。

<img src="http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221011172520406.png" alt="image-20221011172520406"  />

```cpp
//如果 a1 > 1: 
40119f:	b8 01 00 00 00       	mov    $0x1,%eax        		// %eax保存常数1
4011a4:	ba d0 32 60 00       	mov    $0x6032d0,%edx			// %eax保存地址0x6032d0
4011a9:	eb cb                	jmp    401176 <phase_6+0x82>	// 无条件跳转到401176
```

如果指针p是指向链表头的话，下面的循环代表循环`p = p->next` a[i]次。

```cpp
//循环：p指针向后移动a[i]位
401176:	48 8b 52 08          	mov    0x8(%rdx),%rdx 			// %rdx保存m[%rdx + 8] => p = p->next;
40117a:	83 c0 01             	add    $0x1,%eax				// %eax++，做计数器    
40117d:	39 c8                	cmp    %ecx,%eax				// 如果 1 <= a[i],继续循环 
40117f:	75 f5                	jne    401176 <phase_6+0x82>
    
401181:	eb 05                	jmp    401188 <phase_6+0x94> 	//循环结束直接跳转到401188
```

跳转到401188后可以发现这是一个大循环：

```cpp
//循环：
401183:	ba d0 32 60 00       	mov    $0x6032d0,%edx			// 设置指针p指向头节点
    
401188:	48 89 54 74 20       	mov    %rdx,0x20(%rsp,%rsi,2)	// m[rsp + 2 * rsi]保存%rdx保存的链表节点的
40118d:	48 83 c6 04          	add    $0x4,%rsi				// 地址。%rsi做计数器加加4
401191:	48 83 fe 18          	cmp    $0x18,%rsi				// %rsi小于0x18停止，即循环6次
401195:	74 14                	je     4011ab <phase_6+0xb7>	// 循环结束跳转到4011ab
401197:	8b 0c 34             	mov    (%rsp,%rsi,1),%ecx		// 取出ai的值，保存到%ecx中
40119a:	83 f9 01             	cmp    $0x1,%ecx				// 如果节点的值小于等于1，跳转到401183
40119d:	7e e4                	jle    401183 <phase_6+0x8f>	// 开始下一个节点的移动

40119f:	b8 01 00 00 00       	mov    $0x1,%eax				// %eax赋值为1，作为计数器
4011a4:	ba d0 32 60 00       	mov    $0x6032d0,%edx			// %edx指向链表头
4011a9:	eb cb                	jmp    401176 <phase_6+0x82>	// 无条件跳转到401176
```

这个大循环的意思就是让每个节点的指针都走a[i]步，然后将对应节点的值保存到节点指针数组间接调整链表的顺序。调整完毕后跳转到4011ab。

个人感觉这一段汇编很难理解，给一段伪代码帮助理解

<img src="http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221012165057769.png" alt="image-20221012165057769"  />

**5.将数组中的所有节点重新连接起来，形成一个新的链表**。

```cpp
4011ab:	48 8b 5c 24 20       	mov    0x20(%rsp),%rbx			// %rbx保存m[rsp + 32]，即第一个节点的地址
4011b0:	48 8d 44 24 28       	lea    0x28(%rsp),%rax			// %rax保存第二个节点的地址
4011b5:	48 8d 74 24 50       	lea    0x50(%rsp),%rsi			// %rsi保存临界地址
4011ba:	48 89 d9             	mov    %rbx,%rcx 				// %rcx保存第一个节点的地址

// 循环：将所有的链表节点连接起来    
4011bd:	48 8b 10             	mov    (%rax),%rdx				// %rdx保存第二个节点的地址
4011c0:	48 89 51 08          	mov    %rdx,0x8(%rcx)			// 第一个节点->next = 第二个节点
4011c4:	48 83 c0 08          	add    $0x8,%rax				// 继续遍历节点指针数组
4011c8:	48 39 f0             	cmp    %rsi,%rax				// 判断有没有走到尾
4011cb:	74 05                	je     4011d2 <phase_6+0xde>	// 连接完毕后直接跳转到4011d2
4011cd:	48 89 d1             	mov    %rdx,%rcx				// 
4011d0:	eb eb                	jmp    4011bd <phase_6+0xc9> 	// 无条件跳转到4011bd，继续循环
    
4011d2:	48 c7 42 08 00 00 00 	movq   $0x0,0x8(%rdx)			// 循环结束后，%rdx保存的是最后一个节点的地址
4011d9:	00 														// 所以这条指令的意思是最后一个节点的next置空
```

**6.判断新链表是不是降序排列的。如果不是炸弹爆炸**。

```cpp
4011da:	bd 05 00 00 00       	mov    $0x5,%ebp				// %ebp保存5，做计数器
// 循环：比较链表的val值是不是降序排列，否则炸弹爆炸。	   				// %rbp保存的m[rsp + 20]，即第一个节点地址
4011df:	48 8b 43 08          	mov    0x8(%rbx),%rax			// %rax保存的是第二个节点的地址
4011e3:	8b 00                	mov    (%rax),%eax				// 将第二个节点的值保存到%eax 	
4011e5:	39 03                	cmp    %eax,(%rbx)				// 比较第一个节点的值是不是大于第二个节点的值
4011e7:	7d 05                	jge    4011ee <phase_6+0xfa>	// 否则炸弹爆炸
4011e9:	e8 4c 02 00 00       	callq  40143a <explode_bomb>    
4011ee:	48 8b 5b 08          	mov    0x8(%rbx),%rbx			// p = p->next
4011f2:	83 ed 01             	sub    $0x1,%ebp				// %ebp加加，只循环五次
4011f5:	75 e8                	jne    4011df <phase_6+0xeb>
    
//结束代码，释放栈帧空间，弹出被调用者保存寄存器  
4011f7:	48 83 c4 50          	add    $0x50,%rsp
4011fb:	5b                   	pop    %rbx
4011fc:	5d                   	pop    %rbp
4011fd:	41 5c                	pop    %r12
4011ff:	41 5d                	pop    %r13
401201:	41 5e                	pop    %r14
401203:	c3                   	retq       
```

<img src="http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221011172520406.png" alt="image-20221011172520406"  />

由于上面是需要保证链表有序，上面的图片可以看出链表节点val值大小的初始排序是node3 > node4 > node5 > node6 > node1 > node2，最终要保证新链表降序排列，对应节点指针数组的顺序存储。所以新序列为3，4，5，6，1，2。

由于是按照7 - a[i]索引的，所以有a1 = 4, a2 = 3, a3 = 2, a4=1,a5=6,a6=5。所以最终输入的答案就是`4 3 2 1 6 5`。

![image-20221012165401480](http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221012165401480.png)

---

#### secret_phase

![image-20221012211200729](http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221012211200729.png)

通过这句话可以发现是存在隐藏关卡的，但是执行完毕上面的代码以后程序直接就结束了，所以需要想办法进入secret_phase。当你做完phase_6,会发现serct_phase的难度其实不大，最后一关了。

##### 进入secret_phase

<img src="http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221012211507291.png" alt="image-20221012211507291" style="zoom:50%;" />

观看源码发现所有的phase执行完都执行了一个phase_defused()函数，所以可以先看看这个函数的作用。查看phase_defused函数的反汇编代码，在反汇编代码中发现了进入secret_phase的语句，所以猜测是没有问题的。

```cpp
00000000004015c4 <phase_defused>:
4015c4:	48 83 ec 78          	sub    $0x78,%rsp	
4015c8:	64 48 8b 04 25 28 00 	mov    %fs:0x28,%rax				
4015cf:	00 00 														// 设置金丝雀值
4015d1:	48 89 44 24 68       	mov    %rax,0x68(%rsp)	
4015d6:	31 c0                	xor    %eax,%eax
    
4015d8:	83 3d 81 21 20 00 06 	cmpl   $0x6,0x202181(%rip)        # 603760 <num_input_strings>
4015df:	75 5e                	jne    40163f <phase_defused+0x7b> 	// 判断输入的字符串长度是不是不等于6
    																// 如果不等于6直接跳转到401635
4015e1:	4c 8d 44 24 10       	lea    0x10(%rsp),%r8				
4015e6:	48 8d 4c 24 0c       	lea    0xc(%rsp),%rcx
4015eb:	48 8d 54 24 08       	lea    0x8(%rsp),%rdx
4015f0:	be 19 26 40 00       	mov    $0x402619,%esi				// "%d %d %s"
4015f5:	bf 70 38 60 00       	mov    $0x603870,%edi      //0x603870保存的是readline的地址， sscanf函数，从
4015fa:	e8 f1 f5 ff ff       	callq  400bf0 <__isoc99_sscanf@plt> // readline中读入两个整型数据和一个字符串
4015ff:	83 f8 03             	cmp    $0x3,%eax					// 判断是不是读入了三个数据				
401602:	75 31                	jne    401635 <phase_defused+0x71>
    
401604:	be 22 26 40 00       	mov    $0x402622,%esi				// 将"DrEvil"拷贝到%esi中
401609:	48 8d 7c 24 10       	lea    0x10(%rsp),%rdi				// 将读入的字符串拷贝到%rdi中
40160e:	e8 25 fd ff ff       	callq  401338 <strings_not_equal>	// 判断是否相等
401613:	85 c0                	test   %eax,%eax
401615:	75 1e                	jne    401635 <phase_defused+0x71>	// 不相等直接跳转到401635
401617:	bf f8 24 40 00       	mov    $0x4024f8,%edi	//输出"Curses, you've found the secret phase!"
40161c:	e8 ef f4 ff ff       	callq  400b10 <puts@plt>
401621:	bf 20 25 40 00       	mov    $0x402520,%edi	
401626:	e8 e5 f4 ff ff       	callq  400b10 <puts@plt>  
    										//输出"But finding it and solving it are quite different..."
40162b:	b8 00 00 00 00       	mov    $0x0,%eax
401630:	e8 0d fc ff ff       	callq  401242 <secret_phase>		// 进入secret_phase
    
401635:	bf 58 25 40 00       	mov    $0x402558,%edi// %edi保存"Congratulations! You've defused the 
40163a:	e8 d1 f4 ff ff       	callq  400b10 <puts@plt>// bomb!"        将其输出到显示器
    
40163f:	48 8b 44 24 68       	mov    0x68(%rsp),%rax				// 判断金丝雀值
401644:	64 48 33 04 25 28 00 	xor    %fs:0x28,%rax
40164b:	00 00 
40164d:	74 05                	je     401654 <phase_defused+0x90>
40164f:	e8 dc f4 ff ff       	callq  400b30 <__stack_chk_fail@plt>
401654:	48 83 c4 78          	add    $0x78,%rsp
401658:	c3                   	retq   
```

<img src="http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221012214813985.png" alt="image-20221012214813985" style="zoom:67%;" />

注意到sscanf函数后面只有比较读入字符串的部分，所以猜测输入的两个数应该是readline就给出来的。所以这个phase_defused函数的引发应该是由一个输入为两个整型的phase引发的。所以可能是phase_3或者phase_4。试一下就知道了。

<img src="http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221012222225048.png" alt="image-20221012222225048" style="zoom:67%;" />

<img src="http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221012222118639.png" alt="image-20221012222118639" style="zoom:67%;" />

经过测试发现是在phase_4后加入会触发secret_phase。

---

##### 拆除secret_phase

```cpp
0000000000401242 <secret_phase>:
// 输入一个字符串，转换为数字判断是不是小于等于1000，如果大于炸弹爆炸
401242:	53                   	push   %rbx
401243:	e8 56 02 00 00       	callq  40149e <read_line> 	// 读入一行字符串
401248:	ba 0a 00 00 00       	mov    $0xa,%edx 			// base设置为十进制
40124d:	be 00 00 00 00       	mov    $0x0,%esi 			// endptr设置为NULL
401252:	48 89 c7             	mov    %rax,%rdi 			// 读入的字符串作为str
401255:	e8 76 f9 ff ff       	callq  400bd0 <strtol@plt> 	// 函数作用是将字符串转换为数字
40125a:	48 89 c3             	mov    %rax,%rbx 			// %rbx保存转换完得到的数字
40125d:	8d 40 ff             	lea    -0x1(%rax),%eax 		// %rax--
401260:	3d e8 03 00 00       	cmp    $0x3e8,%eax			
401265:	76 05                	jbe    40126c <secret_phase+0x2a> // 判断%eax是不是小于等于0x3e8(十进制1001)
401267:	e8 ce 01 00 00       	callq  40143a <explode_bomb>
    
40126c:	89 de                	mov    %ebx,%esi				  	// %esi保存转换的数字，作为第一个参数
40126e:	bf f0 30 60 00       	mov    $0x6030f0,%edi			  	// 0x6030f0作为第二个参数
401273:	e8 8c ff ff ff       	callq  401204 <fun7>				// 可以使用gdb查看该地址下面是什么
401278:	83 f8 02             	cmp    $0x2,%eax					// 调用func7，返回值如果不等于2，炸弹爆炸
40127b:	74 05                	je     401282 <secret_phase+0x40>	// 所以需要保证返回值等于2
40127d:	e8 b8 01 00 00       	callq  40143a <explode_bomb>
    
401282:	bf 38 24 40 00       	mov    $0x402438,%edi	 // 输出"Wow! You've defused the secret stage!"
401287:	e8 84 f8 ff ff       	callq  400b10 <puts@plt>
40128c:	e8 33 03 00 00       	callq  4015c4 <phase_defused>
401291:	5b                   	pop    %rbx
401292:	c3                   	retq   
```

应该也是一种数据结构，表示的一个首地址，还有表示地址的值。

<img src="http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221013150611645.png" alt="image-20221013150611645" style="zoom:67%;" />

![image-20221013150350704](C:\Users\石钟\AppData\Roaming\Typora\typora-user-images\image-20221013150350704.png)

接下来主要研究函数fun7，有下面函数的递归调用可以猜测是一个二叉树的数据结构。

```cpp
0000000000401204 <fun7>:
401204:	48 83 ec 08          	sub    $0x8,%rsp
401208:	48 85 ff             	test   %rdi,%rdi			// 判断传入的第二个参数，即该指针是不是NULL
40120b:	74 2b                	je     401238 <fun7+0x34>	// 是NULL直接跳转到401238，返回-1
    
40120d:	8b 17                	mov    (%rdi),%edx			// %edx保存首个节点的值
40120f:	39 f2                	cmp    %esi,%edx			// 判断首节点的val是不是小于转换出来的值
401211:	7e 0d                	jle    401220 <fun7+0x1c>	// 小于等于直接跳转到401220
    
401213:	48 8b 7f 08          	mov    0x8(%rdi),%rdi	// %rdi存储自己的后八个字节地址传入fun7,作为第二个参数
401217:	e8 e8 ff ff ff       	callq  401204 <fun7>		// 递归调用
40121c:	01 c0                	add    %eax,%eax			// 返回值乘2，
40121e:	eb 1d                	jmp    40123d <fun7+0x39>	// 跳转到40123d，return 2 * %rax
    
401220:	b8 00 00 00 00       	mov    $0x0,%eax				// 初始化返回值
401225:	39 f2                	cmp    %esi,%edx				// 判断转换出来的数字等于节点的val
401227:	74 14                	je     40123d <fun7+0x39>		// 直接return 0
401229:	48 8b 7f 10          	mov    0x10(%rdi),%rdi	//%rdi存储自己的后16个字节地址传入fun7,作为第二个参数 
40122d:	e8 d2 ff ff ff       	callq  401204 <fun7>			// 递归调用
    
401232:	8d 44 00 01          	lea    0x1(%rax,%rax,1),%eax	// 返回值乘2 + 1
401236:	eb 05                	jmp    40123d <fun7+0x39>		//跳转到40123d，return 2 * %rax + 1
    
401238:	b8 ff ff ff ff       	mov    $0xffffffff,%eax			// 返回-1
40123d:	48 83 c4 08          	add    $0x8,%rsp				// 直接返回
401241:	c3                   	retq   
```

看完fun7的代码，已经很明显了，这个数据结构就是一个二叉树，包含val,*left和 *right两个指针，fun7的伪代码应该也很容易看出来。

![image-20221013162716978](http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221013162716978.png)

代码比较简单，所以接下来的要做的就是将二叉树给画出来。进而求出满足的x的值。通过gdb来查看二叉树的值，并且将其画出来。先随便打印一下内存单元数量，直到可以将所有的节点打印出来。

<img src="http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221013162751730.png" alt="image-20221013162751730" style="zoom:67%;" />

可以通过左右子树的地址将所有的节点连接起来，然后可以将完整的二叉树画出来。

<img src="http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221016085521712.png" alt="image-20221016085521712" style="zoom:67%;" />

我这里所有节点的十进制值都求了出来，然后按地址连接。可以发现**这颗二叉树是一颗二叉搜索树，所以上面的递归也就不是很难了**。

从serect_phase中可以看出需要**fun7的返回值为2**，所以可以结合fun7的伪代码来确定输入的值。观察伪代码可以发现有两种情况。

1. 输入的值等于节点的值，一定会导致最后一个递归fun返回的是0。
2. 输入的值不是节点的值，一定会导致最后一个递归的函数返回的是-1。

所以可以由此向上反推出最后的返回值的所有情况，很明显当输入的值不等于节点的值的时候，一定不会返回2，所以输入的值一定是与节点的一个值相等，看第一个图有两种情况，0 > 1 > 2和0  > 0 > 1 > 2。可以反推出答案，第一种情况要求相等节点出现在第三层，第二种情况要求相等节点出现在第四层。所以此时有两个答案，简单反推可以得到22和80，但是当输入80时，是直接走向了右子树，所以不会与节点80相遇，所以不会出现返回0的情况，可以排除，当输入为22时，是可以相遇，产生返回值为0的情况，所以secret_phase的答案为`22`。

<img src="http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221016091403893.png" alt="image-20221016091403893" style="zoom: 67%;" />

<img src="http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221018170802475.png" alt="image-20221018170802475" style="zoom:67%;" />



<img src="http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221013163820135.png" alt="image-20221013163820135" style="zoom:67%;" />

---

#### Bomb Lab总结

首先说一下自己做这些phase的情况，1，2，3，4都比较简单，很容易就可以看出来，phase_5没有做出来，索引那里没有看出来，网上查了一下就懂了。phase_6就是看天书了，代码特别长，一下也没看出来是来链表，但是但是一些简单的模块还是看出来了，最后也是看了别人写的教程写出来的。phase_6真的特别不错，搞懂以后。自己做secret_phase行云流水，一气呵成。难度简单排个序phase_6 > secret_phase > phase_5。其他的都比较简单。

这几个lab做下来感觉自己的阅读汇编代码的能力大大提升了，以前看的基本不大懂，和看英语一样，认得几个，但是又不太懂，但是现在熟练多了，感觉挺不错的。这个实验没有白做。下面要开始Attack Lab了。

