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

		// hooks; rather redundant 
		static void __fastcall InitialiseScript(CScript* script);
		static eOpcodeResult __fastcall ProcessScriptCommand(CScript* script);
		static void __fastcall CollectScriptParameters(CScript* script, int, uint* pIp, uint numParams);
		static int __fastcall CollectScriptNextParameterWithoutIncreasingPC(CScript* script, int, uint ip);
};

extern ScriptManager scriptMgr;
