#ifndef __SIGNAL_INFO_H__
#define __SIGNAL_INFO_H__

#include <glib-object.h>

G_BEGIN_DECLS

typedef struct _SignalInfo SignalInfo;

struct _SignalInfo
{
	gint id;
	
	gpointer thread;
	
	gpointer instance;
	
	GType instance_type;
	
	guint signal_id;

	GQuark detail;
	
	gint depth;

	gdouble timestamp;

	gdouble elapsed;
};

gint signal_info_next_id();

void signal_info_dump(SignalInfo * self);

G_END_DECLS

#endif /* __SIGNAL_INFO_H__ */
