#include "Game.h"
#include "Script.h"
#include "ScriptManager.h"

#include <cstring>
#include <filesystem>
#include <list>

namespace fs = std::filesystem;

std::list<Script> CustomScripts;

Script*
scriptMgr::StartScript(const char* filepath)
{
		Script& new_script = CustomScripts.emplace_front(filepath, fs::file_size(filepath));

		game.Scripts.pfAddScriptToList(&new_script, game.Scripts.ppActiveScripts);

		return &new_script;
}

void
scriptMgr::TerminateScript(Script* script)
{
		game.Scripts.pfRemoveScriptFromList(script, game.Scripts.ppActiveScripts);

		if (!script->m_bIsPersistent) {
				CustomScripts.remove_if([script](Script& current) {
						&current == script;
				});
		}
}

void
scriptMgr::LoadScripts(bool game_start)
{
		fs::path dir = fs::path(game.Misc.szRootDirName) / "CLEO";

		for (const auto& entry : fs::directory_iterator(dir)) {
				if (entry.is_regular_file() && (entry.path().extension().string() == ".cs" || entry.path().extension().string() == ".csp" && game_start)) {
						StartScript(entry.path().c_str());
				}
		}
}

void
scriptMgr::UnloadScripts(bool game_shutdown)
{
		for (auto it = CustomScripts.begin(); it != CustomScripts.end(); ) {
				if (!it->m_bIsPersistent || game_shutdown) {
						game.Scripts.pfRemoveScriptFromList(&(*it), game.Scripts.ppActiveScripts);
						it = CustomScripts.erase(it);
				} else {
						++it;
				}
		}
}

void
scriptMgr::EnableScripts()
{
		for (Script& script : CustomScripts)
				game.Scripts.pfAddScriptToList(&script, game.Scripts.ppActiveScripts);
}

void
scriptMgr::DisableScripts()
{
		for (Script& script : CustomScripts)
				game.Scripts.pfRemoveScriptFromList(&script, game.Scripts.ppActiveScripts);
}

Script*
scriptMgr::FindScriptNamed(char* name, bool search_generic)
{
		for (Script& script : CustomScripts) {
				if (!std::strncmp(&script.m_acName, name, KEY_LENGTH_IN_SCRIPT))
						return &script;
		}

		if (search_generic) {
				for (Script* script = *game.Scripts.ppActiveScripts; script; script = script->m_pNext) {
						if (!script->m_bIsCustom && !std::strncmp(&script->m_acName, name, KEY_LENGTH_IN_SCRIPT))
								return script;
				}
		}

		return nullptr;
}
