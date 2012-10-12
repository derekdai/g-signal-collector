#include <clutter/clutter.h>

#define STAGE_WIDTH (720)
#define STAGE_HEIGHT (480)
#define N_COLS (10)
#define N_ROWS (1)
#define ACTOR_WIDTH (STAGE_WIDTH/N_COLS)
#define ACTOR_HEIGHT (STAGE_HEIGHT/N_ROWS)

gboolean invert_color(ClutterActor * actor)
{
	ClutterColor color;
	clutter_actor_get_background_color(actor, & color);
	color.red = ~ color.red;
	color.green = ~ color.green;
	color.blue = ~ color.blue;
	clutter_actor_set_background_color(actor, & color);
} 

int main(int argc, char * args[])
{
	if(CLUTTER_INIT_SUCCESS != clutter_init(& argc, & args)) {
		g_error("Failed to init clutter");
	}
	
	ClutterActor * stage = clutter_stage_new();
	clutter_actor_set_size(stage, STAGE_WIDTH, STAGE_HEIGHT);
	g_signal_connect(stage, "destroy", G_CALLBACK(clutter_main_quit), NULL);
	
	GRand * rand = g_rand_new();
	
	int i, j;
	for(i = 0; i < N_ROWS; i ++) {
		for(j = 0; j < N_COLS; j ++) {
			ClutterColor color = {
				(guint8) g_rand_int_range(rand, 0, 255),
				(guint8) g_rand_int_range(rand, 0, 255),
				(guint8) g_rand_int_range(rand, 0, 255),
				255, //(guint8) g_rand_int_range(rand, 0, 255)
			};
			ClutterActor * actor = clutter_actor_new();
			clutter_actor_set_background_color(actor, & color);
			clutter_actor_set_position(actor, j * ACTOR_WIDTH, i * ACTOR_HEIGHT);
			clutter_actor_set_size(actor, ACTOR_WIDTH, ACTOR_HEIGHT);
			clutter_actor_set_reactive(actor, TRUE);
			clutter_actor_add_child(stage, actor);
			g_signal_connect(actor, "button-press-event", G_CALLBACK(invert_color), NULL);
			g_signal_connect(actor, "button-release-event", G_CALLBACK(invert_color), NULL);
		}
	}
	
	clutter_actor_show(stage);
	
	clutter_main();
	
	return 0;
}
