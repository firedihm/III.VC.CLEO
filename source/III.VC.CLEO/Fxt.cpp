#include "Fxt.h"
#include "Game.h"

#include <codecvt>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <locale>
#include <map>
#include <string>

namespace fs = std::filesystem;

std::map<std::string, std::wstring> FxtEntries;

void
fxt::Add(const char* key, const char* text)
{
		// chinese plugin actually makes game text Unicode; they are Extended ASCII with custom encoding by default
		std::wstring wide_text;
		if (game.bIsChinese) {
				std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
				wide_text = converter.from_bytes(text);
		} else {
				wide_text = std::wstring(text, text + std::strlen(text));
		}

		FxtEntries.emplace(key, std::move(wide_text))
}

void
fxt::Remove(const char* key)
{
		FxtEntries.erase(key);
}

void
fxt::LoadEntries()
{
		fs::path dir = fs::path(game.Misc.szRootDirName) / "CLEO" / "CLEO_TEXT";

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
