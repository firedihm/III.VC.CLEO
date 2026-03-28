#pragma once

namespace fxt
{
		void add(const char* key, const char* text);
		void remove(const char* key);

		void load_entries();
		void unload_entries();

		// this is never called; instead, we jump here from CText::Get(const char* key), which is a __thiscall 
		wchar_t* __thiscall get(void* pTheText, const char* key);
};
