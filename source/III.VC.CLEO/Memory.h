#pragma once

// wrapper over memory operations to keep <Windows.h> isolated
namespace memory
{
		int Read(void* dest, size_t count, bool vp = false);

		void Write(void* dest, void* src, size_t count, bool vp);

		template <typename T>
		void Write(void* dest, T value, size_t count = sizeof(value), bool vp = true)
		{
				Write(dest, &value, count, vp);
		}

		void Intercept(uchar op, void* dest, void* addr);
		void MakeCall(void* dest, void* addr) { Intercept(0xE8, dest, addr); }
		void MakeJump(void* dest, void* addr) { Intercept(0xE9, dest, addr); }

		void* LoadLibrary(const char* name);
		void FreeLibrary(const void* handle)
}
