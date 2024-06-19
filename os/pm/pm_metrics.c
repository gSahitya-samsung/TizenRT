/****************************************************************************
 *
 * Copyright 2016 Samsung Electronics All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 *
 ****************************************************************************/

#include <tinyara/config.h>
#include <time.h>
#include <queue.h>
#include <debug.h>
#include <errno.h>
#include "pm_metrics.h"
#include "pm.h"

pm_metric_global_t *g_pm_metrics_globals;

static void pm_print_metrics(void)
{
	double suspend_duration;
	double resume_duration;
	double suspend_percent;
	double idle_counts_percent;
	double total_time;
	int index;
	static const char *state_name[] = {"PM_NORMAL", "PM_IDLE", "PM_STANDBY", "PM_SLEEP", NULL};
	total_time = 0.0;
	for (index = 0; index < PM_COUNT; index++) {
		total_time += (double)TICK2MSEC(g_pm_metrics_globals->sticks[index]);
	}
	pmdbg("\n\n");
	pmdbg("          DOMAIN                |      SUSPEND TIME      | IDLE SUSPEND COUNTS \n");
	pmdbg("--------------------------------|------------------------|---------------------\n");
	for (index = 0; (index < CONFIG_PM_NDOMAINS) && pm_domain_map[index]; index++) {
		suspend_duration = (double)TICK2MSEC(g_pm_metrics_globals->suspend_ticks[index]);
		resume_duration = (double)TICK2MSEC(g_pm_metrics_globals->resume_ticks[index]);
		suspend_percent = (suspend_duration * 100.0) / (suspend_duration + resume_duration);
		idle_counts_percent = (double)(g_pm_metrics_globals->idle_suspend_counts[index]) * 100.0 / (double)(g_pm_metrics_globals->total_suspend_counts);
		pmdbg(" %30s | %10dms (%6.2f%%) | %9d (%6.2f%%) \n", pm_domain_map[index], (int)suspend_duration, suspend_percent,\
		g_pm_metrics_globals->idle_suspend_counts[index], idle_counts_percent);
	}
	pmdbg("\n\n");
	pmdbg("    STATE    |          TIME          \n");
	pmdbg("-------------|------------------------\n");
	for (index = 0; index < PM_COUNT; index++) {
		pmdbg(" %11s | %10dms (%6.2f%%) \n", state_name[index], TICK2MSEC(g_pm_metrics_globals->sticks[index]),\
		(double)TICK2MSEC(g_pm_metrics_globals->sticks[index]) * 100.0 / total_time);
	}
}

time_t time_diff(time_t time1, time_t time2)
{
	if (time1 > time2) {
		return time1 - time2;
	} else {
		return time2 - time1;
	}

}

void pm_domain_metricinitialize(int domain_id)
{
	if (g_pm_metrics_globals) {
		g_pm_metrics_globals->dtime[domain_id] = clock_systimer();
		g_pm_metrics_globals->suspend_ticks[domain_id] = 0;
		g_pm_metrics_globals->resume_ticks[domain_id] = 0;
	}
}

void pm_suspend_metrics_update(int domain_id)
{
	clock_t now;
	if (g_pm_metrics_globals && (g_pmglobals.suspend_count[domain_id] == 0)) {
		now = clock_systimer();
		g_pm_metrics_globals->resume_ticks[domain_id] += now - g_pm_metrics_globals->dtime[domain_id];
		g_pm_metrics_globals->dtime[domain_id] = now;
	}
}

void pm_resume_metrics_update(int domain_id)
{
	clock_t now;
	if (g_pm_metrics_globals && (g_pmglobals.suspend_count[domain_id] == 1)) {
		now = clock_systimer();
		g_pm_metrics_globals->suspend_ticks[domain_id] += now - g_pm_metrics_globals->dtime[domain_id];
		g_pm_metrics_globals->dtime[domain_id] = now;
	}
}

void pm_idle_metrics_update(void)
{
	int index;
	if (g_pm_metrics_globals && (g_pmglobals.recommended != PM_SLEEP)) {
		g_pm_metrics_globals->total_suspend_counts++;
		for (index = 0; index < CONFIG_PM_NDOMAINS; index++) {
			if (g_pmglobals.suspend_count[index] != 0) {
				g_pm_metrics_globals->idle_suspend_counts[index]++;
			}
		}
	}
}

void pm_changestate_metrics_update(void)
{
	clock_t now;
	if (g_pm_metrics_globals) {
		now = clock_systimer();
		g_pm_metrics_globals->sticks[g_pmglobals.state] += now - g_pm_metrics_globals->stime;
		g_pm_metrics_globals->stime = now;
	}
}

/****************************************************************************
 * Name: pm_metrics
 *
 * Description:
 *   This internal function is called to analyze the PM suspend and sleep behaviour.
 *   It gathers the pm metrics statistics for provided time (in msec) to provide the
 *   domain specific pm suspend information.
 *
 * Input Parameters:
 *   milliseconds - the monitoring duration in milliseconds
 *
 * Returned Value:
 *   OK (0)     - On Success
 *   ERROR (-1) - On Error
 *
 ****************************************************************************/
int pm_metrics(int milliseconds)
{
	clock_t now;
	int index;
	if (g_pm_metrics_globals) {
		pmvdbg("PM Metrics already running\n");
		return OK;
	}
	pm_lock();
	g_pm_metrics_globals = pm_alloc(1, sizeof(pm_metric_global_t));
	if (g_pm_metrics_globals == NULL) {
		set_errno(ENOMEM);
		pmdbg("Unable to initialize pm_metrics, error = %d", get_errno());
		return ERROR;
	}
	now = clock_systimer();
	g_pm_metrics_globals->stime = now;
	for (index = 0; (index < CONFIG_PM_NDOMAINS) && pm_domain_map[index]; index++) {
		pm_domain_metricinitialize(index);
		g_pm_metrics_globals->dtime[index] = now;
	}
	for (index = 0; index < PM_COUNT; index++) {
		g_pm_metrics_globals->sticks[index] = 0;
	}
	pm_sleep(TICK2MSEC(MSEC2TICK(milliseconds) - (clock_systimer() - now)));
	now = clock_systimer();
	for (index = 0; (index < CONFIG_PM_NDOMAINS) && pm_domain_map[index]; index++) {
		if (g_pmglobals.suspend_count[index] == 0) {
			g_pm_metrics_globals->resume_ticks[index] += now - g_pm_metrics_globals->dtime[index];
		} else {
			g_pm_metrics_globals->suspend_ticks[index] += now - g_pm_metrics_globals->dtime[index];
		}
	}
	g_pm_metrics_globals->sticks[g_pmglobals.state] += now - g_pm_metrics_globals->stime;
	pm_print_metrics();
	free(g_pm_metrics_globals);
	g_pm_metrics_globals = NULL;
	pm_unlock();
	return OK;
}
