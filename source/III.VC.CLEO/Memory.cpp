#include "Memory.h"

#include <Windows.h>

#include <cassert>
#include <cstring>

void
memory::Write(void* dest, const void* src, size_t count, bool vp)
{
        if (!dest)
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
memory::Intercept(uchar op, void* dest, void* addr)
{
        if (!dest)
                return;

        Write(dest, &op, sizeof(op), true);
        uint offset = (uint)addr - ((uint)dest + 5);
        Write((uint)dest + 1, &offset, sizeof(offset), true);
}
