#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
#include <errno.h>

#include "debug.h"
#include "box64context.h"
#include "dynarec.h"
#include "emu/x64emu_private.h"
#include "emu/x64run_private.h"
#include "x64run.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynarec_arm64.h"
#include "dynarec_arm64_private.h"
#include "arm64_printer.h"
#include "../tools/bridge_private.h"

#include "dynarec_arm64_functions.h"
#include "dynarec_arm64_helper.h"

// emit CMP32 instruction, from cmp s1 , s2, using s3 and s4 as scratch
//void emit_cmp32(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
//{
//    IFX(X_PEND) {
//        STR_IMM9(s1, xEmu, offsetof(x64emu_t, op1));
//        STR_IMM9(s2, xEmu, offsetof(x64emu_t, op2));
//        SET_DF(s4, d_cmp32);
//    } else {
//        SET_DFNONE(s4);
//    }
//    SUBS_REG_LSL_IMM5(s3, s1, s2, 0);   // res = s1 - s2
//    IFX(X_PEND) {
//        STR_IMM9(s3, xEmu, offsetof(x64emu_t, res));
//    }
//    IFX(X_ZF|X_CF) {
//        BIC_IMM8(xFlags, xFlags, (1<<F_ZF)|(1<<F_CF), 0);
//    }
//    IFX(X_ZF) {
//        ORR_IMM8_COND(cEQ, xFlags, xFlags, 1<<F_ZF, 0);
//    }
//    IFX(X_OF) {
//        ORR_IMM8_COND(cVS, xFlags, xFlags, 0b10, 0x0b);
//        BIC_IMM8_COND(cVC, xFlags, xFlags, 0b10, 0x0b);
//    }
//    IFX(X_CF) {
//        // reversed carry
//        ORR_IMM8_COND(cCC, xFlags, xFlags, 1<<F_CF, 0);
//    }
//    IFX(X_SF) {
//        MOV_REG_LSR_IMM5(s4, s3, 31);
//        BFI(xFlags, s4, F_SF, 1);
//    }
//    // and now the tricky ones (and mostly unused), PF and AF
//    IFX(X_AF) {
//        // bc = (res & (~d | s)) | (~d & s)
//        MVN_REG_LSL_IMM5(s4, s1, 0);        // s4 = ~d
//        ORR_REG_LSL_IMM5(s4, s4, s2, 0);    // s4 = ~d | s
//        AND_REG_LSL_IMM5(s4, s4, s3, 0);    // s4 = res & (~d | s)
//        BIC_REG_LSL_IMM5(s3, s2, s1, 0);    // loosing res... s3 = s & ~d
//        ORR_REG_LSL_IMM5(s3, s4, s3, 0);    // s3 = (res & (~d | s)) | (s & ~d)
//        IFX(X_AF) {
//            MOV_REG_LSR_IMM5(s4, s3, 3);
//            BFI(xFlags, s4, F_AF, 1);    // AF: bc & 0x08
//        }
//    }
//    IFX(X_PF) {
//        // PF: (((emu->x64emu_parity_tab[(res) / 32] >> ((res) % 32)) & 1) == 0)
//        IFX(X_CF|X_AF) {
//            SUB_REG_LSL_IMM5(s3, s1, s2, 0);
//        }
//        AND_IMM8(s3, s3, 0xE0); // lsr 5 masking pre-applied
//        MOV32(s4, GetParityTab());
//        LDR_REG_LSR_IMM5(s4, s4, s3, 5-2);   // x/32 and then *4 because array is integer
//        SUB_REG_LSL_IMM5(s3, s1, s2, 0);
//        AND_IMM8(s3, s3, 31);
//        MVN_REG_LSR_REG(s4, s4, s3);
//        BFI(xFlags, s4, F_PF, 1);
//    }
//}

// emit CMP32 instruction, from cmp s1 , 0, using s3 and s4 as scratch
//void emit_cmp32_0(dynarec_arm_t* dyn, int ninst, int s1, int s3, int s4)
//{
//    IFX(X_PEND) {
//        MOVW(s4, 0);
//        STR_IMM9(s1, xEmu, offsetof(x64emu_t, op1));
//        STR_IMM9(s4, xEmu, offsetof(x64emu_t, op2));
//        STR_IMM9(s1, xEmu, offsetof(x64emu_t, res));
//        SET_DF(s4, d_cmp32);
//    } else {
//        SET_DFNONE(s4);
//    }
//    SUBS_IMM8(s3, s1, 0);   // res = s1 - 0
//    // and now the tricky ones (and mostly unused), PF and AF
//    // bc = (res & (~d | s)) | (~d & s) => is 0 here...
//    IFX(X_CF | X_AF | X_ZF) {
//        BIC_IMM8(xFlags, xFlags, (1<<F_CF)|(1<<F_AF)|(1<<F_ZF), 0);
//    }
//    IFX(X_OF) {
//        BFC(xFlags, F_OF, 1);
//    }
//    IFX(X_ZF) {
//        ORR_IMM8_COND(cEQ, xFlags, xFlags, 1<<F_ZF, 0);
//    }
//    IFX(X_SF) {
//        MOV_REG_LSR_IMM5(s4, s3, 31);
//        BFI(xFlags, s4, F_SF, 1);
//    }
//    IFX(X_PF) {
//        emit_pf(dyn, ninst, s1, s3, s4);
//    }
//}

// emit CMP16 instruction, from cmp s1 , s2, using s3 and s4 as scratch
//void emit_cmp16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
//{
//    IFX(X_PEND) {
//        STR_IMM9(s1, xEmu, offsetof(x64emu_t, op1));
//        STR_IMM9(s2, xEmu, offsetof(x64emu_t, op2));
//        SET_DF(s3, d_cmp16);
//    } else {
//        SET_DFNONE(s3);
//    }
//    SUB_REG_LSL_IMM5(s3, s1, s2, 0);   // res = s1 - s2
//    IFX(X_PEND) {
//        STR_IMM9(s3, xEmu, offsetof(x64emu_t, res));
//    }
//    IFX(X_ZF) {
//        MOVW(s4, 0xffff);
//        TSTS_REG_LSL_IMM5(s3, s4, 0);
//        ORR_IMM8_COND(cEQ, xFlags, xFlags, 1<<F_ZF, 0);
//        BIC_IMM8_COND(cNE, xFlags, xFlags, 1<<F_ZF, 0);
//    }
//    IFX(X_SF) {
//        MOV_REG_LSR_IMM5(s4, s3, 15);
//        BFI(xFlags, s4, F_SF, 1);
//    }
//    // bc = (res & (~d | s)) | (~d & s)
//    IFX(X_CF|X_AF|X_OF) {
//        MVN_REG_LSL_IMM5(s4, s1, 0);        // s4 = ~d
//        ORR_REG_LSL_IMM5(s4, s4, s2, 0);    // s4 = ~d | s
//        AND_REG_LSL_IMM5(s4, s4, s3, 0);    // s4 = res & (~d | s)
//        BIC_REG_LSL_IMM5(s3, s2, s1, 0);    // loosing res... s3 = s & ~d
//        ORR_REG_LSL_IMM5(s3, s4, s3, 0);    // s3 = (res & (~d | s)) | (s & ~d)
//        IFX(X_CF) {
//            MOV_REG_LSR_IMM5(s4, s3, 15);
//            BFI(xFlags, s4, F_CF, 1);    // CF : bc & 0x8000
//        }
//        IFX(X_AF) {
//            MOV_REG_LSR_IMM5(s4, s3, 3);
//            BFI(xFlags, s4, F_AF, 1);    // AF: bc & 0x08
//        }
//        IFX(X_OF) {
//            MOV_REG_LSR_IMM5(s4, s3, 14);
//            XOR_REG_LSR_IMM8(s4, s4, s4, 1);
//            BFI(xFlags, s4, F_OF, 1);    // OF: ((bc >> 14) ^ ((bc>>14)>>1)) & 1
//        }
//    }
//    IFX(X_PF) {
//        IFX(X_CF|X_AF|X_OF) {
//            SUB_REG_LSL_IMM5(s3, s1, s2, 0);
//        }
//        AND_IMM8(s3, s3, 0xE0); // lsr 5 masking pre-applied
//        MOV32(s4, GetParityTab());
//        LDR_REG_LSR_IMM5(s4, s4, s3, 5-2);   // x/32 and then *4 because array is integer
//        SUB_REG_LSL_IMM5(s3, s1, s2, 0);
//        AND_IMM8(s3, s3, 31);
//        MVN_REG_LSR_REG(s4, s4, s3);
//        BFI(xFlags, s4, F_PF, 1);
//    }
//}

// emit CMP16 instruction, from cmp s1 , #0, using s3 and s4 as scratch
//void emit_cmp16_0(dynarec_arm_t* dyn, int ninst, int s1, int s3, int s4)
//{
//    IFX(X_PEND) {
//        MOVW(s3, 0);
//        STR_IMM9(s1, xEmu, offsetof(x64emu_t, op1));
//        STR_IMM9(s3, xEmu, offsetof(x64emu_t, op2));
//        STR_IMM9(s1, xEmu, offsetof(x64emu_t, res));
//        SET_DF(s3, d_cmp16);
//    } else {
//        SET_DFNONE(s3);
//    }
//    // bc = (res & (~d | s)) | (~d & s) = 0
//    IFX(X_CF | X_AF | X_ZF) {
//        BIC_IMM8(xFlags, xFlags, (1<<F_CF)|(1<<F_AF)|(1<<F_ZF), 0);
//    }
//    IFX(X_OF) {
//        BFC(xFlags, F_OF, 1);
//    }
//    IFX(X_ZF) {
//        MOVW(s4, 0xffff);
//        TSTS_REG_LSL_IMM5(s1, s4, 0);
//        ORR_IMM8_COND(cEQ, xFlags, xFlags, 1<<F_ZF, 0);
//    }
//    IFX(X_SF) {
//        MOV_REG_LSR_IMM5(s4, s1, 15);
//        BFI(xFlags, s4, F_SF, 1);
//    }
//    IFX(X_PF) {
//        emit_pf(dyn, ninst, s1, s3, s4);
//    }
//}
// emit CMP8 instruction, from cmp s1 , s2, using s3 and s4 as scratch
//void emit_cmp8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
//{
//    IFX(X_PEND) {
//        STR_IMM9(s1, xEmu, offsetof(x64emu_t, op1));
//        STR_IMM9(s2, xEmu, offsetof(x64emu_t, op2));
//        SET_DF(s4, d_cmp8);
//    } else {
//        SET_DFNONE(s4);
//    }
//    SUB_REG_LSL_IMM5(s3, s1, s2, 0);   // res = s1 - s2
//    IFX(X_PEND) {
//        STR_IMM9(s3, xEmu, offsetof(x64emu_t, res));
//    }
//    IFX(X_ZF) {
//        TSTS_IMM8(s3, 0xff);
//        ORR_IMM8_COND(cEQ, xFlags, xFlags, 1<<F_ZF, 0);
//        BIC_IMM8_COND(cNE, xFlags, xFlags, 1<<F_ZF, 0);
//    }
//    IFX(X_SF) {
//        MOV_REG_LSR_IMM5(s4, s3, 7);
//        BFI(xFlags, s4, F_SF, 1);
//    }
//    // bc = (res & (~d | s)) | (~d & s)
//    IFX(X_CF|X_AF|X_OF) {
//        MVN_REG_LSL_IMM5(s4, s1, 0);        // s4 = ~d
//        ORR_REG_LSL_IMM5(s4, s4, s2, 0);    // s4 = ~d | s
//        AND_REG_LSL_IMM5(s4, s4, s3, 0);    // s4 = res & (~d | s)
//        BIC_REG_LSL_IMM5(s3, s2, s1, 0);    // loosing res... s3 = s & ~d
//        ORR_REG_LSL_IMM5(s3, s4, s3, 0);    // s3 = (res & (~d | s)) | (s & ~d)
//        IFX(X_CF) {
//            MOV_REG_LSR_IMM5(s4, s3, 7);
//            BFI(xFlags, s4, F_CF, 1);    // CF : bc & 0x80
//        }
//        IFX(X_AF) {
//            MOV_REG_LSR_IMM5(s4, s3, 3);
//            BFI(xFlags, s4, F_AF, 1);    // AF: bc & 0x08
//        }
//        IFX(X_OF) {
//            MOV_REG_LSR_IMM5(s4, s3, 6);
//            XOR_REG_LSR_IMM8(s4, s4, s4, 1);
//            BFI(xFlags, s4, F_OF, 1);    // OF: ((bc >> 6) ^ ((bc>>6)>>1)) & 1
//        }
//    }
//    IFX(X_PF) {
//        IFX(X_CF|X_AF|X_OF) {
//            SUB_REG_LSL_IMM5(s3, s1, s2, 0);
//        }
//        AND_IMM8(s3, s3, 0xE0); // lsr 5 masking pre-applied
//        MOV32(s4, GetParityTab());
//        LDR_REG_LSR_IMM5(s4, s4, s3, 5-2);   // x/32 and then *4 because array is integer
//        SUB_REG_LSL_IMM5(s3, s1, s2, 0);
//        AND_IMM8(s3, s3, 31);
//        MVN_REG_LSR_REG(s4, s4, s3);
//        BFI(xFlags, s4, F_PF, 1);
//    }
//}
// emit CMP8 instruction, from cmp s1 , 0, using s3 and s4 as scratch
//void emit_cmp8_0(dynarec_arm_t* dyn, int ninst, int s1, int s3, int s4)
//{
//    IFX(X_PEND) {
//        STR_IMM9(s1, xEmu, offsetof(x64emu_t, op1));
//        MOVW(s4, 0);
//        STR_IMM9(s4, xEmu, offsetof(x64emu_t, op2));
//        STR_IMM9(s1, xEmu, offsetof(x64emu_t, res));
//        SET_DF(s3, d_cmp8);
//    } else {
//        SET_DFNONE(s4);
//    }
//    // bc = (res & (~d | s)) | (~d & s) = 0
//    IFX(X_CF | X_AF | X_ZF) {
//        BIC_IMM8(xFlags, xFlags, (1<<F_CF)|(1<<F_AF)|(1<<F_ZF), 0);
//    }
//    IFX(X_OF) {
//        BFC(xFlags, F_OF, 1);
//    }
//    IFX(X_ZF) {
//        TSTS_IMM8(s1, 0xff);
//        ORR_IMM8_COND(cEQ, xFlags, xFlags, 1<<F_ZF, 0);
//    }
//    IFX(X_SF) {
//        MOV_REG_LSR_IMM5(s4, s1, 7);
//        BFI(xFlags, s4, F_SF, 1);
//    }
//    IFX(X_PF) {
//        emit_pf(dyn, ninst, s1, s3, s4);
//    }
//}

// emit TEST32 instruction, from test s1 , s2, using s3 and s4 as scratch
void emit_test32(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4)
{
    IFX(X_PEND) {
        SET_DF(s3, rex.w?d_tst64:d_tst32);
    } else {
        SET_DFNONE(s4);
    }
    IFX(X_ZF|X_CF|X_OF) {
        MOV32w(s3, (1<<F_ZF)|(1<<F_CF)|(1<<F_OF));
        BICw(xFlags, xFlags, s3);
    }
    ANDSxw_REG(s3, s1, s2);   // res = s1 & s2
    IFX(X_PEND) {
        STRxw_U12(s3, xEmu, offsetof(x64emu_t, res));
    }
    IFX(X_ZF) {
        Bcond(cNE, +8);
        ORRw_mask(xFlags, xFlags, 0b011010, 0); // mask=0x40
    }
    IFX(X_SF) {
        LSRxw(s4, s3, rex.w?63:31);
        BFIw(xFlags, s4, F_SF, 1);
    }
    // PF: (((emu->x64emu_parity_tab[(res) / 32] >> ((res) % 32)) & 1) == 0)
    IFX(X_PF) {
        ANDw_mask(s3, s3, 0b011011, 000010); // 0xE0
        LSRw(s3, s3, 5);
        MOV64x(s4, (uintptr_t)GetParityTab());
        LDRw_REG_LSL2(s4, s4, s3);
        ANDw_mask(s3, s1, 0, 0b000100);   // 0x1f
        LSRw_REG(s4, s4, s3);
        MVNx_REG(s4, s4);
        BFIw(xFlags, s4, F_PF, 1);
    }
}

// emit TEST16 instruction, from test s1 , s2, using s3 and s4 as scratch
//void emit_test16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
//{
//    IFX(X_PEND) {
//        SET_DF(s3, d_tst16);
//    } else {
//        SET_DFNONE(s4);
//    }
//    IFX(X_OF) {
//        BFC(xFlags, F_OF, 1);
//    }
//    IFX(X_ZF|X_CF) {
//        BIC_IMM8(xFlags, xFlags, (1<<F_ZF)|(1<<F_CF), 0);
//    }
//    ANDS_REG_LSL_IMM5(s3, s1, s2, 0);   // res = s1 & s2
//    IFX(X_PEND) {
//        STR_IMM9(s3, xEmu, offsetof(x64emu_t, res));
//    }
//    IFX(X_ZF) {
//        ORR_IMM8_COND(cEQ, xFlags, xFlags, 1<<F_ZF, 0);
//    }
//    IFX(X_SF) {
//        MOV_REG_LSR_IMM5(s4, s3, 15);
//        BFI(xFlags, s4, F_SF, 1);
//    }
//    // PF: (((emu->x64emu_parity_tab[(res) / 32] >> ((res) % 32)) & 1) == 0)
//    IFX(X_PF) {
//        AND_IMM8(s3, s3, 0xE0); // lsr 5 masking pre-applied
//        MOV32(s4, GetParityTab());
//        LDR_REG_LSR_IMM5(s4, s4, s3, 5-2);   // x/32 and then *4 because array is integer
//        AND_REG_LSL_IMM5(s3, s1, s2, 0);
//        AND_IMM8(s3, s3, 31);
//        MVN_REG_LSR_REG(s4, s4, s3);
//        BFI(xFlags, s4, F_PF, 1);
//    }
//}

// emit TEST8 instruction, from test s1 , s2, using s3 and s4 as scratch
//void emit_test8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
//{
//    IFX(X_PEND) {
//        SET_DF(s3, d_tst8);
//    } else {
//        SET_DFNONE(s4);
//    }
//    IFX(X_OF) {
//        BFC(xFlags, F_OF, 1);
//    }
//    IFX(X_ZF|X_CF) {
//        BIC_IMM8(xFlags, xFlags, (1<<F_ZF)|(1<<F_CF), 0);
//    }
//    ANDS_REG_LSL_IMM5(s3, s1, s2, 0);   // res = s1 & s2
//    IFX(X_PEND) {
//        STR_IMM9(s3, xEmu, offsetof(x64emu_t, res));
//    }
//    IFX(X_ZF) {
//        ORR_IMM8_COND(cEQ, xFlags, xFlags, 1<<F_ZF, 0);
//    }
//    IFX(X_SF) {
//        MOV_REG_LSR_IMM5(s4, s3, 7);
//        BFI(xFlags, s4, F_SF, 1);
//    }
//    // PF: (((emu->x64emu_parity_tab[(res) / 32] >> ((res) % 32)) & 1) == 0)
//    IFX(X_PF) {
//        AND_IMM8(s3, s3, 0xE0); // lsr 5 masking pre-applied
//        MOV32(s4, GetParityTab());
//        LDR_REG_LSR_IMM5(s4, s4, s3, 5-2);   // x/32 and then *4 because array is integer
//        AND_REG_LSL_IMM5(s3, s1, s2, 0);
//        AND_IMM8(s3, s3, 31);
//        MVN_REG_LSR_REG(s4, s4, s3);
//        BFI(xFlags, s4, F_PF, 1);
//    }
//}