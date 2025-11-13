#pragma once

class CScript;
class std::filesystem::directory_iterator;

struct ScriptManager
{
		CScript* pCusomScripts;
		uint numLoadedCustomScripts;

		ScriptManager();

		void LoadScripts();
		void UnloadScripts();

		void EnableAllScripts();
		void DisableAllScripts();

		// keep track of objects that scripts create, so we won't lose them if scripts get terminated prematurely
		static void SaveMemoryAddress(const void* memory);
		static void DeleteMemoryAddress(const void* memory);
		static void SaveFileStream(const FILE* stream);
		static void DeleteFileStream(const FILE* stream);
		static void SaveFileSearchHandle(const std::filesystem::directory_iterator* handle);
		static void	DeleteFileSearchHandle(const std::filesystem::directory_iterator* handle);

		// hooks
		static void InitScripts_OnGameInit();
		static void InitScripts_OnGameReinit();
		static void InitScripts_OnGameSaveLoad();
		static void OnGameSaveScripts(int a, int b);
		static void OnShutdownGame();
		static void OnMenuDrawing(float x, float y, wchar_t* text);
};

extern ScriptManager scriptMgr;
