#include "kernel/types.h"
#include "user/user.h"
// copy函数作用，把用户的输入复制后输出到屏幕
int main(){
    // buf存储用户的输入
    char buf[64];
    while(1){
        // read函数，系统调用，第一个参数表是从屏幕读取，第二个参数表示读取存数据的地址起点，第三个
        // 参数为最大读取数据的长度。
        int n = read(0,buf,sizeof(buf));
        if(n <= 0){
            break;
        }
        // write 函数，系统调用函数，第一个参数表是输出到屏幕屏幕，第二个参数表示读取存数据的地址起点，第三个
        // 参数为最大读取数据的长度。
        write(1,buf,n);
    }
    exit(0);
}