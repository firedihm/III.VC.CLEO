#pragma once

class Script;

namespace script_mgr
{
		Script* start(int label); // for generic scripts
		Script* start(const char* filepath); // for custom scripts
		Script* terminate(Script* script);

		void load_scripts(bool persistent_scripts_init);
		void unload_scripts(bool persistent_scripts_shutdown);

		// disables custom scripts processed by game to prevent writing them to savefile
		void disable_scripts();
		void enable_scripts();

		Script* find(const char* name);

		void process_persistent_scripts();
}
