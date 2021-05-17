#include "env.h"
#include "mem.h"
#include "reg-alloc.h"
#include "shadow-stack.h"
#include "latx-options.h"

SS_ITEM* ss_pop(SS* ss) 
{
    ss->_ssi_current--;
    assert(ss->_ssi_current>ss->_ssi_first);
    return ss->_ssi_current;
}

SS_ITEM* ss_top(SS* ss) 
{
    assert(ss->_ssi_current>ss->_ssi_first);
    return ss->_ssi_current-1;
}

void ss_init(SS* ss) 
{
    if (option_shadow_stack)
        ss->_ssi_first = (SS_ITEM*) mm_calloc(800000, sizeof(SS_ITEM));
    else
        ss->_ssi_first = (SS_ITEM*) mm_calloc(800, sizeof(SS_ITEM));
    ss->_ssi_first->return_tb = (void*)0;
    ss->_ssi_first->x86_callee_addr = 0xbfffffff;
    ss->_ssi_first->x86_esp = 0xbfffffff;

    ss->_ssi_last = ss->_ssi_first + 800000;

    ss->_ssi_current = ss->_ssi_first+1;
}

void ss_duplicate(SS* ss, SS* from) 
{
    ss_init(ss);
    if(from->_ssi_first !=NULL)
        memcpy(ss->_ssi_first, from->_ssi_first, (from->_ssi_current-from->_ssi_first-1)*sizeof(SS_ITEM));
}

void ss_fini(SS* ss) 
{
    if (ss->_ssi_first)
        mm_free(ss->_ssi_first);
    ss->_ssi_first = 0;
    ss->_ssi_last = 0;
    ss->_ssi_current = 0;
}


void ss_push(SS* ss, ADDRX x86_esp, ADDRX x86_callee_addr, void *return_tb) 
{
    // 1. make sure ss have enough space
    if (ss->_ssi_current == ss->_ssi_last) {
        int curr_size = ss->_ssi_last - ss->_ssi_first;
        int new_size = curr_size << 1;
        ss->_ssi_first = (SS_ITEM*) mm_realloc(ss->_ssi_first, sizeof(SS_ITEM)*new_size);
        ss->_ssi_last = ss->_ssi_first + new_size;
        ss->_ssi_current = ss->_ssi_first + curr_size;
    }

    // 2. push one ss item
    ss->_ssi_current->x86_esp = x86_esp;
    ss->_ssi_current->x86_callee_addr = x86_callee_addr;
    ss->_ssi_current->return_tb = return_tb;
    ss->_ssi_current ++;
}


void ss_pop_till_find(SS* ss, ADDRX x86_esp) 
{
    // 1. find that ss item
    SS_ITEM *ssi = ss_top(ss);
    while (x86_esp > ssi->x86_esp && ss->_ssi_current > ss->_ssi_first+1) {
        ss_pop(ss);
        ssi = ss_top(ss);
    }
    if(x86_esp == ssi->x86_esp)
        ss_pop(ss);
    return;
}

/* etb_qht functions and etb_array */
bool etb_cmp(const void *ap, const void *bp)
{
    ETB *p = (ETB*)ap;
    ETB *q = (ETB*)bp;
    return p->pc == q->pc;
}

void etb_qht_init(void)
{
    unsigned int mode = QHT_MODE_AUTO_RESIZE;

    qht_init(etb_qht, etb_cmp, 1 << 10, mode);
}

bool etb_lookup_custom(const void *ap, const void *bp)
{
    ETB *etb = (ETB*)ap;
    ADDRX pc = *(ADDRX *)bp;
    return etb->pc == pc;
}

void etb_init(ETB *etb)
{
    memset(etb, 0, sizeof(ETB));
    etb->_top_out = -1;
    etb->_top_in = -1;
}

ETB *etb_find(ADDRX pc)
{
    static ETB *fast_table[1 << 10];
    uint32_t hash = pc & 0x3ff;
    if (fast_table[hash] && fast_table[hash]->pc == pc)
        return fast_table[hash];

    ETB *etb = (ETB*)qht_lookup_custom(etb_qht, &pc, hash, etb_lookup_custom);
    if (etb == NULL) {
        etb = (ETB*)mm_malloc(sizeof(ETB));
        etb_init(etb);
        etb->pc = pc;
        lsassertm(etb,"memory is full\n");
        qht_insert(etb_qht, etb, hash, NULL);
    }
    fast_table[hash] = etb;
    return etb;
}
