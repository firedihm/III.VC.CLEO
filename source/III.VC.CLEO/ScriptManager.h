#pragma once

class Script;
class std::FILE;
class std::filesystem::directory_iterator;

namespace scriptMgr
{
		Script* StartScript(const char* filepath);
		void TerminateScript(Script* script);

		void LoadScripts();
		void UnloadScripts();

		void EnableScripts();
		void DisableScripts();

		Script* FindScriptNamed(char* name, bool search_mission = false);

		// hooks
		void OnGameStart();
		void OnGameLoad();
		void OnGameReload();
		void OnGameSaveAllScripts(uchar* buf, uint* size);
		void OnGameShutdown();

		// keep track of objects that scripts create, so we won't lose them if scripts get terminated prematurely
		static void SaveMemoryAddress(const void* memory);
		static void DeleteMemoryAddress(const void* memory);
		static void SaveFileStream(const std::FILE* stream);
		static void DeleteFileStream(const std::FILE* stream);
		static void SaveFileSearchHandle(const std::filesystem::directory_iterator* handle);
		static void	DeleteFileSearchHandle(const std::filesystem::directory_iterator* handle);
};
