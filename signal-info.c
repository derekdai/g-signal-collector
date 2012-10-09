#include "signal-info.h"
#include <string.h>

void signal_info_dump(SignalInfo * self)
{
	gchar * spaces = "";
	if(self->depth) {
		gint n_spaces = self->depth << 1;
		spaces = g_alloca(n_spaces + 1);
		memset(spaces, ' ', n_spaces);
		spaces[n_spaces] = '\0';
	}
	
	g_printerr("%p %13f %9f %p %s%s::%s\n",
		self->thread,
		self->timestamp,
		self->elapsed,
		self->instance,
		spaces,
		g_type_name(self->instance_type),
		g_signal_name(self->signal_id));
}

