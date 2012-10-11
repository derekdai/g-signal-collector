#ifndef __SIGNAL_INFO_POOL_DUMPER_H_
#define __SIGNAL_INFO_POOL_DUMPER_H_

#include <gio/gio.h>

G_BEGIN_DECLS

struct _SignalInfoPool;

typedef struct _SignalInfoPoolDumper SignalInfoPoolDumper;

typedef struct _SignalInfoPoolDumperFuncs SignalInfoPoolDumperFuncs;

typedef void (* SignalInfoPoolDumperInit)(SignalInfoPoolDumper * dumper);

typedef gboolean (* SignalInfoPoolDumperDump)(
	SignalInfoPoolDumper * dumper,
	struct _SignalInfoPool * pool,
	GOutputStream * os,
	GError ** error);

typedef gboolean (* SignalInfoPoolDumperFinish)(
	SignalInfoPoolDumper * dumper,
	GOutputStream * os,
	GError ** error);

typedef void (* SignalInfoPoolDumperDestroy)(SignalInfoPoolDumper * dumper);

struct _SignalInfoPoolDumperFuncs
{
	SignalInfoPoolDumperInit init;
	
	SignalInfoPoolDumperDump dump;
	
	SignalInfoPoolDumperFinish finish;
	
	SignalInfoPoolDumperDestroy destroy;
};

SignalInfoPoolDumper * signal_info_pool_dumper_new(
	gsize instance_size,
	const SignalInfoPoolDumperFuncs * funcs);

SignalInfoPoolDumper * signal_info_pool_raw_dumper_new();

SignalInfoPoolDumper * signal_info_pool_csv_dumper_new();

gboolean signal_info_pool_dumper_dump(
	SignalInfoPoolDumper * self,
	struct _SignalInfoPool * pool,
	GOutputStream * os,
	GError ** error);

void signal_info_pool_dumper_free(SignalInfoPoolDumper * self);

G_END_DECLS

#endif /* __SIGNAL_INFO_POOL_DUMPER_H_ */
