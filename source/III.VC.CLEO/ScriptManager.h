#pragma once

#include "CustomScript.h"

#ifdef CLEO_VC
	#define SIZE_MAIN_SCRIPT = 225512,
	#define SIZE_MISSION_SCRIPT = 35000,
	#define SIZE_SCRIPT_SPACE = SIZE_MAIN_SCRIPT + SIZE_MISSION_SCRIPT
#else
	#define SIZE_MAIN_SCRIPT = 128 * 1024,
	#define SIZE_MISSION_SCRIPT = 32 * 1024,
	#define SIZE_SCRIPT_SPACE = SIZE_MAIN_SCRIPT + SIZE_MISSION_SCRIPT
#endif

#define MAX_NUM_SCRIPTS 128

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
