#include "lab4.h"

double cpu_rates[120] = {0.0};
double mem_rates[120] = {0.0};
time_t boot_time; //系统启动时间

//窗口控件指针声明
GtkWidget *window1;
GtkWidget *btn_new_process;
GtkWidget *searchentry1;
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
GtkWidget *treeview1;
GtkWidget *label_cpu_rate;
GtkWidget *label_mem_rate;
GtkWidget *label_current_time;
GtkWidget *cpu_rate_box;
GtkWidget *mem_rate_box;
GtkWidget *label_hostname;
GtkWidget *label_boot_time;
GtkWidget *label_run_time;
GtkWidget *label_os_version;
GtkWidget *label_cpu_type;
GtkWidget *label_cpu_num;
GtkWidget *label_cpu_speed;

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
	btn_new_process = GTK_WIDGET(gtk_builder_get_object(builder, "btn_new_process"));
	searchentry1 = GTK_WIDGET(gtk_builder_get_object(builder, "searchentry1"));
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
	treeview1 = GTK_WIDGET(gtk_builder_get_object(builder, "treeview1"));
	cpu_rate_box = GTK_WIDGET(gtk_builder_get_object(builder, "cpu_rate_box"));
	mem_rate_box = GTK_WIDGET(gtk_builder_get_object(builder, "mem_rate_box"));
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

	int rs;
	char hostname[100];
	size_t len;
	rs = get_hostname(hostname, &len);
	if (-1 == rs)
		printf("get_hostname failed.\n");
	gtk_label_set_text((GtkLabel *)label_hostname, hostname);
	double total_time, idle_time; //uptime信息
	rs = get_uptime(&total_time, &idle_time);
	if (-1 == rs)
		printf("get_uptime failed.\n");
	boot_time = time(NULL) - (time_t)total_time;
	struct tm *run_tm = localtime(&boot_time);
	char buf[20];
	sprintf(buf, "%d:%d:%d", run_tm->tm_hour, run_tm->tm_min, run_tm->tm_sec);
	gtk_label_set_text((GtkLabel *)label_boot_time, buf);

	char ostype[20];
	char osrelease[30];
	rs = get_osinfo(ostype, osrelease);
	if (-1 == rs)
		printf("get_osinfo failed.\n");
	char osversion[50];
	sprintf(osversion, "%s %s", ostype, osrelease);
	gtk_label_set_text((GtkLabel *)label_os_version, osversion);

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

	g_signal_connect(G_OBJECT(window1), "destroy", G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(G_OBJECT(btn_new_process), "clicked", G_CALLBACK(new_process), NULL);
	g_signal_connect(G_OBJECT(btn_search), "clicked", G_CALLBACK(search_pid), NULL);
	g_signal_connect(G_OBJECT(btn_shutdown), "clicked", G_CALLBACK(confirm_shutdown), NULL);
	g_signal_connect(G_OBJECT(btn_endprocess), "clicked", G_CALLBACK(confirm_kill), NULL);

	g_object_unref(G_OBJECT(builder)); //释放GtkBuilder对象
	gtk_widget_show_all(window1);
	g_thread_new("worker", &collect_rates, NULL);
	gtk_main();
	return 0;
}
void *collect_rates(void *data)
{
	int total0 = 0;
	int idle0 = 0;
	int index = 0;
	UPDATE_LABELS *ulables;
	while (1)
	{
		sleep(1);
		ulables = g_new0(UPDATE_LABELS, 1);
		ulables->cpurate = 0.0;
		ulables->memrate = 0.0;
		ulables->swaprate = 0.0;
		if ((-1 == get_cpu_rate(&total0, &idle0, &(ulables->cpurate))))
			ulables->cpurate = 0.0;
		if ((-1 == get_mem_rate(&(ulables->memrate), &(ulables->swaprate))))
			ulables->memrate = 0.0;
		cpu_rates[index] = ulables->cpurate;
		mem_rates[index++] = ulables->memrate;
		time_t now_time = time(&(ulables->nowtime));
		gdk_threads_add_timeout(0, update_lables, ulables);
	}
}
void new_process(GtkWidget *widget, gpointer data)
{
	printf("new_process is called\n");

	return;
}
void search_pid(GtkWidget *widget, gpointer data)
{
	printf("search_pid is called\n");

	return;
}
void confirm_shutdown(GtkWidget *widget, gpointer data)
{
	printf("confirm_shutdown is called\n");
	//system("poweroff\n");
	return;
}
void confirm_kill(GtkWidget *widget, gpointer data)
{
	printf("confirm_kill is called\n");

	return;
}

gboolean update_lables(gpointer pdata)
{
	char buf[50];
	UPDATE_LABELS *ulables = pdata;
	sprintf(buf, "%.2lf%%", ulables->cpurate);
	gtk_label_set_text((GtkLabel *)label_cpu_rate, buf);
	sprintf(buf, "%.2lf%%", ulables->memrate);
	gtk_label_set_text((GtkLabel *)label_mem_rate, buf);
	//sprintf(buf, "%.2lf%%", ulables->swaprate);
	//gtk_label_set_text((GtkLabel *)label_swap_rate, buf);
	struct tm *now_tm = localtime(&(ulables->nowtime));
	sprintf(buf, "%d:%d:%d", now_tm->tm_hour, now_tm->tm_min, now_tm->tm_sec);
	gtk_label_set_text((GtkLabel *)label_current_time, buf);
	time_t runtime = ulables->nowtime - boot_time;
	struct tm *run_tm = localtime(&runtime);
	sprintf(buf, "%d:%d:%d", run_tm->tm_hour, run_tm->tm_min, run_tm->tm_sec);
	gtk_label_set_text((GtkLabel *)label_run_time, buf);
	g_free(ulables);
	return FALSE;
}