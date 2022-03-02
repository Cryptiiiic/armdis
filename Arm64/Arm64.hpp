//
// Created by cryptic on 10/14/21.
//

#ifndef __ARM_HPP__
#define __ARM_HPP__
#include "../Global.hpp"

class Arm64 {
private:
    std::vector<uint32_t> &_binaryData;

    std::size_t _binarySize;

    std::unordered_map<uint32_t, int> _instructionNames;

    std::unordered_map<uint32_t, std::vector<uint32_t>> _instructions;

    int _instructionCount = 27;

public:
    Arm64(std::vector<uint32_t> &binaryData, std::size_t binarySize);
    void initInstructions();
    void hexPrint(int index);
    void hexDump(int amount);
    int getInstructionType(uint64_t address);
    uint64_t getInstructionImm(int type, uint64_t address);
    uint64_t getInstructionXd(int type, uint64_t address);
    uint64_t getInstructionXn(int type, uint64_t address);
    void printDisassembly(uint64_t address);
    void printAllDisassembly(int amount);
    enum instructions {
        add_im_insn = 0,
        add_ex_insn,
        adr_insn,
        adrp_insn,
        and_im_insn,
        and_sh_insn,
        ands_im_insn,
        b_co_insn,
        b_insn,
        bl_insn,
        blr_insn,
        br_insn,
        cmp_ex_insn,
        ldp_insn,
        ldr_im_insn,
        ldr_li_insn,
        mov_re_insn,
        mov_wi_insn,
        msr_im_insn,
        msr_re_insn,
        nop_insn,
        ret_insn,
        stp_insn,
        sub_im_insn,
        subs_im_insn,
        subs_sh_insn,
        unknown_insn,
    };
    std::array<std::string, 27> instructionNames {
        std::string("ADD"),
        std::string("ADD"),
        std::string("ADR"),
        std::string("ADRP"),
        std::string("AND"),
        std::string("AND"),
        std::string("ANDS"),
        std::string("B"),
        std::string("B"),
        std::string("BL"),
        std::string("BLR"),
        std::string("BR"),
        std::string("CMP"),
        std::string("LDP"),
        std::string("LDR"),
        std::string("LDR"),
        std::string("MOV"),
        std::string("MOV"),
        std::string("MSR"),
        std::string("MSR"),
        std::string("NOP"),
        std::string("RET"),
        std::string("STP"),
        std::string("SUB"),
        std::string("SUBS"),
        std::string("SUBS"),
        std::string("UNKNOWN"),
    };
    std::array<std::string, 27> instructionNamesDebug {
        std::string("ADD (immediate)"),
        std::string("ADD (extended register)"),
        std::string("ADR ()"),
        std::string("ADRP ()"),
        std::string("AND (immediate)"),
        std::string("AND (shifted register)"),
        std::string("ANDS (immediate)"),
        std::string("B.cond (conditional)"),
        std::string("B (unconditional)"),
        std::string("BL ()"),
        std::string("BLR ()"),
        std::string("BR ()"),
        std::string("CMP (extended register)"),
        std::string("LDP ()"),
        std::string("LDR (immediate)"),
        std::string("LDR (literal)"),
        std::string("MOV (register)"),
        std::string("MOV (wide immediate)"),
        std::string("MSR (immediate)"),
        std::string("MSR (register)"),
        std::string("NOP (no operation"),
        std::string("RET ()"),
        std::string("STP ()"),
        std::string("SUB (immediate)"),
        std::string("SUBS (immediate)"),
        std::string("SUBS (shifted register)"),
        std::string("UNKNOWN ()"),
    };
    enum instructionResults {
        add_im_res = 0x11000000,
        add_ex_res = 0x0B000000,
        adr_res = 0x10000000,
        adrp_res = 0x90000000,
        and_im_res = 0x12000000,
        and_sh_res = 0x0A000000,
        ands_im_res = 0x72000000,
        b_co_res = 0x54000000,
        b_res = 0x14000000,
        bl_res = 0x94000000,
        blr_res = 0xD63F0000,
        br_res = 0xD61F0000,
        cmp_ex_res = 0x7F80001F,
        ldp_res = 0x28C00000,
        ldr_im_res = 0x18000400,
        ldr_li_res = 0x18000000,
        mov_re_res = 0x2A0003E0,
        mov_wi_res = 0x52800000,
        msr_im_res = 0xD500401F,
        msr_re_res = 0xD5180000,
        nop_res = 0xD503201F,
        ret_res = 0xD65F0000,
        stp_res = 0x28800000,
        sub_im_res = 0x4B000000,
        subs_im_res = 0x71000000,
        subs_sh_res = 0x6B000000,
    };
    enum instructionMasks {
        add_im_and_im_ands_im_mov_wi_sub_im_subs_im_mask = 0x7F800000,
        add_ex_mask = 0x7FE00000,
        adr_adrp_mask = 0x9F000000,
        and_sh_subs_sh_mask = 0x7F200000,
        b_co_mask = 0xFF000010,
        b_bl_mask = 0xFC000000,
        br_blr_ret_mask = 0xFFFFFC1F,
        cmp_ex_mask = 0x6B00001F,
        ldp_mask = 0x79C00000,
        ldr_im_mask = 0xBFE00C00,
        ldr_li_mask = 0xBF000000,
        mov_re_mask = 0x7FE0FFE0,
        msr_im_mask = 0xFFF8F01F,
        msr_re_mask = 0xFFF80000,
        nop_mask = 0xFFFFFFFF,
        stp_mask = 0x7FC00000,
    };
};

#endif // __ARM_HPP__
