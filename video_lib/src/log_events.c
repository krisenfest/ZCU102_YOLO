/******************************************************************************
 * (c) Copyright 2012-2017 Xilinx, Inc. All rights reserved.
 *
 * This file contains confidential and proprietary information of Xilinx, Inc.
 * and is protected under U.S. and international copyright and other
 * intellectual property laws.
 *
 * DISCLAIMER
 * This disclaimer is not a license and does not grant any rights to the
 * materials distributed herewith. Except as otherwise provided in a valid
 * license issued to you by Xilinx, and to the maximum extent permitted by
 * applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
 * FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
 * MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
 * and (2) Xilinx shall not be liable (whether in contract or tort, including
 * negligence, or under any other theory of liability) for any loss or damage
 * of any kind or nature related to, arising under or in connection with these
 * materials, including for any direct, or any indirect, special, incidental,
 * or consequential loss or damage (including loss of data, profits, goodwill,
 * or any type of loss or damage suffered as a result of any action brought by
 * a third party) even if such damage or loss was reasonably foreseeable or
 * Xilinx had been advised of the possibility of the same.
 *
 * CRITICAL APPLICATIONS
 * Xilinx products are not designed or intended to be fail-safe, or for use in
 * any application requiring fail-safe performance, such as life-support or
 * safety devices or systems, Class III medical devices, nuclear facilities,
 * applications related to the deployment of airbags, or any other applications
 * that could lead to death, personal injury, or severe property or
 * environmental damage (individually and collectively, "Critical
 * Applications"). Customer assumes the sole risk and liability of any use of
 * Xilinx products in Critical Applications, subject only to applicable laws
 * and regulations governing limitations on product liability.
 *
 * THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
 * AT ALL TIMES.
 *******************************************************************************/

#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <glib.h>

#include "common.h"
#include "log_events.h"
#include "video_int.h"

#define SAMPLE_WINDOW	5

struct levents_counter {
	size_t sampled_val[SAMPLE_WINDOW];
	size_t counter_val;
	size_t cur_sample;
	char *nm;
	uint8_t flags;
	uint8_t valid_samples;
};
#define LEVENTS_COUNTER_FLAG_ACTIVE	BIT(0)

static GSList *levents_active_counter;
static pthread_t levents_thread;

static int levents_counter_is_active(struct levents_counter *counter)
{
	return !!(counter->flags & LEVENTS_COUNTER_FLAG_ACTIVE);
}

struct levents_counter *levents_counter_create(const char *name)
{
	struct levents_counter *counter;

	counter = calloc(1, sizeof(*counter));
	if (!counter) {
		return NULL;
	}

	ASSERT2(SAMPLE_WINDOW <= sizeof(counter->valid_samples) * 8, "invalid sample window\n");

	counter->nm = strdup(name);
	if (!counter->nm) {
		vlib_warn("%s\n", strerror(errno));
	}

	return counter;
}

void levents_counter_destroy(struct levents_counter *counter)
{
	ASSERT2(counter, "invalid counter\n");

	if (levents_counter_is_active(counter)) {
		levents_counter_stop(counter);
	}

	free(counter->nm);
	free(counter);
}

static void levents_counter_sample(struct levents_counter *counter)
{
	ASSERT2(counter, "invalid counter\n");

	counter->sampled_val[counter->cur_sample] = counter->counter_val;
	counter->cur_sample++;
	counter->cur_sample %= SAMPLE_WINDOW;
	counter->counter_val = 0;
	counter->valid_samples <<= 1;
	counter->valid_samples |= 1;
}

static void *levents_event_thread(void *ptr)
{
	while (1) {
		vlib_dbg("-------------\n");

		for (GSList *e = levents_active_counter; e; e = g_slist_next(e)) {
			struct levents_counter *c = e->data;

			levents_counter_sample(c);

			vlib_dbg("%s :: %.2f \n", levents_counter_get_name(c),
				 levents_counter_get_value(c));
		}

		sleep(1);
	}

	return NULL;
}

void levents_counter_start(struct levents_counter *counter)
{
	ASSERT2(counter, "invalid counter\n");

	/* reset counter values */
	counter->cur_sample = 0;
	counter->counter_val = 0;
	counter->valid_samples = 0;
	for (size_t i = 0; i < SAMPLE_WINDOW; i++) {
		counter->sampled_val[i] = 0;
	}

	if (!levents_active_counter) {
		/* start event thread */
		int ret = pthread_create(&levents_thread, NULL,
				levents_event_thread, NULL);
		ASSERT2(ret >= 0, "failed to create event thread\n");
	}

	levents_active_counter = g_slist_prepend(levents_active_counter, counter);

	counter->flags |= LEVENTS_COUNTER_FLAG_ACTIVE;
}

void levents_counter_stop(struct levents_counter *counter)
{
	ASSERT2(counter, "invalid counter\n");

	levents_active_counter = g_slist_remove(levents_active_counter, counter);
	counter->flags &= ~LEVENTS_COUNTER_FLAG_ACTIVE;

	if (!levents_active_counter) {
		pthread_cancel(levents_thread);
		int ret = pthread_join(levents_thread, NULL);
		ASSERT2(ret >= 0, "failed to terminate event thread\n");
	}
}

void levents_capture_event(struct levents_counter *counter)
{
	ASSERT2(counter, "invalid counter\n");

	counter->counter_val++;
}

float levents_counter_get_value(struct levents_counter *counter)
{
	ASSERT2(counter, "invalid counter\n");

	size_t i, ret = 0;

	for (i = 0; i < SAMPLE_WINDOW; i++) {
		if (!(counter->valid_samples & (1 << i))) {
			break;
		}
		ret += counter->sampled_val[i];
	}

	return (float)ret / i;
}

const char *levents_counter_get_name(struct levents_counter *counter)
{
	ASSERT2(counter, "invalid counter\n");

	return counter->nm;
}
