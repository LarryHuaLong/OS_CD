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

#define BUFNUM 5	//缓存区数量
#define BUFSIZE 100 //缓存区大小
void openfilechoosedialog(GtkWidget *widget, gpointer data);
void set_s_name(GtkWidget *widget, gpointer data);
void start_copy(GtkWidget *widget, gpointer data);
void update_read_buffer(GtkWidget *widget, gpointer data);
void update_write_buffer(GtkWidget *widget, gpointer data);
void *read_thread(void *);
void *write_thread(void *);
gboolean set_read_bar(gpointer data);
gboolean set_write_bar(gpointer data);
typedef struct buffer
{					   //缓存区结构
	int size;		   //有效字节数
	char buf[BUFSIZE]; //数据缓存区
} BUFFER;
BUFFER *bufs;
sem_t *full, *empty, *s1, *s2;
char *s_name, *d_name;
pid_t pid1 = -1, pid2 = -1;
long *pfilesize;
key_t shm_key = (key_t)14477; //申请共享内存用的键值
//窗口控件指针
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

int main(int argc, char *argv[])
{
	//以下是申请共享内存
	int segment_id;
	char *shard_memory;
	printf("%d,%d\n", sizeof(sem_t), sizeof(BUFFER) * BUFNUM);
	if (-1 == (segment_id = shmget(shm_key, sizeof(sem_t) * 4 + sizeof(BUFFER) * BUFNUM + 104, IPC_CREAT | 0666)))
	{ //申请共享内存
		perror("shmget error at lab3");
		exit(-1);
	}
	if (-1 == (int)(shard_memory = (char *)shmat(segment_id, 0, 0)))
	{ //映射共享内存
		printf("\n shmat error in lab3 : %s\n", strerror(errno));
		exit(-1);
	}
	full = (sem_t *)shard_memory; //映射信号灯在共享内存中
	empty = (sem_t *)(shard_memory + sizeof(sem_t));
	s1 = (sem_t *)(shard_memory + 2 * sizeof(sem_t));
	s2 = (sem_t *)(shard_memory + 3 * sizeof(sem_t));
	sem_init(full, 1, 0);								 //初始化信号灯,满缓存区数目
	sem_init(empty, 1, BUFNUM);							 //空缓存区数目；
	sem_init(s1, 1, 0);									 //控制子进程1能否继续运行
	sem_init(s2, 1, 0);									 //控制子进程2能否继续运行
	bufs = (BUFFER *)(shard_memory + 4 * sizeof(sem_t)); //映射缓存区在共享内存中
	s_name = (char *)(shard_memory + 4 * sizeof(sem_t) + sizeof(BUFFER) * BUFNUM);
	d_name = (char *)(shard_memory + 4 * sizeof(sem_t) + sizeof(BUFFER) * BUFNUM + 50);
	pfilesize = (long *)(shard_memory + 4 * sizeof(sem_t) + sizeof(BUFFER) * BUFNUM + 100);
	//子进程1
	pid1 = fork();
	if (pid1 == 0)
	{
		printf("entered read process\n");
		//1.gtk初始化
		gtk_init(&argc, &argv);
		//2.创建GtkBuilder对象，GtkBuilder在<gtk/gtk.h>声明
		GtkBuilder *builder_read = gtk_builder_new();
		//3.读取test.glade文件的信息，保存在builder中
		if (!gtk_builder_add_from_file(builder_read, "reader.glade", NULL)){
			printf("connot load file!");
		}
		//4.获取窗口指针，注意"window1"要和glade里面的标签名词匹配
		window2 = GTK_WIDGET(gtk_builder_get_object(builder_read, "window2"));
		textview1 = GTK_WIDGET(gtk_builder_get_object(builder_read, "textview1"));
		progressbar1 = GTK_WIDGET(gtk_builder_get_object(builder_read, "progressbar1"));
		printf("progressbar1:%p\n", progressbar1);
		//获取textview的buffer
		buffer_read = gtk_text_view_get_buffer((GtkTextView *)textview1);
		g_signal_connect(G_OBJECT(window2), "destroy", G_CALLBACK(gtk_main_quit), NULL);
		gtk_widget_show_all(window2);
		g_thread_new("reader", &read_thread, NULL); //创建读线程
		gtk_main();
		g_object_unref(G_OBJECT(builder_read)); //释放GtkBuilder对象
		printf("read process exited\n");
		return 0;
	}
	//子进程2
	pid2 = fork();
	if (pid2 == 0)
	{
		printf("entered write process\n");
		//1.gtk初始化
		gtk_init(&argc, &argv);
		//2.创建GtkBuilder对象，GtkBuilder在<gtk/gtk.h>声明
		GtkBuilder *builder_write = gtk_builder_new();
		//3.读取test.glade文件的信息，保存在builder中
		if (!gtk_builder_add_from_file(builder_write, "writer.glade", NULL)){
			printf("connot load file!");
		}
		//4.获取窗口指针，注意"window1"要和glade里面的标签名词匹配
		window3 = GTK_WIDGET(gtk_builder_get_object(builder_write, "window3"));
		textview2 = GTK_WIDGET(gtk_builder_get_object(builder_write, "textview1"));
		progressbar2 = GTK_WIDGET(gtk_builder_get_object(builder_write, "progressbar2"));
		printf("progressbar2:%p\n", progressbar2);
		//获取textview的buffer
		buffer_write = gtk_text_view_get_buffer((GtkTextView *)textview2);
		//定义一个信号
		g_signal_connect(G_OBJECT(window3), "destroy", G_CALLBACK(gtk_main_quit), NULL);
		gtk_widget_show_all(window3);
		g_thread_new("writer", &write_thread, NULL); //创建写线程
		gtk_main();
		g_object_unref(G_OBJECT(builder_write)); //释放GtkBuilder对象
		printf("write process exited\n");
		return 0;
	}
	//1.gtk初始化
	gtk_init(&argc, &argv);
	//2.创建GtkBuilder对象，GtkBuilder在<gtk/gtk.h>声明
	GtkBuilder *builder_main = gtk_builder_new();
	//3.读取test.glade文件的信息，保存在builder中
	if (!gtk_builder_add_from_file(builder_main, "chooser.glade", NULL)){
		printf("connot load file!");
	}
	//4.获取窗口指针，注意"window1"要和glade里面的标签名词匹配
	window1 = GTK_WIDGET(gtk_builder_get_object(builder_main, "window1"));
	button_openfile = GTK_WIDGET(gtk_builder_get_object(builder_main, "button1"));
	entry_save = GTK_WIDGET(gtk_builder_get_object(builder_main, "savefilename"));
	button_start = GTK_WIDGET(gtk_builder_get_object(builder_main, "start"));
	printf("entry_save:%p\n", entry_save);
	g_signal_connect(G_OBJECT(button_openfile), "clicked", G_CALLBACK(openfilechoosedialog), NULL);
	g_signal_connect(G_OBJECT(button_start), "clicked", G_CALLBACK(start_copy), NULL);
	g_signal_connect(G_OBJECT(window1), "destroy", G_CALLBACK(gtk_main_quit), NULL);
	gtk_main();
	g_object_unref(G_OBJECT(builder_main)); //释放GtkBuilder对象
	printf("main process exited\n");
	kill(pid1, 0); //结束两个子进程
	kill(pid2, 0);
	sem_destroy(full); //删除信号灯；
	sem_destroy(empty);
	sem_destroy(s1); //删除信号灯；
	sem_destroy(s2);
	shmctl(segment_id, IPC_RMID, 0); //释放共享内存
	return 0;
}

void openfilechoosedialog(GtkWidget *widget, gpointer data)
{
	printf("openfilechoosedialog called\n");
	GtkWidget *dialog;
	GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
	gint res;
	dialog = gtk_file_chooser_dialog_new("Open File",
										 (GtkWindow *)window1,
										 action,
										 "Cancel",
										 GTK_RESPONSE_CANCEL,
										 "Open",
										 GTK_RESPONSE_ACCEPT,
										 NULL);
	res = gtk_dialog_run(GTK_DIALOG(dialog));
	if (res == GTK_RESPONSE_ACCEPT){
		char *filename;
		GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
		filename = gtk_file_chooser_get_filename(chooser);
		if (filename)
			strcpy(s_name, filename);
		printf("opened file:%s\n", filename);
		g_free(filename);
	}
	gtk_widget_destroy(dialog);
	return;
}
void start_copy(GtkWidget *widget, gpointer data)
{
	printf("start_copy called\n");
	char buf[30] = "";
	printf("entry:%p\n", entry_save);
	if (0 < gtk_entry_get_text_length((GtkEntry *)entry_save))
		strcpy(d_name, gtk_entry_get_text((GtkEntry *)entry_save));
	else
		strcpy(d_name, "Untitled");
	printf("source file:%s\ndest file:%s\n", s_name, d_name);
	struct stat statbuf;
	if (-1 == stat(s_name, &statbuf))
		perror("stat");
	*pfilesize = statbuf.st_size;
	printf("filesize:%ld\n", *pfilesize);
	sem_post(s1);
	sem_post(s2);
	return;
}

gboolean updateread(gpointer data)
{
	GtkTextIter read_buffer_end;
	gtk_text_buffer_get_end_iter(buffer_read, &read_buffer_end);
	gtk_text_buffer_insert(buffer_read, &read_buffer_end, (char *)data, -1);
	return FALSE;
}
gboolean updatewrite(gpointer data)
{
	GtkTextIter write_buffer_end;
	gtk_text_buffer_get_end_iter(buffer_write, &write_buffer_end);
	gtk_text_buffer_insert(buffer_write, &write_buffer_end, (char *)data, -1);
	g_free(data);
	return FALSE;
}
void *read_thread(void *data)
{
	printf("read_thread waiting for s1\n");
	sem_wait(s1);
	printf("read_thread started\n");
	char *message;
	int fd;
	double bytecounts = 0;
	message = g_new0(char, 100);
	sprintf(message, "read process started.\n");
	gdk_threads_add_timeout(0, updateread, message);
	if (-1 == (fd = open(s_name, O_RDONLY))){
		printf("failed to open src_file:%s\n", strerror(errno));
		exit(-1);
	}
	else
		printf("openfilename : %s\n", s_name);
	int buf_index = 0;
	BUFFER readbuf;
	while (1){
		sleep(1); //为观察程序执行过程而设置每秒拷贝一段数据
		int sizeread = readbuf.size = read(fd, readbuf.buf, BUFSIZE);//从文件中读数据
		printf("read %d bytes.\n", sizeread);
		message = g_new0(char, 100);
		sprintf(message, "read %d bytes.\n", sizeread);
		gdk_threads_add_timeout(0, updateread, message);

		if (sizeread < 0)
			printf("read error %d:%s \n", errno, strerror(errno));
		if (readbuf.size <= 0) //如果读到最后一块数据跳出循环
			break;
		buf_index = buf_index % BUFNUM; //缓存区索引，根据缓存区数量循环
		sem_wait(empty);//向缓存区存数据
		memcpy((void *)&bufs[buf_index], (void *)&readbuf, sizeof(BUFFER)); 
		sem_post(full);
		bytecounts += sizeread;
		double *d_read = g_new0(double, 1);
		*d_read = bytecounts;
		gdk_threads_add_timeout(0, set_read_bar, d_read);
		buf_index++;
	}
	close(fd);
	printf("read completed.\n");
	message = g_new0(char, 100);
	sprintf(message, "read completed.\n");
	gdk_threads_add_timeout(0, updateread, message);
	return 0;
}

void *write_thread(void *data)
{
	window3;
	printf("write_thread waiting for s2\n");
	sem_wait(s2);
	printf("write_thread started\n");
	char *message;
	int fd;
	double bytecounts = 0.0;
	message = g_new0(char, 100);
	sprintf(message, "write process started.\n");
	gdk_threads_add_timeout(0, updatewrite, message);
	if (-1 == (fd = open(d_name, O_CREAT | O_RDWR, S_IRWXU | S_IRWXO | S_IRWXG))){
		printf("failed to create dest_file:%s\n", strerror(errno));
		exit(-1);
	}
	else
		printf("savefilename : %s\n", d_name);
	int buf_index = 0;
	BUFFER writebuf;
	while (1){
		sleep(1);						//为观察程序执行过程而设置每秒拷贝一段数据
		buf_index = buf_index % BUFNUM; //缓存区索引，根据缓存区数量循环
		sem_wait(full); //从缓存区取数据
		memcpy((void *)&writebuf, (void *)&bufs[buf_index], sizeof(BUFFER)); 
		sem_post(empty);
		int sizewrited = write(fd, writebuf.buf, writebuf.size); //向文件中写数据
		printf("writed %d bytes.\n", sizewrited);
		message = g_new0(char, 100);
		sprintf(message, "writed %d bytes.\n", sizewrited);
		gdk_threads_add_timeout(0, updatewrite, message);
		if (sizewrited < 0)
			printf("write error %d:%s \n", errno, strerror(errno));
		if (writebuf.size < BUFSIZE) //如果读到最后一块数据跳出循环
			break;
		bytecounts += sizewrited;
		double *d_write = g_new0(double, 1);
		*d_write = bytecounts;
		gdk_threads_add_timeout(0, set_write_bar, (gpointer)d_write);
		buf_index++;
	}
	close(fd);
	printf("write completed.\n");
	message = g_new0(char, 100);
	sprintf(message, "write completed.\n");
	gdk_threads_add_timeout(0, updatewrite, message);
	return 0;
}
gboolean set_read_bar(gpointer data)
{
	double *done = data;
	double progress = *done / (double)*pfilesize;
	gtk_progress_bar_set_fraction((GtkProgressBar *)progressbar1, progress);
	g_free(data);
	return FALSE;
}
gboolean set_write_bar(gpointer data)
{
	double *done = data;
	double progress = *done / (double)*pfilesize;
	gtk_progress_bar_set_fraction((GtkProgressBar *)progressbar2, progress);
	g_free(data);
	return FALSE;
}
