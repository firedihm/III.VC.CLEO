#pragma once

inline constexpr uchar Call = 0xE8;
inline constexpr uchar Jump = 0xE9;

// wrapper over memory patching to keep <Windows.h> isolated
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

		template <uchar OP> requires (OP == Call || OP == Jump)
		void Intercept(void* dest, void* addr)
		{
				Intercept(OP, dest, addr);
		}
}
