#ifndef __SIGNAL_INFO_POOL_H__
#define __SIGNAL_INFO_POOL_H__

#include <glib-object.h>
#include <gio/gio.h>
#include "signal-info.h"
#include "signal-info-pool-dumper.h"

G_BEGIN_DECLS

typedef struct _SignalInfoPool SignalInfoPool;

SignalInfoPool * signal_info_pool_new(SignalInfoPool * next);

void signal_info_pool_free1(SignalInfoPool * self);

void signal_info_pool_free(SignalInfoPool * self);

SignalInfo * signal_info_pool_next_info(SignalInfoPool * self);

gboolean signal_info_pool_is_full(SignalInfoPool * self);

SignalInfoPool * signal_info_pool_prev(SignalInfoPool * self);

SignalInfoPool * signal_info_pool_next(SignalInfoPool * self);

const SignalInfo * signal_info_pool_get_infos(
	SignalInfoPool * self,
	guint * n_infos);
	
gboolean signal_info_pool_dump(
	SignalInfoPool * self,
	struct _SignalInfoPoolDumper * dumper,
	GOutputStream * os,
	GError ** error);

void signal_info_pool_foreach(
	SignalInfoPool * self,
	GFunc func,
	gpointer user_data);
	
void signal_info_pool_foreach_info(
	SignalInfoPool * self,
	GFunc func,
	gpointer user_data);

G_END_DECLS

#endif /* __SIGNAL_INFO_POOL_H__*/

