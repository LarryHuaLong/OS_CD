//Make sure gtk3-devel is installed (in Fedora #dnf install gtk3-devel)
//In Ubuntu: $ sudo apt install libgtk-3-dev
//To compile: 
//gcc draw.c `pkg-config --cflags gtk+-3.0 --libs gtk+-3.0` -o draw
#include <gtk/gtk.h>
//------------------------------------------------------------------


 cairo_t *cpu_cr;
 GtkWidget *window,*drawing_area1,*drawing_area2,*box;
gboolean draw_callback1 (GtkWidget *widget, cairo_t *cr, gpointer data)
{
	cpu_cr = cr;
	return FALSE;
}
gboolean draw_plot (gpointer data)
{
printf("draw_plot is called,widget = %p,cr = %p,data = %p\n",drawing_area1,cpu_cr,data);
guint width,height;
GdkRGBA color;
width=gtk_widget_get_allocated_width(drawing_area1);
height=gtk_widget_get_allocated_height(drawing_area1);



double cpu_rates[120] = {0};
cpu_rates[119] = 0;
cpu_rates[118] = 60;
cpu_rates[117] = 34;
cpu_rates[116] = 23;
cpu_rates[115] = 0;
cpu_rates[114] = 60;
cpu_rates[113] = 0;
cairo_move_to(cpu_cr, width -1, 99*height/100);
for (int i = 0; i < (120 - 1); i++)
	{
		cairo_line_to(cpu_cr, (120 - i )*width/140,(100 - cpu_rates[i] - 1)*height/100);
	}

color.red = 0.0;
color.green = 1.0;
color.blue = 0.0;
color.alpha = 1.0;
gdk_cairo_set_source_rgba(cpu_cr,&color);

cairo_stroke(cpu_cr);
return TRUE;
}
//-------------------------------------------------------------------
gint main(int argc,char *argv[])
{


gtk_init(&argc,&argv);
window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
g_signal_connect(window,"destroy",G_CALLBACK(gtk_main_quit),NULL);

box = gtk_box_new(GTK_ORIENTATION_VERTICAL,2);
gtk_container_add (GTK_CONTAINER (window),box);

drawing_area1=gdk_window_new();
gtk_container_add (GTK_CONTAINER (box),drawing_area1);
gtk_widget_set_size_request(drawing_area1,200,100);
printf("drawing_area = %p\n",drawing_area1);
g_signal_connect(G_OBJECT(drawing_area1),"draw",
G_CALLBACK(draw_callback1),NULL);

gdk_threads_add_timeout (1000,
                         draw_plot,
                         NULL);


gtk_widget_show_all(window);
gtk_main();
return 0;
}
//----------------------------------------------------------------
