#include "Fxt.h"
#include "Game.h"
#include "Log.h"

#include <cstring>
#include <filesystem>

namespace fs = std::filesystem;

FxtEntry* pFxtEntries;

FxtEntry::FxtEntry(char* key, char* text)
{
		size_t len = std::strlen(text);
		m_pText = new wchar_t[len + 1];

		if (game.bIsChinese) {
				CustomText::Utf8ToUtf16(text, m_pText, len, len + 1);
		} else {
				// copy with char to wchar extension
				for (size_t i = 0; i < len; ++i) 
						m_pText[i] = (uchar)text[i];
		}

		m_pText[len] = '\0';
		std::strncpy(m_key, key, 8);
		m_key[7] = '\0';
		m_pNext = nullptr;

		LOGL(LOG_PRIORITY_CUSTOM_TEXT, "Registered custom text: \"%s\", \"%s\"", this->m_key, text);
}

FxtEntry::~FxtEntry()
{
		delete[] m_pText;
		m_pText = nullptr;
}

void CustomText::Utf8ToUtf16(const char *utf8, wchar16_t *utf16, size_t utf8_len, size_t utf16_len)
{
	static auto get_utf8_bytes = [](uint8_t utf8) -> uint8_t
	{
			for (uint8_t i = 0; i < 6; i++)
			{
				if ((utf8 & (0x80 >> i)) == 0)
					return i == 0 ? 1 : i;
			}
			return 1;
	};
	
	size_t len = 0;
	for (size_t i = 0; i < utf8_len && len < utf16_len; i++, len++)
	{
		uint8_t bytes = get_utf8_bytes(static_cast<uint8_t>(utf8[i]));
		if (bytes > 1)
		{
			utf16[len] = utf8[i] & (0xFF >> (bytes + 1));
			for (uint8_t j = 1; j < bytes; j++)
			{
				i++;
				utf16[len] <<= 6;
				utf16[len] += utf8[i] & 0x3F;
			}
		}
		else
		{
			utf16[len] = utf8[i];
		}
	}
	utf16[len] = 0;
}

wchar_t* CustomText::GetText(void* pTheText, int, const char* key)
{
	wchar_t* result = nullptr;
	CustomTextEntry* entry = pCustomTextList;
	while(entry)
	{
		if(!_stricmp(entry->m_key, key))
		{
			result = reinterpret_cast<wchar_t*>(entry->m_pText);
			break;
		}
		entry = entry->m_pNext;
	}
	if(!result)
		result = game.Text.pfGet(pTheText, key);
	if(!result)
		return nullptr;
	return result;
}

char *StrFindKeyBegin(char *str)
{
	while(*str && *str != '\n' && *str != '\r' && *str != ';' && *str != '#')
	{
		if(*str != ' ' && *str != '\t')
			return str;
		str++;
	}
	return nullptr;
}

char *StrFindKeyEnd(char *str)
{
	while(*str && *str != '\n' && *str != '\r' && *str != ';' && *str != '#')
	{
		if(*str == ' ' || *str == '\t')
			return str;
		str++;
	}
	return nullptr;
}

char *StrFindTextBegin(char *str)
{
	while(*str && *str != '\n' && *str != '\r')
	{
		if(str[0] == '\\' && str[1] == '$')
			return str + 2;
		if(*str != ' ' && *str != '\t')
			return str;
		str++;
	}
	return nullptr;
}

char *StrFindTextEnd(char *str)
{
	while(*str != '\0' && *str != '\n' && *str != '\r')
		str++;
	return str;
}

void CustomText::LoadFxtFile(char *filepath)
{
	FILE *fxt = fopen(filepath, "rt");
	char line[512];
	if(fgets(line, 512, fxt))
	{
		do
		{
			char *keyBegin = StrFindKeyBegin(line);
			if(keyBegin)
			{
				char *keyEnd = StrFindKeyEnd(&keyBegin[1]);
				if(keyEnd)
				{
					*keyEnd = '\0';
					char *textBegin = StrFindTextBegin(&keyEnd[1]);
					if(textBegin)
					{
						char *textEnd = StrFindTextEnd(&textBegin[1]);
						*textEnd = '\0';
						CustomTextEntry *entry = new CustomTextEntry(keyBegin, textBegin);
						if(entry)
						{
							entry->m_pNext = CustomText::pCustomTextList;
							CustomText::pCustomTextList = entry;
						}
					}
				}
			}
		}
		while(fgets(line, 512, fxt));
	}
}

void
fxt::Load()
{
		fs::path dir(game.Misc.szRootDirName);
		dir /= "CLEO/CLEO_TEXT";

		for (const auto& entry : fs::directory_iterator(dir)) {
				if (entry.is_regular_file() && entry.path().extension().string() == ".fxt") {
						LoadFxtFile(entry.path().c_str());
				}
		}
}

void
fxt::Unload()
{
		FxtEntry* entry = pFxtEntries;
		while (entry) {
				LOGL(LOG_PRIORITY_CUSTOM_TEXT, "Unloading custom text \"%s\"", entry->m_key);
				FxtEntry* next = entry->m_pNext;
				delete entry;
				entry = next;
		}
		pFxtEntries = nullptr;
}
