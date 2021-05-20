#include "kernel/types.h"
#include "user/user.h"

#define READEND 0
#define WRITEEND 1

int main(){
    // 创建两个管道
    // pipe p1：父进程写，子进程读
    // pipe p2:父进程读。子进程写
    int p1[2],p2[2];
    char buf[10];
    int pid = 0;
    pipe(p1);
    pipe(p2);
    // 创建子进程
    pid = fork();
    if(pid < 0){
        printf("fork build error\n");
        exit(1);
    }

    if(pid == 0){ // 子进程空间，则从p1管道读“ping”，向p2管道写“pong”
        close(p1[WRITEEND]); // 关闭p1的写通道，子进程需要读，所以需要关闭写通道
        close(p2[READEND]);// 关闭p2的读通道，子进程需要写，所以需要关闭读通道
        // 子进程从p1读父进程的输入“ping”
        read(p1[READEND],buf,4);
        printf("%d: received %s\n",getpid(),buf);
        // 子进程向p2中写入“pong”
        write(p2[WRITEEND],"pong",4);
        close(p2[WRITEEND]);
    }
    if(pid > 0){// 父进程空间，则向p1管道写入“ping”，从p2管道读“pong”
        close(p1[READEND]);// 关闭p1的读通道
        close(p2[WRITEEND]);// 关闭p2的写通道
        // 父进程向p1输入“ping”
        write(p1[WRITEEND],"ping",4);
        close(p1[WRITEEND]);
        // 父进程从p2读“pong”
        read(p2[READEND],buf,4);
        printf("%d: received %s\n", getpid(), buf);
    }
    exit(0);
}

