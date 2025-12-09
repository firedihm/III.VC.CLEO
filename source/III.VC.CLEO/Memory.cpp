#include "Memory.h"

#include <Windows.h>

#include <cstring>

void
Write(void* dest, const void* src, size_t count)
{
        if (dest < 0x401000)
                return;

        uint oldProtect;
        VirtualProtect(dest, count, PAGE_EXECUTE_READWRITE, &oldProtect);

        if (src)
                std::memcpy(dest, src, count);
        else
                std::memset(dest, 0x90, count);

        VirtualProtect(dest, count, oldProtect, &oldProtect);
}

void
memory::Nop(uchar* dest, size_t count)
{
        Write(dest, nullptr, count);
}

void
memory::SetChar(uchar* dest, char value)
{
        Write(dest, &value, sizeof(value));
}

void
memory::SetShort(uchar* dest, short value)
{
        Write(dest, &value, sizeof(value));
}

void
SetInt(uchar* dest, int value)
{
        Write(dest, &value, sizeof(value));
}

void
memory::SetFloat(uchar* dest, float value)
{
        Write(dest, &value, sizeof(value));
}

void
memory::SetPointer(uchar* dest, void* value)
{        
        Write(dest, &value, sizeof(value));
}

void
memory::RedirectCall(uchar* dest, uchar* func)
{
        SetChar(dest, 0xE8);
    	SetInt(dest + 1, func - (dest + 5));
}

void
memory::RedirectJump(uchar* dest, uchar* func)
{
    	SetChar(dest, 0xE9);
    	SetInt(dest + 1, func - (dest + 5));
}
