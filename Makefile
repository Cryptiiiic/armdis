#
# Created by cryptic on 10/14/21.
#

build:
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang++ -std=c++17 -stdlib=libc++ -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk -arch x86_64 Arm64/InstructionSet.cpp Logger.cpp main.cpp -o patcher -DNDEBUG -O0

clean:
	rm -rf patcher

all: clean build

.PHONY: all
