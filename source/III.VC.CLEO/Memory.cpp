#include "Memory.h"

#include <Windows.h>

#include <cstring>

void
memory::Write(void* dest, const void* src, size_t count, bool vp)
{
        if (!dest)
                return;

        uint oldProtect;
        if (vp)
                VirtualProtect(dest, count, PAGE_EXECUTE_READWRITE, &oldProtect);

        switch (count) {
        case 1:
        case 2:
        case 4:
                std::memcpy(dest, src, count);
                break;
        default:
                std::memset(dest, *(char*)scr, count);
                break;
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
        ptrdiff_t offset = (uchar*)addr - ((uchar*)dest + 5); // (uchar*) cast allows us to perform bytewise arithmetic on ptrs
        Write((uchar*)dest + 1, &offset, sizeof(offset), true);
}
