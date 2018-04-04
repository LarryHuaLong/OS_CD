#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

int filecopy(char *sname, char *dname)
{
	int fd_s;
	int fd_d;
	char buf[911];
	int sizecopid;
	mm_segment_t fs;
	fs = get_fs();
	set_fs(get_ds());
	if (-1 == (fd_s = sys_open(sname, O_RDONLY, S_IRWXU | S_IRWXO | S_IRWXG)))
	{
		printk("failed to open %s\n", sname);
		return -1;
	}
	if (-1 == (fd_d = sys_open(dname, O_CREAT | O_RDWR, S_IRWXU | S_IRWXO | S_IRWXG)))
	{
		printk("failed to create %s\n", dname);
		return -1;
	}
	while (1)
	{
		if (-1 == (sizecopid = sys_read(fd_s, buf, 900)))
		{
			printk("failed to read %s\n", sname);
			return -1;
		}
		if (sizecopid <= 0)
			break;
		if (-1 == sys_write(fd_d, buf, sizecopid))
		{
			printk("failed to write %s\n", dname);
			return -1;
		}
		printk("copied %d bytes\n", sizecopid);
	}
	sys_close(fd_s);
	sys_close(fd_d);
	set_fs(fs);
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
