# Lab2: system calls
课程实验指导书[Lab2: system calls](https://pdos.csail.mit.edu/6.828/2020/labs/syscall.html)
## 1.System call tracing
### 1.1 实验介绍
实验目的，通过给xv6添加一个trace系统调用来追踪系统调用执行过程。trace函数的输入为各个系统调用标志参数的掩码，即1 << SYS_fork(2的SYS_fork次方)，通过这个参数trace来追踪对应的system call。  
### 1.2 实验涉及的代码空间
涉及到用户空间的代码：`user/user.h`和`user/usys.pl`  
涉及到系统空间的代码：`kernel/sysproc.c`，`kernel/syscall.h` 和 `kernel/syscall.c`  
涉及到进程的代码：`kernel/proc.h`和`kernel/proc.c`  
### 1.3 实现过程
>1. 首先添加trace函数的声明到makefile文件（`Add $U/_trace to UPROGS in Makefile`）
>2. 在 `user/user.h` 中增加trace函数声明 `int trace(int);`
>3. 在 `user/usys.pl` 中添加trace函数的entry
>4. 在 `kernel/proc.h` 的proc结构体中加一个参数mask，来保存trace的参数
>5. 在 `kernel/proc.c` 的fork函数中添加子进程复制父进程的mask代码
>6. 在 `kernel/sysproc.c` 添加trace函数具体实现
>7. 在 `kernel/syscall.h` 添加trace的宏定义 `#define SYS_trace  22`
>8. 在 `kernel/syscall.c` 添加函数定义 `extern uint64 sys_trace(void)`，在函数指针数组 `static uint64 (*syscalls[])(void)` 新增 `[SYS_trace]   sys_trace,`，在`kernel/syscall.c` 新建一个数组存放所有system_call的名字，最后在 `syscall()` 添加追踪功能,代码如下:

    void
    syscall(void)
    {
        int num;
        struct proc *p = myproc();

        num = p->trapframe->a7;
        if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
            p->trapframe->a0 = syscalls[num]();
            // 添加追踪功能
            if (p->mask & (1 << num)){
                printf("%d: syscall %s -> %d\n",p->pid, syscalls_name[num], p->trapframe->a0);
            }
        }else{
            printf("%d %s: unknown sys call %d\n",
            p->pid, p->name, num);
            p->trapframe->a0 = -1;
        }
    }
### 1.4 system call的调用链路
>1. 首先给出函数声明，以及函数的具体实现。
>2. Makefile在编译时通过usys.pl代码进入内核，并且指定寄存器a7的值，来告诉系统执行那一个系统调用。
>3. 之后转到 `syscall.c` 中的函数指针数组，通过寄存器a7的值定位对应的函数指针，来调用函数。
## 2.Sysinfo

>1. 首先添加sysinfotest文件声明到makefile文件（`Add $U/_sysinfotest to UPROGS in Makefile`）
>2. 在 `user/user.h`中增加结构体声明 `struct sysinfo;` 和sysinfo函数声明 `int sysinfo(struct sysinfo *);`
>3. 在 `user/usys.pl` 中添加sysinfo函数的entry
>4. 在 `kernel/syscall.h` 添加sysinfo的宏定义 `#define SYS_sysinfo  23`
>5. 在 `kernel/syscall.c` 添加函数定义 `extern uint64 sys_sysinfo(void)`，在函数指针数组 `static uint64 (*syscalls[])(void)` 新增 `[SYS_sysinfo]   sys_sysinfo,`，在`syscalls_name` 数组中添加sysinfo
>6. 在 `kernel/proc.c` 的添加查询正在使用进程的函数:

    int
    proc_size()
    {
        int i;
        int n = 0;
        for (i = 0; i < NPROC; i++){
            if (proc[i].state != UNUSED) n++;
        }
        return n;
    }
>7. 在 `kernel/kalloc.c` 的添加查询剩余内存容量的函数:

    uint64 
    freememory()
    {
        struct run* p = kmem.freelist;
        uint64 num = 0;
        while (p){
            num ++;
            p = p->next;
        }
    return num * PGSIZE;
    }
>8. 在 `kernel/defs.h` 的添加上面两个函数的声明
>9. 最后在`kernel/sysproc.c`添加sysinfo的具体实现(记得在文件头添加sysinfo头文件):

    uint64
    sys_sysinfo(void)
    {
        struct sysinfo info;
        uint64 addr;
        // 获取用户态传入的sysinfo结构体
        if (argaddr(0, &addr) < 0) 
            return -1;
        struct proc* p = myproc();
        info.freemem = freememory();
        info.nproc = proc_size();
        // 将内核态中的info复制到用户态
        if (copyout(p->pagetable, addr, (char*)&info, sizeof(info)) < 0)
            return -1;
        return 0;
}





