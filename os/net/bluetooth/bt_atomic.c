/****************************************************************************
 * net/bluetooth/bt_atomic.c
 * Linux like atomic operations
 *
 *   Copyright (C) 2018 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <tinyara/config.h>
#include <tinyara/irq.h>

#include "bt_atomic.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

#ifndef CONFIG_HAVE_INLINE
void bt_atomic_set(FAR bt_atomic_t *ptr, bt_atomic_t value)
{
	*ptr = value;
}
#endif

bt_atomic_t bt_atomic_incr(FAR bt_atomic_t *ptr)
{
	irqstate_t flags;
	bt_atomic_t value;

	flags = enter_critical_section();
	value = *ptr;
	*ptr = value + 1;
	leave_critical_section(flags);

	return value;
}

bt_atomic_t bt_atomic_decr(FAR bt_atomic_t *ptr)
{
	irqstate_t flags;
	bt_atomic_t value;

	flags = enter_critical_section();
	value = *ptr;
	*ptr = value - 1;
	leave_critical_section(flags);

	return value;
}

bt_atomic_t bt_atomic_setbit(FAR bt_atomic_t *ptr, bt_atomic_t bitno)
{
	irqstate_t flags;
	bt_atomic_t value;

	flags = enter_critical_section();
	value = *ptr;
	*ptr = value | (1 << bitno);
	leave_critical_section(flags);

	return value;
}

bt_atomic_t bt_atomic_clrbit(FAR bt_atomic_t *ptr, bt_atomic_t bitno)
{
	irqstate_t flags;
	bt_atomic_t value;

	flags = enter_critical_section();
	value = *ptr;
	*ptr = value & ~(1 << bitno);
	leave_critical_section(flags);

	return value;
}

#ifndef CONFIG_HAVE_INLINE
bt_atomic_t bt_atomic_get(FAR bt_atomic_t *ptr)
{
	return *ptr;
}
#endif

#ifndef CONFIG_HAVE_INLINE
bool bt_atomic_testbit(FAR bt_atomic_t *ptr, bt_atomic_t bitno)
{
	return (*ptr & (1 << bitno)) != 0;
}
#endif

bool bt_atomic_testsetbit(FAR bt_atomic_t *ptr, bt_atomic_t bitno)
{
	irqstate_t flags;
	bt_atomic_t value;

	flags = enter_critical_section();
	value = *ptr;
	*ptr = value | (1 << bitno);
	leave_critical_section(flags);

	return (value & (1 << bitno)) != 0;
}

bool bt_atomic_testclrbit(FAR bt_atomic_t *ptr, bt_atomic_t bitno)
{
	irqstate_t flags;
	bt_atomic_t value;

	flags = enter_critical_section();
	value = *ptr;
	*ptr = value & ~(1 << bitno);
	leave_critical_section(flags);

	return (value & (1 << bitno)) != 0;
}
