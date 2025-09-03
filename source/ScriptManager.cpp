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

/*
 *  Persistent scripts are loaded once on game start, unloaded only on game shutdown; run during pause. 
 *	Every persistent script is also custom script, but not vice versa. 
 *	Non-persistent custom scripts are added to game::ppActiveScripts to be processed by game; 
 *	persistent scripts are processed separately in script_mgr::process_persistent_scripts().
 */
std::list<Script> g_custom_scripts;

// separate list is needed to survive game reloads
Script* g_active_persistent_scripts_head;

// case-insensitive strcmp
bool
mystrncasecmp(const char* lhs, const char* rhs, int count)
{
		bool result = true;
		for (int i = 0; result && i < count; ++i)
				result = std::tolower((uchar)lhs[i]) == std::tolower((uchar)rhs[i]);

		return result;
}

Script*
script_mgr::start(int label)
{
		Script* new_script = *game::ppIdleScripts;
		game::RemoveScriptFromList(new_script, 0, game::ppIdleScripts);

		new (new_script) Script(label);
		game::AddScriptToList(new_script, 0, game::ppActiveScripts);

		trace::line("started script at %d", new_script->ip_);

		return new_script;
}

Script*
script_mgr::start(const char* filepath)
{
		Script& new_script = g_custom_scripts.emplace_front(filepath);

		if (new_script.is_persistent_) {
				game::AddScriptToList(&new_script, 0, &g_active_persistent_scripts_head);

				trace::line("started custom persistent script %s", filepath);
		} else {
				game::AddScriptToList(&new_script, 0, game::ppActiveScripts);

				trace::line("started custom script %s", filepath);
		}

		return &new_script;
}

Script*
script_mgr::terminate(Script* script)
{
		Script* next = (Script*)script->next_;

		if (script->mission_flag_)
				*game::pAlreadyRunningAMissionScript = false; // set by LOAD_AND_LAUNCH_MISSION_INTERNAL

		if (!script->is_custom_) {
				trace::line("terminated script %s", script->name_);

				game::RemoveScriptFromList(script, 0, game::ppActiveScripts);
				game::AddScriptToList(script, 0, game::ppIdleScripts);
				script->~Script();
		} else if (script->is_persistent_) {
				trace::line("disabled custom persistent script %s", script->name_);

				game::RemoveScriptFromList(script, 0, &g_active_persistent_scripts_head);
		} else {
				trace::line("terminated custom script %s", script->name_);

				game::RemoveScriptFromList(script, 0, game::ppActiveScripts);
				g_custom_scripts.remove_if([script](Script& current) {
						return &current == script;
				});
		}

		return next;
}

void
script_mgr::load_scripts(bool persistent_scripts_init)
{
		auto load_files = [](const fs::path dir, const char* extension) {
				if (!fs::exists(dir))
						return;

				for (const auto& entry : fs::directory_iterator(dir)) {
						if (entry.is_regular_file() && entry.path().extension().string() == extension)
								start(entry.path().string().c_str());
				}
		};
		
		if (persistent_scripts_init) {
				load_files(fs::path(game::RootDirName) / "CLEO", ".csp");
				load_files(fs::path(game::RootDirName) / "modloader" / "CLEO", ".csp");
		} else {
				load_files(fs::path(game::RootDirName) / "CLEO", ".cs");
				load_files(fs::path(game::RootDirName) / "modloader" / "CLEO", ".cs");
		}
}

void
script_mgr::unload_scripts(bool persistent_scripts_shutdown)
{
		if (persistent_scripts_shutdown) {
				// terminate_script() does not unload persistent scripts...
				for (Script* script = g_active_persistent_scripts_head; script; script = terminate(script));

				// ...so clear them here at once; at this point other custom scripts should've been terminated
				trace::line("terminated all custom persistent scripts");
				g_custom_scripts.clear();
		} else {
				// we unload generic scripts too to avoid memory leaks, as they'll be re-initialised by game on reload
				for (Script* script = *game::ppActiveScripts; script; script = terminate(script));
		}
}

void
script_mgr::disable_scripts()
{
		for (Script& script : g_custom_scripts) {
				if (!script.is_persistent_)
						game::RemoveScriptFromList(&script, 0, game::ppActiveScripts);
		}
}

void
script_mgr::enable_scripts()
{
		for (Script& script : g_custom_scripts) {
				if (!script.is_persistent_)
						game::AddScriptToList(&script, 0, game::ppActiveScripts);
		}
}

Script*
script_mgr::find(const char* name)
{
		for (Script* script = *game::ppActiveScripts; script; script = (Script*)script->next_) {
				if (::mystrncasecmp(script->name_, name, Script::KEY_LENGTH_IN_SCRIPT))
						return script;
		}

		for (Script* script = g_active_persistent_scripts_head; script; script = (Script*)script->next_) {
				if (::mystrncasecmp(script->name_, name, Script::KEY_LENGTH_IN_SCRIPT))
						return script;
		}

		return nullptr;
}

void
script_mgr::process_persistent_scripts()
{
		*game::pCommandsExecuted = 0;
		*game::pScriptsUpdated = 0;

		for (Script* script = g_active_persistent_scripts_head; script; ) {
				Script* next = (Script*)script->next_;

				(*game::pScriptsUpdated)++;

				int time_step = *game::pTimeStep / 50.0 * 1000.0; // GetTimeStepInMilliseconds()
				script->local_timers_[0].nVar += time_step;
				script->local_timers_[1].nVar += time_step;

				script->Process();

				script = next;
		}
		*game::pDbgFlag = false;
}
