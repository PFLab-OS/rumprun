/*-
 * Copyright (c) 2015 Martin Lucina.  All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <hw/types.h>
#include <hw/kernel.h>

#include <arch/x86/cons.h>
#include <arch/x86/hypervisor.h>

#include <bmk-core/printf.h>

static void (*vcons_putc)(int) = vgacons_putc;

void
cons_init(void)
{
	int prefer_serial = 0;
	int hypervisor;

	hypervisor = hypervisor_detect();

	/*
	 * If running under Xen use the serial console.
	 */
	if (hypervisor == HYPERVISOR_XEN)
		prefer_serial = 1;

	prefer_serial = 1;
	if (prefer_serial) {
		serialcons_init(0x3f8, 115200);
		vcons_putc = serialcons_putc;
		cons_puts("Using serial console.\n");
	}
	bmk_printf_init(vcons_putc, NULL);
}

void
cons_putc(int c)
{

	vcons_putc(c);
}

void
cons_puts(const char *s)
{
	int c;

	while ((c = *s++) != 0)
		vcons_putc(c);
}
