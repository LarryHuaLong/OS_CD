#include "lab4.h"

void *collect_cpu_rates(void *data); //线程，收集cpu利用率信息
double cpu_rates[120] = {0.0};
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
	label_cpu_speed = GTK_WIDGET(gtk_builder_get_object(builder, "label_cpu_speed"));
	label_cpu_rate = GTK_WIDGET(gtk_builder_get_object(builder, "label_cpu_rate"));
	label_mem_rate = GTK_WIDGET(gtk_builder_get_object(builder, "label_mem_rate"));
	label_current_time = GTK_WIDGET(gtk_builder_get_object(builder, "label_current_time"));

	g_signal_connect(G_OBJECT(window1), "destroy", G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(G_OBJECT(btn_new_process), "clicked", G_CALLBACK(new_process), NULL);
	g_signal_connect(G_OBJECT(btn_search), "clicked", G_CALLBACK(search_pid), NULL);
	g_signal_connect(G_OBJECT(btn_shutdown), "clicked", G_CALLBACK(confirm_shutdown), NULL);
	g_signal_connect(G_OBJECT(btn_endprocess), "clicked", G_CALLBACK(confirm_kill), NULL);

	g_object_unref(G_OBJECT(builder)); //释放GtkBuilder对象
	gtk_widget_show_all(window1);
	g_thread_new("worker", &collect_cpu_rates, NULL); //创建写线程
	gtk_main();
	return 0;
}
void *collect_cpu_rates(void *data)
{
	int total0 = 0;
	int idle0 = 0;
	int index = 0;
	double rate = 0.0;
	char buf[20];
	while (1)
	{
		sleep(1);
		if ((-1 == get_cpu_rate(&total0, &idle0, &rate)))
			rate = 0.0;
		cpu_rates[index++] = rate;
		sprintf(buf, "%.2lf%%", rate);
		gtk_label_set_text((GtkLabel *)label_cpu_rate, buf);
	}
}