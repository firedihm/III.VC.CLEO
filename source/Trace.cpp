#include "CLEO.h"
#include "Game.h"
#include "Script.h"
#include "Trace.h"

#include <cstdarg>
#include <cstdio>
#include <fstream>
#include <string>

std::ofstream g_trace_log;
bool g_trace_generic = false;
bool g_trace_custom = false;

void
trace::open()
{
		const char* game_ver_str[(int)game::Version::count] = {
				"GTA III v1.0",
				"GTA III v1.1",
				"GTA III Steam",
				"GTA Vice City v1.0",
				"GTA Vice City v1.1",
				"GTA Vice City Steam"
		};

		if (!g_trace_log.is_open()) {
				g_trace_log.open("CLEO.log", std::ios::trunc);

				uint cleo_ver = cleo::version();
				line("CLEO v%d.%d.%d", (cleo_ver & 0xFF000000) >> 24, (cleo_ver & 0x00FF0000) >> 16, (cleo_ver & 0x0000FF00) >> 8);
				line("%s\n", game_ver_str[(int)game::version]);
		}

		if (std::ifstream inifile("CLEO.ini"); !inifile.fail()) {
				std::string line(256, '\0');

				while (std::getline(inifile, line)) {
						const char* whitespaces = " \f\n\r\t\v";

						size_t start = line.find_first_not_of(whitespaces);
						if (line[start] == '#' || line[start] == ';' || line[start] == '/' && line[start + 1] == '/')
								continue;

						if (size_t key = line.find("trace_generic"); key != line.npos)
								g_trace_generic = line.find("true", key) != line.npos;

						if (size_t key = line.find("trace_custom"); key != line.npos)
								g_trace_custom = line.find("true", key) != line.npos;
				}
		}
}

void
trace::close()
{
		if (g_trace_log.is_open())
				g_trace_log.close();
}

void
trace::line(const char* format, ...)
{
		char buff[512];
		std::va_list args;

		va_start(args, format);
		int num_written = std::vsnprintf(buff, sizeof(buff), format, args);
		va_end(args);

		if (num_written >= sizeof(buff))
				num_written = sizeof(buff) - 1;

		buff[num_written] = '\n';
		g_trace_log.write(buff, num_written + 1).flush();
}

void
trace::opcode(Script* script, ushort op)
{
		if (g_trace_generic && !script->is_custom_ || g_trace_custom && script->is_custom_)
				line("%s: executed %04X", script->name_, op);
}