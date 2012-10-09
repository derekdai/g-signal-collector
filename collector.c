#define _GNU_SOURCE
#include "signal-info.h"
#include "signal-info-pool.h"
#include <dlfcn.h>

static SignalInfoPool * signal_info_pool_get();

static SignalInfo * next_signal_info();

static void init() __attribute__((constructor));

static void deinit() __attribute__((destructor));

static void (* orig_g_signal_emit_by_name)(
	gpointer instance,
	const gchar *detailed_signal,
	...);

static void (* orig_g_signal_emitv)(
	const GValue *instance_and_params,
	guint signal_id,
	GQuark detail,
	GValue *return_value);

static void (* orig_g_signal_emit_valist)(
	gpointer instance,
	guint signal_id,
	GQuark detail,
	va_list var_args);

static GTimer * timer = NULL;

static GList * pool_list = NULL;

static __thread SignalInfoPool * pool = NULL;

static __thread SignalInfo * info = NULL;

static __thread gint depth = 0;

G_LOCK_DEFINE_STATIC(pool_list_lock);

#define START_COLLECT() \
	SignalInfo * signal_info = next_signal_info(); \
	gdouble profiling_start = g_timer_elapsed(timer, NULL); \
	depth ++;

#define END_COLLECT(inst, sid, sdetail, inst_type) \
	depth --; \
	gdouble profiling_end = g_timer_elapsed(timer, NULL); \
	signal_info->timestamp = profiling_start; \
	signal_info->instance = inst; \
	signal_info->signal_id = sid; \
	signal_info->instance_type = inst_type; \
	signal_info->elapsed = profiling_end - profiling_start; \
	signal_info->thread = g_thread_self(); \
	signal_info->depth = depth; \
	signal_info_dump(signal_info);

void g_signal_emit(
	gpointer instance,
	guint signal_id,
	GQuark detail,
	...)
{
	g_return_if_fail(instance);
	
	va_list var_args;
	va_start(var_args, detail);
	
	START_COLLECT();
	orig_g_signal_emit_valist(instance, signal_id, detail, var_args);
	END_COLLECT(instance, signal_id, detail, G_OBJECT_TYPE(instance));
	
	va_end(var_args);
}

void g_signal_emit_valist(
	gpointer instance,
	guint signal_id,
	GQuark detail,
	va_list var_args)
{
	g_return_if_fail(instance);
	
	START_COLLECT();
	orig_g_signal_emit_valist(instance, signal_id, detail, var_args);
	END_COLLECT(instance, signal_id, detail, G_OBJECT_TYPE(instance));
}

void g_signal_emit_by_name(
	gpointer instance,
	const gchar *detailed_signal,
	...)
{
	g_return_if_fail(instance);
	g_return_if_fail(detailed_signal && * detailed_signal);
	
	va_list var_args;
	va_start(var_args, detailed_signal);

	const gchar * detail_str = g_strrstr(detailed_signal, "::");
	GQuark detail = detail_str
		? g_quark_try_string(detail_str + 2)
		: 0;
	GType instance_type = G_OBJECT_TYPE(instance);
	guint signal_id = g_signal_lookup(detailed_signal, instance_type);

	START_COLLECT();
	orig_g_signal_emit_valist(instance, signal_id, detail, var_args);
	END_COLLECT(instance, signal_id, detail, instance_type);
	
	va_end(var_args);
}

void g_signal_emitv(
	const GValue *instance_and_params,
	guint signal_id,
	GQuark detail,
	GValue *return_value)
{
	g_return_if_fail(instance_and_params);

	gpointer instance = g_value_get_pointer(instance_and_params);
	GType instance_type = G_VALUE_TYPE(instance_and_params);
	
	START_COLLECT();
	orig_g_signal_emitv(instance_and_params, signal_id, detail, return_value);
	END_COLLECT(instance, signal_id, detail, instance_type);
}

static SignalInfoPool * signal_info_pool_get()
{
	if(! pool) {
		pool = signal_info_pool_new(NULL);
		
		// add pool to cross-thread pool list
		G_LOCK(pool_list_lock);
		pool_list = g_list_prepend(pool_list, pool);
		G_UNLOCK(pool_list_lock);
	}

	if(signal_info_pool_is_full(pool)) {
		pool = signal_info_pool_new(pool);
	}
	
	return pool;
}

static SignalInfo * next_signal_info()
{
	SignalInfoPool * pool = signal_info_pool_get();
	
	return & pool->infos[pool->count ++];
}

static void init()
{
	g_message("Signal intercepter initializing");

	orig_g_signal_emit_by_name = dlsym(RTLD_NEXT, "g_signal_emit_by_name");
	orig_g_signal_emitv = dlsym(RTLD_NEXT, "g_signal_emitv");
	orig_g_signal_emit_valist = dlsym(RTLD_NEXT, "g_signal_emit_valist");
	
	timer = g_timer_new();
	g_timer_start(timer);
}

static void deinit()
{
	g_message("Signal intercepter deinitializing");
	
	g_timer_stop(timer);
	g_timer_destroy(timer);
	timer = NULL;
}

