//
// Created by cryptic on 10/20/21.
//

#ifndef __INSTRUCTIONSET_HPP__
#define __INSTRUCTIONSET_HPP__
#include "../Global.hpp"

namespace Arm64 {

    #define INSTRUCTION_COUNT 43
    #define VARIANT_COUNT 17

    enum instructionTypes {
        ADD_INSN = 0,
        ADDS_INSN,
        ADR_INSN,
        ADRP_INSN,
        AND_INSN,
        ANDS_INSN,
        B_INSN,
        BL_INSN,
        BLR_INSN,
        BR_INSN,
        CMP_INSN,
        LDP_INSN,
        LDR_INSN,
        MOV_INSN,
        MOVK_INSN,
        MOVN_INSN,
        MOVZ_INSN,
        MRS_INSN,
        MSR_INSN,
        MSUB_INSN,
        MUL_INSN,
        MVN_INSN,
        NOP_INSN,
        ORR_INSN,
        ORR__INSN,
        RET_INSN,
        RETAA_INSN,
        RETAB_INSN,
        ROR_INSN,
        STP_INSN,
        STR_INSN,
        STRB_INSN,
        STRH_INSN,
        STUR_INSN,
        STURB_INSN,
        STURH_INSN,
        SUB_INSN,
        SUBS_INSN,
        SVC_INSN,
        SYS_INSN,
        TBNZ_INSN,
        TBZ_INSN,
        UNKNOWN_INSN,
    };

    enum instructionVariants {
        NONE_VART = 0,
        IMMEDIATE_VART,
        REGISTER_VART,
        REGISTER_SHIFTED_REGISTER_VART,
        SP_PLUS_IMMEDIATE_VART,
        SP_PLUS_REGISTER_VART,
        IMMEDIATE_TO_PC_VART,
        EXTENDED_REGISTER_VART,
        SHIFTED_REGISTER_VART,
        CONDITIONAL_VART,
        UNCONDITIONAL_VART,
        LITERAL_VART,
        SP_VART,
        INVERTED_WIDE_IMMEDIATE_VART,
        BITMASK_WIDE_IMMEDIATE_VART,
        WIDE_IMMEDIATE_VART,
        UNKNOWN_VART,
    };

    enum instructionMasks {
        M_7F200000_MASK = 0x7F200000,
        M_7F800000_MASK = 0x7F800000,
        M_7FE00000_MASK = 0x7FE00000,
        M_7FE0000F_MASK = 0x7FE0000F,
        M_9F000000_MASK = 0x9F000000,
        M_FC000000_MASK = 0xFC000000,
        M_FF000000_MASK = 0xFF000000,
        M_UNKOWN_MASK = 0x69696969,
    };

    enum instructionResults {
        R_0A000000_RESULT = 0x0A000000,
        R_0B000000_RESULT = 0x0B000000,
        R_0B200000_RESULT = 0x0B200000,
        R_10000000_RESULT = 0x10000000,
        R_11000000_RESULT = 0x11000000,
        R_31000000_RESULT = 0x31000000,
        R_58000000_RESULT = 0x58000000,
        R_6B20000F_RESULT = 0x6B20000F,
        R_90000000_RESULT = 0x90000000,
        R_94000000_RESULT = 0x94000000,
        R_UNKOWN_RESULT = 0x69696969,
    };

    extern std::array<std::string, INSTRUCTION_COUNT> instructionNames;
    extern std::array<std::string, VARIANT_COUNT> variantNames;

    class InstructionMap {
    public:
        InstructionMap();
        std::unordered_map<int, std::pair<std::pair<int, int>, std::pair<int, int>>> _instructionTypes;
//        std::unordered_map<uint32_t, std::vector<uint32_t>> _instructions;
    };

    class Instruction {
    private:
        InstructionMap _instructionMap;
        std::vector<uint32_t> &_buffer;
        uint64_t _address;
        uint64_t _baseAddress;
        uint32_t _instruction;
        int _mask;
        int _type;
        int _result;
        int _variant;
        int _variantOther;
        int _rd;
        int _rn;
        bool _isImmUsed;
        bool _isImmSigned;
        union {
            uint64_t _imm;
            int64_t _immSigned;
        } _imm;
    public:
        Instruction(uint64_t address, std::vector<uint32_t> &buffer);
        bool decodeType();
        bool decodeImm();
        bool decodeRd();
        bool decodeRn();
        void decodeInstruction();
        int getType(){return _type;};
        int getVariant(){return _variant;};
        int getVariantOther(){return _variantOther;};
        int getRd(){return _rd;};
        int getRn(){return _rn;};
        bool isImmUsed(){return _isImmUsed;};
        bool isImmSigned(){return _isImmSigned;};
        uint64_t getImm(){return _imm._imm;};
        int64_t getImmSigned(){return _imm._immSigned;};
        void printDisassembly();
    };
}

#endif //__INSTRUCTIONSET_HPP__