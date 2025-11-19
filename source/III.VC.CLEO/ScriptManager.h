#pragma once

class CScript;
class std::filesystem::directory_iterator;

// manages custom .cs scripts in a separate list
class ScriptManager
{
	public:
		CScript* pCusomScripts = nullptr;
		CScript* pPersistentScripts = nullptr;

		void LoadScripts();
		void UnloadScripts();

		void EnableScripts();
		void DisableScripts();

		// hooks
		void OnGameStart();
		void OnGameLoad();
		void OnGameReload();
		void OnGameSaveAllScripts(uchar* buf, uint* size);
		void OnGameShutdown();

		// keep track of objects that scripts create, so we won't lose them if scripts get terminated prematurely
		static void SaveMemoryAddress(const void* memory);
		static void DeleteMemoryAddress(const void* memory);
		static void SaveFileStream(const FILE* stream);
		static void DeleteFileStream(const FILE* stream);
		static void SaveFileSearchHandle(const std::filesystem::directory_iterator* handle);
		static void	DeleteFileSearchHandle(const std::filesystem::directory_iterator* handle);
};

extern ScriptManager scriptMgr;
