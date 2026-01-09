#include "Fxt.h"
#include "Game.h"
#include "Log.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <unordered_map>

namespace fs = std::filesystem;

std::unordered_map<std::string, std::wstring> FxtEntries;

void
Utf8ToUtf16(const char *utf8, wchar16_t *utf16, size_t utf8_len, size_t utf16_len)
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

void
fxt::Add(const char* key, const wchar_t* text)
{
		if (game.bIsChinese)
				std::wstring text(line.substr(text_start, text_end));
		else
				;

		if (auto [iter, result] = FxtEntries.emplace(key, text); result)
				LOGL(LOG_PRIORITY_CUSTOM_TEXT, "Loaded fxt entry: \"%s\"", iter->second.c_str());
}

void
fxt::Remove(const char* key)
{
		LOGL(LOG_PRIORITY_CUSTOM_TEXT, "Unloading custom text \"%s\"", key);
		FxtEntries.erase(key);
}

void
fxt::LoadEntries()
{
		fs::path dir(game.Misc.szRootDirName);
		dir /= "CLEO/CLEO_TEXT";

		for (const auto& entry : fs::directory_iterator(dir)) {
				if (entry.is_regular_file() && entry.path().extension().string() == ".fxt") {
						std::ifstream file(entry.path().string());
						std::string line(512, '\0');

						while (std::getline(file, line)) {
								const char* whitespaces = " \f\n\r\t\v";

								size_t key_start = line.find_first_not_of(whitespaces);
								size_t key_end = line.find_first_of(whitespaces, key_start);
								size_t text_start = line.find_first_not_of(whitespaces, key_end);
								size_t text_end = line.find_first_of(whitespaces, text_start);

								if (key_start == line.npos || text_start == line.npos)
										continue;

								if (line[key_start] == ';' || line[key_start] == '#')
										continue;

								// what is this?
								if (line[text_start] == '\\' && line[text_start + 1] == '$')
										text_start += 2;

								Add(line.substr(key_start, key_end).c_str(), line.substr(text_start, text_end).c_str());
						}
				}
		}
}

void
fxt::UnloadEntries()
{
		LOGL(LOG_PRIORITY_CUSTOM_TEXT, "Unloading fxt entries");
		FxtEntries.clear();
}

wchar_t*
fxt::Get(void* pTheText, const char* key)
{
		if (auto iter = FxtEntries.find(key); iter != FxtEntries.end())
				return iter->second.c_str();
		else
				return game.Text.pfGet(pTheText, key);
}
