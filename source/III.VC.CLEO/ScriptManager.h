#pragma once

class Script;

namespace script_mgr
{
		Script* start_script(const char* filepath);
		void terminate_script(Script* script);

		void load_scripts(bool game_start);
		void unload_scripts(bool game_shutdown);

		// temporarily disables custom scripts to prevent them from being saved during saving
		void enable_scripts();
		void disable_scripts();

		Script* find_script(const char* name, bool search_generic = false);
};
