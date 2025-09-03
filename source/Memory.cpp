#include "Memory.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <cstring>

int
memory::read(void* dest, size_t count, bool vp)
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
memory::write(void* dest, void* src, size_t count, bool vp)
{
        DWORD oldProtect;
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
                std::memset(dest, *(uchar*)src, count);
                break;
        }

        if (vp)
                ::VirtualProtect(dest, count, oldProtect, &oldProtect);
}

void
memory::intercept(uchar op, void* dest, void* addr)
{
		ptrdiff_t offset = (uchar*)addr - ((uchar*)dest + 5); // (uchar*) cast allows us to perform bytewise arithmetic on ptrs

		write(dest, op);
		write((uchar*)dest + 1, offset);
}

void*
memory::load_library(const char* name)
{
		return ::LoadLibraryA(name);
}

void
memory::free_library(const void* handle)
{
		::FreeLibrary((HMODULE)handle);
}

void*
memory::get_proc_address(const void* handle, const char* proc_name)
{
		return ::GetProcAddress((HMODULE)handle, proc_name);
}

short
memory::get_key_state(int virtual_key)
{
		return ::GetKeyState(virtual_key);
}
