#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
//
int filecopy(char *sname, char *dname)
{
	int fd_s;
	int fd_d;
	char buf[900]; //缓存区
	int sizecopid;
	//打开源文件
	if (-1 == (fd_s = open(sname, O_RDONLY, S_IRWXU | S_IRWXO | S_IRWXG)))
	{
		printf("failed to open %s\n", sname);
		return -1;
	}
	//创建副本文件
	if (-1 == (fd_d = open(dname, O_CREAT | O_RDWR, S_IRWXU | S_IRWXO | S_IRWXG)))
	{
		printf("failed to create %s\n", dname);
		return -1;
	}
	//循环从源文件读取数据和向副本文件写数据
	while (1)
	{
		if (-1 == (sizecopid = read(fd_s, buf, 900)))
		{
			printf("failed to read %s\n", sname);
			return -1;
		}
		//如果到达文件尾则跳出循环
		if (sizecopid <= 0)
			break;
		if (-1 == write(fd_d, buf, sizecopid))
		{
			printf("failed to write %s:%s\n", dname, strerror(errno));
			return -1;
		}
		//输出拷贝日志信息
		printf("copied %d bytes\n", sizecopid);
	}
	close(fd_s);
	close(fd_d);
	return 0;
}

int main(int argc, char *const argv[])
{
	if (argc != 3)
	{
		printf("argc != 3 !\n");
		return -1;
	}
	filecopy(argv[1], argv[2]);
	printf("file copied!\n");
	return 0;
}
