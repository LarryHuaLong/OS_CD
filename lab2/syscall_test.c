#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
int main(int argc,char *const argv[]){
	//调用新的系统功能调用
	syscall(223,argv[1],argv[2]);
	printf("file copied!\n");
	return 0;
}







