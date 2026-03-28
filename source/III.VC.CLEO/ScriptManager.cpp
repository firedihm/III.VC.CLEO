#include "Game.h"
#include "Script.h"
#include "ScriptManager.h"

#include <cstring>
#include <filesystem>
#include <list>

namespace fs = std::filesystem;

std::list<Script> g_custom_scripts;

Script*
script_mgr::start_script(const char* filepath)
{
		Script& new_script = g_custom_scripts.emplace_front(filepath);

		game::AddScriptToList(&new_script, game::ppActiveScripts);

		return &new_script;
}

void
script_mgr::terminate_script(Script* script)
{
		game::RemoveScriptFromList(script, game::ppActiveScripts);

		g_custom_scripts.remove_if([script](Script& current) {
				&current == script;
		});
}

void
script_mgr::load_scripts(bool game_start)
{
		fs::path dir = fs::path(game::RootDirName) / "CLEO";

		for (const auto& entry : fs::directory_iterator(dir)) {
				if (entry.is_regular_file() && (!game_start && entry.path().extension().string() == ".cs" || game_start && entry.path().extension().string() == ".csp")) {
						start_script(entry.path().c_str());
				}
		}
}

void
script_mgr::unload_scripts(bool game_shutdown)
{
		for (auto it = g_custom_scripts.begin(); it != g_custom_scripts.end(); ) {
				if (!it->is_persistent_ || game_shutdown) {
						game::RemoveScriptFromList(&(*it), game::ppActiveScripts);
						it = g_custom_scripts.erase(it);
				} else {
						++it;
				}
		}
}

void
script_mgr::enable_scripts()
{
		for (Script& script : g_custom_scripts)
				game::AddScriptToList(&script, game::ppActiveScripts);
}

void
script_mgr::disable_scripts()
{
		for (Script& script : g_custom_scripts)
				game::RemoveScriptFromList(&script, game::ppActiveScripts);
}

Script*
script_mgr::find_script(const char* name, bool search_generic)
{
		for (Script& script : g_custom_scripts) {
				if (!std::strncmp(&script.name_, name, KEY_LENGTH_IN_SCRIPT))
						return &script;
		}

		if (search_generic) {
				for (Script* script = *game::ppActiveScripts; script; script = script->next_) {
						if (!script->is_custom_ && !std::strncmp(&script->name_, name, KEY_LENGTH_IN_SCRIPT))
								return script;
				}
		}

		return nullptr;
}
