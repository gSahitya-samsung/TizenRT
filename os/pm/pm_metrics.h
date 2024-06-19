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

#ifndef __OS_PM_PM_METRICS_H
#define __OS_PM_PM_METRICS_H

#include <time.h>
#include <queue.h>
#include "pm.h"

struct pm_statechange_s {
	sq_entry_t entry;
	int state;
	time_t timestamp;
};

struct pm_time_in_each_s {
	time_t normal;
	time_t idle;
	time_t standby;
	time_t sleep;
};

typedef struct pm_time_in_each_s pm_time_in_each_t;

struct pm_metric_global_s {
	clock_t suspend_ticks[CONFIG_PM_NDOMAINS];
	clock_t resume_ticks[CONFIG_PM_NDOMAINS];
	clock_t dtime[CONFIG_PM_NDOMAINS];
	uint32_t idle_suspend_counts[CONFIG_PM_NDOMAINS];
	clock_t sticks[PM_COUNT];
	clock_t stime;
	uint32_t total_suspend_counts;
};

typedef struct pm_metric_global_s pm_metric_global_t;

#ifdef CONFIG_PM_METRICS
extern struct pm_global_s g_pmglobals;
extern pm_metric_global_t *g_pm_metrics_globals;

void pm_get_domainmetrics(struct pm_time_in_each_s *mtrics);
void pm_prune_history(sq_queue_t *q);
void pm_domain_metricinitialize(int domain_id);
void pm_suspend_metrics_update(int domain_id);
void pm_resume_metrics_update(int domain_id);
void pm_changestate_metrics_update(void);
void pm_idle_metrics_update(void);
#endif
#endif
