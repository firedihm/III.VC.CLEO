#pragma once

#include "Script.h"

class std::filesystem::directory_iterator;

class ScriptManager
{
	public:
		CScript gameScripts[MAX_NUM_SCRIPTS];
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
};

extern ScriptManager scriptMgr;
