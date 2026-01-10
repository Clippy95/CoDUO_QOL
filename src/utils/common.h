#pragma once
#include "..\framework.h"
std::wstring thisModuleFileName();
std::wstring GetModulePath(HMODULE hModule);

typedef struct {
    union {
        uint32_t value;
        struct {
            uint32_t CF : 1;  // Bit 0  - Carry Flag
            uint32_t _res1 : 1;  // Bit 1  - Reserved (always 1)
            uint32_t PF : 1;  // Bit 2  - Parity Flag
            uint32_t _res3 : 1;  // Bit 3  - Reserved (always 0)
            uint32_t AF : 1;  // Bit 4  - Auxiliary Carry Flag
            uint32_t _res5 : 1;  // Bit 5  - Reserved (always 0)
            uint32_t ZF : 1;  // Bit 6  - Zero Flag
            uint32_t SF : 1;  // Bit 7  - Sign Flag
            uint32_t TF : 1;  // Bit 8  - Trap Flag
            uint32_t IF : 1;  // Bit 9  - Interrupt Enable Flag
            uint32_t DF : 1;  // Bit 10 - Direction Flag
            uint32_t OF : 1;  // Bit 11 - Overflow Flag
            uint32_t IOPL : 2;  // Bits 12-13 - I/O Privilege Level
            uint32_t NT : 1;  // Bit 14 - Nested Task
            uint32_t _res15 : 1;  // Bit 15 - Reserved (always 0)
            uint32_t RF : 1;  // Bit 16 - Resume Flag
            uint32_t VM : 1;  // Bit 17 - Virtual 8086 Mode
            uint32_t AC : 1;  // Bit 18 - Alignment Check
            uint32_t VIF : 1;  // Bit 19 - Virtual Interrupt Flag
            uint32_t VIP : 1;  // Bit 20 - Virtual Interrupt Pending
            uint32_t ID : 1;  // Bit 21 - ID Flag
            uint32_t _res22 : 10; // Bits 22-31 - Reserved
        };
    };
} EFlags32;

template <size_t count = 1, typename... Args>
inline hook::pattern find_pattern(Args... args)
{
    hook::pattern pattern;
    ((pattern = hook::pattern(args), !pattern.count_hint(count).empty()) || ...);
    return pattern;

}
inline bool is_wine()
{
    HMODULE ntdllMod = GetModuleHandleA("ntdll.dll");

    if (ntdllMod && GetProcAddress(ntdllMod, "wine_get_version"))
        return true;

    return false;
}