#include "signal-info-pool.h"
#include "signal-info-pool-dumper.h"

typedef struct _SignalInfoRawDumper SignalInfoPoolRawDumper;

typedef struct _SignalInfoCsvDumper SignalInfoPoolCsvDumper;

struct _SignalInfoPoolDumper
{
	const SignalInfoPoolDumperFuncs * funcs;
};

struct _SignalInfoRawDumper
{
	SignalInfoPoolDumper base;
};

struct _SignalInfoCsvDumper
{
	SignalInfoPoolDumper base;
	
	GString * buf;
};

static void raw_dumper_init(SignalInfoPoolDumper * dumper);

static gboolean raw_dumper_dump(
	SignalInfoPoolDumper * dumper,
	SignalInfoPool * pool,
	GOutputStream * os,
	GError ** error);

static gboolean raw_dumper_finish(
	SignalInfoPoolDumper * dumper,
	GOutputStream * os,
	GError ** error);

static void raw_dumper_destroy(SignalInfoPoolDumper * dumper);

static void csv_dumper_init(SignalInfoPoolDumper * dumper);

static gboolean csv_dumper_dump(
	SignalInfoPoolDumper * dumper,
	SignalInfoPool * pool,
	GOutputStream * os,
	GError ** error);

static gboolean csv_dumper_finish(
	SignalInfoPoolDumper * dumper,
	GOutputStream * os,
	GError ** error);

static void csv_dumper_destroy(SignalInfoPoolDumper * dumper);

static const SignalInfoPoolDumperFuncs raw_dumper_funcs = {
	.init = raw_dumper_init,
	.dump = raw_dumper_dump,
	.finish = raw_dumper_finish,
	.destroy = raw_dumper_destroy
};

static const SignalInfoPoolDumperFuncs csv_dumper_funcs = {
	.init = csv_dumper_init,
	.dump = csv_dumper_dump,
	.finish = csv_dumper_finish,
	.destroy = csv_dumper_destroy
};

SignalInfoPoolDumper * signal_info_pool_dumper_new(
	gsize instance_size,
	const SignalInfoPoolDumperFuncs * funcs)
{
	g_return_val_if_fail(sizeof(SignalInfoPoolDumper) <= instance_size, NULL);
	g_return_val_if_fail(funcs && funcs->dump, NULL);
	
	SignalInfoPoolDumper * self = g_malloc(instance_size);
	self->funcs = funcs;
	if(self->funcs->init) {
		self->funcs->init(self);
	}
	
	return self;
}

SignalInfoPoolDumper * signal_info_pool_raw_dumper_new()
{
	return signal_info_pool_dumper_new(
		sizeof(SignalInfoPoolCsvDumper),
		& raw_dumper_funcs);
}

SignalInfoPoolDumper * signal_info_pool_csv_dumper_new()
{
	return signal_info_pool_dumper_new(
		sizeof(SignalInfoPoolRawDumper),
		& csv_dumper_funcs);
}

gboolean signal_info_pool_dumper_dump(
	SignalInfoPoolDumper * self,
	SignalInfoPool * pool,
	GOutputStream * os,
	GError ** error)
{
	g_return_val_if_fail(self && self->funcs->dump, FALSE);
	g_return_val_if_fail(os, FALSE);
	g_return_val_if_fail(! error || ! * error, FALSE);
	
	return self->funcs->dump(self, pool, os, error);
}

gboolean signal_info_pool_dumper_finish(
	SignalInfoPoolDumper * self,
	GOutputStream * os,
	GError ** error)
{
	g_return_val_if_fail(self, FALSE);
	g_return_val_if_fail(os, FALSE);
	g_return_val_if_fail(! error || ! * error, FALSE);
	
	if(! self->funcs->finish) {
		return TRUE;
	}
	
	return self->funcs->finish(self, os, error);
}

void signal_info_pool_dumper_free(SignalInfoPoolDumper * self)
{
	g_return_if_fail(self);
	
	if(self->funcs->destroy) {
		self->funcs->destroy(self);
	}
	
	g_free(self);
}

static void raw_dumper_init(SignalInfoPoolDumper * dumper)
{
}

static gboolean raw_dumper_dump(
	SignalInfoPoolDumper * dumper,
	SignalInfoPool * pool,
	GOutputStream * os,
	GError ** error)
{
	return TRUE;
}

static gboolean raw_dumper_finish(
	SignalInfoPoolDumper * dumper,
	GOutputStream * os,
	GError ** error)
{
	return TRUE;
}

static void raw_dumper_destroy(SignalInfoPoolDumper * dumper)
{
}

static void csv_dumper_init(SignalInfoPoolDumper * dumper)
{
	SignalInfoPoolCsvDumper * self = (SignalInfoPoolCsvDumper *) dumper;
	self->buf = g_string_new("");
}

static gboolean csv_dumper_dump(
	SignalInfoPoolDumper * dumper,
	SignalInfoPool * pool,
	GOutputStream * os,
	GError ** error)
{
	SignalInfoPoolCsvDumper * self = (SignalInfoPoolCsvDumper *) dumper;
	GString * buf = self->buf;
	gint n_infos;
	const SignalInfo * info = signal_info_pool_get_infos(pool, & n_infos);
	for(-- n_infos; n_infos >= 0; n_infos --) {
		g_string_append_printf(
			buf,
			"%d\t%p\t%13f\t%9f\t%p\t",
			info->id,
			info->thread,
			info->timestamp,
			info->elapsed,
			info->instance);
		gint i = info->depth - 1;
		for(; i >= 0; i --) {
			g_string_append(buf, ">");
		}
		g_string_append_printf(
			buf,
			"%s::%s",
			g_type_name(info->instance_type),
			g_signal_name(info->signal_id));
		g_string_append_c(buf, '\n');
		info ++;
		
		if((4 * 1024) > buf->len) {
			continue;
		}
		
		GError * tmp_error = NULL;
		g_output_stream_write(os, buf->str, buf->len, NULL, & tmp_error);
		g_string_truncate(buf, 0);
		if(tmp_error) {
			g_propagate_error(error, tmp_error);
			return FALSE;
		}
	}
	
	return TRUE;
}

static gboolean csv_dumper_finish(
	SignalInfoPoolDumper * dumper,
	GOutputStream * os,
	GError ** error)
{
	SignalInfoPoolCsvDumper * self = (SignalInfoPoolCsvDumper *) dumper;
	GString * buf = self->buf;
	if(! buf->len) {
		return TRUE;
	}
	
	gssize n_bytes = g_output_stream_write(
		os,
		buf->str, buf->len,
		NULL,
		error);
	g_string_truncate(buf, 0);

	return -1 != n_bytes;
}

static void csv_dumper_destroy(SignalInfoPoolDumper * dumper)
{
	SignalInfoPoolCsvDumper * self = (SignalInfoPoolCsvDumper *) dumper;
	g_string_free(self->buf, TRUE);
}
