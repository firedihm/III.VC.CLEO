#include "Fxt.h"
#include "Game.h"

#include <cctype>
#include <codecvt>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <locale>
#include <map>
#include <string>

namespace fs = std::filesystem;

std::map<std::string, std::wstring> g_fxt_entries;

std::string
to_upper(const char* c_str)
{
		std::string result;
		result.resize_and_overwrite(std::strlen(c_str), [c_str](char* buf, size_t buf_size) {
				buf[buf_size] = '\0';
				for (int i = 0; i < buf_size; ++i)
						buf[i] = std::toupper(c_str[i]);

				return buf_size;
		});

		return result;
}

void
fxt::add(const char* key, const char* text)
{
		// CJK support makes game text Unicode; it's Extended ASCII with custom encoding by default
		std::wstring wide_text;
		if (game::cjk_lib) {
				std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
				wide_text = converter.from_bytes(text);
		} else {
				wide_text = std::wstring(text, text + std::strlen(text));
		}

		g_fxt_entries.emplace(to_upper(key), std::move(wide_text))
}

void
fxt::remove(const char* key)
{
		g_fxt_entries.erase(to_upper(key));
}

void
fxt::load_entries()
{
		fs::path dir = fs::path(game::RootDirName) / "CLEO" / "CLEO_TEXT";
		std::string line(512, '\0');

		for (const auto& entry : fs::directory_iterator(dir)) {
				if (entry.is_regular_file() && entry.path().extension().string() == ".fxt") {
						std::ifstream file(entry.path().string());

						while (std::getline(file, line)) {
								const char* whitespaces = " \f\n\r\t\v";

								size_t key_start = line.find_first_not_of(whitespaces);
								size_t key_end = line.find_first_of(whitespaces, key_start);
								size_t text_start = line.find_first_not_of(whitespaces, key_end);

								if (key_start == line.npos || text_start == line.npos)
										continue;

								if (line[key_start] == ';' || line[key_start] == '#')
										continue;

								// what is this?
								if (line[text_start] == '\\' && line[text_start + 1] == '$')
										text_start += 2;

								add(line.substr(key_start, key_end).c_str(), line.substr(text_start).c_str());
						}
				}
		}
}

void
fxt::unload_entries()
{
		g_fxt_entries.clear();
}

wchar_t*
fxt::get(void* pTheText, const char* key)
{
		std::string key_upper = to_upper(key);

		if (auto it = g_fxt_entries.find(key_upper); it != g_fxt_entries.end())
				return it->second.c_str();
		else
				return game::GetText(pTheText, key_upper.c_str());
}
