/*-
 * Copyright (c) 2014 Antti Kantee.  All Rights Reserved.
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
#include <hw/multiboot.h>
#include <hw/kernel.h>

#include <bmk-core/core.h>
#include <bmk-core/pgalloc.h>
#include <bmk-core/printf.h>
#include <bmk-core/string.h>

#include <bmk-pcpu/pcpu.h>

static int
parsemem(uint32_t addr, uint32_t len)
{
    /*
     * We assume physical memory chunks layout of MinnowBoard Turbot:
     *
     * start       end         size    pages  type
     * ===========================================
     * ...
     * 0x100000    0x20000000  511.0MB 130816 1
     * ...
     * 0x20100000  0x75a74000  1.3GB   350580 1
     * ...
     */

#define MEMCHUNK1_START 0x100000
#define MEMCHUNK1_END   0x20000000
#define MEMCHUNK1_LEN (MEMCHUNK1_START - MEMCHUNK1_END)
#define MEMCHUNK1_PAGES (MEMCHUNK1_LEN / BMK_PCPU_PAGE_SIZE)

#define MEMCHUNK2_START 0x20100000

    struct multiboot_mmap_entry *mbm;
	unsigned long osend;
	extern char _end[];
    uint32_t off;

    /*
     * Look for our memory.  We assume it's just in one chunk
     * starting at MEMSTART.
     */
    bmk_printf("mem:  start         end   size  pages type\n");
 	for (off = 0; off < len; off += mbm->size + sizeof(mbm->size)) {
        mbm = (void*)(uintptr_t)(addr + off);

        bmk_printf("0x%09llx 0x%09llx ",
                mbm->addr, mbm->addr + mbm->len);
        if (mbm->len >= (1 << 30)) {
            bmk_printf("%4lluGB", mbm->len / (1 << 30));
        } else if (mbm->len >= (1 << 20)) {
            bmk_printf("%4lluMB", mbm->len / (1 << 20));
        } else {
            bmk_printf("%4lluKB", mbm->len / (1 << 10));
        }
        bmk_printf(" %6llu %4u\n",
                mbm->len / BMK_PCPU_PAGE_SIZE, mbm->type);

        if (mbm->addr == MEMCHUNK2_START) {
            bmk_assert(mbm->type == MULTIBOOT_MEMORY_AVAILABLE);
            break;
        }
    }

    if (!(off < len))
        bmk_platform_halt("multiboot memory chunk not found");

	osend = bmk_round_page((unsigned long)_end);
    bmk_assert(osend > MEMCHUNK1_START && osend < MEMCHUNK1_END);

	bmk_pgalloc_loadmem(osend, MEMCHUNK1_START + MEMCHUNK1_LEN + mbm->len);
	bmk_memsize = MEMCHUNK1_START + MEMCHUNK1_LEN + mbm->len - osend;
    // bmk_printf("bmk_memsize: %lu\n", bmk_memsize);

	return 0;
}

char multiboot_cmdline[BMK_MULTIBOOT_CMDLINE_SIZE];

void
multiboot(struct multiboot_info *mbi)
{
	unsigned long cmdlinelen;
	char *cmdline = NULL,
	     *mbm_name;
	struct multiboot_mod_list *mbm;

	bmk_core_init(BMK_THREAD_STACK_PAGE_ORDER);

	/*
	 * First (and for now, only) multiboot module loaded is used as a
	 * preferred source for configuration (currently overloaded to
	 * `cmdline').
	 * TODO: Split concept of `cmdline' and `config'.
	 */
	if (mbi->flags & MULTIBOOT_INFO_MODS &&
			mbi->mods_count >= 1 &&
			mbi->mods_addr != 0) {
		mbm = (struct multiboot_mod_list *)(uintptr_t)mbi->mods_addr;
		mbm_name = (char *)(uintptr_t)mbm[0].cmdline;
		cmdline = (char *)(uintptr_t)mbm[0].mod_start;
		cmdlinelen =
			mbm[0].mod_end - mbm[0].mod_start;
		if (cmdlinelen >= (BMK_MULTIBOOT_CMDLINE_SIZE - 1))
			bmk_platform_halt("command line too long, "
			    "increase BMK_MULTIBOOT_CMDLINE_SIZE");

		bmk_printf("multiboot: Using configuration from %s\n",
			mbm_name ? mbm_name : "(unnamed module)");
		bmk_memcpy(multiboot_cmdline, cmdline, cmdlinelen);
		multiboot_cmdline[cmdlinelen] = 0;
	}

	/* If not using multiboot module for config, save the command line
	 * before something overwrites it */
	if (cmdline == NULL && mbi->flags & MULTIBOOT_INFO_CMDLINE) {
		cmdline = (char *)(uintptr_t)mbi->cmdline;
		cmdlinelen = bmk_strlen(cmdline);
		if (cmdlinelen >= BMK_MULTIBOOT_CMDLINE_SIZE)
			bmk_platform_halt("command line too long, "
			    "increase BMK_MULTIBOOT_CMDLINE_SIZE");
		bmk_strcpy(multiboot_cmdline, cmdline);
	}

	/* No configuration/cmdline found */
	if (cmdline == NULL)
		multiboot_cmdline[0] = 0;

	if ((mbi->flags & MULTIBOOT_INFO_MEMORY) == 0)
		bmk_platform_halt("multiboot memory info not available\n");

	if (parsemem(mbi->mmap_addr, mbi->mmap_length) != 0)
		bmk_platform_halt("multiboot memory parse failed");

	intr_init();
}
