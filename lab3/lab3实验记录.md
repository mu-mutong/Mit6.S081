# Lab3: page tables
课程实验指导书[Lab3: page tables](https://pdos.csail.mit.edu/6.828/2020/labs/pgtbl.html)
## 1.Print a page table
### 1.1 实验准备
1. 参考xv6 book 第三章
2. 虚拟内存代码：`kernel/vm.c`
3. 物理内存的分配和释放：`kernel/kalloc.c`
### 1.2 实验要求
实验目的：为了方便熟悉和调试内存页表内容，需要写一个 `vmprint()` 函数来打印页表内容。格式如下：

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
1. 在 `kernel/vm.c` 中实现 `vmprint()`
2. 可以参考 `kernel/freewalk.c` 的代码
3. 在 `kernel/defs.h` 中声明 `vmprint()` 函数
### 1.4 具体实现
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
### 1.5总结

## A kernel page table per process
