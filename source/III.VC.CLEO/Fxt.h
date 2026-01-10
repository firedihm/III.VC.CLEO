#pragma once

namespace fxt
{
		void Add(const char* key, const char* text);
		void Remove(const char* key);

		void LoadEntries();
		void UnloadEntries();

		// this is never called; instead, we jump here from CText::Get(const char* key), which is a __thiscall 
		wchar_t* __thiscall Get(void* pTheText, const char* key);
};
