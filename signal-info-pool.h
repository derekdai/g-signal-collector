#ifndef __SIGNAL_INFO_POOL_H__
#define __SIGNAL_INFO_POOL_H__

#include <glib-object.h>
#include "signal-info.h"

G_BEGIN_DECLS

typedef struct _SignalInfoPool SignalInfoPool;

struct _SignalInfoPool
{
	gint count;
	
	SignalInfoPool * prev;
	
	SignalInfoPool * next;
	
	SignalInfo infos[128];
};

SignalInfoPool * signal_info_pool_new(SignalInfoPool * prev);

void signal_info_pool_free(SignalInfoPool * self);

gboolean signal_info_pool_is_full(SignalInfoPool * pool);

G_END_DECLS

#endif /* __SIGNAL_INFO_POOL_H__*/

