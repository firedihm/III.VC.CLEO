#pragma once

// wrapper over memory patching to keep <Windows.h> isolated
namespace memory
{
		void Write(void* dest, const void* src, size_t count, bool vp);

		template <typename T>
		void Write(uchar* dest, T value, size_t count = sizeof(value), bool vp = true)
		{
				Write(dest, &value, count, vp);
		}

		void RedirectCall(uchar* dest, uchar* addr);
		void RedirectJump(uchar* dest, uchar* addr);
}
