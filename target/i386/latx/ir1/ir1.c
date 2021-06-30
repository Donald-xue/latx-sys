#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "ir1.h"

IR1_OPND al_ir1_opnd;
IR1_OPND ah_ir1_opnd;
IR1_OPND ax_ir1_opnd;
IR1_OPND eax_ir1_opnd;
IR1_OPND rax_ir1_opnd;

IR1_OPND cl_ir1_opnd;
IR1_OPND ch_ir1_opnd;
IR1_OPND cx_ir1_opnd;
IR1_OPND ecx_ir1_opnd;
IR1_OPND rcx_ir1_opnd;

IR1_OPND dl_ir1_opnd;
IR1_OPND dh_ir1_opnd;
IR1_OPND dx_ir1_opnd;
IR1_OPND edx_ir1_opnd;
IR1_OPND rdx_ir1_opnd;

IR1_OPND bl_ir1_opnd;
IR1_OPND bh_ir1_opnd;
IR1_OPND bx_ir1_opnd;
IR1_OPND ebx_ir1_opnd;
IR1_OPND rbx_ir1_opnd;

IR1_OPND esp_ir1_opnd;
IR1_OPND rsp_ir1_opnd;

IR1_OPND ebp_ir1_opnd;
IR1_OPND rbp_ir1_opnd;

IR1_OPND si_ir1_opnd;
IR1_OPND esi_ir1_opnd;
IR1_OPND rsi_ir1_opnd;

IR1_OPND di_ir1_opnd;
IR1_OPND edi_ir1_opnd;
IR1_OPND rdi_ir1_opnd;

IR1_OPND eax_mem8_ir1_opnd;
IR1_OPND ecx_mem8_ir1_opnd;
IR1_OPND edx_mem8_ir1_opnd;
IR1_OPND ebx_mem8_ir1_opnd;
IR1_OPND esp_mem8_ir1_opnd;
IR1_OPND ebp_mem8_ir1_opnd;
IR1_OPND esi_mem8_ir1_opnd;
IR1_OPND edi_mem8_ir1_opnd;

IR1_OPND eax_mem16_ir1_opnd;
IR1_OPND ecx_mem16_ir1_opnd;
IR1_OPND edx_mem16_ir1_opnd;
IR1_OPND ebx_mem16_ir1_opnd;
IR1_OPND esp_mem16_ir1_opnd;
IR1_OPND ebp_mem16_ir1_opnd;
IR1_OPND esi_mem16_ir1_opnd;
IR1_OPND edi_mem16_ir1_opnd;

IR1_OPND eax_mem32_ir1_opnd;
IR1_OPND ecx_mem32_ir1_opnd;
IR1_OPND edx_mem32_ir1_opnd;
IR1_OPND ebx_mem32_ir1_opnd;
IR1_OPND esp_mem32_ir1_opnd;
IR1_OPND ebp_mem32_ir1_opnd;
IR1_OPND esi_mem32_ir1_opnd;
IR1_OPND edi_mem32_ir1_opnd;

csh handle;

static IR1_OPND ir1_opnd_new_static_reg(IR1_OPND_TYPE opnd_type, int size,
                                        x86_reg reg)
{
    IR1_OPND ir1_opnd;
    IR1_OPND *opnd = &ir1_opnd;

    opnd->type = opnd_type;
    lsassert(size % 8 == 0);
    opnd->size = size / 8;
    opnd->reg = reg;
    opnd->avx_bcast = X86_AVX_BCAST_INVALID;

    return ir1_opnd;
}

static IR1_OPND ir1_opnd_new_static_mem(IR1_OPND_TYPE opnd_type, int size,
                                        x86_reg reg, uint64_t imm)
{
    switch (reg) {
    case X86_REG_EAX:
    case X86_REG_ECX:
    case X86_REG_EBX:
    case X86_REG_EDX:
    case X86_REG_ESI:
    case X86_REG_EDI:
    case X86_REG_ESP:
    case X86_REG_EBP:
        break;
    default:
        lsassert(0);
    }

    IR1_OPND ir1_opnd;
    IR1_OPND *opnd = &ir1_opnd;

    opnd->type = opnd_type;
    lsassert(size % 8 == 0);
    opnd->size = size / 8;
    opnd->size = size / 8;
    opnd->mem.base = reg;

    opnd->mem.index = X86_REG_INVALID;
    opnd->mem.segment = X86_REG_INVALID;
    opnd->mem.scale = 0;
    opnd->mem.disp = imm;

    opnd->avx_bcast = X86_AVX_BCAST_INVALID;

    return ir1_opnd;
}
static void __attribute__((__constructor__)) x86tomisp_ir1_init(void)
{
    al_ir1_opnd = ir1_opnd_new_static_reg(X86_OP_REG, 8, X86_REG_AL);
    ah_ir1_opnd = ir1_opnd_new_static_reg(X86_OP_REG, 8, X86_REG_AH);
    ax_ir1_opnd = ir1_opnd_new_static_reg(X86_OP_REG, 16, X86_REG_AX);
    eax_ir1_opnd = ir1_opnd_new_static_reg(X86_OP_REG, 32, X86_REG_EAX);
    rax_ir1_opnd = ir1_opnd_new_static_reg(X86_OP_REG, 64, X86_REG_RAX);

    cl_ir1_opnd = ir1_opnd_new_static_reg(X86_OP_REG, 8, X86_REG_CL);
    ch_ir1_opnd = ir1_opnd_new_static_reg(X86_OP_REG, 8, X86_REG_CH);
    cx_ir1_opnd = ir1_opnd_new_static_reg(X86_OP_REG, 16, X86_REG_CX);
    ecx_ir1_opnd = ir1_opnd_new_static_reg(X86_OP_REG, 32, X86_REG_ECX);
    rcx_ir1_opnd = ir1_opnd_new_static_reg(X86_OP_REG, 64, X86_REG_RCX);

    dl_ir1_opnd = ir1_opnd_new_static_reg(X86_OP_REG, 8, X86_REG_DL);
    dh_ir1_opnd = ir1_opnd_new_static_reg(X86_OP_REG, 8, X86_REG_DH);
    dx_ir1_opnd = ir1_opnd_new_static_reg(X86_OP_REG, 16, X86_REG_DX);
    edx_ir1_opnd = ir1_opnd_new_static_reg(X86_OP_REG, 32, X86_REG_EDX);
    rdx_ir1_opnd = ir1_opnd_new_static_reg(X86_OP_REG, 64, X86_REG_RDX);

    bl_ir1_opnd = ir1_opnd_new_static_reg(X86_OP_REG, 8, X86_REG_BL);
    bh_ir1_opnd = ir1_opnd_new_static_reg(X86_OP_REG, 8, X86_REG_BH);
    bx_ir1_opnd = ir1_opnd_new_static_reg(X86_OP_REG, 16, X86_REG_BX);
    ebx_ir1_opnd = ir1_opnd_new_static_reg(X86_OP_REG, 32, X86_REG_EBX);
    rbx_ir1_opnd = ir1_opnd_new_static_reg(X86_OP_REG, 64, X86_REG_RBX);

    esp_ir1_opnd = ir1_opnd_new_static_reg(X86_OP_REG, 32, X86_REG_ESP);
    rsp_ir1_opnd = ir1_opnd_new_static_reg(X86_OP_REG, 64, X86_REG_RSP);

    ebp_ir1_opnd = ir1_opnd_new_static_reg(X86_OP_REG, 32, X86_REG_EBP);
    rbp_ir1_opnd = ir1_opnd_new_static_reg(X86_OP_REG, 64, X86_REG_RBP);

    si_ir1_opnd = ir1_opnd_new_static_reg(X86_OP_REG, 16, X86_REG_SI);
    esi_ir1_opnd = ir1_opnd_new_static_reg(X86_OP_REG, 32, X86_REG_ESI);
    rsi_ir1_opnd = ir1_opnd_new_static_reg(X86_OP_REG, 64, X86_REG_RSI);

    di_ir1_opnd = ir1_opnd_new_static_reg(X86_OP_REG, 16, X86_REG_DI);
    edi_ir1_opnd = ir1_opnd_new_static_reg(X86_OP_REG, 32, X86_REG_EDI);
    rdi_ir1_opnd = ir1_opnd_new_static_reg(X86_OP_REG, 64, X86_REG_RDI);

    eax_mem8_ir1_opnd = ir1_opnd_new_static_mem(X86_OP_MEM, 8, X86_REG_EAX, 0);
    ecx_mem8_ir1_opnd = ir1_opnd_new_static_mem(X86_OP_MEM, 8, X86_REG_ECX, 0);
    edx_mem8_ir1_opnd = ir1_opnd_new_static_mem(X86_OP_MEM, 8, X86_REG_EDX, 0);
    ebx_mem8_ir1_opnd = ir1_opnd_new_static_mem(X86_OP_MEM, 8, X86_REG_EBX, 0);
    esp_mem8_ir1_opnd = ir1_opnd_new_static_mem(X86_OP_MEM, 8, X86_REG_ESP, 0);
    ebp_mem8_ir1_opnd = ir1_opnd_new_static_mem(X86_OP_MEM, 8, X86_REG_EBP, 0);
    esi_mem8_ir1_opnd = ir1_opnd_new_static_mem(X86_OP_MEM, 8, X86_REG_ESI, 0);
    edi_mem8_ir1_opnd = ir1_opnd_new_static_mem(X86_OP_MEM, 8, X86_REG_EDI, 0);

    eax_mem16_ir1_opnd =
        ir1_opnd_new_static_mem(X86_OP_MEM, 16, X86_REG_EAX, 0);
    ecx_mem16_ir1_opnd =
        ir1_opnd_new_static_mem(X86_OP_MEM, 16, X86_REG_ECX, 0);
    edx_mem16_ir1_opnd =
        ir1_opnd_new_static_mem(X86_OP_MEM, 16, X86_REG_EDX, 0);
    ebx_mem16_ir1_opnd =
        ir1_opnd_new_static_mem(X86_OP_MEM, 16, X86_REG_EBX, 0);
    esp_mem16_ir1_opnd =
        ir1_opnd_new_static_mem(X86_OP_MEM, 16, X86_REG_ESP, 0);
    ebp_mem16_ir1_opnd =
        ir1_opnd_new_static_mem(X86_OP_MEM, 16, X86_REG_EBP, 0);
    esi_mem16_ir1_opnd =
        ir1_opnd_new_static_mem(X86_OP_MEM, 16, X86_REG_ESI, 0);
    edi_mem16_ir1_opnd =
        ir1_opnd_new_static_mem(X86_OP_MEM, 16, X86_REG_EDI, 0);

    eax_mem32_ir1_opnd =
        ir1_opnd_new_static_mem(X86_OP_MEM, 32, X86_REG_EAX, 0);
    ecx_mem32_ir1_opnd =
        ir1_opnd_new_static_mem(X86_OP_MEM, 32, X86_REG_ECX, 0);
    edx_mem32_ir1_opnd =
        ir1_opnd_new_static_mem(X86_OP_MEM, 32, X86_REG_EDX, 0);
    ebx_mem32_ir1_opnd =
        ir1_opnd_new_static_mem(X86_OP_MEM, 32, X86_REG_EBX, 0);
    esp_mem32_ir1_opnd =
        ir1_opnd_new_static_mem(X86_OP_MEM, 32, X86_REG_ESP, 0);
    ebp_mem32_ir1_opnd =
        ir1_opnd_new_static_mem(X86_OP_MEM, 32, X86_REG_EBP, 0);
    esi_mem32_ir1_opnd =
        ir1_opnd_new_static_mem(X86_OP_MEM, 32, X86_REG_ESI, 0);
    edi_mem32_ir1_opnd =
        ir1_opnd_new_static_mem(X86_OP_MEM, 32, X86_REG_EDI, 0);
};

ADDRX ir1_disasm(IR1_INST *ir1, uint8_t *addr, ADDRX t_pc, int ir1_num, void *pir1_base)
{
    cs_insn *info;
    uint32_t nop = 0x401f0f;
    if (((*((uint32_t *)addr))&0xf8ffffff) == 0xc81e0ff3) {
        //repleace endbr32/rdsspd with 4 bytes nop, just a temporary solution
        addr = (uint8_t *)&nop;
    }

    /* FIXME:the count parameter in cs_disasm is 1, it means we translte 1 insn at a time,
     * there should be a performance improvement if we increase the number, but
     * for now there are some problems if we change it. It will be settled later.
     */
    int count = cs_disasm(handle, addr, 15, (uint64_t)t_pc, 1, &info, ir1_num, pir1_base);

    ir1->info = info;

    if (count != 1) {
        fprintf(stderr, "ERROR : disasm, ADDR : 0x%" PRIx64 "\n", (uint64_t)t_pc);
        exit(-1);
    }

    ir1->flags = 0;
    ir1->_eflag_def = 0;
    ir1->_eflag_use = 0;
    ir1->_native_inst_num = 0;

    // xtm treat opnd with default seg(without segment-override prefix) as a mem
    // opnd so, we make it invalid
    if (info->detail->x86.prefix[1] != X86_PREFIX_CS &&
        info->detail->x86.prefix[1] != X86_PREFIX_DS &&
        info->detail->x86.prefix[1] != X86_PREFIX_SS &&
        info->detail->x86.prefix[1] != X86_PREFIX_ES &&
        info->detail->x86.prefix[1] != X86_PREFIX_FS &&
        info->detail->x86.prefix[1] != X86_PREFIX_GS) {
        for (int i = 0; i < info->detail->x86.op_count; i++) {
            if (info->detail->x86.operands[i].type == X86_OP_MEM) {
                info->detail->x86.operands[i].mem.segment = X86_REG_INVALID;
            }
        }
    }

    return (ADDRX)(ir1->info->address + ir1->info->size);
}

void ir1_opnd_build_reg(IR1_OPND *opnd, int size, x86_reg reg)
{
    opnd->type = X86_OP_REG;
    lsassert(size % 8 == 0);
    opnd->size = size / 8;
    opnd->reg = reg;
    opnd->avx_bcast = X86_AVX_BCAST_INVALID;
}

void ir1_opnd_build_imm(IR1_OPND *opnd, int size, int64_t imm)
{
    opnd->type = X86_OP_IMM;
    lsassert(size % 8 == 0);
    opnd->size = size / 8;
    opnd->imm = imm;
    opnd->avx_bcast = X86_AVX_BCAST_INVALID;
}

void ir1_opnd_build_mem(IR1_OPND *opnd, int size, x86_reg base, int64_t disp)
{
    opnd->type = X86_OP_MEM;
    lsassert(size % 8 == 0);
    opnd->size = size / 8;
    opnd->mem.base = base;
    opnd->mem.disp = disp;
    opnd->mem.index = X86_REG_INVALID;
    opnd->mem.scale = 0;
    opnd->mem.segment = X86_REG_INVALID;
    opnd->avx_bcast = X86_AVX_BCAST_INVALID;
}

void ir1_opnd_check(IR1_OPND *opnd) { lsassert(0); }

IR1_OPND_TYPE ir1_opnd_type(IR1_OPND *opnd) { return opnd->type; }

int ir1_opnd_size(IR1_OPND *opnd) { return opnd->size << 3; }

int ir1_opnd_base_reg_bits_start(IR1_OPND *opnd)
{
    lsassert(opnd->type == X86_OP_REG && opnd->reg > X86_REG_INVALID &&
             opnd->reg < X86_REG_ENDING);
    switch (opnd->reg) {
    case X86_REG_AH:
    case X86_REG_BH:
    case X86_REG_CH:
    case X86_REG_DH:
        return 8;
    default:
        return 0;
    }
}

x86_reg ir1_opnd_index_reg(IR1_OPND *opnd) { return opnd->mem.index; }

x86_reg ir1_opnd_base_reg(IR1_OPND *opnd) { return opnd->mem.base; }

int ir1_opnd_scale(IR1_OPND *opnd) { return opnd->mem.scale; }

int ir1_opnd_index_reg_num(IR1_OPND *opnd)
{
    switch (opnd->mem.index) {
    case X86_REG_AL:
    case X86_REG_AH:
    case X86_REG_AX:
    case X86_REG_EAX:
        return eax_index;
    case X86_REG_BL:
    case X86_REG_BH:
    case X86_REG_BX:
    case X86_REG_EBX:
        return ebx_index;
    case X86_REG_CL:
    case X86_REG_CH:
    case X86_REG_CX:
    case X86_REG_ECX:
        return ecx_index;
    case X86_REG_DL:
    case X86_REG_DH:
    case X86_REG_DX:
    case X86_REG_EDX:
        return edx_index;
    case X86_REG_BP:
    case X86_REG_EBP:
        return ebp_index;
    case X86_REG_SI:
    case X86_REG_ESI:
        return esi_index;
    case X86_REG_DI:
    case X86_REG_EDI:
        return edi_index;
    case X86_REG_SP:
    case X86_REG_ESP:
        return esp_index;
    case X86_REG_CS:
        return cs_index;
    case X86_REG_DS:
        return ds_index;
    case X86_REG_SS:
        return ss_index;
    case X86_REG_ES:
        return es_index;
    case X86_REG_GS:
        return gs_index;
    case X86_REG_FS:
        return fs_index;

    default:
        lsassert(0);
        // may fpu?
        return -1;
    }
}
int ir1_opnd_base_reg_num(IR1_OPND *opnd)
{
    // xtm : base may not a mem opnd
    if (opnd->type == X86_OP_MEM && opnd->mem.base == X86_REG_INVALID) {
        return -1;
    }
    //    lsassert(opnd->type == X86_OP_MEM || opnd->type == X86_OP_REG);
    switch (opnd->type == X86_OP_REG ? opnd->reg : opnd->mem.base) {
    case X86_REG_AL:
    case X86_REG_AH:
    case X86_REG_AX:
    case X86_REG_EAX:
        return eax_index;
    case X86_REG_BL:
    case X86_REG_BH:
    case X86_REG_BX:
    case X86_REG_EBX:
        return ebx_index;
    case X86_REG_CL:
    case X86_REG_CH:
    case X86_REG_CX:
    case X86_REG_ECX:
        return ecx_index;
    case X86_REG_DL:
    case X86_REG_DH:
    case X86_REG_DX:
    case X86_REG_EDX:
        return edx_index;
    case X86_REG_BP:
    case X86_REG_EBP:
        return ebp_index;
    case X86_REG_SI:
    case X86_REG_ESI:
        return esi_index;
    case X86_REG_DI:
    case X86_REG_EDI:
        return edi_index;
    case X86_REG_SP:
    case X86_REG_ESP:
        return esp_index;
    case X86_REG_CS:
        return cs_index;
    case X86_REG_DS:
        return ds_index;
    case X86_REG_SS:
        return ss_index;
    case X86_REG_ES:
        return es_index;
    case X86_REG_GS:
        return gs_index;
    case X86_REG_FS:
        return fs_index;
    case X86_REG_MM0:
        return 0;
    case X86_REG_MM1:
        return 1;
    case X86_REG_MM2:
        return 2;
    case X86_REG_MM3:
        return 3;
    case X86_REG_MM4:
        return 4;
    case X86_REG_MM5:
        return 5;
    case X86_REG_MM6:
        return 6;
    case X86_REG_MM7:
        return 7;
    case X86_REG_XMM0:
        return 0;
    case X86_REG_XMM1:
        return 1;
    case X86_REG_XMM2:
        return 2;
    case X86_REG_XMM3:
        return 3;
    case X86_REG_XMM4:
        return 4;
    case X86_REG_XMM5:
        return 5;
    case X86_REG_XMM6:
        return 6;
    case X86_REG_XMM7:
        return 7;
    case X86_REG_YMM0:
        return 0;
    case X86_REG_YMM1:
        return 1;
    case X86_REG_YMM2:
        return 2;
    case X86_REG_YMM3:
        return 3;
    case X86_REG_YMM4:
        return 4;
    case X86_REG_YMM5:
        return 5;
    case X86_REG_YMM6:
        return 6;
    case X86_REG_YMM7:
        return 7;
    case X86_REG_ST0:
        return 0;
    case X86_REG_ST1:
        return 1;
    case X86_REG_ST2:
        return 2;
    case X86_REG_ST3:
        return 3;
    case X86_REG_ST4:
        return 4;
    case X86_REG_ST5:
        return 5;
    case X86_REG_ST6:
        return 6;
    case X86_REG_ST7:
        return 7;
    default:
        lsassert(0);
        return -1;
    }
}

longx ir1_opnd_simm(IR1_OPND *opnd)
{
    if (ir1_opnd_type(opnd) == X86_OP_IMM) {
        switch (ir1_opnd_size(opnd)) {
        case 8:
            return (longx)((int8_t)(opnd->imm));
        case 16:
            return (longx)((int16_t)(opnd->imm));
        case 32:
            return (longx)((int32_t)(opnd->imm));
        /* case 64:
         *     lsassert(0);
         */
        default:
            lsassert(0);
        }
    } else if (ir1_opnd_type(opnd) == X86_OP_MEM) {
        return (longx)(opnd->mem.disp);
    } else {
        lsassertm(0, "REG opnd has no imm\n");
    }
    abort();
}

ulongx ir1_opnd_uimm(IR1_OPND *opnd)
{
    if (ir1_opnd_type(opnd) == X86_OP_IMM) {
        return (ulongx)(opnd->imm);
    } else if (ir1_opnd_type(opnd) == X86_OP_MEM) {
        return (ulongx)(opnd->mem.disp);
    } else {
        lsassertm(0, "REG opnd has no imm\n");
    }
    abort();
}

x86_reg ir1_opnd_seg_reg(IR1_OPND *opnd) { return opnd->mem.segment; }

int ir1_opnd_is_imm(IR1_OPND *opnd) { return opnd->type == X86_OP_IMM; }

int ir1_opnd_is_8h(IR1_OPND *opnd)
{
    return opnd->type == X86_OP_REG && opnd->size == 1 &&
           (opnd->reg == X86_REG_AH || opnd->reg == X86_REG_BH ||
            opnd->reg == X86_REG_CH || opnd->reg == X86_REG_DH);
}

int ir1_opnd_is_8l(IR1_OPND *opnd)
{
    return opnd->type == X86_OP_REG && opnd->size == 1 &&
           (opnd->reg == X86_REG_AL || opnd->reg == X86_REG_BL ||
            opnd->reg == X86_REG_CL || opnd->reg == X86_REG_DL);
}

int ir1_opnd_is_16l(IR1_OPND *opnd)
{
    return opnd->type == X86_OP_REG && opnd->size == 2 &&
           (opnd->reg == X86_REG_AX || opnd->reg == X86_REG_BX ||
            opnd->reg == X86_REG_CX || opnd->reg == X86_REG_DX);
}

int ir1_opnd_is_gpr(IR1_OPND *opnd)
{
    if (opnd->type != X86_OP_REG) {
        return 0;
    }
    switch (opnd->reg) {
    case X86_REG_AL:
    case X86_REG_AH:
    case X86_REG_AX:
    case X86_REG_EAX:
    case X86_REG_BL:
    case X86_REG_BH:
    case X86_REG_BX:
    case X86_REG_EBX:
    case X86_REG_CL:
    case X86_REG_CH:
    case X86_REG_CX:
    case X86_REG_ECX:
    case X86_REG_DL:
    case X86_REG_DH:
    case X86_REG_DX:
    case X86_REG_EDX:

    case X86_REG_BP:
    case X86_REG_EBP:
    case X86_REG_SI:
    case X86_REG_ESI:
    case X86_REG_DI:
    case X86_REG_EDI:
    case X86_REG_SP:
    case X86_REG_ESP:
        return 1;
    default:
        return 0;
    }
}

int ir1_opnd_is_uimm_within_16bit(IR1_OPND *opnd)
{
    return ir1_opnd_is_imm(opnd) && ir1_opnd_uimm(opnd) < 4096;
}

int ir1_opnd_is_simm_within_16bit(IR1_OPND *opnd)
{
    return ir1_opnd_is_imm(opnd) && ir1_opnd_simm(opnd) >= -2048&&
           ir1_opnd_simm(opnd) < 2047;
}

int ir1_opnd_is_gpr_used(IR1_OPND *opnd, uint8_t gpr_index)
{
    if (ir1_opnd_is_gpr(opnd)) {
        return ir1_opnd_base_reg_num(opnd) == gpr_index;
    } else if (ir1_opnd_is_mem(opnd)) {
        if (ir1_opnd_has_base(opnd)) {
            return ir1_opnd_base_reg_num(opnd) == gpr_index;
        }
        if (ir1_opnd_has_index(opnd)) {
            return ir1_opnd_index_reg_num(opnd) == gpr_index;
        }
    }
    return 0;
}

int ir1_opnd_is_mem(IR1_OPND *opnd) { return opnd->type == X86_OP_MEM; }

int ir1_opnd_is_fpr(IR1_OPND *opnd)
{
    if (opnd->type != X86_OP_REG) {
        return 0;
    }
    switch (opnd->reg) {
    case X86_REG_ST0:
    case X86_REG_ST1:
    case X86_REG_ST2:
    case X86_REG_ST3:
    case X86_REG_ST4:
    case X86_REG_ST5:
    case X86_REG_ST6:
    case X86_REG_ST7:
        return 1;
    default:
        return 0;
    }
}

int ir1_opnd_is_seg(IR1_OPND *opnd)
{
    if (opnd->type != X86_OP_REG) {
        return 0;
    }
    switch (opnd->reg) {
    case X86_REG_CS:
    case X86_REG_DS:
    case X86_REG_SS:
    case X86_REG_ES:
    case X86_REG_FS:
    case X86_REG_GS:
        return 1;
    default:
        return 0;
    }
}

int ir1_opnd_is_mmx(IR1_OPND *opnd)
{
    if (opnd->type != X86_OP_REG) {
        return 0;
    }
    switch (opnd->reg) {
    case X86_REG_MM0:
    case X86_REG_MM1:
    case X86_REG_MM2:
    case X86_REG_MM3:
    case X86_REG_MM4:
    case X86_REG_MM5:
    case X86_REG_MM6:
    case X86_REG_MM7:
        return 1;
    default:
        return 0;
    }
}

int ir1_opnd_is_xmm(IR1_OPND *opnd)
{
    if (opnd->type != X86_OP_REG) {
        return 0;
    }
    switch (opnd->reg) {
    case X86_REG_XMM0:
    case X86_REG_XMM1:
    case X86_REG_XMM2:
    case X86_REG_XMM3:
    case X86_REG_XMM4:
    case X86_REG_XMM5:
    case X86_REG_XMM6:
    case X86_REG_XMM7:
        return 1;
    default:
        return 0;
    }
}

int ir1_opnd_is_ymm(IR1_OPND *opnd)
{
    if (opnd->type != X86_OP_REG) {
        return 0;
    }
    switch (opnd->reg) {
    case X86_REG_YMM0:
    case X86_REG_YMM1:
    case X86_REG_YMM2:
    case X86_REG_YMM3:
    case X86_REG_YMM4:
    case X86_REG_YMM5:
    case X86_REG_YMM6:
    case X86_REG_YMM7:
        return 1;
    default:
        return 0;
    }
}

int ir1_opnd_has_base(IR1_OPND *opnd)
{
    // may unnecessary to judge mem opnd
    return opnd->mem.base != X86_REG_INVALID;
}

int ir1_opnd_has_index(IR1_OPND *opnd)
{
    return opnd->mem.index != X86_REG_INVALID;
}

int ir1_opnd_has_seg(IR1_OPND *opnd)
{
    return opnd->mem.segment != X86_REG_INVALID;
}

int ir1_opnd_get_seg_index(IR1_OPND *opnd)
{
    lsassert(ir1_opnd_type(opnd) == X86_OP_MEM);
    x86_reg seg = opnd->mem.segment;
    if (seg == X86_REG_ES) {
        return es_index;
    }
    if (seg == X86_REG_CS) {
        return cs_index;
    }
    if (seg == X86_REG_SS) {
        return ss_index;
    }
    if (seg == X86_REG_DS) {
        return ds_index;
    }
    if (seg == X86_REG_FS) {
        return fs_index;
    }
    if (seg == X86_REG_GS) {
        return gs_index;
    }
    abort();
    //     IR1_OPND_MEM_ES,
    //     IR1_OPND_MEM_CS,
    //     IR1_OPND_MEM_SS,
    //     IR1_OPND_MEM_DS,
    //     IR1_OPND_MEM_FS,
    //     IR1_OPND_MEM_GS,
}

IR1_PREFIX ir1_prefix(IR1_INST *ir1)
{
    // TODO: one ins may has two or more PREFIX (rarely)
    // only support rep now
    return ir1->info->detail->x86.prefix[0];  // only lock rep
}

int ir1_opnd_num(IR1_INST *ir1) { return ir1->info->detail->x86.op_count; }

ADDRX ir1_addr(IR1_INST *ir1) { return ir1->info->address; }

ADDRX ir1_addr_next(IR1_INST *ir1)
{
    return ir1->info->address + ir1->info->size;
}

ADDRX ir1_target_addr(IR1_INST *ir1)
{
    lsassert(ir1_opnd_type(&(ir1->info->detail->x86.operands[0])) ==
             X86_OP_IMM);
    return (ADDRX)ir1_opnd_uimm(&(ir1->info->detail->x86.operands[0]));
}

// IR1_OPCODE ir1_opcode(IR1_INST *ir1){
//     return ir1->detail->x86.opcode;
// }

IR1_OPCODE ir1_opcode(IR1_INST *ir1) { return ir1->info->id; }

int ir1_is_branch(IR1_INST *ir1)
{
    switch (ir1->info->id) {
    case X86_INS_JO:
    case X86_INS_JNO:
    case X86_INS_JB:
    case X86_INS_JAE:
    case X86_INS_JE:
    case X86_INS_JNE:
    case X86_INS_JBE:
    case X86_INS_JA:
    case X86_INS_JS:
    case X86_INS_JNS:
    case X86_INS_JP:
    case X86_INS_JNP:
    case X86_INS_JL:
    case X86_INS_JGE:
    case X86_INS_JLE:
    case X86_INS_JG:

    case X86_INS_LOOPNE:
    case X86_INS_LOOPE:
    case X86_INS_LOOP:
    case X86_INS_JCXZ:
    case X86_INS_JECXZ:
    case X86_INS_JRCXZ:

        return 1;

    default:
        return 0;
    }
}

int ir1_is_jump(IR1_INST *ir1) { return ir1->info->id == X86_INS_JMP; }

int ir1_is_call(IR1_INST *ir1)
{
    // lsassert(0);
    // TODO : only direct?
    return ir1->info->id == X86_INS_CALL;
}

int ir1_is_return(IR1_INST *ir1)
{
    return ir1->info->id == X86_INS_RET || ir1->info->id == X86_INS_IRET ||
           ir1->info->id == X86_INS_RETF;
}

int ir1_is_indirect(IR1_INST *ir1)
{
    lsassert(0);
    // TODO : some indirect call/jmp
    return ir1->info->id == X86_INS_LJMP || ir1->info->id == X86_INS_LCALL;
}

int ir1_is_syscall(IR1_INST *ir1)
{
    // TODO : 0x80?
    return ir1->info->id == X86_INS_INT;
    // X86_INS_SYSCALL,X86_INS_SYSENTER,X86_INS_SYSEXIT,X86_INS_SYSRET
}
bool ir1_is_tb_ending(IR1_INST *ir1)
{
    // return ir1_is_branch(ir1) || ir1_is_jump(ir1) || ir1_is_call(ir1) ||
    // ir1_is_return(ir1) || ir1_is_indirect(ir1) || ir1_is_syscall(ir1);
    if (ir1_opcode(ir1) == X86_INS_CALL && !ir1_is_indirect_call(ir1) &&
        ir1_addr_next(ir1) == ir1_target_addr(ir1)) {
        return false;
    }
    return ir1_is_branch(ir1) || ir1_is_jump(ir1) || ir1_is_call(ir1) ||
           ir1_is_return(ir1) || ir1_is_syscall(ir1);
}

bool ir1_is_cf_use(IR1_INST *ir1)
{
    return BITS_ARE_SET(ir1->_eflag_use, 1 << 0);
}
bool ir1_is_pf_use(IR1_INST *ir1)
{
    return BITS_ARE_SET(ir1->_eflag_use, 1 << 1);
}
bool ir1_is_af_use(IR1_INST *ir1)
{
    return BITS_ARE_SET(ir1->_eflag_use, 1 << 2);
}
bool ir1_is_zf_use(IR1_INST *ir1)
{
    return BITS_ARE_SET(ir1->_eflag_use, 1 << 3);
}
bool ir1_is_sf_use(IR1_INST *ir1)
{
    return BITS_ARE_SET(ir1->_eflag_use, 1 << 4);
}
bool ir1_is_of_use(IR1_INST *ir1)
{
    return BITS_ARE_SET(ir1->_eflag_use, 1 << 5);
}

bool ir1_is_cf_def(IR1_INST *ir1)
{
    return BITS_ARE_SET(ir1->_eflag_def, 1 << 0);
}
bool ir1_is_pf_def(IR1_INST *ir1)
{
    return BITS_ARE_SET(ir1->_eflag_def, 1 << 1);
}
bool ir1_is_af_def(IR1_INST *ir1)
{
    return BITS_ARE_SET(ir1->_eflag_def, 1 << 2);
}
bool ir1_is_zf_def(IR1_INST *ir1)
{
    return BITS_ARE_SET(ir1->_eflag_def, 1 << 3);
}
bool ir1_is_sf_def(IR1_INST *ir1)
{
    return BITS_ARE_SET(ir1->_eflag_def, 1 << 4);
}
bool ir1_is_of_def(IR1_INST *ir1)
{
    return BITS_ARE_SET(ir1->_eflag_def, 1 << 5);
}

uint8_t ir1_get_eflag_use(IR1_INST *ir1) { return ir1->_eflag_use; }
uint8_t ir1_get_eflag_def(IR1_INST *ir1) { return ir1->_eflag_def; }
void ir1_set_eflag_use(IR1_INST *ir1, uint8_t use) { ir1->_eflag_use = use; }
void ir1_set_eflag_def(IR1_INST *ir1, uint8_t def) { ir1->_eflag_def = def; }

uint8_t ir1_get_eflag_inherent_use(IR1_INST *ir1)
{
    lsassert(0);
    uint8_t tmpef = 0;
    x86_insn op = ir1_opcode(ir1);
    if (op == X86_INS_FCMOVU || op == X86_INS_FCMOVNU) {
        return (uint8_t)1 << PF_USEDEF_BIT_INDEX;
    }
    if (op == X86_INS_FCMOVB || op == X86_INS_FCMOVNB) {
        return (uint8_t)1 << CF_USEDEF_BIT_INDEX;
    }
    if (op == X86_INS_FCMOVE || op == X86_INS_FCMOVNE) {
        return (uint8_t)1 << ZF_USEDEF_BIT_INDEX;
    }
    if (op == X86_INS_FCMOVBE || op == X86_INS_FCMOVNBE) {
        return (uint8_t)1 << ZF_USEDEF_BIT_INDEX & (uint8_t)1
                                                       << CF_USEDEF_BIT_INDEX;
    }

    if (X86_EFLAGS_TEST_CF & ir1->info->detail->x86.eflags) {
        tmpef |= (uint8_t)1 << CF_USEDEF_BIT_INDEX;
    }
    if (X86_EFLAGS_TEST_ZF & ir1->info->detail->x86.eflags) {
        tmpef |= (uint8_t)1 << ZF_USEDEF_BIT_INDEX;
    }
    if (X86_EFLAGS_TEST_AF & ir1->info->detail->x86.eflags) {
        tmpef |= (uint8_t)1 << AF_USEDEF_BIT_INDEX;
    }
    if (X86_EFLAGS_TEST_PF & ir1->info->detail->x86.eflags) {
        tmpef |= (uint8_t)1 << PF_USEDEF_BIT_INDEX;
    }
    if (X86_EFLAGS_TEST_SF & ir1->info->detail->x86.eflags) {
        tmpef |= (uint8_t)1 << SF_USEDEF_BIT_INDEX;
    }
    if (X86_EFLAGS_TEST_OF & ir1->info->detail->x86.eflags) {
        tmpef |= (uint8_t)1 << OF_USEDEF_BIT_INDEX;
    }
    if (X86_EFLAGS_TEST_DF & ir1->info->detail->x86.eflags) {
        tmpef |= (uint8_t)1 << DF_USEDEF_BIT_INDEX;
    }
    return tmpef;
}
uint8_t ir1_get_eflag_inherent_def(IR1_INST *ir1)
{
    lsassert(0);
    uint8_t tmpef = 0;
    // some fpu ins not provide eflag info

    x86_insn op = ir1_opcode(ir1);
    if (op == X86_INS_FCOMI || op == X86_INS_FCOMIP || op == X86_INS_FUCOMI ||
        op == X86_INS_FUCOMIP || op == X86_INS_COMISS || op == X86_INS_COMISD ||
        op == X86_INS_UCOMISS || op == X86_INS_UCOMISD) {
        return (uint8_t)0x3f;
    }

    if ((X86_EFLAGS_MODIFY_CF | X86_EFLAGS_RESET_CF | X86_EFLAGS_SET_CF) &
        ir1->info->detail->x86.eflags) {
        tmpef |= (uint8_t)1 << (CF_USEDEF_BIT_INDEX);
    }
    if ((X86_EFLAGS_MODIFY_ZF | X86_EFLAGS_RESET_ZF | X86_EFLAGS_SET_ZF) &
        ir1->info->detail->x86.eflags) {
        tmpef |= (uint8_t)1 << (ZF_USEDEF_BIT_INDEX);
    }
    if ((X86_EFLAGS_MODIFY_AF | X86_EFLAGS_RESET_AF | X86_EFLAGS_SET_AF) &
        ir1->info->detail->x86.eflags) {
        tmpef |= (uint8_t)1 << (AF_USEDEF_BIT_INDEX);
    }
    if ((X86_EFLAGS_MODIFY_PF | X86_EFLAGS_RESET_PF | X86_EFLAGS_SET_PF) &
        ir1->info->detail->x86.eflags) {
        tmpef |= (uint8_t)1 << (PF_USEDEF_BIT_INDEX);
    }
    if ((X86_EFLAGS_MODIFY_SF | X86_EFLAGS_RESET_SF | X86_EFLAGS_SET_SF) &
        ir1->info->detail->x86.eflags) {
        tmpef |= (uint8_t)1 << (SF_USEDEF_BIT_INDEX);
    }
    if ((X86_EFLAGS_MODIFY_OF | X86_EFLAGS_RESET_OF | X86_EFLAGS_SET_OF) &
        ir1->info->detail->x86.eflags) {
        tmpef |= (uint8_t)1 << (OF_USEDEF_BIT_INDEX);
    }
    if ((X86_EFLAGS_MODIFY_DF | X86_EFLAGS_RESET_DF | X86_EFLAGS_SET_DF) &
        ir1->info->detail->x86.eflags) {
        tmpef |= (uint8_t)1 << (DF_USEDEF_BIT_INDEX);
    }
    return tmpef;
}
uint8_t ir1_get_eflag_inherent_undef(IR1_INST *ir1)
{
    lsassert(0);
    uint8_t tmpef = 0;
    if (X86_EFLAGS_UNDEFINED_CF & ir1->info->detail->x86.eflags) {
        tmpef |= (uint8_t)1 << (CF_USEDEF_BIT_INDEX);
    }
    if (X86_EFLAGS_UNDEFINED_ZF & ir1->info->detail->x86.eflags) {
        tmpef |= (uint8_t)1 << (ZF_USEDEF_BIT_INDEX);
    }
    if (X86_EFLAGS_UNDEFINED_AF & ir1->info->detail->x86.eflags) {
        tmpef |= (uint8_t)1 << (AF_USEDEF_BIT_INDEX);
    }
    if (X86_EFLAGS_UNDEFINED_PF & ir1->info->detail->x86.eflags) {
        tmpef |= (uint8_t)1 << (PF_USEDEF_BIT_INDEX);
    }
    if (X86_EFLAGS_UNDEFINED_SF & ir1->info->detail->x86.eflags) {
        tmpef |= (uint8_t)1 << (SF_USEDEF_BIT_INDEX);
    }
    if (X86_EFLAGS_UNDEFINED_OF & ir1->info->detail->x86.eflags) {
        tmpef |= (uint8_t)1 << (OF_USEDEF_BIT_INDEX);
    }

    return tmpef;
}

IR1_OPND *ir1_get_src_opnd(IR1_INST *ir1, int i)
{
    int src_index = -1;
    for (int j = 0; j < ir1_get_opnd_num(ir1); j++) {
        IR1_OPND *tmp = ir1_get_opnd(ir1, j);
        if ((tmp->access & CS_AC_READ) != 0 ||
            ir1_opnd_type(tmp) == X86_OP_IMM) {
            src_index++;
            if (src_index == i) {
                return tmp;
            }
        }
    }
    return ir1_get_src_opnd_implicit(ir1, i - src_index - 1);
}
IR1_OPND *ir1_get_dest_opnd(IR1_INST *ir1, int i)
{
    int dest_index = -1;
    for (int j = 0; j < ir1_get_opnd_num(ir1); j++) {
        IR1_OPND *tmp = ir1_get_opnd(ir1, j);
        if ((tmp->access & CS_AC_WRITE) != 0) {
            dest_index++;
            if (dest_index == i) {
                return tmp;
            }
        }
    }
    return ir1_get_dest_opnd_implicit(ir1, i - dest_index - 1);
}
IR1_OPND *ir1_get_src_opnd_implicit(IR1_INST *ir1, int i)
{
    switch (ir1_opcode(ir1)) {
    /* 1. no implicit operand, or only one implicit operand */
    case X86_INS_PUSH:
        return i == 0 ? &esp_ir1_opnd : NULL;
    case X86_INS_DAA:
        return i == 0 ? &al_ir1_opnd : NULL;
    case X86_INS_DAS:
        return i == 0 ? &al_ir1_opnd : NULL;
    case X86_INS_AAA:
        return i == 0 ? &ax_ir1_opnd : NULL;
    case X86_INS_AAS:
        return i == 0 ? &ax_ir1_opnd : NULL;
    case X86_INS_CBW:
        return i == 0 ? &al_ir1_opnd : NULL;
    case X86_INS_CWDE:
        return i == 0 ? &ax_ir1_opnd : NULL;
    case X86_INS_CDQE:
        return i == 0 ? &eax_ir1_opnd : NULL;
    case X86_INS_CWD:
        return i == 0 ? &ax_ir1_opnd : NULL;
    case X86_INS_CDQ:
        return i == 0 ? &eax_ir1_opnd : NULL;
    case X86_INS_CQO:
        return i == 0 ? &rax_ir1_opnd : NULL;
    case X86_INS_PUSHF:
        return i == 0 ? &esp_ir1_opnd : NULL;
    case X86_INS_SAHF:
        return i == 0 ? &ah_ir1_opnd : NULL;
    case X86_INS_ENTER:
        return i == 0 ? &esp_ir1_opnd : NULL;
    case X86_INS_AAM:
        return i == 0 ? &al_ir1_opnd : NULL;
    case X86_INS_AAD:
        return i == 0 ? &ax_ir1_opnd : NULL;
    case X86_INS_CALL:
        return i == 0 ? &esp_ir1_opnd : NULL;
    case X86_INS_RDMSR:
        return i == 0 ? &ecx_ir1_opnd : NULL;
    case X86_INS_RDPMC:
        return i == 0 ? &ecx_ir1_opnd : NULL;
    /* 2. more than one implicit operand */
    case X86_INS_POP:
    case X86_INS_POPF:
    case X86_INS_POPAL:
    case X86_INS_POPAW:
    case X86_INS_RET:
        if (i == 0) {
            return &esp_ir1_opnd;
        } else if (i == 1) {
            return &esp_mem32_ir1_opnd;  // POPA?
        } else {
            return NULL;
        }
    case X86_INS_IMUL:
    case X86_INS_MUL:
        if (ir1_opnd_num(ir1) == 1 && i == 0) {
            if (ir1_opnd_size(ir1_get_opnd(ir1, 0)) == 8) {
                return &al_ir1_opnd;
            } else if (ir1_opnd_size(ir1_get_opnd(ir1, 0)) == 16) {
                return &ax_ir1_opnd;
            } else if (ir1_opnd_size(ir1_get_opnd(ir1, 0)) == 32) {
                return &eax_ir1_opnd;
            } else {
                return &rax_ir1_opnd;
            }
        } else {
            return NULL;
        }
    case X86_INS_IDIV:
    case X86_INS_DIV:
        if (i == 0) {
            if (ir1_opnd_size(ir1_get_opnd(ir1, 0)) == 8) {
                return &ax_ir1_opnd;
            } else if (ir1_opnd_size(ir1_get_opnd(ir1, 0)) == 16) {
                return &ax_ir1_opnd;
            } else if (ir1_opnd_size(ir1_get_opnd(ir1, 0)) == 32) {
                return &eax_ir1_opnd;
            } else {
                return &rax_ir1_opnd;
            }
        } else if (i == 1) {
            if (ir1_opnd_size(ir1_get_opnd(ir1, 0)) == 8) {
                return NULL;
            } else if (ir1_opnd_size(ir1_get_opnd(ir1, 0)) == 16) {
                return &dx_ir1_opnd;
            } else if (ir1_opnd_size(ir1_get_opnd(ir1, 0)) == 32) {
                return &edx_ir1_opnd;
            } else {
                return &rdx_ir1_opnd;
            }
        } else {
            return NULL;
        }
    case X86_INS_LEAVE:
        if (i == 0) {
            return &ebp_ir1_opnd;
        } else if (i == 1) {
            return &ebp_mem32_ir1_opnd;
        } else {
            return NULL;
        }
    case X86_INS_CMPXCHG:
        if (i == 0) {
            if (ir1_opnd_size(ir1_get_opnd(ir1, 0)) == 8) {
                return &al_ir1_opnd;
            } else if (ir1_opnd_size(ir1_get_opnd(ir1, 0)) == 16) {
                return &ax_ir1_opnd;
            } else if (ir1_opnd_size(ir1_get_opnd(ir1, 0)) == 32) {
                return &eax_ir1_opnd;
            } else {
                return &rax_ir1_opnd;
            }
        } else {
            return NULL;
        }
    case X86_INS_CPUID:
        if (i == 0) {
            return &eax_ir1_opnd;
        } else if (i == 1) {
            return &ecx_ir1_opnd;
        } else {
            return NULL;
        }
    default:
        //lsassertm(
        //    t->_opcode_type == IR1_OPCODE_FPU ||
        //        t->_opcode_type == IR1_OPCODE_SSE,
        //    "the implicit source operand information is not available for %s\n",
        //    ir1_name(ir1_opcode(ir1)));
        return NULL;
    }
}

IR1_OPND *ir1_get_dest_opnd_implicit(IR1_INST *ir1, int i)
{
    switch (ir1_opcode(ir1)) {
    case X86_INS_POP:
        return i == 0 ? &esp_ir1_opnd : NULL;
    case X86_INS_DAA:
        return i == 0 ? &al_ir1_opnd : NULL;
    case X86_INS_DAS:
        return i == 0 ? &al_ir1_opnd : NULL;
    case X86_INS_AAA:
        return i == 0 ? &ax_ir1_opnd : NULL;
    case X86_INS_AAS:
        return i == 0 ? &ax_ir1_opnd : NULL;
    case X86_INS_CBW:
        return i == 0 ? &ax_ir1_opnd : NULL;
    case X86_INS_CWDE:
        return i == 0 ? &eax_ir1_opnd : NULL;
    case X86_INS_CDQE:
        return i == 0 ? &rax_ir1_opnd : NULL;
    case X86_INS_POPF:
        return i == 0 ? &esp_ir1_opnd : NULL;
    case X86_INS_LAHF:
        return i == 0 ? &ah_ir1_opnd : NULL;
    case X86_INS_RET:
        return i == 0 ? &esp_ir1_opnd : NULL;
    case X86_INS_AAM:
        return i == 0 ? &ax_ir1_opnd : NULL;
    case X86_INS_AAD:
        return i == 0 ? &ax_ir1_opnd : NULL;

    case X86_INS_PUSH:
    case X86_INS_PUSHF:
    case X86_INS_PUSHAL:
    case X86_INS_PUSHAW:
    case X86_INS_CALL:
        if (i == 0) {
            return &esp_ir1_opnd;
        } else if (i == 1) {
            return &esp_mem32_ir1_opnd;
        } else {
            return NULL;
        }
    case X86_INS_IMUL:
    case X86_INS_MUL:
        if (i == 0) {
            if (ir1_opnd_size(ir1_get_opnd(ir1, 0)) == 8) {
                return &ax_ir1_opnd;
            } else if (ir1_opnd_size(ir1_get_opnd(ir1, 0)) == 16) {
                return &ax_ir1_opnd;
            } else if (ir1_opnd_size(ir1_get_opnd(ir1, 0)) == 32) {
                return &eax_ir1_opnd;
            } else {
                return &rax_ir1_opnd;
            }
        } else if (i == 1) {
            if (ir1_opnd_size(ir1_get_opnd(ir1, 0)) == 8) {
                return NULL;
            } else if (ir1_opnd_size(ir1_get_opnd(ir1, 0)) == 16) {
                return &dx_ir1_opnd;
            } else if (ir1_opnd_size(ir1_get_opnd(ir1, 0)) == 32) {
                return &edx_ir1_opnd;
            } else {
                return &rdx_ir1_opnd;
            }
        } else {
            return NULL;
        }
    case X86_INS_IDIV:
    case X86_INS_DIV:
        if (i == 0) {
            if (ir1_opnd_size(ir1_get_opnd(ir1, 0)) == 8) {
                return &ax_ir1_opnd;
            } else if (ir1_opnd_size(ir1_get_opnd(ir1, 0)) == 16) {
                return &ax_ir1_opnd;
            } else if (ir1_opnd_size(ir1_get_opnd(ir1, 0)) == 32) {
                return &eax_ir1_opnd;
            } else {
                return &rax_ir1_opnd;
            }
        } else if (i == 1) {
            if (ir1_opnd_size(ir1_get_opnd(ir1, 0)) == 8) {
                return NULL;
            } else if (ir1_opnd_size(ir1_get_opnd(ir1, 0)) == 16) {
                return &dx_ir1_opnd;
            } else if (ir1_opnd_size(ir1_get_opnd(ir1, 0)) == 32) {
                return &edx_ir1_opnd;
            } else {
                return &rdx_ir1_opnd;
            }
        } else {
            return NULL;
        }
    case X86_INS_LEAVE:
        if (i == 0) {
            return &esp_ir1_opnd;
        } else if (i == 1) {
            return &ebp_ir1_opnd;
        } else {
            return NULL;
        }
    case X86_INS_CMPXCHG:
        if (i == 0) {
            if (ir1_opnd_size(ir1_get_opnd(ir1, 0)) == 8) {
                return &al_ir1_opnd;
            } else if (ir1_opnd_size(ir1_get_opnd(ir1, 0)) == 16) {
                return &ax_ir1_opnd;
            } else if (ir1_opnd_size(ir1_get_opnd(ir1, 0)) == 32) {
                return &eax_ir1_opnd;
            } else {
                return &rax_ir1_opnd;
            }
        } else {
            return NULL;
        }
    case X86_INS_RDTSC:
        if (i == 0) {
            return &eax_ir1_opnd;
        } else if (i == 1) {
            return &edx_ir1_opnd;
        } else {
            return NULL;
        }
    case X86_INS_CPUID:
        if (i == 0) {
            return &eax_ir1_opnd;
        } else if (i == 1) {
            return &ebx_ir1_opnd;
        } else if (i == 2) {
            return &ecx_ir1_opnd;
        } else if (i == 3) {
            return &edx_ir1_opnd;
        } else {
            return NULL;
        }
    case X86_INS_CDQ:
        if (i == 0) {
            return NULL;
        } else if (i == 1) {
            return &edx_ir1_opnd;
        } else {
            return NULL;
        }
    default:
        // lsassertm(
        //     t->_opcode_type == IR1_OPCODE_FPU ||
        //         t->_opcode_type == IR1_OPCODE_SSE,
        //     "the implicit dest operand information is not available for
        //     %s\n", ir1_name(ir1_opcode(ir1)));
        return NULL;
    }
    //     {IR1_OPCODE_NONE, 1, 0, __esp, -1},   /* x86_pop implicit operand */
    //     {IR1_OPCODE_NONE, 0, 0, __al, __al},  /* x86_daa implicit operand */
    //     {IR1_OPCODE_NONE, 0, 0, __al, __al},  /* x86_das implicit operand */
    //     {IR1_OPCODE_NONE, 0, 0, __ax, __ax},  /* x86_aaa implicit operands,
    //     AL/AH */ {IR1_OPCODE_NONE, 0, 0, __ax, __ax},  /* x86_aas implicit
    //     operands, AL/AH */ {IR1_OPCODE_NONE, 0, 0, __ax, __al},  /* x86_cbw
    //     implicit operands */ {IR1_OPCODE_NONE, 0, 0, __eax, __ax}, /*
    //     x86_cwde implicit operands */ {IR1_OPCODE_NONE, 0, 0, __rax, __eax},
    //     /* x86_cdqe implicit operands */ {IR1_OPCODE_NONE, 0, 0, __esp, -1},
    //     /* x86_popf implicit operands */ {IR1_OPCODE_NONE, 0, 0, __ah, 0}, /*
    //     x86_lahf implicit operands */ {IR1_OPCODE_RET, 0, 0, __esp, -1}, /*
    //     x86_ret implicit operands */ {IR1_OPCODE_NONE, 0, 0, __ax, __al}, /*
    //     x86_aam implicit operands */ {IR1_OPCODE_NONE, 0, 0, __ax, __ax}, /*
    //     x86_aad implicit operands */
}

void ir1_make_ins_JMP(IR1_INST *ir1, ADDRX addr, int32 off)
{
    cs_insn *info = cs_malloc(handle);

    info->bytes[0] = 0x69;
    *(int32_t *)(info->bytes + 1) = off;
    info->address = (uint64_t)addr;
    info->size = 5;
    info->detail->x86.opcode[0] = 0xe9;
    info->detail->x86.addr_size = 4;
    info->detail->x86.op_count = 1;
    info->detail->x86.operands[0].type = X86_OP_IMM;
    info->detail->x86.operands[0].imm = (int64_t)off + (int64_t)addr;
    info->detail->x86.operands[0].size = 4;
    info->id = X86_INS_JMP;
    // TODO : other field in ir1 and detail->x86
    // another way : use capstone to disasm 0xe9 | off
    ir1->info = info;
}

void ir1_make_ins_NOP(IR1_INST *ir1, ADDRX addr) { lsassert(0); }
void ir1_make_ins_RET(IR1_INST *ir1, ADDRX addr) { lsassert(0); }
void ir1_make_ins_LIBFUNC(IR1_INST *ir1, ADDRX addr) { lsassert(0); }

int ir1_dump(IR1_INST *ir1)
{
    fprintf(stderr, "0x%" PRIx64 ":\t%s\t\t%s\n", ir1->info->address,
            ir1->info->mnemonic, ir1->info->op_str);
    return 0;
}

int ir1_opcode_dump(IR1_INST *ir1)
{
    fprintf(stderr, "0x%" PRIx64 ":\t%s\n", ir1->info->address,
            ir1->info->mnemonic);
    return 0;
}

const char * ir1_name(IR1_OPCODE op){
    return cs_insn_name(handle, op);
}

const char * ir1_group_name(x86_insn_group grp){
    return cs_group_name(handle, grp);
}

uint8_t ir1_get_opnd_num(IR1_INST *ir1)
{
    return ir1->info->detail->x86.op_count;
}

IR1_OPND *ir1_get_opnd(IR1_INST *ir1, int i)
{
    lsassert(i <= ir1->info->detail->x86.op_count);
    return &(ir1->info->detail->x86.operands[i]);
}

bool ir1_is_indirect_call(IR1_INST *ir1)
{
    return !ir1_opnd_is_imm(ir1_get_opnd(ir1, 0));
}
bool ir1_is_indirect_jmp(IR1_INST *ir1)
{
    return !ir1_opnd_is_imm(ir1_get_opnd(ir1, 0));
}

bool ir1_is_prefix_lock(IR1_INST *ir1)
{
    return ir1->info->detail->x86.prefix[0] == X86_PREFIX_LOCK;
}

const char *ir1_reg_name(x86_reg reg) { return cs_reg_name(handle, reg); }
