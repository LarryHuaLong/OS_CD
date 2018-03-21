#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <gtk/gtk.h>

#define BUFNUM 5								//缓存区数量
#define BUFSIZE 100							//缓存区大小
void openfilechoosedialog(GtkWidget *widget, gpointer data);
void set_s_name(GtkWidget *widget, gpointer data);
void start_copy(GtkWidget *widget, gpointer data);
void update_read_buffer(GtkWidget *widget, gpointer data);
void update_write_buffer(GtkWidget *widget, gpointer data);
void* read_thread(void*);
void* write_thread(void*);
void set_read_progress(GtkWidget *widget, gpointer data);
void set_write_progress(GtkWidget *widget, gpointer data);
typedef struct buffer{							//缓存区结构
	int size;									//有效字节数
	char buf[BUFSIZE];							//数据缓存区
}BUFFER;
BUFFER *bufs;
sem_t *full,*empty,*s1,*s2;
char *s_name,*d_name;
pid_t pid1 = -1,pid2 = -1;

key_t shm_key = (key_t)14477;					//申请共享内存用的键值
//窗口控件指针
	GtkBuilder *builder;
	GtkWidget *window1;  
	GtkWidget *button_openfile; 
	GtkWidget *dialog_openfile;
	GtkWidget *entry_save; 
	GtkWidget *button_start;
	GtkWidget *window2;  
	GtkWidget *textview1; 		
	GtkWidget *progressbar1; 
	GtkWidget *window3;  
	GtkWidget *textview2; 		
	GtkWidget *progressbar2; 
	GtkTextBuffer *buffer_read;
	GtkTextBuffer *buffer_write;
	GtkTextIter *read_buffer_end;
	GtkTextIter *write_buffer_end;
	
int main(int argc,char *argv[]){
	//以下是申请共享内存
	int segment_id;
	char* shard_memory;
	int sizeGtkTextIter = sizeof(GtkTextIter)*2;
	if(-1 == (segment_id = shmget(shm_key, sizeof(sem_t)*4 + sizeof(BUFFER)*BUFNUM + 200, IPC_CREAT|0666))){	//申请共享内存
		printf("shmget error at lab3 : %s\n",strerror(errno));
		exit(-1);
	}
	if(-1 == (int)(shard_memory = (char*)shmat(segment_id,0,0))){	//映射共享内存
		printf("\n shmat error in lab3 : %s\n",strerror(errno));
		exit(-1);
	}
	full = (sem_t*)shard_memory;				//映射信号灯在共享内存中
	empty = (sem_t*)(shard_memory + sizeof(sem_t));
	s1 = (sem_t*)(shard_memory + 2*sizeof(sem_t));
	s2 = (sem_t*)(shard_memory + 3*sizeof(sem_t));
	sem_init(full,1,0);							//初始化信号灯,满缓存区数目
	sem_init(empty,1,BUFNUM);					//空缓存区数目；
	sem_init(s1,1,0);//控制子进程1能否继续运行
	sem_init(s2,1,0);//控制子进程2能否继续运行
	bufs = (BUFFER*)(shard_memory + 4*sizeof(sem_t));//映射缓存区在共享内存中
	s_name = (char*)(shard_memory + 4*sizeof(sem_t) + sizeof(BUFFER)*BUFNUM);
	d_name = (char*)(shard_memory + 4*sizeof(sem_t) + sizeof(BUFFER)*BUFNUM + 100);
	strcpy(d_name,"Untitled");
	//子进程1
	pid1 = fork();
	if(pid1 == 0 ){	
		printf("entered read process\n");
		//1.gtk初始化  
		gtk_init(&argc,&argv);  
		//2.创建GtkBuilder对象，GtkBuilder在<gtk/gtk.h>声明  
		builder = gtk_builder_new();  
		//3.读取test.glade文件的信息，保存在builder中  
		if ( !gtk_builder_add_from_file(builder,"reader.glade", NULL)) {  
			printf("connot load file!");  
		}  
		//4.获取窗口指针，注意"window1"要和glade里面的标签名词匹配  
		window2 = GTK_WIDGET(gtk_builder_get_object(builder,"window2"));  
		textview1 = GTK_WIDGET(gtk_builder_get_object(builder, "textview1")); 		
		progressbar1 = GTK_WIDGET(gtk_builder_get_object(builder, "progressbar1"));
		//获取textview的buffer
		buffer_read = gtk_text_view_get_buffer((GtkTextView*)textview1);
		
		//定义一个信号
		guint sig_update_read = g_signal_new ("updateread",
												  G_TYPE_OBJECT,
												  G_SIGNAL_RUN_FIRST,
												  0,
												  NULL,
												  NULL,
												  g_cclosure_marshal_VOID__VOID ,
												  G_TYPE_NONE,
												  1,
												  G_TYPE_STRING);
	  	guint sig_read_progress = g_signal_new ("setreadprogress",
												  G_TYPE_OBJECT,
												  G_SIGNAL_RUN_FIRST,
												  0,
												  NULL,
												  NULL,
												  g_cclosure_marshal_VOID__VOID ,
												  G_TYPE_NONE,
												  1,
												  G_TYPE_DOUBLE);
		g_signal_connect(G_OBJECT(window2), "updateread",G_CALLBACK(update_read_buffer),NULL);
		g_signal_connect(G_OBJECT(window2), "setreadprogress",G_CALLBACK(set_read_progress), NULL);
		g_signal_connect(G_OBJECT(window2), "destroy",G_CALLBACK(gtk_main_quit), NULL);
	
		g_object_unref(G_OBJECT(builder));//释放GtkBuilder对象
		
		pthread_t p1;
		pthread_create(&p1,NULL,&read_thread,window2);		//创建读线程
		
    	gtk_main ();  
    	
		//pthread_join(p1,NULL);						//等待读线程结束
		
		printf ("read process exited\n");
		return 0;
	}
	//子进程2
	pid2 = fork();
	if(pid2 == 0){
		printf("entered write process\n");
		//1.gtk初始化  
		gtk_init(&argc,&argv);  
		//2.创建GtkBuilder对象，GtkBuilder在<gtk/gtk.h>声明  
		builder = gtk_builder_new();  
		//3.读取test.glade文件的信息，保存在builder中  
		if ( !gtk_builder_add_from_file(builder,"writer.glade", NULL)) {  
			printf("connot load file!");  
		}  
		//4.获取窗口指针，注意"window1"要和glade里面的标签名词匹配  
		window3 = GTK_WIDGET(gtk_builder_get_object(builder,"window3"));  
		textview2 = GTK_WIDGET(gtk_builder_get_object(builder, "textview1")); 		
		progressbar2 = GTK_WIDGET(gtk_builder_get_object(builder, "progressbar2")); 
		//获取textview的buffer
		buffer_write = gtk_text_view_get_buffer((GtkTextView*)textview2);
		//定义一个信号
		guint sig_update_write = g_signal_new ("updatewrite",
												  G_TYPE_OBJECT,
												  G_SIGNAL_RUN_FIRST,
												  0,
												  NULL,
												  NULL,
												  g_cclosure_marshal_VOID__VOID ,
												  G_TYPE_NONE,
												  1,
												  G_TYPE_STRING);
		guint sig_write_progress = g_signal_new ("setwriteprogress",
												  G_TYPE_OBJECT,
												  G_SIGNAL_RUN_FIRST,
												  0,
												  NULL,
												  NULL,
												  g_cclosure_marshal_VOID__VOID ,
												  G_TYPE_NONE,
												  1,
												  G_TYPE_DOUBLE);
		g_signal_connect(G_OBJECT(window3), "updatewrite",G_CALLBACK(update_write_buffer), NULL);
		g_signal_connect(G_OBJECT(window3), "setwriteprogress",G_CALLBACK(set_write_progress), NULL);
		g_signal_connect(G_OBJECT(window3), "destroy",G_CALLBACK(gtk_main_quit), NULL);
		
		g_object_unref(G_OBJECT(builder));//释放GtkBuilder对象
		
		pthread_t p2;
		pthread_create(&p2,NULL,&write_thread,window3);		//创建写线程
		
    	gtk_main ();  
    	
		//pthread_join(p2,NULL);						//等待写线程结束
		
		printf ("write process exited\n");
		return 0;
	}
	//1.gtk初始化  
	gtk_init(&argc,&argv);  
	//2.创建GtkBuilder对象，GtkBuilder在<gtk/gtk.h>声明  
	builder = gtk_builder_new();  
	//3.读取test.glade文件的信息，保存在builder中  
	if ( !gtk_builder_add_from_file(builder,"chooser.glade", NULL)) {  
	    printf("connot load file!");  
	}  
	//4.获取窗口指针，注意"window1"要和glade里面的标签名词匹配  
	window1 = GTK_WIDGET(gtk_builder_get_object(builder,"window1"));  
	button_openfile = GTK_WIDGET(gtk_builder_get_object(builder, "button1"));
	entry_save = GTK_WIDGET(gtk_builder_get_object(builder, "savefilename")); 
	button_start = GTK_WIDGET(gtk_builder_get_object(builder, "start"));
	
	g_signal_connect(G_OBJECT(button_openfile), "clicked",G_CALLBACK(openfilechoosedialog), NULL);
	g_signal_connect(G_OBJECT(button_start), "clicked",G_CALLBACK(start_copy), NULL);
	g_signal_connect(G_OBJECT(window1), "destroy",G_CALLBACK(gtk_main_quit), NULL);
	
	g_object_unref(G_OBJECT(builder));//释放GtkBuilder对象
	
    gtk_main ();  
    
	printf ("main process exited\n");
	
	kill(pid1,0);								//结束两个子进程结束
	kill(pid2,0);
	sem_destroy(full);							//删除信号灯；
	sem_destroy(empty);
	sem_destroy(s1);							//删除信号灯；
	sem_destroy(s2);
	shmctl(segment_id,IPC_RMID,0);				//释放共享内存
	return 0;
}

void openfilechoosedialog(GtkWidget *widget, gpointer data)
{
	printf("openfilechoosedialog called\n");
	GtkWidget *dialog;
	GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
	gint res;

	dialog = gtk_file_chooser_dialog_new ("Open File",
                                      (GtkWindow*)window1,
                                      action,
                                      "Cancel",
                                      GTK_RESPONSE_CANCEL,
                                      "Open",
                                      GTK_RESPONSE_ACCEPT,
                                      NULL);

	res = gtk_dialog_run (GTK_DIALOG (dialog));
	if (res == GTK_RESPONSE_ACCEPT)
	  {
		char *filename;
		GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
		filename = gtk_file_chooser_get_filename (chooser);
		if(filename)
			strcpy(s_name,filename);
		printf("opened file:%s\n",filename);
		g_free (filename);
	  }

	gtk_widget_destroy (dialog);
	return;
}
void start_copy(GtkWidget *widget, gpointer data)
{
	printf("start_copy called\n");
	sem_post(s1);
	sem_post(s2);
	return;
}
void set_read_progress(GtkWidget *widget, gpointer data)
{
	printf("set_read_progress called:%p,%lf",data,*(double*)data);
	
	//gtk_progress_bar_set_fraction((GtkProgressBar *)progressbar1,*data);
	

}
void set_write_progress(GtkWidget *widget, gpointer data)
{
	printf("set_write_progress called:%p,%lf",data,*(double*)data);
	
	
	

}
void update_read_buffer(GtkWidget *widget, gpointer data)
{
	buffer_read;
	
	GtkTextIter read_buffer_end;
	gtk_text_buffer_get_end_iter(buffer_read,&read_buffer_end);
	gtk_text_buffer_insert (buffer_read,&read_buffer_end, (char*)data,-1);
	
	return;
	
}
void update_write_buffer(GtkWidget *widget, gpointer data)
{
	buffer_write;
	
	GtkTextIter write_buffer_end;
	gtk_text_buffer_get_end_iter(buffer_write,&write_buffer_end);
	gtk_text_buffer_insert (buffer_write,&write_buffer_end, (char*)data,-1);
	
	return;
}
void* read_thread(void* data)
{
	window2;
	
	printf("read_thread waiting for s1\n");
	sem_wait(s1);
	printf("read_thread started\n");
	
	char message[100];
	sprintf(message,"read process started.\n");
	g_signal_emit_by_name(window2,"updateread",message);
	
	int fd;
	if(-1 == (fd = open(s_name,O_RDONLY))){
		printf("failed to open src_file:%s\n",strerror(errno));
		exit(-1);
	}
	else
		printf("openfilename : %s\n",s_name);
	int buf_index = 0;
	BUFFER readbuf;
	while(1){
		int sizeread = readbuf.size = read(fd,readbuf.buf,BUFSIZE);	//从文件中读数据
		
		printf("read %d bytes.\n",sizeread);
		sprintf(message,"read %d bytes.\n",sizeread);
		g_signal_emit_by_name(window2,"updateread",message);
		
		if(sizeread < 0)
			printf("read error %d:%s \n",errno,strerror(errno));
		if(readbuf.size <= 0)					//如果读到最后一块数据跳出循环
			break;
		buf_index = buf_index % BUFNUM;			//缓存区索引，根据缓存区数量循环
		sem_wait(empty);
		memcpy((void*)&bufs[buf_index],(void*)&readbuf,sizeof(BUFFER));	//向缓存区存数据
		sem_post(full);
		buf_index++;
	}
	double a = 1.0;
	//g_signal_emit_by_name(window2,"setreadprogress",&a);
	
	printf("read completed.\n");
	sprintf(message,"read completed.\n");
	g_signal_emit_by_name(window2,"updateread",message);
	close(fd);
	return 0;
}
void* write_thread(void* data)
{
	window3;
	
	printf("write_thread waiting for s2\n");
	sem_wait(s2);
	printf("write_thread started\n");
	
	char message[100];
	sprintf(message,"write process started.\n");
	g_signal_emit_by_name(window3,"updatewrite",message);
	
	int fd;
	if(-1 == (fd = open(d_name,O_CREAT|O_RDWR,S_IRWXU|S_IRWXO|S_IRWXG))){
		printf("failed to create dest_file:%s\n",strerror(errno));
		exit(-1);
	}
	else
		printf("savefilename : %s\n",d_name);
	int buf_index = 0;
	BUFFER writebuf;
	while(1){
		buf_index = buf_index % BUFNUM;			//缓存区索引，根据缓存区数量循环
		sem_wait(full);
		memcpy((void*)&writebuf,(void*)&bufs[buf_index],sizeof(BUFFER));	//从缓存区取数据
		sem_post(empty);
		int sizewrited = write(fd,writebuf.buf,writebuf.size);	//向文件中写数据
		
		printf("writed %d bytes.\n",sizewrited);
		sprintf(message,"writed %d bytes.\n",sizewrited);
		g_signal_emit_by_name(window3,"updatewrite",message);
		
		if(sizewrited < 0)
			printf("write error %d:%s \n",errno,strerror(errno));
		if(writebuf.size < BUFSIZE)				//如果读到最后一块数据跳出循环
			break;
		buf_index++;
	}
	double b = 1.0;
	//g_signal_emit_by_name(window3,"setwriteprogress",&b);
	
	printf("write completed.\n");
	sprintf(message,"write completed.\n");
	g_signal_emit_by_name(window3,"updatewrite",message);
	close(fd);
	return 0;
}




