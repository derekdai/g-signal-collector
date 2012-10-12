#include <gtk/gtk.h>
#include <gdk/gdk.h>

#define STAGE_WIDTH (720)
#define STAGE_HEIGHT (480)
#define N_COLS (10)
#define N_ROWS (1)
#define ACTOR_WIDTH (STAGE_WIDTH/N_COLS)
#define ACTOR_HEIGHT (STAGE_HEIGHT/N_ROWS)

GRand * rand;

gboolean invert_color(GtkWidget * widget)
{
	GdkRGBA color = {
		g_rand_double_range(rand, 0, 1.0),
		g_rand_double_range(rand, 0, 1.0),
		g_rand_double_range(rand, 0, 1.0),
		g_rand_double_range(rand, 0, 1.0)
	};
	gtk_widget_override_background_color(
		widget,
		GTK_STATE_FLAG_NORMAL,
		& color);
} 

int main(int argc, char * args[])
{
	gtk_init(& argc, & args);
	
	GtkWidget * win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_resize((GtkWindow *) win, STAGE_WIDTH, STAGE_HEIGHT);
	g_signal_connect(win, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	
	GtkWidget * fixed = gtk_fixed_new();
	gtk_container_add((GtkContainer *) win, fixed);
	
	rand = g_rand_new();
	
	int i, j;
	for(i = 0; i < N_ROWS; i ++) {
		for(j = 0; j < N_COLS; j ++) {
			GdkRGBA color = {
				g_rand_double_range(rand, 0, 1.0),
				g_rand_double_range(rand, 0, 1.0),
				g_rand_double_range(rand, 0, 1.0),
				g_rand_double_range(rand, 0, 1.0)
			};
			GtkWidget * widget = gtk_drawing_area_new();
			gtk_widget_override_background_color(
				widget,
				GTK_STATE_FLAG_NORMAL,
				& color);
			gtk_widget_set_size_request(widget, ACTOR_WIDTH, ACTOR_HEIGHT);
			gtk_fixed_put((GtkFixed *) fixed, widget, j * ACTOR_WIDTH, i * ACTOR_HEIGHT);
			gtk_widget_add_events(widget, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
			g_signal_connect(widget, "button-press-event", G_CALLBACK(invert_color), NULL);
			g_signal_connect(widget, "button-release-event", G_CALLBACK(invert_color), NULL);
		}
	}
	
	gtk_widget_show_all(win);
	
	gtk_main();
	
	return 0;
}

