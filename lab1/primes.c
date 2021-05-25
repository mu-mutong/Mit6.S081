#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define STDDER_FILENO 2

#define READEND 0
#define WRITEEND 1

typedef int pid_t;

int main(void){
    //题目要求只输出35以内的质数。因此定义一个大小为36的数组来存放2-35。
    int numbers[36], fd[2];
    int i, index = 0;
    pid_t pid;
    // 0和1不是质数也不是合数，因此从2开始存
    
    for (i = 2; i <= 35; i++){
        numbers[index++] = i;
    }
    // 根据筛素数的算法要求，每次筛去数组中是numbers[0]的倍数的数字，index为当前数组内所剩余数字的数目，如果index==0， 则证明所有数字已经筛完，循环结束。
    while (index > 0){
        pipe(fd);
        if ((pid = fork()) < 0) {
            fprintf(STDDER_FILENO, "fork error\n");
            exit(0);
        }
        // 父进程空间负责写所有剩余的数字进入管道中
        else if (pid > 0){
            //printf("cpid: %d\n", pid);
            close(fd[READEND]);
            //printf("%d\n", index);
            for (i = 0; i < index; i++){
                write(fd[WRITEEND], &numbers[i], sizeof(numbers[i]));
                
            }
            close(fd[WRITEEND]);
            wait((int *)0);
            exit(0);
        }
        //子进程空间则读当前管道里的数字，并且判断是否为numbers[0]的倍数。不是则重新存回数组
        else {
            //printf("fpid: %d\n", pid);
            close(fd[WRITEEND]);
            int prime = 0;
            int temp = 0;
            index = -1;
            
            while (read(fd[READEND], &temp, sizeof(temp)) != 0){
                // the first number must be prime ans must in numbers[0]
                if (index < 0) prime = temp, index ++;
                else{
                    if (temp % prime != 0) numbers[index++] = temp;
                }
            }
            printf("prime %d\n", prime);
            // fork again until no prime
            close(fd[READEND]);
        }
    }
    exit(0);
}