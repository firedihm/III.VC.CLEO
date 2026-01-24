#pragma once

class Script;
class std::FILE;
class std::filesystem::directory_iterator;

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

		// hooks
		void OnGameStart();
		void OnGameLoad();
		void OnGameReload();
		void OnGameSaveAllScripts(uchar* buf, uint* size);
		void OnGameShutdown();

		// keep track of objects that scripts create, so we won't lose them if scripts get terminated prematurely
		void SaveMemoryAddress(const void* memory);
		void DeleteMemoryAddress(const void* memory);
		void SaveFileStream(const std::FILE* stream);
		void DeleteFileStream(const std::FILE* stream);
		void SaveFileSearchHandle(const std::filesystem::directory_iterator* handle);
		void DeleteFileSearchHandle(const std::filesystem::directory_iterator* handle);
};
