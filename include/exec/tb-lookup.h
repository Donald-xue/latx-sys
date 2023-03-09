/*
 * Copyright (C) 2017, Emilio G. Cota <cota@braap.org>
 *
 * License: GNU GPL, version 2 or later.
 *   See the COPYING file in the top-level directory.
 */
#ifndef EXEC_TB_LOOKUP_H
#define EXEC_TB_LOOKUP_H

#ifdef NEED_CPU_H
#include "cpu.h"
#else
#include "exec/poison.h"
#endif

#include "exec/exec-all.h"
#include "exec/tb-hash.h"

#if defined(CONFIG_SOFTMMU) && defined(CONFIG_LATX)
#include "latxs-cc-pro.h"
#endif

/* Might cause an exception, so have a longjmp destination ready */
static inline TranslationBlock *tb_lookup(CPUState *cpu, target_ulong pc,
                                          target_ulong cs_base,
                                          uint32_t flags, uint32_t cflags)
{
    TranslationBlock *tb;
    uint32_t hash;

    /* we should never be trying to look up an INVALID tb */
    tcg_debug_assert(!(cflags & CF_INVALID));

    hash = tb_jmp_cache_hash_func(pc);
    if (qemu_tcg_bg_jc_enabled(cpu)) {
        tb = qatomic_rcu_read(&cpu->tcg_bg_jc[hash]);
    } else {
        tb = qatomic_rcu_read(&cpu->tb_jmp_cache[hash]);
    }

    int jc_lookup_ok = 0;
    if (likely(tb &&
               tb->pc == pc &&
               tb->cs_base == cs_base &&
               tb->trace_vcpu_dstate == *cpu->trace_dstate &&
               tb_cflags(tb) == cflags)) {
#if defined(CONFIG_LATX) && defined(CONFIG_SOFTMMU)
        if (latxs_cc_pro_tb_flags_cmp(tb, flags)) {
            jc_lookup_ok = 1;
        }
#else
        if (tb->flags == flags) {
            jc_lookup_ok = 1;
        }
#endif
        if(jc_lookup_ok) return tb;
    }
    tb = tb_htable_lookup(cpu, pc, cs_base, flags, cflags);
    if (tb == NULL) {
        return NULL;
    }
    if (qemu_tcg_bg_jc_enabled(cpu)) {
        qatomic_set(&cpu->tcg_bg_jc[hash], tb);
    } else {
        qatomic_set(&cpu->tb_jmp_cache[hash], tb);
    }
    return tb;
}

#endif /* EXEC_TB_LOOKUP_H */
