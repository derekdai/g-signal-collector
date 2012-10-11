#include <clutter/clutter.h>

int main(int argc, char * args[])
{
	if(CLUTTER_INIT_SUCCESS != clutter_init(& argc, & args)) {
		g_error("Failed to init clutter");
	}
	
	clutter_main();
	
	return 0
}
