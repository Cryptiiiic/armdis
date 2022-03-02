//
// Created by cryptic on 10/14/21.
//

#ifndef __GLOBAL_HPP__
#define __GLOBAL_HPP__
#include <functional>
#include <chrono>
#include <thread>
#include <iostream>
#include <codecvt>
#include <locale>
#include <string>
#include <cassert>
#include <map>
#include <string_view>
#include <iomanip>
#include <fstream>
#include <array>
#include <unordered_map>
#include <vector>
#ifdef _MSC_VER
#include <stdlib.h>
#define bswap_32(x) _byteswap_ulong(x)
#define bswap_64(x) _byteswap_uint64(x)
#elif defined(__APPLE__)
#include <libkern/OSByteOrder.h>
#define bswap_32(x) OSSwapInt32(x)
#define bswap_64(x) OSSwapInt64(x)
#else
#include <byteswap.h>
#endif
#endif // __GLOBAL_HPP__