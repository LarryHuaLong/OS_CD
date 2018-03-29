#include "lab4.h"

time_t boot_time;			//系统启动时间
CPU_RATES cpurates = {0.0}; //CPU历史利用率
MEM_RATES memrates = {0.0}; //MEM历史利用率
//窗口控件指针声明
GtkWidget *window1;
GtkWidget *btn_new_thread;
GtkWidget *pidentry1;
GtkWidget *commandentry2;
GtkWidget *btn_search;
GtkWidget *btn_shutdown;
GtkWidget *btn_endprocess;
GtkWidget *p_pid;
GtkWidget *p_name;
GtkWidget *p_state;
GtkWidget *p_ppid;
GtkWidget *p_priority;
GtkWidget *p_nice;
GtkWidget *p_memsize;
GtkWidget *label_cpu_rate;
GtkWidget *label_mem_rate;
GtkWidget *label_current_time;
GtkWidget *cpu_rate_plot;
GtkWidget *mem_rate_plot;
GtkWidget *label_hostname;
GtkWidget *label_boot_time;
GtkWidget *label_run_time;
GtkWidget *label_os_version;
GtkWidget *label_cpu_type;
GtkWidget *label_cpu_num;
GtkWidget *label_cpu_speed;
GtkWidget *scrolledwindow1;
GtkWidget *treeview1;
GtkListStore *liststore1;
GtkTreeIter list_iter;
GtkCellRenderer *renderer;
GtkTreeSelection *treeselection;

int main(int argc, char *argv[])
{
	//1.gtk初始化
	gtk_init(&argc, &argv);
	//2.创建GtkBuilder对象，GtkBuilder在<gtk/gtk.h>声明
	GtkBuilder *builder = gtk_builder_new();
	//3.读取lab4.glade文件的信息，保存在builder中
	if (!gtk_builder_add_from_file(builder, "lab4.glade", NULL))
	{
		printf("connot load file!");
	}
	//4.获取控件指针
	window1 = GTK_WIDGET(gtk_builder_get_object(builder, "window1"));
	btn_new_thread = GTK_WIDGET(gtk_builder_get_object(builder, "btn_new_thread"));
	pidentry1 = GTK_WIDGET(gtk_builder_get_object(builder, "entry1"));
	commandentry2 = GTK_WIDGET(gtk_builder_get_object(builder, "entry2"));
	btn_search = GTK_WIDGET(gtk_builder_get_object(builder, "btn_search"));
	btn_shutdown = GTK_WIDGET(gtk_builder_get_object(builder, "btn_shutdown"));
	btn_endprocess = GTK_WIDGET(gtk_builder_get_object(builder, "btn_endprocess"));
	p_pid = GTK_WIDGET(gtk_builder_get_object(builder, "p_pid"));
	p_name = GTK_WIDGET(gtk_builder_get_object(builder, "p_name"));
	p_state = GTK_WIDGET(gtk_builder_get_object(builder, "p_state"));
	p_ppid = GTK_WIDGET(gtk_builder_get_object(builder, "p_ppid"));
	p_priority = GTK_WIDGET(gtk_builder_get_object(builder, "p_priority"));
	p_nice = GTK_WIDGET(gtk_builder_get_object(builder, "p_nice"));
	p_memsize = GTK_WIDGET(gtk_builder_get_object(builder, "p_memsize"));
	cpu_rate_plot = GTK_WIDGET(gtk_builder_get_object(builder, "drawingarea1"));
	mem_rate_plot = GTK_WIDGET(gtk_builder_get_object(builder, "drawingarea2"));
	label_hostname = GTK_WIDGET(gtk_builder_get_object(builder, "label_hostname"));
	label_boot_time = GTK_WIDGET(gtk_builder_get_object(builder, "label_boot_time"));
	label_run_time = GTK_WIDGET(gtk_builder_get_object(builder, "label_run_time"));
	label_os_version = GTK_WIDGET(gtk_builder_get_object(builder, "label_os_version"));
	label_cpu_type = GTK_WIDGET(gtk_builder_get_object(builder, "label_cpu_type"));
	label_cpu_num = GTK_WIDGET(gtk_builder_get_object(builder, "label_cpu_num"));
	label_cpu_speed = GTK_WIDGET(gtk_builder_get_object(builder, "label_cpu_speed"));
	label_cpu_rate = GTK_WIDGET(gtk_builder_get_object(builder, "label_cpu_rate"));
	label_mem_rate = GTK_WIDGET(gtk_builder_get_object(builder, "label_mem_rate"));
	label_current_time = GTK_WIDGET(gtk_builder_get_object(builder, "label_current_time"));
	treeview1 = GTK_WIDGET(gtk_builder_get_object(builder, "treeview1"));
	GtkTreeViewColumn *column_name = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "column_name"));
	GtkTreeViewColumn *column_pid = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "column_pid"));
	GtkTreeViewColumn *column_ppid = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "column_ppid"));
	GtkTreeViewColumn *column_memsize = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "column_memsize"));
	GtkTreeViewColumn *column_priority = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "column_priority"));
	//创建进程列表存储器
	liststore1 = gtk_list_store_new(N_COLUMNS, G_TYPE_STRING, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT);
	gtk_tree_view_set_model(GTK_TREE_VIEW(treeview1), GTK_TREE_MODEL(liststore1));
	renderer = gtk_cell_renderer_text_new();
	//设置列表得列标题和属性
	gtk_tree_view_column_pack_start(column_name, renderer, TRUE);
	gtk_tree_view_column_add_attribute((column_name), renderer, "text", COLUMN_NAME);
	gtk_tree_view_column_pack_start(column_pid, renderer, TRUE);
	gtk_tree_view_column_add_attribute((column_pid), renderer, "text", COLUMN_PID);
	gtk_tree_view_column_pack_start(column_ppid, renderer, TRUE);
	gtk_tree_view_column_add_attribute((column_ppid), renderer, "text", COLUMU_PPID);
	gtk_tree_view_column_pack_start(column_memsize, renderer, TRUE);
	gtk_tree_view_column_add_attribute((column_memsize), renderer, "text", COLUMU_MEMSIZE);
	gtk_tree_view_column_pack_start(column_priority, renderer, TRUE);
	gtk_tree_view_column_add_attribute((column_priority), renderer, "text", COLUMU_PRIORITY);

	int rs;
	//获取主机名
	char hostname[100];
	size_t len;
	rs = get_hostname(hostname, &len);
	if (-1 == rs)
		printf("get_hostname failed.\n");
	gtk_label_set_text((GtkLabel *)label_hostname, hostname);
	//获取系统启动时间
	double total_time, idle_time; //uptime信息
	rs = get_uptime(&total_time, &idle_time);
	if (-1 == rs)
		printf("get_uptime failed.\n");
	boot_time = time(NULL) - (time_t)total_time;
	struct tm *run_tm = localtime(&boot_time);
	char buf[20];
	sprintf(buf, "%d:%d:%d", run_tm->tm_hour, run_tm->tm_min, run_tm->tm_sec);
	gtk_label_set_text((GtkLabel *)label_boot_time, buf);
	//获取操作系统类型和版本
	char ostype[20];
	char osrelease[30];
	rs = get_osinfo(ostype, osrelease);
	if (-1 == rs)
		printf("get_osinfo failed.\n");
	char osversion[50];
	sprintf(osversion, "%s %s", ostype, osrelease);
	gtk_label_set_text((GtkLabel *)label_os_version, osversion);
	//获取CPU型号和CPU个数及主频大小
	CPUINFO CPUs[8];
	int CPUnum = 0;
	rs = get_CPUinfo(CPUs, &CPUnum);
	if (-1 == rs)
		printf("get_CPUinfo failed.\n");
	gtk_label_set_text((GtkLabel *)label_cpu_type, CPUs[0].type);
	sprintf(buf, "%d", CPUnum);
	gtk_label_set_text((GtkLabel *)label_cpu_num, buf);
	double cpuspeed = 0.0;
	for (int i = 0; i < CPUnum; i++)
		cpuspeed += CPUs[i].speed;
	cpuspeed /= CPUnum;
	sprintf(buf, "%.2lf MHz", cpuspeed);
	gtk_label_set_text((GtkLabel *)label_cpu_speed, buf);
	//连接信号与回调函数
	g_signal_connect(G_OBJECT(window1), "destroy", G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(G_OBJECT(btn_new_thread), "clicked", G_CALLBACK(new_thread), NULL);
	g_signal_connect(G_OBJECT(btn_search), "clicked", G_CALLBACK(search_pid), NULL);
	g_signal_connect(G_OBJECT(btn_shutdown), "clicked", G_CALLBACK(confirm_shutdown), NULL);
	g_signal_connect(G_OBJECT(btn_endprocess), "clicked", G_CALLBACK(confirm_kill), NULL);
	g_signal_connect(G_OBJECT(cpu_rate_plot), "draw", G_CALLBACK(update_cpu_plots), NULL);
	g_signal_connect(G_OBJECT(mem_rate_plot), "draw", G_CALLBACK(update_mem_plots), NULL);
	//显示所有窗口控件
	gtk_widget_show_all(window1);
	//开启两个工作线程，收集CPU和内存得利用率以及进程列表信息
	g_thread_new("worker1", &collect_rates, NULL);
	g_thread_new("worker2", &collect_pids, NULL);
	gtk_main();
	g_object_unref(G_OBJECT(builder)); //释放GtkBuilder对象
	return 0;
}

void *collect_rates(void *data)
{
	int total0 = 0;
	int idle0 = 0;
	UPDATE_LABELS *ulables;
	while (1)
	{
		sleep(1); //隔1s更新一次数据
		ulables = g_new0(UPDATE_LABELS, 1);
		ulables->cpurate = 0.0;
		ulables->memrate = 0.0;
		ulables->swaprate = 0.0;
		//获取CPU使用率和内存使用率
		if ((-1 == get_cpu_rate(&total0, &idle0, &(ulables->cpurate))))
			ulables->cpurate = 0.0;
		if ((-1 == get_mem_rate(&(ulables->memrate), &(ulables->swaprate))))
			ulables->memrate = 0.0;
		//更新CPU和内存使用率历史数据
		for (int i = 0; i < 120 - 1; i++)
		{
			cpurates.cpu_rates[i] = cpurates.cpu_rates[i + 1];
			memrates.mem_rates[i] = memrates.mem_rates[i + 1];
		}
		cpurates.cpu_rates[119] = ulables->cpurate;
		memrates.mem_rates[119] = ulables->memrate;
		//更新CPU和内存使用率曲线图
		gtk_widget_queue_draw(cpu_rate_plot);
		gtk_widget_queue_draw(mem_rate_plot);
		//获取当前时间
		time_t now_time = time(&(ulables->nowtime));
		gdk_threads_add_timeout(0, update_lables, ulables);
	}
	return NULL;
}

void *collect_pids(void *data)
{
	int pids[1000];
	int pidnum = 0;
	while (1)
	{
		sleep(2); //隔2s更新一次列表
		//获取所有进程的pid和进程个数
		int rs = get_all_pids(pids, &pidnum);
		if (-1 == rs)
			printf("get_all_pid failed.\n");
		PIDINFO *pidinfos = g_new0(PIDINFO, pidnum);
		//对每个pid获取其进程信息
		for (int i = 0; i < pidnum; i++)
		{
			int rs = get_info_pid(&pidinfos[i], pids[i]);
			if (-1 == rs)
				printf("get_info_pid failed.\n");
		}
		PROCESS_list *pro_list = g_new0(PROCESS_list, 1);
		pro_list->pidinfos = pidinfos;
		pro_list->num = pidnum;
		//更新进程列表
		gdk_threads_add_timeout_full(G_PRIORITY_HIGH_IDLE, 0, update_list, pro_list, NULL);
	}
	return NULL;
}

void *newthread(void *data)
{
	char *buffer = data;
	int res = system(buffer);
	printf("system(\"%s\") returned %d\n", buffer, res);
	g_free(buffer);
	return NULL;
}

void new_thread(GtkWidget *widget, gpointer data)
{
	printf("new_thread is called\n");
	char *buffer = g_new0(char, 100);
	//获取命令输入框中得命令
	strcpy(buffer, gtk_entry_get_text((GtkEntry *)commandentry2));
	//创建新线程，终端执行buffer中得命令
	g_thread_new("new_thread", &newthread, buffer);
	return;
}

void search_pid(GtkWidget *widget, gpointer data)
{
	printf("search_pid is called\n");
	char buf[10];
	int pid;
	//获取输入得pid
	sscanf(gtk_entry_get_text(GTK_ENTRY(pidentry1)), "%d", &pid);
	if (pid <= 0) //如果pid非法，则返回
		return;
	PIDINFO pidinfo;
	//获取pid所指进程的详细信息
	int rs = get_info_pid(&pidinfo, pid);
	if (-1 == rs)
		printf("get_CPUinfo failed.\n");
	//显示详细信息到用户界面
	show_detail(NULL, &pidinfo);
	return;
}

void confirm_shutdown(GtkWidget *widget, gpointer data)
{
	printf("confirm_shutdown is called\n");
	GtkWidget *dialog;
	gint res;
	//显示对话框以确认关机
	dialog = gtk_message_dialog_new((GtkWindow *)window1,
									GTK_DIALOG_MODAL,
									GTK_MESSAGE_WARNING,
									GTK_BUTTONS_YES_NO,
									"确认关机吗？");
	res = gtk_dialog_run(GTK_DIALOG(dialog));
	if (res == GTK_RESPONSE_YES)
		system("poweroff\n"); //执行关机指令
	gtk_widget_destroy(dialog);
	return;
}

void confirm_kill(GtkWidget *widget, gpointer data)
{
	printf("confirm_kill is called\n");
	int pid;
	char buf[20];
	//显示对话框以确认杀死进程
	strcpy(buf, gtk_label_get_text((GtkLabel *)p_pid));
	if (buf[0] < '0' || buf[0] > '9')
		return;
	sscanf(buf, "%d", &pid);
	GtkWidget *dialog;
	gint res;
	dialog = gtk_message_dialog_new((GtkWindow *)window1,
									GTK_DIALOG_MODAL,
									GTK_MESSAGE_WARNING,
									GTK_BUTTONS_YES_NO,
									"确认结束进程吗？");
	res = gtk_dialog_run(GTK_DIALOG(dialog));
	if (res == GTK_RESPONSE_YES)
	{
		sprintf(buf, "kill %d", pid);
		system(buf);
	}
	gtk_widget_destroy(dialog);
	return;
}

void show_detail(GtkWidget *widget, gpointer data)
{
	printf("show_detail is called\n");
	PIDINFO *pidinfo = data;
	char buf[20];
	gtk_label_set_text((GtkLabel *)p_name, pidinfo->comm);
	sprintf(buf, "%d", pidinfo->pid);
	gtk_label_set_text((GtkLabel *)p_pid, buf);
	sprintf(buf, "%c", pidinfo->state);
	gtk_label_set_text((GtkLabel *)p_state, buf);
	sprintf(buf, "%d", pidinfo->ppid);
	gtk_label_set_text((GtkLabel *)p_ppid, buf);
	sprintf(buf, "%d", pidinfo->priority);
	gtk_label_set_text((GtkLabel *)p_priority, buf);
	sprintf(buf, "%d", pidinfo->nice);
	gtk_label_set_text((GtkLabel *)p_nice, buf);
	sprintf(buf, "%d", pidinfo->size);
	gtk_label_set_text((GtkLabel *)p_memsize, buf);
	return;
}

gboolean update_cpu_plots(GtkWidget *widget, cairo_t *cr, gpointer data)
{
	guint cpu_width, cpu_height;
	//获取要显示得历史数据
	CPU_RATES *prates = g_new0(CPU_RATES, 1);
	memcpy(prates, &cpurates, sizeof(CPU_RATES));
	//获取绘图区域大小
	cpu_width = gtk_widget_get_allocated_width(cpu_rate_plot);
	cpu_height = gtk_widget_get_allocated_height(cpu_rate_plot);
	//绘制曲线路径
	cairo_move_to(cr, 0, cpu_height);
	for (int i = 0; i < (120); i++)
	{
		cairo_line_to(cr, i * cpu_width / 120, (100 - (prates->cpu_rates[i]) - 1) * cpu_height / 100);
	}
	cairo_set_line_width(cr, 1.0); //设置曲线宽度
	GdkRGBA color;
	color.red = 0.0;
	color.green = 1.0;
	color.blue = 0.0;
	color.alpha = 1.0;
	gdk_cairo_set_source_rgba(cr, &color); //设置线条颜色
	cairo_stroke(cr);
	g_free(prates);
	return FALSE;
}
gboolean update_mem_plots(GtkWidget *widget, cairo_t *cr, gpointer data)
{
	guint mem_width, mem_height;
	//获取要显示得历史数据
	MEM_RATES *prates = g_new0(MEM_RATES, 1);
	memcpy(prates, &memrates, sizeof(MEM_RATES));
	//获取绘图区域大小
	mem_width = gtk_widget_get_allocated_width(mem_rate_plot);
	mem_height = gtk_widget_get_allocated_height(mem_rate_plot);
	//绘制曲线路径
	cairo_move_to(cr, 0, mem_height);
	for (int i = 0; i < (120); i++)
	{
		cairo_line_to(cr, i * mem_width / 120, (100 - (prates->mem_rates[i]) - 1) * mem_height / 100);
	}
	cairo_set_line_width(cr, 1.0); //设置曲线宽度
	GdkRGBA color;
	color.red = 0.0;
	color.green = 1.0;
	color.blue = 0.0;
	color.alpha = 1.0;
	gdk_cairo_set_source_rgba(cr, &color); //设置线条颜色
	cairo_stroke(cr);
	g_free(prates);
	return FALSE;
}
gboolean update_lables(gpointer pdata)
{
	char buf[100];
	UPDATE_LABELS *ulables = pdata;
	sprintf(buf, "%.2lf%%", ulables->cpurate);
	gtk_label_set_text((GtkLabel *)label_cpu_rate, buf);
	sprintf(buf, "%.2lf%%", ulables->memrate);
	gtk_label_set_text((GtkLabel *)label_mem_rate, buf);
	struct tm *now_tm = localtime(&(ulables->nowtime));
	sprintf(buf, "%02d:%02d:%02d", now_tm->tm_hour, now_tm->tm_min, now_tm->tm_sec);
	gtk_label_set_text((GtkLabel *)label_current_time, buf);
	time_t runtime = ulables->nowtime - boot_time;
	struct tm *run_tm = localtime(&runtime);
	sprintf(buf, "%02d:%02d:%02d", run_tm->tm_hour, run_tm->tm_min, run_tm->tm_sec);
	gtk_label_set_text((GtkLabel *)label_run_time, buf);
	g_free(ulables);
	return FALSE;
}

gboolean update_list(gpointer pdata)
{
	PROCESS_list *pro_list = pdata;
	PIDINFO *pidinfos = pro_list->pidinfos;
	gtk_list_store_clear(liststore1);
	for (int i = 0; i < pro_list->num; i++)
	{
		gtk_list_store_append(liststore1, &list_iter);
		gtk_list_store_set(liststore1, &list_iter,
						   COLUMN_NAME, pidinfos[i].comm,
						   COLUMN_PID, pidinfos[i].pid,
						   COLUMU_PPID, pidinfos[i].ppid,
						   COLUMU_MEMSIZE, pidinfos[i].size,
						   COLUMU_PRIORITY, pidinfos[i].priority, -1);
	}
	g_free(pro_list->pidinfos);
	g_free(pro_list);
	return FALSE;
}