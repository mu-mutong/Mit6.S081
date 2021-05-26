# Lab1: Xv6 and Unix utilities
课程实验指导书[Lab1: Xv6 and Unix utilities](https://link.zhihu.com/?target=https%3A//pdos.csail.mit.edu/6.828/2020/labs/util.html)
## 1.sleep
sleep实验主要练习调用系统函数`sleep()`，但是需要调用时输入一个sleep时长，如果没有输入时长，则应该有报错提示。
## 2.pingpong
练习使用管道进行进程间通信，要注意管道只支持单向传输，双向通信需要建立两个管道。
## 3.primes 
在实现`pingpong()`的基础上，进一步利用管道，实现进程之间的通信，需要自己实现一下埃拉托斯特尼素数筛算法，才能更好的理解如何利用管道。
## 4.find
实现一个查询文件的功能函数，其中大部分代码和`user/ls.c`中类似，可以借鉴其中的查询思想。  
首先需要理解清楚fstat文件信息描述符的功能，在xv6中文件信息结构描述为：

    struct stat {
    int dev;     // 文件系统设备号
    uint ino;    // Inode 值
    short type;  // 文件类型
    short nlink; // 文件被链接数
    uint64 size; // 文件大小
    };
`type == 0` 表示文件是一个file普通文件，`type == 1` 表示是一个目录文件。普通文件就检查文件名是否为查找对象，如果是目录文件就遍历目录项进行查找。需要注意跳过`.`和`..`目录，防止进入死循环。
## 5.xargs
Unix系统的xarg函数[xargs](http://www.ruanyifeng.com/blog/2019/08/xargs-tutorial.html)。需要在子进程空间调用`exec()`函数。

