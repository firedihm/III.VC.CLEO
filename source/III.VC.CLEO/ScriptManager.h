#pragma once

class Script;

namespace scriptMgr
{
		Script* StartScript(const char* filepath);
		void TerminateScript(Script* script);

		void LoadScripts(bool game_start);
		void UnloadScripts(bool game_shutdown);

		// temporarily disables custom scripts to prevent them from being saved during saving
		void EnableScripts();
		void DisableScripts();

		Script* FindScriptNamed(char* name, bool search_generic = false);
};
