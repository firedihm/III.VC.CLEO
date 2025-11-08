#include "Memory.h"
#include "windows.h"

#include <cstring>

using namespace memory;

void
Write(char* dest, const char* src, size_t count)
{
        if (!dest)
                return;

        ulong oldProtect;
        VirtualProtect(dest, count, PAGE_EXECUTE_READWRITE, &oldProtect);

        if (src)
                std::memcpy(dest, src, count);
        else
                std::memset(dest, 0x90, count);

        VirtualProtect(dest, count, oldProtect, &oldProtect);
}

void
Nop(char* dest, size_t count)
{
        Write(dest, nullptr, count);
}

void
SetChar(char* dest, char value)
{
        Write(dest, &value, sizeof(value));
}

void
SetShort(char* dest, short value)
{
        Write(dest, &value, sizeof(value));
}

void
SetInt(char* dest, int value)
{
        Write(dest, &value, sizeof(value));
}

void
SetFloat(char* dest, float value)
{
        Write(dest, &value, sizeof(value));
}

void
SetPointer(char* dest, char* value)
{        
        Write(dest, &value, sizeof(value));
}

void
RedirectCall(char* dest, char* func)
{
        SetChar(dest, 0xE8);
    	SetInt(dest + 1, func - (dest + 5));
}

void
RedirectJump(char* dest, char* func)
{
    	SetChar(dest, 0xE9);
    	SetInt(dest + 1, func - (dest + 5));
}
