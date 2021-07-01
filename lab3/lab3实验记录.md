# Lab3: page tables
课程实验指导书[Lab3: page tables](https://pdos.csail.mit.edu/6.828/2020/labs/pgtbl.html)
## 1.Print a page table
### 1.1 实验准备
1. 参考xv6 book 第三章。
2. 虚拟内存代码：`kernel/vm.c`。
3. 物理内存的分配和释放：`kernel/kalloc.c`。
### 1.2 实验要求
实验目的

为了方便熟悉和调试内存页表内容，需要写一个 `vmprint()` 函数来打印页表内容。格式如下：

    page table 0x0000000087f6e000
    ..0: pte 0x0000000021fda801 pa 0x0000000087f6a000
    .. ..0: pte 0x0000000021fda401 pa 0x0000000087f69000
    .. .. ..0: pte 0x0000000021fdac1f pa 0x0000000087f6b000
    .. .. ..1: pte 0x0000000021fda00f pa 0x0000000087f68000
    .. .. ..2: pte 0x0000000021fd9c1f pa 0x0000000087f67000
    ..255: pte 0x0000000021fdb401 pa 0x0000000087f6d000
    .. ..511: pte 0x0000000021fdb001 pa 0x0000000087f6c000
    .. .. ..510: pte 0x0000000021fdd807 pa 0x0000000087f76000
    .. .. ..511: pte 0x0000000020001c0b pa 0x0000000080007000

在`exec.c` 中 `return argc`之前插入`vmprint()`输出页表内容，即`if(p->pid==1) vmprint(p->pagetable)`。
### 1.3 实验hints：
1. 在 `kernel/vm.c` 中实现 `vmprint()`。
2. 可以参考 `kernel/freewalk.c` 的代码。
3. 在 `kernel/defs.h` 中声明 `vmprint()` 函数。
### 1.4 具体实现
```cpp
void _vmprint(pagetable_t pagetable, int level)
{
for(int i = 0; i < 512; i++){
    pte_t pte = pagetable[i];
    if((pte & PTE_V)){
    // this PTE points to a lower-level page table.
    for (int j = 0; j < level; j++)
    {
        if (j == 0) printf("..");
        else printf(" ..");
    }
    uint64 child = PTE2PA(pte);
    printf("%d: pte %p pa %p\n", i, pte, child);
    // 查看flag位是否被设置，若被设置则为最低一层，
    // 见vm.c161行，可以看到只有最底层被设置了符号位
    if ((pte & (PTE_R|PTE_W|PTE_X)) == 0)
         _vmprint((pagetable_t)child, level + 1);
    }
}
}

void vmprint(pagetable_t pagetable)
{
    printf("page table %p\n", pagetable);
    _vmprint(pagetable, 1);
}
```
### 1.5总结
![figure2-1](../Figure/f2-1.png)
参考xv6 book中关于内存页表的描述，页表以树型结构管理物理内存。一个x86 页表就是一个包含 2^20（1,048,576）条*页表条目*（PTEs）的数组。每条 PTE 包含了一个 20 位的物理页号（PPN）及一些标志位。分页硬件要找到一个虚拟地址对应的 PTE，只需使用其高20位来找到该虚拟地址在页表中的索引，然后把其高 20 位替换为对应 PTE 的 PPN。而低 12 位是会被分页硬件原样复制的。因此在虚拟地址-物理地址的翻译机制下，页表可以为操作系统提供对一块块大小为 4096（2^12）字节的内存片，这样的一个内存片就是一*页*。

地址翻译的过程分为两步，首先从地址位的高10位决定对应的页目录条目，然后根据低10位确定对应的 `PTE`。页表的根就负责存放*页目录*，大小为4096字节。其中包含了1024个类似PTE的条目，每个条目指向了一个*页表页*的引用，每一个页表页包含1024个32位PTE数组。所以一个x86页表可以包含1024*1024个PTE，即2^20。
## 2.A kernel page table per process
### 2.1 实验要求
实验目的

Xv6 有一个内核页表，每当程序在内核中执行时都会使用它。 内核页表是直接映射到物理地址的。 Xv6 还为每个进程的用户地址空间提供了一个单独的页表，仅包含该进程的用户内存的映射，从虚拟地址 0 开始。 由于内核页表不包含这些映射，因此用户地址在内核中无效。当内核需要使用在系统调用中传递的用户指针（例如，传递给 write() 的缓冲区指针）时，内核必须首先将用户指针转换为物理地址。 实验2和实验3的目的就是允许内核直接解析用户指针。

实验2要求更改内核，以便每个进程在内核中执行时都使用自己的内核页表副本。 修改 `struct proc` 为每个进程维护一个内核页表，修改调度器在切换进程时切换内核页表。 对于这一步，每个进程内核页表应该与现有的全局内核页表相同。
### 2.2 具体实现
按照实验手册的hint步骤实现即可。
1. 在`proc.h`的结构体`proc`中添加一个user的`kernelpage table`结构体声明。
```cpp
// Per-process state
struct proc {
  struct spinlock lock;

  // p->lock must be held when using these:
  enum procstate state;        // Process state
  struct proc *parent;         // Parent process
  void *chan;                  // If non-zero, sleeping on chan
  int killed;                  // If non-zero, have been killed
  int xstate;                  // Exit status to be returned to parent's wait
  int pid;                     // Process ID

  // these are private to the process, so p->lock need not be held.
  uint64 kstack;               // Virtual address of kernel stack
  uint64 sz;                   // Size of process memory (bytes)
  pagetable_t kernelpt;        // 添加用户的内核映射表
  pagetable_t pagetable;       // User page table
  struct trapframe *trapframe; // data page for trampoline.S
  struct context context;      // swtch() here to run process
  struct file *ofile[NOFILE];  // Open files
  struct inode *cwd;           // Current directory
  char name[16];               // Process name (debugging)
};
```
2. 参考`vm.c`中的`kvminit`函数，在`vm.c`编写用户进程的`kernel-pagetable`初始化函数。
```cpp
pagetable_t
proc_kpt_init()
{

  pagetable_t kpt;
  kpt = uvmcreate();
  if(kpt == 0) return 0;

  // uart registers
  uvmmap(kpt, UART0, UART0, PGSIZE, PTE_R | PTE_W);

  // virtio mmio disk interface
  uvmmap(kpt, VIRTIO0, VIRTIO0, PGSIZE, PTE_R | PTE_W);

  // CLINT
  uvmmap(kpt, CLINT, CLINT, 0x10000, PTE_R | PTE_W);

  // PLIC
  uvmmap(kpt, PLIC, PLIC, 0x400000, PTE_R | PTE_W);

  // map kernel text executable and read-only.
  uvmmap(kpt, KERNBASE, KERNBASE, (uint64)etext-KERNBASE, PTE_R | PTE_X);

  // map kernel data and the physical RAM we'll make use of.
  uvmmap(kpt, (uint64)etext, (uint64)etext, PHYSTOP-(uint64)etext, PTE_R | PTE_W);

  // map the trampoline for trap entry/exit to
  // the highest virtual address in the kernel.
  uvmmap(kpt, TRAMPOLINE, (uint64)trampoline, PGSIZE, PTE_R | PTE_X);
  return kpt;
}

void
uvmmap(pagetable_t pagetable, uint64 va, uint64 pa, uint64 sz, int perm)
{
  if(mappages(pagetable, va, sz, pa, perm) != 0)
    panic("uvmmap");
}
```
修改`kvmpa`中的pte
```cpp
pte = walk(myproc()->kernelpt, va, 0);
```
myproc()方法调用，需要在proc.c头部添加头文件引用
```cpp
#include "spinlock.h"
#include "proc.h"
#include "defs.h"
```
3. 在`proc.c`的`allocproc`函数中对用户的`kernel-pagetable`进行初始化。然后要求每个`kernel pagetable`都添加一个对自己`kernel stack`的映射，未修改前，`kernal stack`初始化在`kernel/proc.c/procinit`里，将`stack`初始化的部分移入`proc.c/allocproc`函数中，至此完成对`allocproc`函数的修改。
```cpp
static struct proc*
allocproc(void)
{
  struct proc *p;

  for(p = proc; p < &proc[NPROC]; p++) {
    acquire(&p->lock);
    if(p->state == UNUSED) {
      goto found;
    } else {
      release(&p->lock);
    }
  }
  return 0;

found:
  p->pid = allocpid();

  // Allocate a trapframe page.
  if((p->trapframe = (struct trapframe *)kalloc()) == 0){
    release(&p->lock);
    return 0;
  }

  // An empty user page table.
  p->pagetable = proc_pagetable(p);
  if(p->pagetable == 0){
    freeproc(p);
    release(&p->lock);
    return 0;
  }
  // 添加kernel pagetable
  p->kernelpt = proc_kpt_init();
  if (p->kernelpt == 0){
    freeproc(p);
    release(&p->lock);
    return 0;
  }
  // 把内核映射放到到进程的内核栈里
  // Allocate a page for the process's kernel stack.
  // Map it high in memory, followed by an invalid
  // guard page.
  char *pa = kalloc();
  if(pa == 0) panic("kalloc");
  uint64 va = KSTACK((int) (p - proc));
  // 添加kernel stack的映射到用户的kernel pagetable里
  uvmmap(p->kernelpt, va, (uint64)pa, PGSIZE, PTE_R | PTE_W);
  p->kstack = va;
  // Set up new context to start executing at forkret,
  // which returns to user space.
  memset(&p->context, 0, sizeof(p->context));
  p->context.ra = (uint64)forkret;
  p->context.sp = p->kstack + PGSIZE;

  return p;
}
```
5. 修改`proc.c/scheduler`函数，在进程切换时把用户的`kernel-pagetable`放入到 *satp* 寄存器（参考`vm.c/kvminithart`函数）；然后如果没有进程在运行，就使用内核自己的`kernel pagetable`。
```cpp
void
scheduler(void)
{
  struct proc *p;
  struct cpu *c = mycpu();
  
  c->proc = 0;
  for(;;){
    // Avoid deadlock by ensuring that devices can interrupt.
    intr_on();
    
    int found = 0;
    for(p = proc; p < &proc[NPROC]; p++) {
      acquire(&p->lock);
      if(p->state == RUNNABLE) {
        // Switch to chosen process.  It is the process's job
        // to release its lock and then reacquire it
        // before jumping back to us.
        p->state = RUNNING;
        c->proc = p;
        // 将当前进程的kernel page存入stap寄存器中
        w_satp(MAKE_SATP(p->kernelpt));
        sfence_vma();
        swtch(&c->context, &p->context);
        
        // Process is done running for now.
        // It should have changed its p->state before coming back.
        c->proc = 0;

        found = 1;
      }
      release(&p->lock);
    }
#if !defined (LAB_FS)
    if(found == 0) {
      intr_on();
       // 没有进程在运行则使用内核原来的kernel pagtable
      w_satp(MAKE_SATP(kernel_pagetable));
      sfence_vma();
      asm volatile("wfi");
    }
#else
    ;
#endif
  }
}
```
6. 在进程销毁部分需要添加对用户`kernal pagetable`的释放操作。要注意的是之前我们将`kernel stack`的初始化移入了`allocproc`函数中，原来它是放在了内核的`kernel pagetable`里，但现在放在了我们的`kernel pagetable`里，故先要对`kernel stack`进行查找释放（之前内核释放的时候帮我们处理了`kernel stack`的销毁）。
```cpp
static void
freeproc(struct proc *p)
{
  if(p->trapframe)
    kfree((void*)p->trapframe);
  p->trapframe = 0;
  // 删除kernel stack
  if (p->kstack)
  {
    pte_t* pte = walk(p->kernelpt, p->kstack, 0);
    if (pte == 0)
      panic("freeproc: kstack");
    kfree((void*)PTE2PA(*pte));
  }
  p->kstack = 0;
  if(p->pagetable)
    proc_freepagetable(p->pagetable, p->sz);
    // 删除kernel pagetable
  if (p->kernelpt)
    proc_freekpt(p->kernelpt);
  p->pagetable = 0;
  p->sz = 0;
  p->pid = 0;
  p->parent = 0;
  p->name[0] = 0;
  p->chan = 0;
  p->killed = 0;
  p->xstate = 0;
  p->state = UNUSED;
}
```
7. 然后是对kernel pagetable的释放。
```cpp
// free kernel pagetable
// 模仿vm.c中的freewalk，但注意物理地址没有释放
// 最后一层叶节点没有释放，标志位并没有重置
// 故需要修改一下
void 
proc_freekpt(pagetable_t pagetable)
{
  // there are 2^9 = 512 PTEs in a page table.
  for(int i = 0; i < 512; i++){
    pte_t pte = pagetable[i];
    if((pte & PTE_V)){
      pagetable[i] = 0;
      if ((pte & (PTE_R|PTE_W|PTE_X)) == 0)
      {
        uint64 child = PTE2PA(pte);
        proc_freekpt((pagetable_t)child);
      }
    } else if(pte & PTE_V){
      panic("proc free kpt: leaf");
    }
  }
  kfree((void*)pagetable);
}
```
8. 在`defs.h`中添加对新加函数的说明
```cpp
// vm.c
void            vmprint(pagetable_t pagetable);
pagetable_t     proc_kpt_init(void);
void            uvmmap(pagetable_t, uint64, uint64, uint64, int);
// proc.c
void            proc_freekpt(pagetable_t);
```
