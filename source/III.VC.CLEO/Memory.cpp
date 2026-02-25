#include "Memory.h"

#include <Windows.h>

#include <cstring>

int
memory::Read(void* dest, size_t count, bool vp)
{
		switch (count) {
		case 1:
				return *(char*)dest;
		case 2:
				return *(short*)dest;
		case 4:
				return *(int*)dest;
		default:
				return 0;
		}
}

void
memory::Write(void* dest, void* src, size_t count, bool vp)
{
        uint oldProtect;
        if (vp)
                ::VirtualProtect(dest, count, PAGE_EXECUTE_READWRITE, &oldProtect);

        switch (count) {
        case 1:
                *(uchar*)dest = *(uchar*)src;
                break;
        case 2:
                *(ushort*)dest = *(ushort*)src;
                break;
        case 4:
                *(uint*)dest = *(uint*)src;
                break;
        default:
                std::memset(dest, *(uchar*)scr, count);
                break;
        }

        if (vp)
                ::VirtualProtect(dest, count, oldProtect, &oldProtect);
}

void
memory::Intercept(uchar op, void* dest, void* addr)
{
		ptrdiff_t offset = (uchar*)addr - ((uchar*)dest + 5); // (uchar*) cast allows us to perform bytewise arithmetic on ptrs

		Write(dest, op);
		Write((uchar*)dest + 1, offset);
}

void*
memory::LoadLibrary(const char* name)
{
		return ::LoadLibraryA(name);
}

void
memory::FreeLibrary(const void* handle)
{
		::FreeLibrary(handle);
}
