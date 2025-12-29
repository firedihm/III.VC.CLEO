#pragma once

class FxtEntry
{
public:
		wchar_t* m_pText;
		char m_key[8];
		FxtEntry* m_pNext;

		FxtEntry(char* key, char* text);
		~FxtEntry() = delete;
};

namespace fxt
{
	void Load();
	void Unload();

	wchar_t* __fastcall GetText(void* pTheText, int, const char* key);

	void LoadFxtFile(char* filepath);

	void Utf8ToUtf16(const char* utf8, wchar_t* utf16, size_t utf8_len, size_t utf16_len);
};
