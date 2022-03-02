//
// Created by cryptic on 10/20/21.
//

#include "../Global.hpp"
#include "../Logger.hpp"
#include "InstructionSet.hpp"

extern Logger *logger;

namespace Arm64 {

    std::array<std::string, INSTRUCTION_COUNT> instructionNames {
            std::string("ADD"),
            std::string("ADDS"),
            std::string("ADR"),
            std::string("ADRP"),
            std::string("AND"),
            std::string("ANDS"),
            std::string("B"),
            std::string("BL"),
            std::string("BLR"),
            std::string("BR"),
            std::string("CMP"),
            std::string("LDP"),
            std::string("LDR"),
            std::string("MOV"),
            std::string("MOVK"),
            std::string("MOVN"),
            std::string("MOVZ"),
            std::string("MRS"),
            std::string("MSR"),
            std::string("MSUB"),
            std::string("MUL"),
            std::string("MVN"),
            std::string("NOP"),
            std::string("ORR"),
            std::string("ORR"),
            std::string("RET"),
            std::string("RETAA"),
            std::string("RETAB"),
            std::string("ROR"),
            std::string("STP"),
            std::string("STR"),
            std::string("STRB"),
            std::string("STRH"),
            std::string("STUR"),
            std::string("STURB"),
            std::string("STURH"),
            std::string("SUB"),
            std::string("SUBS"),
            std::string("SVC"),
            std::string("SYS"),
            std::string("TBNZ"),
            std::string("TBZ"),
            std::string("UNKNOWN"),
    };

    std::array<std::string, VARIANT_COUNT> variantNames {
            std::string("NONE"),
            std::string("IMMEDIATE"),
            std::string("REGISTER"),
            std::string("REGISTER_SHIFTED_REGISTER"),
            std::string("SP_PLUS_IMMEDIATE"),
            std::string("SP_PLUS_REGISTER"),
            std::string("IMMEDIATE_TO_PC"),
            std::string("EXTENDED_REGISTER"),
            std::string("SHIFTED_REGISTER"),
            std::string("CONDITIONAL"),
            std::string("UNCONDITIONAL"),
            std::string("LITERAL"),
            std::string("SP"),
            std::string("INVERTED_WIDE_IMMEDIATE"),
            std::string("BITMASK_WIDE_IMMEDIATE"),
            std::string("WIDE_IMMEDIATE"),
            std::string("UNKNOWN"),
    };

    Instruction::Instruction(uint64_t address, std::vector<uint32_t> &buffer) : _buffer(buffer) {
        _instructionMap = InstructionMap();
        _address = address;
        _baseAddress = *(uint64_t *)(*(uint64_t *)(&_buffer) + 0x300);
        _instruction = _buffer[address / 0x4];
        _type = instructionTypes::UNKNOWN_INSN;
        _variant = instructionVariants::UNKNOWN_VART;
        _variantOther = -1;
        _rd = -1;
        _rn = -1;
        _isImmUsed = false;
        _isImmSigned = false;
        _imm._imm = -1;
        _imm._immSigned = -1;
        if(address % 0x4 != 0x0)
            return;
        logger->debugPrint("DEBUG: Decoding instruction: 0x%08X at: 0x%08X\n", _instruction, address);
        decodeInstruction();
    }

    bool Instruction::decodeType() {
        for( const auto& [key, value] : _instructionMap._instructionTypes ) {
            _mask = value.first.first;
            _result = value.first.second;
            _type = value.second.first;
            _variant = value.second.second;
                if ((_instruction & _mask) == _result) {
                    logger->debugPrint("DEBUG: insn: 0x%08X mask: 0x%08X insn & mask: 0x%08X\n", _instruction, _mask, _result);
                    logger->debugPrint("DEBUG: type: %s, %d\n", instructionNames[_type].c_str(), _type);
                    logger->debugPrint("DEBUG: variant: %s, %d\n", variantNames[_variant].c_str(), _variant);
                    return true;
                }
        }
        _mask = -1;
        _result = -1;
        _type = -1;
        _variant = -1;
        logger->debugPrint("DEBUG: insn: 0x%08X\n", _instruction);
        logger->debugPrint("DEBUG: _type: %s, %d\n", instructionNames[instructionTypes::UNKNOWN_INSN].c_str(), instructionTypes::UNKNOWN_INSN);
        return false;
    }

    bool Instruction::decodeImm() {
//        uint64_t setbits1 = (1 << ((0x15)-(0xC)+1));
//        logger->print("setbits: 0x%016X\n", setbits);
        switch(_type) {
            case instructionTypes::ADD_INSN:
            case instructionTypes::ADR_INSN:
            case instructionTypes::ADRP_INSN:
            case instructionTypes::SUB_INSN:
            case instructionTypes::SUBS_INSN:
            case instructionTypes::BL_INSN: {
                uint64_t tmp = _instruction % (1 << 0x1A);
                uint64_t tmp2 = (tmp & 1 << (0x18)) >> 0x18;
                for(int i = 0x19; i < 0x40; i++) {
                    tmp |= tmp2 << i;
                }
                _isImmUsed = true;
                _isImmSigned = false;
                _imm._imm = tmp << 2;
                logger->debugPrint("DEBUG: _imm._imm: 0x%016X\n", _imm._imm);
                return true;
            }
            case instructionTypes::LDR_INSN:
                if(_variant == instructionVariants::IMMEDIATE_VART) {
                    if((((_instruction | 0x40000000) >> 0x16) % 0x400) == 0x3E5) {
                        _isImmUsed = true;
                        _isImmSigned = false;
                        _imm._imm = ((_instruction >> 0xA) % 0x1000) << ((_instruction >> 0x1E) % 0x4);
                        logger->debugPrint("DEBUG: _imm._imm: 0x%016X\n", _imm._imm);
                        return true;
                    }
                    if(((_instruction >> 0x18) | 0x4)) {
                        _isImmUsed = true;
                        _isImmSigned = false;
                        _imm._imm = (((_instruction >> 0xA) % 0x1000) << (_instruction >> 0x1E));
                        logger->debugPrint("DEBUG: _imm._imm: 0x%016X\n", _imm._imm);
                        return true;
                    } else {
                        uint64_t tmp = ((_instruction >> 0xC) % 0x400);
                        uint64_t tmp2 = (tmp & 1 << (0x8)) >> 0x8;
                        for(int i = 0x9; i < 0x40; i++) {
                            tmp |= tmp2 << i;
                        }
                        _isImmUsed = true;
                        _isImmSigned = true;
                        _imm._immSigned = tmp;
                        logger->debugPrint("DEBUG: _imm._imm: 0x%016X\n", _imm._immSigned);
                        return true;
                    }
                } else if (_variant == instructionVariants::LITERAL_VART) {
                    _isImmUsed = true;
                    _isImmSigned = false;
                    _imm._imm = (((_instruction >> 0x5) % 0x80000) << 0x2);
                    logger->debugPrint("DEBUG: _imm._imm: 0x%016X\n", _imm._imm);
                }
                break;
//        default:
//            imm = 0;
//            break;
        }
        return (_imm._imm != -1) || (_imm._immSigned != -1);
    }

    bool Instruction::decodeRd() {
        if (_type != -1 && _variant != -1) {
            switch (_variant) {
                case instructionVariants::IMMEDIATE_VART:
                case instructionVariants::SP_PLUS_REGISTER_VART:
                case instructionVariants::REGISTER_SHIFTED_REGISTER_VART:
                case instructionVariants::EXTENDED_REGISTER_VART:
                case instructionVariants::SHIFTED_REGISTER_VART:
                case instructionVariants::LITERAL_VART:
                    _rd = _instruction % (1 << 5);
                    logger->debugPrint("DEBUG: _rd: %d\n", _rd);
                    return _rd <= 40;
                default:
                    logger->debugPrint("DEBUG: _rd: NONE%s", "\n");
                    return false;
            }
        }
        logger->debugPrint("DEBUG: _rd: NONE%s", "\n");
        return false;
    }

    bool Instruction::decodeRn() {
        if (_type != -1 && _variant != -1 && _type != instructionTypes::ADRP_INSN &&
        _type != instructionTypes::ADR_INSN && _type != instructionTypes::LDR_INSN) {
            switch (_variant) {
                case instructionVariants::IMMEDIATE_VART:
                case instructionVariants::SP_PLUS_REGISTER_VART:
                case instructionVariants::REGISTER_SHIFTED_REGISTER_VART:
                case instructionVariants::EXTENDED_REGISTER_VART:
                case instructionVariants::SHIFTED_REGISTER_VART:
                    _rn = (_instruction >> 5) % (1 << ((9)-(5)+1));
                    logger->debugPrint("DEBUG: _rn: %d\n", _rd);
                    return _rn <= 40;
                default:
                    logger->debugPrint("DEBUG: _rn: NONE%s", "\n");
                    return false;
            }
        }
        logger->debugPrint("DEBUG: _rn: NONE%s", "\n");
        return false;
    }

    void Instruction::printDisassembly() {
        std::string type;
        bool unknown = false;
        if(_type == -1) {
            type = std::string("UNKNOWN");
            unknown = true;
        } else {
            type = instructionNames[_type];
        }
        logger->print("0x%016llX:\t%s\t\t", _baseAddress + _address, type.c_str());
        if(!unknown) {
            if(_rd != -1) {
                logger->print("x%d", _rd);
                if(_rn != -1) {
                    logger->print(", x%d", _rn);
                }
            }
            if(_isImmUsed) {
                if (_type == instructionTypes::LDR_INSN) {
                    if (_isImmSigned) {
                        logger->print(", =0x%016llX", *(int64_t * )(*(int64_t * )(&_buffer) + _address +
                                                                    _imm._immSigned));
                    } else {
                        logger->print(", =0x%016llX", *(uint64_t * )(*(uint64_t * )(&_buffer) + _address +
                                                                     _imm._imm));
                    }
                } else if (_type == instructionTypes::ADR_INSN || _type == instructionTypes::ADRP_INSN) {
                    if (_isImmSigned) {
                        logger->print(", 0x%016llX", _baseAddress + _address + _imm._immSigned);
                    } else {
                        logger->print(", 0x%016llX", _baseAddress + _address + _imm._imm);
                    }
                } else if (_type == instructionTypes::BL_INSN) {
                    if (_isImmSigned) {
                        logger->print("0x%016llX", _baseAddress + _address + _imm._immSigned);
                    } else {
                        logger->print("0x%016llX", _baseAddress + _address + _imm._imm);
                    }
                }
            }
        }
        logger->print("%s", "\n");
    }

    void Instruction::decodeInstruction() {
        decodeType();
        decodeImm();
        decodeRd();
        decodeRn();
        printDisassembly();
    }

    InstructionMap::InstructionMap() {
        int key = -1;
        std::pair<int, int> value1;
        std::pair<int, int> value2;
        std::pair<std::pair<int, int>, std::pair<int, int>> value;
        /*****************************************************************************
         *  The ADD (extended register) instruction
         *  Mask:       0x7FE00000(0b01111111111000000000000000000000)
         *  Result:     0x0B200000(0b00001011001000000000000000000000)
         *  match if instruction(?) & mask(0x7FE00000) == result(0x0B200000)        */
        value1 = std::make_pair(instructionMasks::M_7FE00000_MASK, instructionResults::R_0B200000_RESULT);
        value2 = std::make_pair(instructionTypes::ADD_INSN, instructionVariants::EXTENDED_REGISTER_VART);
        value = std::make_pair(value1, value2);
        _instructionTypes.insert(std::make_pair(++key, value));
        /*****************************************************************************
         *  The ADD (immediate) instruction
         *  Mask:       0x7F800000(0b01111111100000000000000000000000)
         *  Result:     0x11000000(0b00010001000000000000000000000000)
         *  match if instruction(?) & mask(0x7F800000) == result(0x11000000)        */
        value1 = std::make_pair(instructionMasks::M_7F800000_MASK, instructionResults::R_11000000_RESULT);
        value2 = std::make_pair(instructionTypes::ADD_INSN, instructionVariants::IMMEDIATE_VART);
        value = std::make_pair(value1, value2);
        _instructionTypes.insert(std::make_pair(++key, value));
        /*****************************************************************************
         *  The ADD (shifted register) instruction
         *  Mask:       0x7F200000(0b01111111001000000000000000000000)
         *  Result:     0x0B000000(0b00001011000000000000000000000000)
         *  match if instruction(?) & mask(0x7F200000) == result(0x0B000000)        */
        value1 = std::make_pair(instructionMasks::M_7F200000_MASK, instructionResults::R_0B000000_RESULT);
        value2 = std::make_pair(instructionTypes::ADD_INSN, instructionVariants::SHIFTED_REGISTER_VART);
        value = std::make_pair(value1, value2);
        _instructionTypes.insert(std::make_pair(++key, value));
        /*****************************************************************************
         *  The ADDS (extended register) instruction
         *  Mask:       0x7FE00000(0b1111111111000000000000000000000)
         *  Result:     ?         (?)
         *  match if instruction(?) & mask(0x7FE00000) == result(?)                 */
//        value1 = std::make_pair(instructionMasks::M_7FE00000_MASK, ?);
//        value2 = std::make_pair(instructionTypes::ADDS_INSN, instructionVariants::EXTENDED_REGISTER_VART);
//        value = std::make_pair(value1, value2);
//        _instructionTypes.insert(std::make_pair(++key, value));
        /*****************************************************************************
         *  The ADDS (immediate) instruction
         *  Mask:       0x7F800000(0b01111111100000000000000000000000)
         *  Result:     0x31000000(0b00110001000000000000000000000000)
         *  match if instruction(?) & mask(0x7F800000) == result(0x31000000)        */
        value1 = std::make_pair(instructionMasks::M_7F800000_MASK, instructionResults::R_31000000_RESULT);
        value2 = std::make_pair(instructionTypes::ADDS_INSN, instructionVariants::IMMEDIATE_VART);
        value = std::make_pair(value1, value2);
        _instructionTypes.insert(std::make_pair(++key, value));
        /*****************************************************************************
         *  The ADDS (shifted register) instruction
         *  Mask:       0x7F200000(0b01111111001000000000000000000000)
         *  Result:     ?         (?)
         *  match if instruction(?) & mask(0x7F200000) == result(?)                 */
//        value1 = std::make_pair(instructionMasks::M_7F200000_MASK, ?);
//        value2 = std::make_pair(instructionTypes::ADDS_INSN, instructionVariants::SHIFTED_REGISTER_VART);
//        value = std::make_pair(value1, value2);
//        _instructionTypes.insert(std::make_pair(++key, value));
        /*****************************************************************************
         *  The ADR instruction
         *  Mask:       0x9F000000(0b10011111000000000000000000000000)
         *  Result:     0x10000000(0b00010000000000000000000000000000)
         *  match if instruction(?) & mask(0x9F000000) == result(0x10000000)        */
        value1 = std::make_pair(instructionMasks::M_9F000000_MASK, instructionResults::R_10000000_RESULT);
        value2 = std::make_pair(instructionTypes::ADR_INSN, instructionVariants::NONE_VART);
        value = std::make_pair(value1, value2);
        _instructionTypes.insert(std::make_pair(++key, value));
        /*****************************************************************************
         *  The ADRP instruction
         *  Mask:       0x9F000000(0b10011111000000000000000000000000)
         *  Result:     0x90000000(0b10010000000000000000000000000000)
         *  match if instruction(?) & mask(0x9F000000) == result(0x90000000)        */
        value1 = std::make_pair(instructionMasks::M_9F000000_MASK, instructionResults::R_90000000_RESULT);
        value2 = std::make_pair(instructionTypes::ADRP_INSN, instructionVariants::IMMEDIATE_VART);
        value = std::make_pair(value1, value2);
        _instructionTypes.insert(std::make_pair(++key, value));
        /*****************************************************************************
         *  The AND (shifted register) instruction
         *  Mask:       0x7F200000(0b01111111001000000000000000000000)
         *  Result:     0x0A000000(0b00001010000000000000000000000000)
         *  match if instruction(?) & mask(0x7F200000) == result(0x0A000000)        */
        value1 = std::make_pair(instructionMasks::M_7F200000_MASK, instructionResults::R_0A000000_RESULT);
        value2 = std::make_pair(instructionTypes::AND_INSN, instructionVariants::SHIFTED_REGISTER_VART);
        value = std::make_pair(value1, value2);
        _instructionTypes.insert(std::make_pair(++key, value));
        /*****************************************************************************
         *  The AND (shifted register) instruction
         *  Mask:       0x7F200000(0b01111111001000000000000000000000)
         *  Result:     0x0A000000(0b00001010000000000000000000000000)
         *  match if instruction(?) & mask(0x7F200000) == result(0x0A000000)        */
        value1 = std::make_pair(instructionMasks::M_7F200000_MASK, instructionResults::R_0A000000_RESULT);
        value2 = std::make_pair(instructionTypes::AND_INSN, instructionVariants::SHIFTED_REGISTER_VART);
        value = std::make_pair(value1, value2);
        _instructionTypes.insert(std::make_pair(++key, value));
        /*****************************************************************************
         *  The BL instruction
         *  Mask:       0xFC000000(0b11111100000000000000000000000000)
         *  Result:     0x94000000(0b10010100000000000000000000000000)
         *  match if instruction(?) & mask(0xFC000000) == result(0x94000000)        */
        value1 = std::make_pair(instructionMasks::M_FC000000_MASK, instructionResults::R_94000000_RESULT);
        value2 = std::make_pair(instructionTypes::BL_INSN, instructionVariants::UNCONDITIONAL_VART);
        value = std::make_pair(value1, value2);
        _instructionTypes.insert(std::make_pair(++key, value));
        /*****************************************************************************
         *  The LDR (literal) instruction
         *  Mask:       0xFF000000(0b11111111000000000000000000000000)
         *  Result:     0x58000000(0b01011000000000000000000000000000)
         *  match if instruction(?) & mask(0xFF000000) == result(0x58000000)        */
        value1 = std::make_pair(instructionMasks::M_FF000000_MASK, instructionResults::R_58000000_RESULT);
        value2 = std::make_pair(instructionTypes::LDR_INSN, instructionVariants::LITERAL_VART);
        value = std::make_pair(value1, value2);
        _instructionTypes.insert(std::make_pair(++key, value));
    }
}