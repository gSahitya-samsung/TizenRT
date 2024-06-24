/****************************************************************************
 *
 * Copyright 2024 Samsung Electronics All Rights Reserved.
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

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <tinyara/config.h>
#include <assert.h>
#include <tinyara/pm/pm.h>
#include <tinyara/clock.h>
#include <tinyara/irq.h>
#include <tinyara/arch.h>

#include "pm.h"
#ifdef CONFIG_PM_METRICS
#include "pm_metrics.h"
#endif

#ifdef CONFIG_PM

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: pm_idle
 *
 * Description:
 *   This function is called by IDLE thread to make board sleep. This function
 *   also allow to set wake up timer & handler and do all the PM pre processing
 *   required before going to sleep.
 *
 * Input Parameters:
 *   None
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

void pm_idle(void)
{
	irqstate_t flags;
	enum pm_state_e newstate;
#ifdef CONFIG_PM_TIMEDWAKEUP
	clock_t delay;
#endif
	/* Decide, which power saving level can be obtained */
	flags = enter_critical_section();
	newstate = pm_checkstate();
	/* Perform state-dependent logic here */
	pmvdbg("newstate= %d\n", newstate);
	/* Then force the global state change */
	if (pm_changestate(newstate) < 0) {
		/* The new state change failed */
		pmdbg("State change failed! newstate = %d\n", newstate);
		leave_critical_section(flags);
		/* core power efficiency during idle time */
		up_core_low_power();
		return;
	}
	/* If current state is not good to go sleep then do core power saving*/
	if (g_pmglobals.state != PM_SLEEP) {
#ifdef CONFIG_PM_METRICS
		pm_idle_metrics_update();
#endif
		leave_critical_section(flags);
		/* core power efficiency during idle time */
		up_core_low_power();
		return;
	}
#ifdef CONFIG_PM_TIMEDWAKEUP
	/* set wakeup timer */
	delay = wd_getwakeupdelay();
	if (delay >= CONFIG_PM_MIN_SLEEP_TIME) {
		pmvdbg("Setting timer and Board will wake up after %d millisecond\n", delay);
		up_set_pm_timer(TICK2USEC(delay));
	} else {
		pmvdbg("Min Sleep Time should be %d\n", CONFIG_PM_MIN_SLEEP_TIME);
	}
#endif
	up_pm_board_sleep(pm_wakehandler);
	leave_critical_section(flags);
}

#endif /* CONFIG_PM */
