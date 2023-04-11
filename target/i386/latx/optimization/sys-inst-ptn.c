#include "common.h"
#include "ir1.h"
#include "translate.h"
#include "etb.h"
#include "latx-options.h"
#include "latx-sys-inst-ptn.h"

#ifdef LATXS_INSTPTN_ENABLE

#define WRAP(ins) (X86_INS_##ins)

static int ir1_can_pattern(IR1_INST *pir1)
{
    switch (ir1_opcode(pir1)) {
    case WRAP(JNS):
    case WRAP(JNO):
    case WRAP(JO):
    case WRAP(JS):
    case WRAP(JB):
    case WRAP(JAE):
    case WRAP(JE):
    case WRAP(JNE):
    case WRAP(JBE):
    case WRAP(JA):
    case WRAP(JL):
    case WRAP(JGE):
    case WRAP(JLE):
    case WRAP(JG):

    case WRAP(CMP):
    case WRAP(TEST):
        return 1;
    default:
        return 0;
    }
}

static int ir1_is_pattern_head(IR1_INST *pir1)
{
    switch (ir1_opcode(pir1)) {
    case WRAP(CMP):
    case WRAP(TEST):
        return 1;
    default:
        return 0;
    }
}



#define SCAN_CHECK(buf, i, ret) do { \
    if (buf[i] == -1) return ret; \
} while (0)
#define SCAN_IDX(buf, i)        (buf[i])
#define SCAN_IR1(tb, buf, i)    (tb_ir1_inst(tb, SCAN_IDX(buf, i)))

typedef int scan_elem_t;

static void scan_clear(scan_elem_t *scan)
{
    memset(scan, -1, sizeof(scan_elem_t) * INSTPTN_BUF_SIZE);
}

static void scan_push(scan_elem_t *scan, int pir1_index)
{
    for(int i = INSTPTN_BUF_SIZE - 1; i > 0; --i) {
        scan[i] = scan[i-1];
    }
    scan[0] = pir1_index;
}



static int inst_pattern(TranslationBlock *tb,
        IR1_INST *pir1, scan_elem_t *scan)
{
    IR1_INST *ir1 = NULL;
    IR1_OPND *opnd0 = NULL;
    IR1_OPND *opnd1 = NULL;

    /*
     * pir1 is pattern head
     * scan[] contains ir1 following the head
     */
    switch (ir1_opcode(pir1)) {
    case WRAP(CMP):
        SCAN_CHECK(scan, 0, 0);

        ir1 = SCAN_IR1(tb, scan, 0);
        switch (ir1_opcode(ir1)) {
        case WRAP(JB):
        case WRAP(JAE):
        case WRAP(JE):
        case WRAP(JNE):
        case WRAP(JBE):
        case WRAP(JA):
        case WRAP(JL):
        case WRAP(JGE):
        case WRAP(JLE):
        case WRAP(JG):
            pir1->instptn.opc  = INSTPTN_OPC_CMP_JCC;
            pir1->instptn.next = SCAN_IDX(scan, 0);
            ir1->instptn.opc  = INSTPTN_OPC_NOP;
            ir1->instptn.next = -1;
            return 1;
        default:
            return 0;
        }
    case WRAP(TEST):
        SCAN_CHECK(scan, 0, 0);

        opnd0 = ir1_get_opnd(pir1, 0);
        opnd1 = ir1_get_opnd(pir1, 1);
        if (!ir1_opnd_is_same_reg(opnd0, opnd1)) {
            return 0;
        }

        ir1 = SCAN_IR1(tb, scan, 0);
        switch (ir1_opcode(ir1)) {
        case WRAP(JE):
        case WRAP(JNE):
        case WRAP(JS):
        case WRAP(JNS):
        case WRAP(JLE):
        case WRAP(JG):
        case WRAP(JNO):
        case WRAP(JO):
        case WRAP(JB):
        case WRAP(JBE):
        case WRAP(JA):
        case WRAP(JAE):
            pir1->instptn.opc  = INSTPTN_OPC_TEST_JCC;
            pir1->instptn.next = SCAN_IDX(scan, 0);
            ir1->instptn.opc  = INSTPTN_OPC_NOP;
            ir1->instptn.next = -1;
            return 1;
        default:
            return 0;
        }
    default:
        return 0;
    }
}

static void inst_pattern_scan(TranslationBlock *tb,
        scan_elem_t *scan,
        IR1_INST *pir1, int pir1_index)
{
    if (!ir1_can_pattern(pir1)) {
        scan_clear(scan);
        return;
    }

    if (!ir1_is_pattern_head(pir1)) {
        scan_push(scan, pir1_index);
        return;
    }

    /* pir1 must be pattern head */
    if (inst_pattern(tb, pir1, scan)) {
        scan_clear(scan);
    } else {
        scan_push(scan, pir1_index);
    }
}

void __latxs_inst_pattern(TranslationBlock *tb)
{
    latxs_instptn_check_void();
    latxs_instptn_cpl_filter_void(tb);

    scan_elem_t scan[INSTPTN_BUF_SIZE];
    scan_clear(scan);

    IR1_INST *ir1 = NULL;
    int i = tb_ir1_num(tb) - 1;
    for (; i >= 0; --i) {
        ir1 = tb_ir1_inst(tb, i);
        inst_pattern_scan(tb, scan, ir1, i);
    }
}

#endif
