#include "signal-info-pool.h"
#include "signal-info-pool-dumper.h"

#define DEFAULT_POOL_SIZE (32768)
//#define DEFAULT_POOL_SIZE (1000)

typedef struct _ForeachInfoContext ForeachInfoContext;

typedef struct _DumpPoolContext DumpPoolContext;

struct _SignalInfoPool
{
	gint count;
	
	SignalInfoPool * prev;
	
	SignalInfoPool * next;
	
	SignalInfo infos[DEFAULT_POOL_SIZE];
};

struct _ForeachInfoContext
{
	GFunc func;
	
	gpointer user_data;
};

struct _DumpPoolContext
{
	SignalInfoPoolDumper * dumper;
	
	GOutputStream * os;
	
	gboolean result;
	
	GError ** error;
};

SignalInfoPool * signal_info_pool_new(SignalInfoPool * next)
{
	SignalInfoPool * self = g_new(SignalInfoPool, 1);
	self->count = 0;
	self->next = next;
	self->prev = NULL;
	if(next) {
		next->prev = self;
	}
	
	return self;
}

void signal_info_pool_free(SignalInfoPool * self)
{
	g_return_if_fail(self);
	
	SignalInfoPool * curr = self->next;
	while(curr) {
		SignalInfoPool * next = curr->next;
		g_free(curr);
		curr = next;
	}

	curr = self->prev;
	while(curr) {
		SignalInfoPool * prev = curr->prev;
		g_free(curr);
		curr = prev;
	}
	
	g_free(self);
}

gboolean signal_info_pool_is_full(SignalInfoPool * self)
{
	g_return_val_if_fail(self, FALSE);
	
	return self->count == G_N_ELEMENTS(self->infos);
}

SignalInfo * signal_info_pool_next_info(SignalInfoPool * self)
{
	g_return_val_if_fail(self, NULL);
	g_return_val_if_fail(! signal_info_pool_is_full(self), NULL);
	
	SignalInfo * info = & self->infos[self->count ++];
	info->id = signal_info_next_id();
	
	return info;
}

SignalInfoPool * signal_info_pool_prev(SignalInfoPool * self)
{
	g_return_val_if_fail(self, NULL);
	
	return self->prev;
}

SignalInfoPool * signal_info_pool_next(SignalInfoPool * self)
{
	g_return_val_if_fail(self, NULL);
	
	return self->next;
}

const SignalInfo * signal_info_pool_get_infos(
	SignalInfoPool * self,
	guint * n_infos)
{
	g_return_val_if_fail(self, NULL);
	g_return_val_if_fail(n_infos, NULL);
	
	* n_infos = self->count;
	
	return self->infos;
}

void signal_info_pool_foreach(
	SignalInfoPool * self,
	GFunc func,
	gpointer user_data)
{
	g_return_if_fail(func);
	
	SignalInfoPool * curr;
	for(curr = self; curr; curr = curr->next) {
		func(curr, user_data);
	}
	for(curr = self->prev; curr; curr = curr->prev) {
		func(curr, user_data);
	}
}

static void foreach_info(
	SignalInfoPool * pool,
	ForeachInfoContext * context)
{
	gint i = pool->count - 1;
	SignalInfo * curr_info = pool->infos;
	GFunc func = context->func;
	gpointer user_data = context->user_data;
	for(; i >= 0; i --) {
		func(curr_info ++, user_data);
	}
}

void signal_info_pool_foreach_info(
	SignalInfoPool * self,
	GFunc func,
	gpointer user_data)
{
	ForeachInfoContext context = {
		.func = func,
		.user_data = user_data
	};
	
	signal_info_pool_foreach(self, (GFunc) foreach_info, & context);
}

static void dump_pool(SignalInfoPool * pool, DumpPoolContext * context)
{
	if(! context->result) {
		return;
	}
	
	signal_info_pool_dumper_dump(
		context->dumper,
		pool,
		context->os,
		context->error);
}

gboolean signal_info_pool_dump(
	SignalInfoPool * self,
	struct _SignalInfoPoolDumper * dumper,
	GOutputStream * os,
	GError ** error)
{
	g_return_val_if_fail(self, FALSE);
	g_return_val_if_fail(dumper, FALSE);
	g_return_val_if_fail(os, FALSE);
	g_return_val_if_fail(! error || ! * error, FALSE);
	
	DumpPoolContext context = {
		.dumper = dumper,
		.os = os,
		.error = error,
		.result = TRUE
	};
	signal_info_pool_foreach(self, (GFunc) dump_pool, & context);
	
	return context.result;
}
