#pragma once

namespace memory
{
		void Nop(uchar* dest, size_t count);

		void SetChar(uchar* dest, char value);
		void SetShort(uchar* dest, short value);
		void SetInt(uchar* dest, int value);
		void SetFloat(uchar* dest, float value);
		void SetPointer(uchar* dest, uchar* value);

		void RedirectCall(uchar* dest, uchar* func);
		void RedirectJump(uchar* dest, uchar* func);
}
