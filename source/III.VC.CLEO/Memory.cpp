#include "Memory.h"

#include <Windows.h>

#include <cassert>
#include <cstring>

void
memory::Write(void* dest, const void* src, size_t count, bool vp)
{
        // practically, if nullptr is passed, then call is skipped
        if (dest < 0x401000)
                return;

        uint oldProtect;
        if (vp)
                VirtualProtect(dest, count, PAGE_EXECUTE_READWRITE, &oldProtect);

        if (count == 1 || count == 2 || count == 4) {
                std::memcpy(dest, src, count);
        } else {
                std::memset(dest, *(char*)scr, count);
        }

        if (vp)
                VirtualProtect(dest, count, oldProtect, &oldProtect);
}

void
memory::Intercept(uchar op, uchar* dest, uchar* addr)
{
        assert(op == 0xE8 || op == 0xE9);

        Write(dest, &op, sizeof(op), true);
        ptrdiff_t offset = addr - (dest + 5);
        Write(dest + 1, &offset, sizeof(offset), true);
}
