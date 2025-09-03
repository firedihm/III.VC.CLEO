#pragma once

namespace fxt
{
		void add(const char* key, const char* text);
		void remove(const char* key);

		void load_entries();
		void unload_entries();

		const wchar_t* __fastcall find(void* pTheText, int, const char* key);
}
