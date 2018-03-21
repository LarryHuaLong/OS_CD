#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>

int get_hostname(char* hostname,size_t *plen)
{
	printf("getting hostname...\n");
	char filename[] = "/proc/sys/kernel/hostname";
	FILE *fin;
	if(hostname == NULL){
		printf("error:hostname(NULL)\n");
		return -1;
	}
	
	if(plen == NULL){
		printf("error:plen(NULL)\n");
		return -1;
	}
	if(NULL == (fin = fopen(filename,"r"))){
		printf("failed to open %s:%s\n",filename,strerror(errno));
		return -1;
	}
	if( 1 != fscanf(fin,"%s",hostname)){
		printf("failed to fscanf from %s:%s\n",filename,strerror(errno));
		return -1;
	}
	fclose(fin);
	*plen = strlen(hostname);
	printf("hostname:%s,len=%d\n",hostname,*plen);
	
	return 0;
}

int get_uptime(double* total_time,double* free_time)
{
	printf("getting uptime...\n");
	char filename[] = "/proc/uptime";
	FILE *fin;
	if(total_time == NULL){
		printf("error:total_time(NULL)\n");
		return -1;
	}
	if(free_time == NULL){
		printf("error:free_time(NULL)\n");
		return -1;
	}
	if(NULL == (fin = fopen(filename,"r"))){
		printf("failed to open %s:%s\n",filename,strerror(errno));
		return -1;
	}
	if( 2 != fscanf(fin,"%lf%lf",total_time,free_time)){
		printf("failed to fscanf from %s:%s\n",filename,strerror(errno));
		return -1;
	}
	fclose(fin);
	printf("total_time=%lf,free_time=%lf\n",*total_time,*free_time);
	
	return 0;
}
int get_osinfo(char* ostype,char* osrelease)
{
	printf("getting osinfo...\n");
	char filename[] = "/proc/version";
	FILE *fin;
	if(ostype == NULL){
		printf("error:ostype(NULL)\n");
		return -1;
	}
	if(osrelease == NULL){
		printf("error:osrelease(NULL)\n");
		return -1;
	}
	if(NULL == (fin = fopen(filename,"r"))){
		printf("failed to open %s:%s\n",filename,strerror(errno));
		return -1;
	}
	if( 2 != fscanf(fin,"%s%*s%s",ostype,osrelease)){
		printf("failed to fscanf from %s:%s\n",filename,strerror(errno));
		return -1;
	}
	fclose(fin);
	printf("ostype=%s,osrelease=%s\n",ostype,osrelease);
	
	return 0;
}
typedef struct CPUINFO{
	char type[100];
	double speed;
}CPUINFO;
int get_CPUinfo(CPUINFO *CPUs,int* CPUnum)
{
	printf("getting CPUinfo...\n");
	char filename[] = "/proc/cpuinfo";
	FILE *fin;
	if(CPUs == NULL){
		printf("error:CPUtypes(NULL)\n");
		return -1;
	}
	if(NULL == (fin = fopen(filename,"r"))){
		printf("failed to open %s:%s\n",filename,strerror(errno));
		return -1;
	}
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	int count = 0;
	while(-1 != (read = getline(&line,&len,fin))){
		char *ptr;
		if(ptr = strstr(line,"model name")){
			ptr = strstr(line,":");
			strcpy(CPUs[count].type,ptr+1);
			continue;
		}
		if(ptr = strstr(line,"cpu MHz")){
			ptr = strstr(line,":");
			sscanf(ptr+1,"%lf",&(CPUs[count].speed));
			count++;
			continue;
		}		
	}
	free(line);
	fclose(fin);
	for(int i = 0;i < count;i++)
		printf("CPU%d:%s%.3lf MHz\n",i,CPUs[i].type,CPUs[i].speed);
	return 0;
}
typedef struct PIDINFO{
	int pid;//进程号
	char comm[20];//程序名
	char state;//程序状态
	int ppid; //父进程id
	int priority;//动态优先级
	int nice;//静态优先级
	int size;//占用内存大小	
}PIDINFO;

int get_info_pid(PIDINFO *pidinfo,int pid)
{
	if(pidinfo == NULL){
		printf("error:pidinfo(NULL)\n");
		return -1;
	}
	printf("getting PIDinfo...\n");
	char filename[50];
	sprintf(filename,"/proc/%d/stat",pid);
	FILE *fin;
	if(NULL == (fin = fopen(filename,"r"))){
		printf("failed to open %s:%s\n",filename,strerror(errno));
		return -1;
	}
	fscanf(fin,"%d",&(pidinfo->pid));
	fscanf(fin,"%*[ (]%[^)]",pidinfo->comm);
	fscanf(fin,"%*[) ]%c",&(pidinfo->state));
	fscanf(fin,"%d",&(pidinfo->ppid));
	for(int i = 0;i<13;i++)
		fscanf(fin,"%*d");
	fscanf(fin,"%d",&(pidinfo->priority));
	fscanf(fin,"%d",&(pidinfo->nice));
	fclose(fin);
	sprintf(filename,"/proc/%d/statm",pid);
	if(NULL == (fin = fopen(filename,"r"))){
		printf("failed to open %s:%s\n",filename,strerror(errno));
		return -1;
	}
	fscanf(fin,"%d",&(pidinfo->size));
	pidinfo->size *= 4;
	fclose(fin);
	printf("pid:%d\tcomm:%s\tstate:%c\tppid:%d\tpriority:%d\tnice:%d\tmemsize:%d\n",pidinfo->pid,pidinfo->comm,pidinfo->state,pidinfo->ppid,pidinfo->priority,pidinfo->nice,pidinfo->size);
	return 0;
}
int get_all_pids(int *pids,int * pidnum)
{
	int count = 0;
	
	DIR *p_dir;
	struct dirent *p_dirent;

	if(NULL == (p_dir = opendir("/proc"))){		//打开文件目录
		perror("opendir");
		return -1;
	}
	while(NULL != (p_dirent = readdir(p_dir))){	//读到一个目录项
		if(p_dirent->d_name[0] > '0' && p_dirent->d_name[0] < '9'){
			pids[count++] = atoi(p_dirent->d_name);
		}
	}
	closedir(p_dir);
	printf("%d\n",count);
	for(int i = 0;i < count;i++){
		printf("%d\t",pids[i]);
		if(i%10 == 9)
			putchar('\n');
	}
	putchar('\n');
	return 0;
}
int get_CPU_stat(int *p_total,int *p_idle)
{
	printf("getting CPU usage...\n");
	int user,nice,system,idle,iowait,irq,softirq;
	char filename[] = "/proc/stat";
	FILE *fin;
	if(p_total == NULL){
		printf("error:total(NULL)\n");
		return -1;
	}
	if(p_idle == NULL){
		printf("error:idle(NULL)\n");
		return -1;
	}
	if(NULL == (fin = fopen(filename,"r"))){
		printf("failed to open %s:%s\n",filename,strerror(errno));
		return -1;
	}
	fscanf(fin,"%*s%d%d%d%d%d%d%d",&user,&nice,&system,&idle,&iowait,&irq,&softirq);
	*p_total = user + nice + system + idle + iowait + irq + softirq;
	*p_idle = idle;
	printf("cputotal:%d,cpuidle:%d\n",*p_total,*p_idle);
	fclose(fin);
	return 0;
}
double get_cpu_rate(double *cpu_rate)
{	
	if(cpu_rate == NULL){
		printf("error:cpu_rate(NULL)\n");
		return -1;
	}
	int total1,total2;
	int idle1,idle2;
	double total_d,idle_d,rate;
	int rs = get_CPU_stat(&total1,&idle1);
	if(-1 == rs)
		printf("get_CPU_stat failed.\n");
	sleep(1);
	rs = get_CPU_stat(&total2,&idle2);
	if(-1 == rs)
		printf("get_CPU_stat failed.\n");
	total_d = total2 - total1;
	idle_d = idle2 - idle1;
	if(total_d)
		rate = 100.0*(1.0 - idle_d/total_d);
	*cpu_rate = rate;
	printf("CPU rate:%.2lf%%\n",rate);
	return rate;
}
double get_mem_rate(double *mem_rate,double *swap_rate)
{
	if(mem_rate == NULL){
		printf("error:mem_rate(NULL)\n");
		return -1;
	}
	if(swap_rate == NULL){
		printf("error:swap_rate(NULL)\n");
		return -1;
	}
	printf("getting Memary usage...\n");
	double memtotal,memfree,swaptotal,swapfree;
	char filename[] = "/proc/meminfo";
	FILE *fin;
	if(NULL == (fin = fopen(filename,"r"))){
		printf("failed to open %s:%s\n",filename,strerror(errno));
		return -1;
	}
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	int count = 0;
	while(-1 != (read = getline(&line,&len,fin))){
		char *ptr;
		if(ptr = strstr(line,"MemTotal")){
			ptr = strstr(line,":");
			sscanf(ptr+1,"%lf",&memtotal);
			printf("MemTotal:\t%lf\n",memtotal);
			continue;
		}	
		if(ptr = strstr(line,"MemFree")){
			ptr = strstr(line,":");
			sscanf(ptr+1,"%lf",&memfree);
			printf("MemFree:\t%lf\n",memfree);
			continue;
		}	
		if(ptr = strstr(line,"SwapTotal")){
			ptr = strstr(line,":");
			sscanf(ptr+1,"%lf",&swaptotal);
			printf("SwapTotal:\t%lf\n",swaptotal);
			continue;
		}	
		if(ptr = strstr(line,"SwapFree")){
			ptr = strstr(line,":");
			sscanf(ptr+1,"%lf",&swapfree);
			printf("SwapFree:\t%lf\n",swapfree);
			continue;
		}		
	}
	free(line);
	fclose(fin);	
	if(memtotal)
		*mem_rate = 100*(memtotal-memfree) / memtotal; 
	if(swaptotal > 0.0)
		*swap_rate = 100*(swaptotal-swapfree) / swaptotal; 
	else
		*swap_rate = 0.0;
	printf("mem:%.2lf%%,swap:%.2lf%%\n",*mem_rate,*swap_rate);
	return 0;
}



