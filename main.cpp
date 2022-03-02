//
// Created by cryptic on 10/14/21.
//

#include "Global.hpp"
#include "Logger.hpp"
#include "Arm64/InstructionSet.hpp"

Logger *logger = nullptr;
Arm64::Instruction *binary = nullptr;

int main() {
    logger = new Logger(true, false);
    std::cout << "🔥🌸 Patcher" << std::endl;
    std::ifstream fileStream("iboot.bin");
    assert(fileStream.good());
    fileStream.seekg(0, std::ios_base::end);
    std::size_t fileSize = fileStream.tellg();
    std::cout << "Filesize: 0x" << std::hex << fileSize << std::endl;
    fileStream.seekg(0, std::ios_base::beg);
    std::vector<uint32_t> binaryData(fileSize / sizeof(uint32_t));
    fileStream.read((char*)&binaryData[0], fileSize);
    fileStream.close();
    for(int i = 0x0; i < 0x20; i+=4) {
        binary = new Arm64::Instruction(i, binaryData);
    }


/* Examples: */
//    std::array<uint32_t, 8> addrs {
//    0x48954, // ADD (immediate)
//    0x115CC, // ADD (extended register)
//    0x7544, // ADD (shifted register)
//    0x73B24, // ADDS (immediate)
//    0x6E0, // ADR
//    0x0, // ADRP
//    0x31F70, // AND (shifted register)
//    0x4981C, // CMP (extended register)
//    };
//    for(auto addr : addrs) {
//        binary = new Arm64::Instruction(addr, binaryData);
//        delete binary;
//    }
    delete logger;
    return 0;
}