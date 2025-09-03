#pragma once

class Script;

namespace script_mgr
{
		Script* start_script(const char* filepath);
		void terminate_script(Script* script);
		bool terminate_script(const char* name);

		void load_scripts(bool exe_start);
		void unload_scripts(bool exe_terminate);

		// temporarily disables custom scripts to prevent them from being saved during saving
		void enable_scripts();
		void disable_scripts();

		Script* find_script(const char* name, bool search_generic = false);

		void process_persistent_scripts();
}
