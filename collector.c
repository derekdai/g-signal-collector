#define _GNU_SOURCE
#include "signal-info.h"
#include "signal-info-pool.h"
#include <dlfcn.h>
#include <gio/gio.h>
#include <stdlib.h>

#define ENV_GSC_OUTPUT_FILE "GSC_OUTPUT_FILE"

#define ENV_GSC_OUTPUT_FORMAT "GSC_OUTPUT_FORMAT"

typedef struct _DumpContext DumpContext;

struct _DumpContext
{
	SignalInfoPoolDumper * dumper;
	
	GOutputStream * os;
	
	gboolean result;
	
	GError ** error;
};

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

static const char * output_format = NULL;

static SignalInfoPoolDumper * dumper = NULL;

static __thread SignalInfoPool * pool = NULL;

static __thread gint depth = 0;

static GList * pool_heads = NULL;

G_LOCK_DEFINE_STATIC(pool_heads_lock);

#define START_COLLECT(inst, sid, sdetail, inst_type) \
	SignalInfo * signal_info = next_signal_info(); \
	gdouble profiling_start = g_timer_elapsed(timer, NULL); \
	signal_info->timestamp = profiling_start; \
	signal_info->instance = inst; \
	signal_info->signal_id = sid; \
	signal_info->instance_type = inst_type; \
	signal_info->thread = g_thread_self(); \
	signal_info->depth = depth; \
	depth ++;

#define END_COLLECT() \
	depth --; \
	gdouble profiling_end = g_timer_elapsed(timer, NULL); \
	signal_info->elapsed = profiling_end - profiling_start; \

void g_signal_emit(
	gpointer instance,
	guint signal_id,
	GQuark detail,
	...)
{
	g_return_if_fail(instance);
	
	va_list var_args;
	va_start(var_args, detail);
	
	START_COLLECT(instance, signal_id, detail, G_OBJECT_TYPE(instance));
	orig_g_signal_emit_valist(instance, signal_id, detail, var_args);
	END_COLLECT();
	
	va_end(var_args);
}

void g_signal_emit_valist(
	gpointer instance,
	guint signal_id,
	GQuark detail,
	va_list var_args)
{
	g_return_if_fail(instance);
	
	START_COLLECT(instance, signal_id, detail, G_OBJECT_TYPE(instance));
	orig_g_signal_emit_valist(instance, signal_id, detail, var_args);
	END_COLLECT();
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

	START_COLLECT(instance, signal_id, detail, instance_type);
	orig_g_signal_emit_valist(instance, signal_id, detail, var_args);
	END_COLLECT();
	
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
	
	START_COLLECT(instance, signal_id, detail, instance_type);
	orig_g_signal_emitv(instance_and_params, signal_id, detail, return_value);
	END_COLLECT();
}

static SignalInfo * next_signal_info()
{
	if(G_UNLIKELY(! pool)) {
		pool = signal_info_pool_new(NULL);
		
		// add pool to cross-thread pool list
		G_LOCK(pool_heads_lock);
		pool_heads = g_list_prepend(pool_heads, pool);
		G_UNLOCK(pool_heads_lock);
	}
	else if(G_UNLIKELY(signal_info_pool_is_full(pool))) {
		pool = signal_info_pool_new(pool);
	}

	return signal_info_pool_next_info(pool);
}

static void dump_info_pool(SignalInfoPool * pool, DumpContext * context)
{
	if(! context->result) {
		return;
	}
	
	context->result = signal_info_pool_dump(
		pool,
		context->dumper,
		context->os,
		context->error);
}

static void dump_info_pools()
{
	gint count = 0;
	char filename[256];
	const char * prog_name = g_get_prgname();
	GFile * file;
	
	while(TRUE) {
		g_snprintf(filename, sizeof(filename), "%s.%d.%s",
			prog_name,
			count,
			output_format);
		file = g_file_new_for_path(filename);
		if(! g_file_query_exists(file, NULL)) {
			break;
		}
		
		g_object_unref(file);
		count ++;
	}
	
	GError * error = NULL;
	GFileOutputStream * os = g_file_create(
		file,
		G_FILE_CREATE_NONE,
		NULL,
		& error);
	if(! os) {
		g_warning("Failed to create output file: %s", filename);
		return;
	}

	DumpContext context = {
		.dumper = signal_info_pool_csv_dumper_new(),
		.result = TRUE,
		.error = & error,
		.os = (GOutputStream *) os,
	};
	g_list_foreach(pool_heads, (GFunc) dump_info_pool, & context);
	if(! error) {
		signal_info_pool_dumper_finish(
			context.dumper,
			os,
			& error);
	}
	if(error) {
		g_warning("%s", error->message);
		g_error_free(error);
		error = NULL;
	}
	
	g_output_stream_close((GOutputStream *) os, NULL, NULL);
	g_object_unref(os);
}

static void parse_params()
{
	output_format = g_getenv(ENV_GSC_OUTPUT_FORMAT);
	if(! output_format) {
		output_format = "gcs";
	}
	
	if(! g_strcmp0("csv", output_format)) {
		dumper = signal_info_pool_csv_dumper_new();
	}
	else if(! g_strcmp0("gsc", output_format)) {
		dumper = signal_info_pool_raw_dumper_new();
	}
	else {
		g_printerr("Unkonwn output format %s. Supported formats are csv, gsc.\n",
			output_format);
		exit(1);
	}
}

static void init()
{
	g_printerr("Signal collector initializing\n");

	parse_params();

	orig_g_signal_emit_by_name = dlsym(RTLD_NEXT, "g_signal_emit_by_name");
	orig_g_signal_emitv = dlsym(RTLD_NEXT, "g_signal_emitv");
	orig_g_signal_emit_valist = dlsym(RTLD_NEXT, "g_signal_emit_valist");

	timer = g_timer_new();
	g_timer_start(timer);
}

static void deinit()
{
	g_printerr("Signal collector deinitializing\n");
	
	g_timer_stop(timer);
	g_timer_destroy(timer);
	timer = NULL;
	
	dump_info_pools();
	
	signal_info_pool_dumper_free(dumper);
	dumper = NULL;
	
	g_list_foreach(pool_heads, (GFunc) signal_info_pool_free, NULL);
	pool_heads = NULL;
}

