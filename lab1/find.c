#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

// 在路径里找到最后一个 file的名字，如果长度不超过14（DIRSIZ）就
// 存入buf,如果超过，就直接返回名字
// 假如path=“./a/b”
// 则 b就是path的最后一个文件名，也可以理解为最后一个/后的字符是什么
char* fmtname(char *path)
{
  static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  buf[strlen(p)] = 0;
  return buf;
}

void find(char *path,char *name)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;
  //printf("%s\n", path);
    // 打开路径
  if((fd = open(path, 0)) < 0){
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }

 //把路径最终得到的file信息存入st
  if(fstat(fd, &st) < 0){
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  }
//判断给出的路径最终指向的是一个file还是一个目录（directory）
  switch(st.type){
// st.type，可以查看是path是普通文件还是目录文件，如果是普通文件就查看文件名是否对应一样，目录则遍历其目录项递归查找。
  case T_FILE:
    if(strcmp(fmtname(path),name) == 0){
        printf("%s\n", path);
    }
    
    break;
// 假如path是一个目录。如path==‘.’, 开始遍历目录中的文件，判断有没有和寻找文件名相同的file
// buf 负责存遍历过的路径，指针p指向buf的末尾，保证每次找的路径都可以加入buf。同时在每一层目录末尾加'/'

  case T_DIR:
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof (buf)){
      printf("find: path too long\n");
      break;
    }
    strcpy(buf, path);
    p = buf+strlen(buf);
    *p++ = '/';
    // 每次从当前目录读取de字节大小的目录信息
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
        // 判断索引是否为0，为0则不做后续的判断
      if(de.inum == 0)
        continue;
      //printf("%s\n",buf);
    // 不为0先把de的name存入buf,这里p没有移动位置，一直指向buf的末尾，即每次循环都会覆盖上一次存的de.name
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      //加入de.name=='.'(本目录)或者de.name==".."(上级目录)则跳过，防止死循环
      if (!strcmp(de.name, ".") || !strcmp(de.name, "..")) continue;

      find(buf, name);
    }
    break;
  }
  close(fd);
}


int main(int argc, char *argv[])
{

  if(argc != 3){
    fprintf(2, "usage: find <path> <name>\n");
    exit(1);
  }
  
  find(argv[1], argv[2]);
  exit(0);
}