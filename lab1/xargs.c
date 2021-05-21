#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

#define STDIN_FILENO 0
#define MAXLINE 1024

int main(int argc, char *argv[])
{
    char line[MAXLINE];
    char* params[MAXLINE];
    int n, args_index = 0;
    int i;
    // echo hello too | xargs echo bye
    //题目简化了xargs函数，即默认输入的第二个参数即为要调用的系统函数(如‘echo’)，将函数名存入cmd中
    char* cmd = argv[1];
    //把需要拼接的输出先存入输出序列中(如‘bye’)
    for(i = 1; i < argc; i++) params[args_index++] = argv[i];
    // 读取前一个echo的输出，存入params
    while((n = read(STDIN_FILENO, line, MAXLINE)) > 0){
        if(fork() == 0){ //子进程空间 ,调用exec函数，
            char* arg = (char*) malloc(sizeof(line));
            int index = 0;
            for(i = 0; i < n; i++){
                if(line[i] == ' ' || line[i] == '\n'){
                    arg[index] = 0;
                    params[args_index++] = arg;
                    index = 0;
                    arg = (char*) malloc(sizeof(line));
                }else{
                    arg[index++] = line[i]; 
                }
            }
            arg[index] = 0;
            params[args_index] = 0;
            //printf("%s\n",cmd);
            exec(cmd,params);
        }else{
            wait((int*)0);
        }
    }
    exit(0);
}