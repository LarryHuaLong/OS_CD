#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
int main(void)
{
    int fd,myseek,readsize,writesize;
    char buf[200] = "";
    fd = open("/dev/hualong", O_RDWR);
    if(fd < 0){
        printf("can't open!\n");
    }
    const char c[] = "0123456789abcdefghijklmnopqrstuvwxwzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    
	writesize = write(fd, c, 55);
    printf("write: %d bytes\n",writesize);
    myseek = lseek(fd,0,SEEK_CUR);
    printf("myseek = %d\n",myseek);	
    
    readsize = read(fd,buf,20);
    printf("read: %d bytes\n",readsize);
    myseek = lseek(fd,0,SEEK_CUR);	
    printf("myseek = %d\n",myseek);
    
    myseek = lseek(fd,10,SEEK_SET);	
    printf("myseek = %d\n",myseek);
    readsize = read(fd,buf,20);
    printf("read: %d bytes\n",readsize);
    myseek = lseek(fd,0,SEEK_CUR);
    printf("myseek = %d\n",myseek);
    
    for(int i = 0;i<20;i++)
    	putchar(buf[i]);
    putchar('\n');
	close(fd);
    return 0;
}
