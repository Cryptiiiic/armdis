//
// Created by cryptic on 10/14/21.
//

#include "../Global.hpp"
#include "Arm.hpp"
#include "../Logger.hpp"

Arm64::Arm64(std::vector<uint32_t> &binaryData, std::size_t binarySize) : _binaryData(binaryData), _binarySize(binarySize) {
    initInstructions();
};

void Arm64::hexPrint(int index) {
    printf("0x%08X: 0x%08X\n", index, _binaryData[index]);
}

void Arm64::hexDump(int amount) {
    int idx = 0;
    for(uint32_t op : _binaryData) {
        if(amount == -1 && idx < (_binarySize / 4))
            hexPrint(idx);
        else if(amount > -1 && idx <= amount && idx < (_binarySize / 4))
            hexPrint(idx);
        else
            return;
        idx++;
    }
}

int Arm64::getInstructionType(uint64_t address) {
    if(address % 4 != 0)
        return instructions::unknown_insn;
    uint32_t instruction = _binaryData[address / 4];
    for( const auto& [key, value] : _instructions ) {
        for (auto value1: value) {
            if ((instruction & key) == value1) {
                logger->debugPrint("DEBUG: 0x%016llX: insn: 0x%08X mask: 0x%08X insn & mask: 0x%08X\n", address, instruction, key, value1);
                return _instructionNames.at(value1);
            }
        }
    }
    return instructions::unknown_insn;
}

uint64_t Arm64::getInstructionImm(int type, uint64_t address) {
    uint32_t imm = 0x0;
    uint32_t instruction = _binaryData[address / 0x4];
    switch(type) {
        case instructions::add_im_insn:
        case instructions::sub_im_insn:
        case instructions::subs_im_insn:
            imm = (instruction >> 0xA) % 0x1000 << (((instruction >> 0x16) & 0x1) * 0xC);
            break;
//        default:
//            imm = 0;
//            break;
    }
    return imm;
}

uint64_t Arm64::getInstructionXd(int type, uint64_t address) {
    uint32_t xd = 0x0;
    uint32_t instruction = _binaryData[address / 0x4];
    switch(type) {
        case instructions::add_im_insn:
        case instructions::adr_insn:
        case instructions::adrp_insn:
        case instructions::sub_im_insn:
        case instructions::mov_re_insn:
        case instructions::and_im_insn:
            xd = instruction % 0x20;
            break;
    //        default:
    //            imm = 0;
    //            break;
    }
    return xd;
}
uint64_t Arm64::getInstructionXn(int type, uint64_t address) {
    uint32_t xn = 0x0;
    uint32_t instruction = _binaryData[address / 0x4];
    switch(type) {
        case instructions::add_im_insn:
        case instructions::sub_im_insn:
        case instructions::subs_im_insn:
        case instructions::br_insn:
        case instructions::mov_re_insn:
        case instructions::and_im_insn:
        case instructions::ldr_im_insn:
        case instructions::stp_insn:
            xn = (instruction >> 0x5) % 0x20;
            break;
            //        default:
            //            imm = 0;
            //            break;
    }
    return xn;
}

void Arm64::printDisassembly(uint64_t address) {
    int type = getInstructionType(address);
    std::string name = instructionNames[type];
    #define PRINTINSN(str) do { \
        logger->print("0x%016llX     %s%s%s\n", address, name.c_str(), std::string(12 - name.size(), ' ').c_str(), str); \
    } while(false)
    char *rvalue;
    switch(type) {
        case instructions::add_im_insn:
            sprintf(rvalue, "x%d, x%d, #0x%llX", (int)getInstructionXd(type, address), (int)getInstructionXn(type, address), getInstructionImm(type, address));
            PRINTINSN(rvalue);
            break;
        case instructions::adrp_insn:
            sprintf(rvalue, "x%d, #0x%llX", (int)getInstructionXd(type, address), getInstructionImm(type, address));
            PRINTINSN(rvalue);
            break;
        case instructions::unknown_insn:
            PRINTINSN("");
            break;
        default:
            PRINTINSN("XD/XN/LABEL not implemented yet");
            break;
    }
}

void Arm64::printAllDisassembly(int amount) {
    uint64_t idx = 0;
    for(uint32_t instruction : _binaryData) {
        if(amount == -1 && idx < (_binarySize))
            printDisassembly(idx);
        else if(amount > -1 && idx <= amount && idx < (_binarySize))
            printDisassembly(idx);
        else
            return;
        idx += 4;
    }
}

//void Arm64::decodeInstructions() {
//    uint64_t idx = -1;
//    for(uint32_t instruction : _binaryData) {
//        idx++;
//        if(idx >= (_binarySize / 4) + 4)
//            return;
//        bool matched = false;
//        for( const auto& [key, value] : _instructions ) {
//            for(auto value1 : value) {
//                if((instruction & key) == value1) {
//                    std::string name = instructionNames[_instructionNames.at(value1)];
//                    std::string nameDebug = instructionNamesDebug[_instructionNames.at(value1)];
//                    logger->debugPrint("DEBUG: Found: %s:%s insn: 0x%08X mask: 0x%08X insn & mask: 0x%08X\n", nameDebug.c_str(), std::string(23 - strlen(nameDebug.c_str()), ' ').c_str(), instruction, key, value1);
//                    logger->print("0x%016llX: %s%sRD/XN HERE\n", idx * 4, name.c_str(), std::string(12 - strlen(name.c_str()), ' ').c_str());
//                    matched = true;
//                }
//            }
//            if(matched)
//                break;
//        }
//        if(!matched) {
//            logger->debugPrint("DEBUG: Found: UNKNOWN:%s insn: 0x%08X\n", std::string(23 - strlen("UNKNOWN"), ' ').c_str(), instruction);
//            logger->print("0x%016llX: UNKNOWN\n", idx * 4);
//        }
//    }
//}

void Arm64::initInstructions() {
/*****************************************************************************
 *  The ADD (immediate) instruction
 *  Mask:       0x7F800000(0b01111111100000000000000000000000)
 *  Result:     0x11000000(0b00010001000000000000000000000000)
 *  match if instruction(?) & mask(0x7F800000) == result(0x11000000)        */
    _instructionNames.insert(std::make_pair<uint32_t, int>(instructionResults::add_im_res, instructions::add_im_insn));
    _instructions.emplace(instructionMasks::add_im_and_im_ands_im_mov_wi_sub_im_subs_im_mask, std::vector<uint32_t>());
    _instructions.at(instructionMasks::add_im_and_im_ands_im_mov_wi_sub_im_subs_im_mask).emplace_back(instructionResults::add_im_res);
/*****************************************************************************
 *  The ADD (extended register) instruction
 *  Mask:       0x7FE00000(0b01111111111000000000000000000000)
 *  Result:     0x0B000000(0b00001011000000000000000000000000)
 *  match if instruction(?) & mask(0x7FE00000) == result(0x0B000000)        */
    _instructionNames.insert(std::make_pair<uint32_t, int>(instructionResults::add_ex_res, instructions::add_ex_insn));
    _instructions.emplace(instructionMasks::add_ex_mask, std::vector<uint32_t>());
    _instructions.at(instructionMasks::add_ex_mask).emplace_back(instructionResults::add_ex_res);
/*****************************************************************************
 *  The ADR instruction
 *  Mask:       0x9F000000(0b10011111000000000000000000000000)
 *  Result:     0x10000000(0b00010000000000000000000000000000)
 *  match if instruction(?) & mask(0x9F000000) == result(0x10000000)        */
    _instructionNames.insert(std::make_pair<uint32_t, int>(instructionResults::adr_res, instructions::adr_insn));
    _instructions.emplace(instructionMasks::adr_adrp_mask, std::vector<uint32_t>());
    _instructions.at(instructionMasks::adr_adrp_mask).emplace_back(instructionResults::adr_res);
/*****************************************************************************
 *  The ADRP instruction
 *  Mask:       0x9F000000(0b10011111000000000000000000000000)
 *  Result:     0x90000000(0b10010000000000000000000000000000)
 *  match if instruction(?) & mask(0x9F000000) == result(0x90000000)        */
    _instructionNames.insert(std::make_pair<uint32_t, int>(instructionResults::adrp_res, instructions::adrp_insn));
    _instructions.emplace(instructionMasks::adr_adrp_mask, std::vector<uint32_t>());
    _instructions.at(instructionMasks::adr_adrp_mask).emplace_back(instructionResults::adrp_res);
/*****************************************************************************
 *  The AND (immediate) instruction
 *  Mask:       0x7F800000(0b01111111100000000000000000000000)
 *  Result:     0x12000000(0b00010010000000000000000000000000)
 *  match if instruction(?) & mask(0x7F800000) == result(0x12000000)        */
    _instructionNames.insert(std::make_pair<uint32_t, int>(instructionResults::and_im_res, instructions::and_im_insn));
    _instructions.emplace(instructionMasks::add_im_and_im_ands_im_mov_wi_sub_im_subs_im_mask, std::vector<uint32_t>());
    _instructions.at(instructionMasks::add_im_and_im_ands_im_mov_wi_sub_im_subs_im_mask).emplace_back(instructionResults::and_im_res);
/*****************************************************************************
 *  The AND (shifted register) instruction
 *  Mask:       0x7F200000(0b01111111001000000000000000000000)
 *  Result:     0x0A000000(0b00001010000000000000000000000000)
 *  match if instruction(?) & mask(0x7F200000) == result(0x0A000000)        */
    _instructionNames.insert(std::make_pair<uint32_t, int>(instructionResults::and_sh_res, instructions::and_sh_insn));
    _instructions.emplace(instructionMasks::and_sh_subs_sh_mask, std::vector<uint32_t>());
    _instructions.at(instructionMasks::and_sh_subs_sh_mask).emplace_back(instructionResults::and_sh_res);
/*****************************************************************************
 *  The ANDS (immediate) instruction
 *  Mask:       0x7F800000(0b01111111100000000000000000000000)
 *  Result:     0x72000000(0b01110010000000000000000000000000)
 *  match if instruction(?) & mask(0x7F800000) == result(0x72000000)        */
    _instructionNames.insert(std::make_pair<uint32_t, int>(instructionResults::ands_im_res, instructions::ands_im_insn));
    _instructions.emplace(instructionMasks::add_im_and_im_ands_im_mov_wi_sub_im_subs_im_mask, std::vector<uint32_t>());
    _instructions.at(instructionMasks::add_im_and_im_ands_im_mov_wi_sub_im_subs_im_mask).emplace_back(instructionResults::ands_im_res);
/*****************************************************************************
 *  The B.cond instruction
 *  Mask:       0xFF000010(0b11111111000000000000000000010000)
 *  Result:     0x54000000(0b01010100000000000000000000000000)
 *  match if instruction(?) & mask(0xFF000010) == result(0x54000000)        */
    _instructionNames.insert(std::make_pair<uint32_t, int>(instructionResults::b_co_res, instructions::b_co_insn));
    _instructions.emplace(instructionMasks::b_co_mask, std::vector<uint32_t>());
    _instructions.at(instructionMasks::b_co_mask).emplace_back(instructionResults::b_co_res);
/*****************************************************************************
 *  The B instruction
 *  Mask:       0xFC000000(0b11111100000000000000000000000000)
 *  Result:     0x94000000(0b10010100000000000000000000000000)
 *  match if instruction(?) & mask(0xFC000000) == result(0x94000000)        */
    _instructionNames.insert(std::make_pair<uint32_t, int>(instructionResults::b_res, instructions::b_insn));
    _instructions.emplace(instructionMasks::br_blr_ret_mask, std::vector<uint32_t>());
    _instructions.at(instructionMasks::br_blr_ret_mask).emplace_back(instructionResults::b_res);
/*****************************************************************************
 *  The BL instruction
 *  Mask:       0xFC000000(0b11111100000000000000000000000000)
 *  Result:     0x94000000(0b10010100000000000000000000000000)
 *  match if instruction(?) & mask(0xFC000000) == result(0x94000000)        */
    _instructionNames.insert(std::make_pair<uint32_t, int>(instructionResults::bl_res, instructions::bl_insn));
    _instructions.emplace(instructionMasks::br_blr_ret_mask, std::vector<uint32_t>());
    _instructions.at(instructionMasks::br_blr_ret_mask).emplace_back(instructionResults::bl_res);
/*****************************************************************************
 *  The BLR instruction
 *  Mask:       0xFFFFFC1F(0b11111111111111111111110000011111)
 *  Result:     0xD63F0000(0b11010110001111110000000000000000)
 *  match if instruction(?) & mask(0xFFFFFC1F) == result(0xD63F0000)        */
    _instructionNames.insert(std::make_pair<uint32_t, int>(instructionResults::blr_res, instructions::blr_insn));
    _instructions.emplace(instructionMasks::br_blr_ret_mask, std::vector<uint32_t>());
    _instructions.at(instructionMasks::br_blr_ret_mask).emplace_back(instructionResults::blr_res);
/*****************************************************************************
 *  The BR instruction
 *  Mask:       0xFFFFFC1F(0b11111111111111111111110000011111)
 *  Result:     0xD61F0000(0b11010110000111110000000000000000)
 *  match if instruction(?) & mask(0xFFFFFC1F) == result(0xD61F0000)        */
    _instructionNames.insert(std::make_pair<uint32_t, int>(instructionResults::br_res, instructions::br_insn));
    _instructions.emplace(instructionMasks::br_blr_ret_mask, std::vector<uint32_t>());
    _instructions.at(instructionMasks::br_blr_ret_mask).emplace_back(instructionResults::br_res);
/*****************************************************************************
 *  The CMP (extended register) instruction
 *  Mask:       0x7F80001F(0b01111111100000000000000000011111)
 *  Result:     0x6B00001F(0b01101011000000000000000000011111)
 *  match if instruction(?) & mask(0x7F80001F) == result(0x6B00001F)        */
    _instructionNames.insert(std::make_pair<uint32_t, int>(instructionResults::cmp_ex_res, instructions::cmp_ex_insn));
    _instructions.emplace(instructionMasks::cmp_ex_mask, std::vector<uint32_t>());
    _instructions.at(instructionMasks::cmp_ex_mask).emplace_back(instructionResults::cmp_ex_res);
/*****************************************************************************
 *  The IC IALLU, Instruction Cache Invalidate All to PoU instruction
 *  Mask:       (0b)
 *  Result:     (0b)
 *  match if instruction(?) & mask() == result()        */
//    _instructionNames.insert(std::make_pair<uint32_t, int>(, "IC IALLU"));
//    _instructions.emplace(, std::vector<uint32_t>());
//    _instructions.at().emplace_back();
/*****************************************************************************
 *  The LDP instruction
 *  Mask:       0x79C00000(0b01111001110000000000000000000000)
 *  Result:     0x28C00000(0b00101000110000000000000000000000)
 *  match if instruction(?) & mask(0x79C00000) == result(0x28C00000)        */
    _instructionNames.insert(std::make_pair<uint32_t, int>(instructionResults::ldp_res, instructions::ldp_insn));
    _instructions.emplace(instructionMasks::ldp_mask, std::vector<uint32_t>());
    _instructions.at(instructionMasks::ldp_mask).emplace_back(instructionResults::ldp_res);
/*****************************************************************************
 *  The LDR (immediate) instruction
 *  Mask:       0xBFE00C00(0b10111111111000000000110000000000)
 *  Result:     0x18000400(0b00011000000000000000010000000000)
 *  match if instruction(?) & mask(0xBFE00C00) == result(0x18000400)        */
    _instructionNames.insert(std::make_pair<uint32_t, int>(instructionResults::ldr_im_res, instructions::ldr_im_insn));
    _instructions.emplace(instructionMasks::ldr_im_mask, std::vector<uint32_t>());
    _instructions.at(instructionMasks::ldr_im_mask).emplace_back(instructionResults::ldr_im_res);
/*****************************************************************************
 *  The LDR (literal) instruction
 *  Mask:       0xBF000000(0b10111111000000000000000000000000)
 *  Result:     0x18000000(0b00011000000000000000000000000000)
 *  match if instruction(?) & mask(0xBF000000) == result(0x18000000)        */
    _instructionNames.insert(std::make_pair<uint32_t, int>(instructionResults::ldr_li_res, instructions::ldr_li_insn));
    _instructions.emplace(instructionMasks::ldr_li_mask, std::vector<uint32_t>());
    _instructions.at(instructionMasks::ldr_li_mask).emplace_back(instructionResults::ldr_li_res);
/*****************************************************************************
 *  The MOV (register) instruction
 *  Mask:       0x7FE0FFE0(0b01111111111000001111111111100000)
 *  Result:     0x2A0003E0(0b00101010000000000000001111100000)
 *  match if instruction(?) & mask(0x7FE0FFE0) == result(0x2A0003E0)        */
    _instructionNames.insert(std::make_pair<uint32_t, int>(instructionResults::mov_re_res, instructions::mov_re_insn));
    _instructions.emplace(instructionMasks::mov_re_mask, std::vector<uint32_t>());
    _instructions.at(instructionMasks::mov_re_mask).emplace_back(instructionResults::mov_re_res);
/*****************************************************************************
 *  The MOV (wide immediate) instruction
 *  Mask:       0x7F800000(0b01111111100000000000000000000000)
 *  Result:     0x52800000(0b01010010100000000000000000000000)
 *  match if instruction(?) & mask(0x7F800000) == result(0x52800000)        */
    _instructionNames.insert(std::make_pair<uint32_t, int>(instructionResults::mov_wi_res, instructions::mov_wi_insn));
    _instructions.emplace(instructionMasks::add_im_and_im_ands_im_mov_wi_sub_im_subs_im_mask, std::vector<uint32_t>());
    _instructions.at(instructionMasks::add_im_and_im_ands_im_mov_wi_sub_im_subs_im_mask).emplace_back(instructionResults::mov_wi_res);
/*****************************************************************************
 *  The MSR (immediate) instruction
 *  Mask:       0xFFF8F01F(0b11111111111110001111000000011111)
 *  Result:     0xD500401F(0b11010101000000000100000000011111)
 *  match if instruction(?) & mask(0xFFF8F01F) == result(0xD500401F)        */
    _instructionNames.insert(std::make_pair<uint32_t, int>(instructionResults::msr_im_res, instructions::msr_im_insn));
    _instructions.emplace(instructionMasks::msr_im_mask, std::vector<uint32_t>());
    _instructions.at(instructionMasks::msr_im_mask).emplace_back(instructionResults::msr_im_res);
/*****************************************************************************
 *  The MSR (register) instruction
 *  Mask:       0xFFF80000(0b11111111111110000000000000000000)
 *  Result:     0xD5180000(0b11010101000110000000000000000000)
 *  match if instruction(?) & mask(0xFFF80000) == result(0xD5180000)        */
    _instructionNames.insert(std::make_pair<uint32_t, int>(instructionResults::msr_re_res, instructions::msr_re_insn));
    _instructions.emplace(instructionMasks::msr_re_mask, std::vector<uint32_t>());
    _instructions.at(instructionMasks::msr_re_mask).emplace_back(instructionResults::msr_re_res);
/*****************************************************************************
 *  The NOP instruction
 *  Mask:       0xFFFFFFFF(0b11111111111111111111111111111111)
 *  Result:     0xD503201F(0b11010101000000110010000000011111)
 *  match if instruction(?) & mask(0xFFFFFFFF) == result(0xD503201F)        */
    _instructionNames.insert(std::make_pair<uint32_t, int>(instructionResults::nop_res, instructions::nop_insn));
    _instructions.emplace(instructionMasks::nop_mask, std::vector<uint32_t>());
    _instructions.at(instructionMasks::nop_mask).emplace_back(instructionResults::nop_res);
/*****************************************************************************
 *  The RET instruction
 *  Mask:       0xFFFFFC1F(0b11111111111111111111110000011111)
 *  Result:     0xD65F0000(0b11010110010111110000000000000000)
 *  match if instruction(?) & mask(0xFFFFFC1F) == result(0xD65F0000)        */
    _instructionNames.insert(std::make_pair<uint32_t, int>(instructionResults::ret_res, instructions::ret_insn));
    _instructions.emplace(instructionMasks::br_blr_ret_mask, std::vector<uint32_t>());
    _instructions.at(instructionMasks::br_blr_ret_mask).emplace_back(instructionResults::ret_res);
/*****************************************************************************
 *  The STP instruction
 *  Mask:       0x7FC00000(0b01111111110000000000000000000000)
 *  Result:     0x28800000(0b00101000100000000000000000000000)
 *  match if instruction(?) & mask(0x7FC00000) == result(0x28800000)        */
    _instructionNames.insert(std::make_pair<uint32_t, int>(instructionResults::stp_res, instructions::stp_insn));
    _instructions.emplace(instructionMasks::stp_mask, std::vector<uint32_t>());
    _instructions.at(instructionMasks::stp_mask).emplace_back(instructionResults::stp_res);
/*****************************************************************************
 *  The SUB (immediate) instruction
 *  Mask:       0x7F800000(0b01111111100000000000000000000000)
 *  Result:     0x4B000000(0b01001011000000000000000000000000)
 *  match if instruction(?) & mask(0x7F800000) == result(0x4B000000)        */
    _instructionNames.insert(std::make_pair<uint32_t, int>(instructionResults::sub_im_res, instructions::sub_im_insn));
    _instructions.emplace(instructionMasks::add_im_and_im_ands_im_mov_wi_sub_im_subs_im_mask, std::vector<uint32_t>());
    _instructions.at(instructionMasks::add_im_and_im_ands_im_mov_wi_sub_im_subs_im_mask).emplace_back(instructionResults::sub_im_res);
/*****************************************************************************
 *  The SUBS (immediate) instruction
 *  Mask:       0x7F800000(0b01111111100000000000000000000000)
 *  Result:     0x71000000(0b01110001000000000000000000000000)
 *  match if instruction(?) & mask(0x7F800000) == result(0x71000000)        */
    _instructionNames.insert(std::make_pair<uint32_t, int>(instructionResults::subs_im_res, instructions::subs_im_insn));
    _instructions.emplace(instructionMasks::add_im_and_im_ands_im_mov_wi_sub_im_subs_im_mask, std::vector<uint32_t>());
    _instructions.at(instructionMasks::add_im_and_im_ands_im_mov_wi_sub_im_subs_im_mask).emplace_back(instructionResults::subs_im_res);
/*****************************************************************************
 *  The SUBS (shifted register) instruction
 *  Mask:       0x7F200000(0b01111111001000000000000000000000)
 *  Result:     0x6B000000(0b01101011000000000000000000000000)
 *  match if instruction(?) & mask(0x7F200000) == result(0x6B000000)        */
    _instructionNames.insert(std::make_pair<uint32_t, int>(instructionResults::subs_sh_res,instructions::subs_sh_insn));
    _instructions.emplace(instructionMasks::and_sh_subs_sh_mask, std::vector<uint32_t>());
    _instructions.at(instructionMasks::and_sh_subs_sh_mask).emplace_back(instructionResults::subs_sh_res);
/*****************************************************************************
*****************************************************************************/
}
