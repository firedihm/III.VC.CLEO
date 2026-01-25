#include "Fxt.h"
#include "Game.h"
#include "Script.h"
#include "ScriptManager.h"

#include <cstring>
#include <filesystem>
#include <list>

namespace fs = std::filesystem;

std::list<Script> CustomScripts;
std::list<Script> PersistentScripts;

Script*
scriptMgr::StartScript(const char* filepath)
{
		Script script(filepath);

		std::list<Script>& list = script.m_bIsPersistent ? PersistentScripts : CustomScripts;

		list.push_front(std::move(script));

		game.Scripts.pfAddScriptToList(&list.front(), game.Scripts.ppActiveScripts);
		list.front().m_bIsActive = true;

		return &list.front();
}

void
scriptMgr::TerminateScript(Script* script)
{
		game.Scripts.pfRemoveScriptFromList(script, game.Scripts.ppActiveScripts);
		script->m_bIsActive = false;

		std::list<Script>& list = script->m_bIsPersistent ? PersistentScripts : CustomScripts;
		list.remove_if([script](Script& current) {
				&current == script;
		});
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
		auto unload_list = [](std::list<Script>& list) {
				for (auto it = list.begin(); it != list.end(); ) {
						game.Scripts.pfRemoveScriptFromList(&(*it), game.Scripts.ppActiveScripts);
						it->m_bIsActive = false;

						it = list.erase(it);
				}
		};

		unload_list(CustomScripts);
		if (game_shutdown)
				unload_list(PersistentScripts);
}

void
scriptMgr::EnableScripts()
{
		for (Script& script : CustomScripts)
				game.Scripts.pfAddScriptToList(&script, game.Scripts.ppActiveScripts);

		for (Script& script : PersistentScripts)
				game.Scripts.pfAddScriptToList(&script, game.Scripts.ppActiveScripts);
}

void
scriptMgr::DisableScripts()
{
		for (Script& script : CustomScripts)
				game.Scripts.pfRemoveScriptFromList(&script, game.Scripts.ppActiveScripts);

		for (Script& script : PersistentScripts)
				game.Scripts.pfRemoveScriptFromList(&script, game.Scripts.ppActiveScripts);
}

Script*
scriptMgr::FindScriptNamed(char* name, bool search_generic)
{
		for (Script& script : CustomScripts) {
				if (!std::strncmp(&script.m_acName, name, KEY_LENGTH_IN_SCRIPT))
						return &script;
		}

		for (Script& script : PersistentScripts) {
				if (!std::strncmp(&script.m_acName, name, KEY_LENGTH_IN_SCRIPT))
						return &script;
		}

		if (search_generic) {
				for (Script* script = *game.Scripts.ppActiveScripts; script; script = script->m_pNext) {
						if (!std::strncmp(&script->m_acName, name, KEY_LENGTH_IN_SCRIPT))
								return script;
				}
		}

		return nullptr;
}
