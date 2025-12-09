#include "Memory.h"

#include <Windows.h>

#include <cstring>

void
memory::Write(void* dest, const void* src, size_t count, bool vp)
{
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
memory::RedirectCall(uchar* dest, uchar* addr)
{
        uchar op = 0xE8;
        Write(dest, &op, sizeof(op), true);

        ptrdiff_t offset = addr - (dest + 5);
        Write(dest + 1, &offset, sizeof(offset), true);
}

void
memory::RedirectJump(uchar* dest, uchar* addr)
{
        uchar op = 0xE9;
        Write(dest, &op, sizeof(op), true);

        ptrdiff_t offset = addr - (dest + 5);
        Write(dest + 1, &offset, sizeof(offset), true);
}
