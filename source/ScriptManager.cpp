#include "Game.h"
#include "Script.h"
#include "ScriptManager.h"
#include "Trace.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <filesystem>
#include <list>

namespace fs = std::filesystem;

std::list<Script> g_custom_scripts;
Script* pActivePersistentScripts = nullptr;

Script*
script_mgr::start_script(const char* filepath)
{
		trace::line("starting custom script %s", filepath);

		Script& new_script = g_custom_scripts.emplace_front(filepath);

		Script** list = new_script.is_persistent_ ? &pActivePersistentScripts : game::ppActiveScripts;
		game::AddScriptToList(&new_script, 0, list);

		return &new_script;
}

std::list<Script>::iterator
terminate_script(std::list<Script>::iterator it, bool exe_terminate = false)
{
		Script** list = it->is_persistent_ ? &pActivePersistentScripts : game::ppActiveScripts;
		game::RemoveScriptFromList(&(*it), 0, list);

		if (!it->is_persistent_ || exe_terminate) {
				trace::line("terminated custom script %s", it->name_);
				it = g_custom_scripts.erase(it);
		} else {
				it++;
		}
		
		return it;
}

void
script_mgr::terminate_script(Script* script)
{
		// raw pointers will point to active scripts only
		auto it = std::find_if(g_custom_scripts.begin(), g_custom_scripts.end(), [script](Script& current) {
				return &current == script;
		});

		::terminate_script(it);
}

bool
script_mgr::terminate_script(const char* name)
{
		// find by name and terminate if found
		auto it = std::find_if(g_custom_scripts.begin(), g_custom_scripts.end(), [name](Script& current) {
				return !std::strncmp(current.name_, name, Script::KEY_LENGTH_IN_SCRIPT);
		});

		if (it != g_custom_scripts.end()) {
				::terminate_script(it);
				return true;
		} else {
				return false;
		}
}

void
script_mgr::load_scripts(bool exe_start)
{
		fs::path dir = fs::path(game::RootDirName) / "CLEO";

		for (const auto& entry : fs::directory_iterator(dir)) {
				if (entry.is_regular_file() && (!exe_start && entry.path().extension().string() == ".cs" || exe_start && entry.path().extension().string() == ".csp")) {
						start_script(entry.path().string().c_str());
				}
		}
}

void
script_mgr::unload_scripts(bool exe_terminate)
{
		for (auto it = g_custom_scripts.begin(); it != g_custom_scripts.end(); )
				it = ::terminate_script(it, exe_terminate);
}

void
script_mgr::enable_scripts()
{
		for (Script& script : g_custom_scripts)
				game::AddScriptToList(&script, 0, game::ppActiveScripts);
}

void
script_mgr::disable_scripts()
{
		for (Script& script : g_custom_scripts)
				game::RemoveScriptFromList(&script, 0, game::ppActiveScripts);
}

bool
mystrncasecmp(const char* lhs, const char* rhs, int count)
{
		bool result = true;
		for (int i = 0; result && i < count; ++i)
				result = std::tolower(lhs[i]) == std::tolower(rhs[i]);

		return result;
}

Script*
script_mgr::find_script(const char* name, bool search_generic)
{
		for (Script& script : g_custom_scripts) {
				if (!std::strncmp(script.name_, name, Script::KEY_LENGTH_IN_SCRIPT))
						return &script;
		}

		if (search_generic) {
				for (Script* script = *game::ppActiveScripts; script; script = (Script*)script->next_) {
						if (!script->is_custom_ && ::mystrncasecmp(script->name_, name, Script::KEY_LENGTH_IN_SCRIPT))
								return script;
				}
		}

		return nullptr;
}

void
script_mgr::process_persistent_scripts()
{
		*game::pCommandsExecuted = 0;
		*game::pScriptsUpdated = 0;

		Script* script = pActivePersistentScripts;
		while (script) {
				Script* next = (Script*)script->next_;

				(*game::pScriptsUpdated)++;

				int time_step = *game::TimeStep / 50.0 * 1000.0; // GetTimeStepInMilliseconds;
				script->local_timers_[0].nVar += time_step;
				script->local_timers_[1].nVar += time_step;

				game::ProcessScript(script, 0);

				script = next;
		}
		*game::pDbgFlag = false;
}