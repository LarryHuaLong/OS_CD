#include <gtk/gtk.h>
#include <gdk/gdk.h>

typedef struct tictack{
	int times_count;
	int num;
}TICTACK;

static int t = 0;
static int i = 0;
gboolean fun1(gpointer pdata)
{
    //sleep(1);
	TICTACK *tic = pdata;
    printf("%d:fun1:%d\n",tic->times_count,tic->num);
	g_free(tic);
    return FALSE;
}

void* test(void* data)
{	
	
	TICTACK *tic;
	while(i<10){
		tic = g_new0(TICTACK,1);
		tic->times_count = t;
		tic->num = i;
		gdk_threads_add_timeout(0,fun1,tic);
		
		printf("%d,%d\n",t++,i++);

	} 
	printf("test done.");
	return NULL;
}
int main( int argc, char *argv[] )
{

    gtk_init ( &argc, &argv );
	g_thread_new("worker",&test,NULL);		//创建写线程
    gtk_main();
    return 0;
}



