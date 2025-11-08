#pragma once

namespace memory
{
		void Nop(char* dest, size_t count);

		void SetChar(char* dest, char value);
		void SetShort(char* dest, short value);
		void SetInt(char* dest, int value);
		void SetFloat(char* dest, float value);
		void SetPointer(char* dest, char* value);

		void RedirectCall(char* dest, char* func);
		void RedirectJump(char* dest, char* func);
}
