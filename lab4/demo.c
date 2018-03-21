#include "lab4.h"

void test_get_hostname()
{	
	char hostname[100];
	size_t len;
	int rs = get_hostname(hostname,&len);
	if(-1 == rs)
		printf("get_hostname failed.\n");
}
void test_get_uptime()
{	
	double total_time,free_time;

	int rs = get_uptime(&total_time,&free_time);
	if(-1 == rs)
		printf("get_uptime failed.\n");
}
void test_get_osinfo()
{	
	char ostype[20];
	char osrelease[30];

	int rs = get_osinfo(ostype,osrelease);
	if(-1 == rs)
		printf("get_osinfo failed.\n");
}
void test_get_cpuinfo()
{	
	CPUINFO CPUs[8];
	int CPUnum = 0;
	int rs = get_CPUinfo(CPUs,&CPUnum);
	if(-1 == rs)
		printf("get_CPUinfo failed.\n");
}
void test_get_info_pid()
{	
	PIDINFO pidinfo;
	int pid = 1;
	int rs = get_info_pid(&pidinfo,pid);
	if(-1 == rs)
		printf("get_CPUinfo failed.\n");
}
void test_get_all_pids()
{	
	int pids[1000];
	int pidnum = 0;
	int rs = get_all_pids(pids,&pidnum);
	if(-1 == rs)
		printf("get_all_pid failed.\n");
}


int main( int argc, char *argv[] )
{
	
    test_get_hostname();
	test_get_uptime();
    test_get_osinfo();
    test_get_cpuinfo();
    test_get_info_pid();
    test_get_all_pids();
    double cpu_rate;
    get_cpu_rate(&cpu_rate);
    double mem_rate,swap_rate;
    get_mem_rate(&mem_rate,&swap_rate);
	getchar();
    return 0;
}
typedef struct tictack
{
	int times_count;
	int num;
} TICTACK;

static int t = 0;
static int i = 0;
gboolean fun1(gpointer pdata)
{
	TICTACK *tic = pdata;
	printf("%d:fun1:%d\n", tic->times_count, tic->num);
	g_free(tic);
	return FALSE;
}

void *test(void *data)
{

	TICTACK *tic;
	while (i < 10)
	{
		tic = g_new0(TICTACK, 1);
		tic->times_count = t;
		tic->num = i;
		gdk_threads_add_timeout(0, fun1, tic);

		printf("%d,%d\n", t++, i++);
	}
	printf("test done.");
	return NULL;
}


