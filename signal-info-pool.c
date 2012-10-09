#include "signal-info-pool.h"

SignalInfoPool * signal_info_pool_new(SignalInfoPool * prev)
{
	SignalInfoPool * self = g_new(SignalInfoPool, 1);
	self->count = 0;
	self->prev = prev;
	if(prev) {
		prev->next = self;
	}
	
	return self;
}

void signal_info_pool_free(SignalInfoPool * self)
{
	g_return_if_fail(self);

	if(self->next) {
		self->next->prev = self->prev;
	}
	if(self->prev) {
		self->prev->next = self->next;
	}
	g_free(self);
}

gboolean signal_info_pool_is_full(SignalInfoPool * pool)
{
	g_return_val_if_fail(pool, FALSE);
	
	return pool->count == G_N_ELEMENTS(pool->infos);
}
