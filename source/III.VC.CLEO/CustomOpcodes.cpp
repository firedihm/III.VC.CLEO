#include "CleoVersion.h"
#include "CustomOpcodes.h"
#include "Fxt.h"
#include "Game.h"
#include "Log.h"
#include "Memory.h"
#include "OpcodesSystem.h"
#include "Script.h"
#include "ScriptManager.h"

#include <cstring>
#include <filesystem>
#include <fstream>
#include <limits>

namespace fs = std::filesystem;

#define HELP_MSG_LENGTH 256

int format(Script *script, char *str, size_t len, const char *format);

ScriptParam CustomOpcodes::SHARED_VAR[0xFFFF];

// Exports
CLEOAPI unsigned CLEO_GetVersion() { return CLEO_VERSION; }
CLEOAPI char* CLEO_GetScriptSpaceAddress() { return game.Scripts.pScriptSpace; }
CLEOAPI ScriptParam* CLEO_GetParamsAddress() { return game.Scripts.pScriptParams; }

#ifdef __cplusplus
extern "C" {
#endif
	unsigned __stdcall _CLEO_GetVersion() { return CLEO_GetVersion(); }
	char* __stdcall _CLEO_GetScriptSpaceAddress() { return CLEO_GetScriptSpaceAddress(); }
	ScriptParam* __stdcall _CLEO_GetParamsAddress() { return CLEO_GetParamsAddress(); }
	bool __stdcall CLEO_RegisterOpcode(unsigned short id, Opcode func) { return Opcodes::RegisterOpcode(id, func); }

	// Script methods
	void __stdcall CLEO_Collect(Script* script, unsigned int numParams) { script->Collect(numParams); }
	void __stdcall CLEO_CollectAt(Script* script, unsigned int* pIp, unsigned int numParams) { script->Collect(pIp, numParams); }
	int __stdcall CLEO_CollectNextWithoutIncreasingPC(Script* script, unsigned int ip) { return script->CollectNextWithoutIncreasingPC(ip); }
	eParamType __stdcall CLEO_GetNextParamType(Script* script) { return script->GetNextParamType(); }
	void __stdcall CLEO_Store(Script* script, unsigned int numParams) { script->Store(numParams); }
	void __stdcall CLEO_ReadShortString(Script* script, char* out) { script->ReadShortString(out); }
	void __stdcall CLEO_UpdateCompareFlag(Script* script, bool result) { script->UpdateCompareFlag(result); }
	void* __stdcall CLEO_GetPointerToScriptVariable(Script* script) { return script->GetPointerToScriptVariable(); }
	void __stdcall CLEO_JumpTo(Script* script, int address) { script->JumpTo(address); }
#ifdef __cplusplus
}
#endif

void CustomOpcodes::Register()
{
	Opcodes::RegisterOpcode(0x0002, GOTO);
	Opcodes::RegisterOpcode(0x004C, GOTO_IF_TRUE);
	Opcodes::RegisterOpcode(0x004D, GOTO_IF_FALSE);
	Opcodes::RegisterOpcode(0x0050, GOSUB);
	Opcodes::RegisterOpcode(0x05DC, TERMINATE_CUSTOM_THREAD);
	Opcodes::RegisterOpcode(0x05DD, TERMINATE_NAMED_CUSTOM_THREAD);
	Opcodes::RegisterOpcode(0x05DE, START_CUSTOM_THREAD);
	Opcodes::RegisterOpcode(0x05DF, MEMORY_WRITE);
	Opcodes::RegisterOpcode(0x05E0, MEMORY_READ);
	Opcodes::RegisterOpcode(0x05E1, CALL);
	Opcodes::RegisterOpcode(0x05E2, CALL_FUNCTION);
	Opcodes::RegisterOpcode(0x05E3, CALL_METHOD);
	Opcodes::RegisterOpcode(0x05E4, CALL_FUNCTION_METHOD);
	Opcodes::RegisterOpcode(0x05E5, GET_GAME_VERSION);
	Opcodes::RegisterOpcode(0x05E6, GET_CHAR_STRUCT);
	Opcodes::RegisterOpcode(0x05E7, GET_CAR_STRUCT);
	Opcodes::RegisterOpcode(0x05E8, GET_OBJECT_STRUCT);
	Opcodes::RegisterOpcode(0x05E9, GET_CHAR_HANDLE);
	Opcodes::RegisterOpcode(0x05EA, GET_CAR_HANDLE);
	Opcodes::RegisterOpcode(0x05EB, GET_OBJECT_HANDLE);
	Opcodes::RegisterOpcode(0x05EC, GET_THREAD_POINTER);
	Opcodes::RegisterOpcode(0x05ED, GET_NAMED_THREAD_POINTER);
	Opcodes::RegisterOpcode(0x05EE, IS_KEY_PRESSED);
	Opcodes::RegisterOpcode(0x05EF, FIND_RANDOM_CHAR);
	Opcodes::RegisterOpcode(0x05F0, FIND_RANDOM_CAR);
	Opcodes::RegisterOpcode(0x05F1, FIND_RANDOM_OBJECT);
	Opcodes::RegisterOpcode(0x05F2, CALL_POP_FLOAT);
	Opcodes::RegisterOpcode(0x05F3, MATH_EXP);
	Opcodes::RegisterOpcode(0x05F4, MATH_LOG);
	Opcodes::RegisterOpcode(0x05F5, CALL_SCM_FUNCTION);
	Opcodes::RegisterOpcode(0x05F6, SCM_FUNCTION_RET);
	Opcodes::RegisterOpcode(0x05F7, GET_LABEL_OFFSET);
	Opcodes::RegisterOpcode(0x05F8, GET_VAR_OFFSET);
	Opcodes::RegisterOpcode(0x05F9, BIT_AND);
	Opcodes::RegisterOpcode(0x05FA, BIT_OR);
	Opcodes::RegisterOpcode(0x05FB, BIT_XOR);
	Opcodes::RegisterOpcode(0x05FC, BIT_NOT);
	Opcodes::RegisterOpcode(0x05FD, BIT_MOD);
	Opcodes::RegisterOpcode(0x05FE, BIT_SHR);
	Opcodes::RegisterOpcode(0x05FF, BIT_SHL);

	//CLEO4 SA opcodes including duplicates with new ids
	Opcodes::RegisterOpcode(0x0A8C, MEMORY_WRITE);
	Opcodes::RegisterOpcode(0x0A8D, MEMORY_READ);
	Opcodes::RegisterOpcode(0x0A8E, OPCODE_0A8E);
	Opcodes::RegisterOpcode(0x0A8F, OPCODE_0A8F);
	Opcodes::RegisterOpcode(0x0A90, OPCODE_0A90);
	Opcodes::RegisterOpcode(0x0A91, OPCODE_0A91);
	Opcodes::RegisterOpcode(0x0A92, START_CUSTOM_THREAD_VSTRING);
	Opcodes::RegisterOpcode(0x0A93, TERMINATE_CUSTOM_THREAD);
	Opcodes::RegisterOpcode(0x0A94, DUMMY);
	Opcodes::RegisterOpcode(0x0A95, DUMMY);
	Opcodes::RegisterOpcode(0x0A96, GET_CHAR_STRUCT);
	Opcodes::RegisterOpcode(0x0A97, GET_CAR_STRUCT);
	Opcodes::RegisterOpcode(0x0A98, GET_OBJECT_STRUCT);
	Opcodes::RegisterOpcode(0x0A99, OPCODE_0A99);
	Opcodes::RegisterOpcode(0x0A9A, OPCODE_0A9A);
	Opcodes::RegisterOpcode(0x0A9B, OPCODE_0A9B);
	Opcodes::RegisterOpcode(0x0A9C, OPCODE_0A9C);
	Opcodes::RegisterOpcode(0x0A9D, OPCODE_0A9D);
	Opcodes::RegisterOpcode(0x0A9E, OPCODE_0A9E);
	Opcodes::RegisterOpcode(0x0A9F, GET_THREAD_POINTER);
	Opcodes::RegisterOpcode(0x0AA0, OPCODE_0AA0);
	Opcodes::RegisterOpcode(0x0AA1, OPCODE_0AA1);
	Opcodes::RegisterOpcode(0x0AA2, OPCODE_0AA2);
	Opcodes::RegisterOpcode(0x0AA3, OPCODE_0AA3);
	Opcodes::RegisterOpcode(0x0AA4, OPCODE_0AA4);
	Opcodes::RegisterOpcode(0x0AA5, CALL);
	Opcodes::RegisterOpcode(0x0AA6, CALL_METHOD);
	Opcodes::RegisterOpcode(0x0AA7, CALL_FUNCTION);
	Opcodes::RegisterOpcode(0x0AA8, CALL_FUNCTION_METHOD);
	Opcodes::RegisterOpcode(0x0AA9, OPCODE_0AA9);
	Opcodes::RegisterOpcode(0x0AAA, GET_NAMED_THREAD_POINTER);
	Opcodes::RegisterOpcode(0x0AAB, OPCODE_0AAB);
	Opcodes::RegisterOpcode(0x0AAC, DUMMY);
	Opcodes::RegisterOpcode(0x0AAD, DUMMY);
	Opcodes::RegisterOpcode(0x0AAE, DUMMY);
	Opcodes::RegisterOpcode(0x0AAF, DUMMY);
	Opcodes::RegisterOpcode(0x0AB0, IS_KEY_PRESSED);
	Opcodes::RegisterOpcode(0x0AB1, CALL_SCM_FUNCTION);
	Opcodes::RegisterOpcode(0x0AB2, SCM_FUNCTION_RET);
	Opcodes::RegisterOpcode(0x0AB3, OPCODE_0AB3);
	Opcodes::RegisterOpcode(0x0AB4, OPCODE_0AB4);
	Opcodes::RegisterOpcode(0x0AB5, DUMMY);
	Opcodes::RegisterOpcode(0x0AB6, DUMMY);
	Opcodes::RegisterOpcode(0x0AB7, OPCODE_0AB7);
	Opcodes::RegisterOpcode(0x0AB8, OPCODE_0AB8);
	Opcodes::RegisterOpcode(0x0AB9, DUMMY);
	Opcodes::RegisterOpcode(0x0ABA, TERMINATE_NAMED_CUSTOM_THREAD);
	Opcodes::RegisterOpcode(0x0ABB, DUMMY);
	Opcodes::RegisterOpcode(0x0ABC, DUMMY);
	Opcodes::RegisterOpcode(0x0ABD, OPCODE_0ABD);
	Opcodes::RegisterOpcode(0x0ABE, OPCODE_0ABE);
	Opcodes::RegisterOpcode(0x0ABF, OPCODE_0ABF);
	Opcodes::RegisterOpcode(0x0AC0, DUMMY);
	Opcodes::RegisterOpcode(0x0AC1, DUMMY);
	Opcodes::RegisterOpcode(0x0AC2, DUMMY);
	Opcodes::RegisterOpcode(0x0AC3, DUMMY);
	Opcodes::RegisterOpcode(0x0AC4, DUMMY);
	Opcodes::RegisterOpcode(0x0AC5, DUMMY);
	Opcodes::RegisterOpcode(0x0AC6, GET_LABEL_OFFSET);
	Opcodes::RegisterOpcode(0x0AC7, GET_VAR_OFFSET);
	Opcodes::RegisterOpcode(0x0AC8, OPCODE_0AC8);
	Opcodes::RegisterOpcode(0x0AC9, OPCODE_0AC9);
	Opcodes::RegisterOpcode(0x0ACA, OPCODE_0ACA);
	Opcodes::RegisterOpcode(0x0ACB, OPCODE_0ACB);
	Opcodes::RegisterOpcode(0x0ACC, OPCODE_0ACC);
	Opcodes::RegisterOpcode(0x0ACD, OPCODE_0ACD);
	Opcodes::RegisterOpcode(0x0ACE, OPCODE_0ACE);
	Opcodes::RegisterOpcode(0x0ACF, OPCODE_0ACF);
	Opcodes::RegisterOpcode(0x0AD0, OPCODE_0AD0);
	Opcodes::RegisterOpcode(0x0AD1, OPCODE_0AD1);
	Opcodes::RegisterOpcode(0x0AD2, DUMMY);
	Opcodes::RegisterOpcode(0x0AD3, OPCODE_0AD3);
	Opcodes::RegisterOpcode(0x0AD4, OPCODE_0AD4);
	Opcodes::RegisterOpcode(0x0AD5, OPCODE_0AD5);
	Opcodes::RegisterOpcode(0x0AD6, OPCODE_0AD6);
	Opcodes::RegisterOpcode(0x0AD7, OPCODE_0AD7);
	Opcodes::RegisterOpcode(0x0AD8, OPCODE_0AD8);
	Opcodes::RegisterOpcode(0x0AD9, OPCODE_0AD9);
	Opcodes::RegisterOpcode(0x0ADA, OPCODE_0ADA);
	Opcodes::RegisterOpcode(0x0ADB, OPCODE_0ADB);
	Opcodes::RegisterOpcode(0x0ADC, OPCODE_0ADC);
	Opcodes::RegisterOpcode(0x0ADD, OPCODE_0ADD);
	Opcodes::RegisterOpcode(0x0ADE, OPCODE_0ADE);
	Opcodes::RegisterOpcode(0x0ADF, OPCODE_0ADF);
	Opcodes::RegisterOpcode(0x0AE0, OPCODE_0AE0);
	Opcodes::RegisterOpcode(0x0AE1, FIND_RANDOM_CHAR);
	Opcodes::RegisterOpcode(0x0AE2, FIND_RANDOM_CAR);
	Opcodes::RegisterOpcode(0x0AE3, FIND_RANDOM_OBJECT);
	Opcodes::RegisterOpcode(0x0AE4, OPCODE_0AE4);
	Opcodes::RegisterOpcode(0x0AE5, OPCODE_0AE5);
	Opcodes::RegisterOpcode(0x0AE6, OPCODE_0AE6);
	Opcodes::RegisterOpcode(0x0AE7, OPCODE_0AE7);
	Opcodes::RegisterOpcode(0x0AE8, OPCODE_0AE8);
	Opcodes::RegisterOpcode(0x0AE9, CALL_POP_FLOAT);
	Opcodes::RegisterOpcode(0x0AEA, GET_CHAR_HANDLE);
	Opcodes::RegisterOpcode(0x0AEB, GET_CAR_HANDLE);
	Opcodes::RegisterOpcode(0x0AEC, GET_OBJECT_HANDLE);
	Opcodes::RegisterOpcode(0x0AED, DUMMY);
	Opcodes::RegisterOpcode(0x0AEE, MATH_EXP);
	Opcodes::RegisterOpcode(0x0AEF, MATH_LOG);

	//CLEO 2 opcodes
	Opcodes::RegisterOpcode(0x0600, START_CUSTOM_THREAD_VSTRING);
	Opcodes::RegisterOpcode(0x0601, IS_BUTTON_PRESSED_ON_PAD);
	Opcodes::RegisterOpcode(0x0602, EMULATE_BUTTON_PRESS_ON_PAD);
	Opcodes::RegisterOpcode(0x0603, IS_CAMERA_IN_WIDESCREEN_MODE);
	Opcodes::RegisterOpcode(0x0604, GET_MODEL_ID_FROM_WEAPON_ID);
	Opcodes::RegisterOpcode(0x0605, GET_WEAPON_ID_FROM_MODEL_ID);
	Opcodes::RegisterOpcode(0x0606, SET_MEM_OFFSET);
	Opcodes::RegisterOpcode(0x0607, GET_CURRENT_WEATHER);
	Opcodes::RegisterOpcode(0x0608, SHOW_TEXT_POSITION);
	Opcodes::RegisterOpcode(0x0609, SHOW_FORMATTED_TEXT_POSITION);
	Opcodes::RegisterOpcode(0x0673, PLAY_ANIMATION);

#if CLEO_VC
	//Scrapped opcodes (VC)
	Opcodes::RegisterOpcode(0x016F, DRAW_SHADOW);
	Opcodes::RegisterOpcode(0x0349, SET_TEXT_DRAW_FONT);
#else
	//Original opcodes added since VC
	Opcodes::RegisterOpcode(0x04C2, STORE_COORDS_FROM_OBJECT_WITH_OFFSET); //0400
	Opcodes::RegisterOpcode(0x04C3, STORE_COORDS_FROM_CAR_WITH_OFFSET); //0407
	Opcodes::RegisterOpcode(0x04C4, STORE_COORDS_FROM_ACTOR_WITH_OFFSET);

	Opcodes::RegisterOpcode(0x046F, STORE_PLAYER_CURRENTLY_ARMED_WEAPON);
	Opcodes::RegisterOpcode(0x04DD, GET_CHAR_ARMOUR);

	Opcodes::RegisterOpcode(0x04C9, PLAYER_DRIVING_PLANE);
	Opcodes::RegisterOpcode(0x04A8, PLAYER_DRIVING_BOAT);
	Opcodes::RegisterOpcode(0x04AA, PLAYER_DRIVING_HELI);
	Opcodes::RegisterOpcode(0x047E, PLAYER_DRIVING_A_MOTORBIKE);
	Opcodes::RegisterOpcode(0x0485, IS_PC_VERSION);
	Opcodes::RegisterOpcode(0x059A, IS_AUSTRALIAN_GAME);
#endif

	// CLEO 2.1 opcodes
	Opcodes::RegisterOpcode(0x0AF8, SET_CLEO_ARRAY);
	Opcodes::RegisterOpcode(0x0AF9, GET_CLEO_ARRAY);
	Opcodes::RegisterOpcode(0x0AFA, GET_CLEO_ARRAY_OFFSET);
	Opcodes::RegisterOpcode(0x0AFB, GET_CLEO_ARRAY_SCRIPT);
	Opcodes::RegisterOpcode(0x0DD5, GET_PLATFORM);
}

eOpcodeResult
CustomOpcodes::DUMMY(Script* script)
{
		return OR_CONTINUE;
}

eOpcodeResult
CustomOpcodes::GOTO(Script* script)
{
		script->Collect(1);

		script->JumpTo(game.Scripts.pScriptParams[0].nVar);

		return OR_CONTINUE;
}

eOpcodeResult CustomOpcodes::GOTO_IF_TRUE(Script *script)
{
	script->Collect(1);
	if(script->m_bCondResult)
		script->JumpTo(game.Scripts.pScriptParams[0].nVar);
	return OR_CONTINUE;
}

eOpcodeResult CustomOpcodes::GOTO_IF_FALSE(Script *script)
{
	script->Collect(1);
	if(!script->m_bCondResult)
		script->JumpTo(game.Scripts.pScriptParams[0].nVar);
	return OR_CONTINUE;
}

eOpcodeResult
CustomOpcodes::GOSUB(Script* script)
{
		script->Collect(1);

		script->m_anGosubStack[script->m_nGosubStackPointer++] = script->m_nIp;
		script->JumpTo(game.Scripts.pScriptParams[0].nVar);

		return OR_CONTINUE;
}

eOpcodeResult
CustomOpcodes::TERMINATE_CUSTOM_THREAD(Script* script)
{
		LOGL(LOG_PRIORITY_OPCODE, "TERMINATE_CUSTOM_THREAD: Terminating custom script \"%s\"", &script->m_acName);
		scriptMgr::TerminateScript(script);

		return OR_TERMINATE;
}

eOpcodeResult
CustomOpcodes::TERMINATE_NAMED_CUSTOM_THREAD(Script* script)
{
		char name[KEY_LENGTH_IN_SCRIPT];
		script->ReadShortString(&name);

		if (Script* found = scriptMgr::FindScriptNamed(&name); found) {
				script->UpdateCompareFlag(true);

				LOGL(LOG_PRIORITY_OPCODE, "TERMINATE_NAMED_CUSTOM_THREAD: Terminating custom script \"%s\"", found->m_acName);
				scriptMgr::TerminateScript(found);

				return (found == script) ? OR_TERMINATE : OR_CONTINUE;
		} else {
				script->UpdateCompareFlag(false);

				LOGL(LOG_PRIORITY_OPCODE, "TERMINATE_NAMED_CUSTOM_THREAD: Found no scripts named \"%s\"", &name);

				return OR_CONTINUE;
		}
}

eOpcodeResult
CustomOpcodes::START_CUSTOM_THREAD(Script* script)
{
		// there's START_CUSTOM_THREAD_VSTRING that accepts strings instead of labels
		char filename[KEY_LENGTH_IN_SCRIPT];
		script->ReadShortString(&filename);

		fs::path filepath = fs::path(game.Misc.szRootDirName) / "CLEO" / &filename;

		try {
				LOGL(LOG_PRIORITY_OPCODE, "START_CUSTOM_THREAD: Starting new script \"%s\"", filepath.c_str());
				Script* new_script = scriptMgr::StartScript(filepath.c_str());

				for (int i = 0; script->GetNextParamType(); ++i) {
						script->Collect(1);
						new_script->m_aLVars[i].nVar = game.Scripts.pScriptParams[0].nVar;
				}
				script->m_nIp++;

				script->UpdateCompareFlag(true);

				return OR_CONTINUE;
		} catch (const char* e) {
				LOGL(LOG_PRIORITY_OPCODE, "START_CUSTOM_THREAD: Script loading failed, \"%s\"", e);
		} catch (...) {
				LOGL(LOG_PRIORITY_OPCODE, "START_CUSTOM_THREAD: Script loading failed, \"%s\"", "Unknown exception");
		}

		// just skip all params if we fail
		while (script->GetNextParamType()) {
				script->Collect(1);
		}
		script->m_nIp++;

		script->UpdateCompareFlag(false);

		return OR_CONTINUE;
}

eOpcodeResult
CustomOpcodes::MEMORY_WRITE(Script* script)
{
		script->Collect(4);

		memory::Write(game.Scripts.pScriptParams[0].pVar, game.Scripts.pScriptParams[2].nVar, game.Scripts.pScriptParams[1].nVar, game.Scripts.pScriptParams[3].nVar);

		return OR_CONTINUE;
}

eOpcodeResult
CustomOpcodes::MEMORY_READ(Script* script)
{
		script->Collect(3);

		game.Scripts.pScriptParams[0].nVar = memory::Read(game.Scripts.pScriptParams[0].pVar, game.Scripts.pScriptParams[1].nVar, game.Scripts.pScriptParams[2].nVar);
		script->Store(1);

		return OR_CONTINUE;
}

eOpcodeResult CustomOpcodes::CALL(Script *script)
{
	script->Collect(3);
	void *func = game.Scripts.pScriptParams[0].pVar;
	unsigned int popsize = game.Scripts.pScriptParams[2].nVar * 4;
	for(int i = 0; i < game.Scripts.pScriptParams[1].nVar; i++)
	{
		script->Collect(1);
		unsigned int param = game.Scripts.pScriptParams[0].nVar;
		__asm push param
	}
	__asm call func
	__asm add esp, popsize
	while((*(ScriptParamType *)(&game.Scripts.pScriptSpace[script->m_nIp])).type)
		script->Collect(1);
	script->m_nIp++;
	return OR_CONTINUE;
}

eOpcodeResult CustomOpcodes::CALL_FUNCTION(Script *script)
{
	script->Collect(3);
	void *func = game.Scripts.pScriptParams[0].pVar;
	int func_result;
	unsigned int popsize = game.Scripts.pScriptParams[2].nVar * 4;
	for(int i = 0; i < game.Scripts.pScriptParams[1].nVar; i++)
	{
		script->Collect(1);
		unsigned int param = game.Scripts.pScriptParams[0].nVar;
		__asm push param
	}
	__asm call func
	__asm mov func_result, eax
	__asm add esp, popsize
	game.Scripts.pScriptParams[0].nVar = func_result;
	script->Store(1);
	while((*(ScriptParamType *)(&game.Scripts.pScriptSpace[script->m_nIp])).type)
		script->Collect(1);
	script->m_nIp++;
	return OR_CONTINUE;
}

eOpcodeResult CustomOpcodes::CALL_METHOD(Script *script)
{
	script->Collect(4);
	void *func = game.Scripts.pScriptParams[0].pVar;
	void *object = game.Scripts.pScriptParams[1].pVar;
	unsigned int popsize = game.Scripts.pScriptParams[3].nVar * 4;
	for(int i = 0; i < game.Scripts.pScriptParams[2].nVar; i++)
	{
		script->Collect(1);
		unsigned int param = game.Scripts.pScriptParams[0].nVar;
		__asm push param
	}
	__asm mov ecx, object
	__asm call func
	__asm add esp, popsize
	while((*(ScriptParamType *)(&game.Scripts.pScriptSpace[script->m_nIp])).type)
		script->Collect(1);
	script->m_nIp++;
	return OR_CONTINUE;
}

eOpcodeResult CustomOpcodes::CALL_FUNCTION_METHOD(Script *script)
{
	script->Collect(4);
	void *func = game.Scripts.pScriptParams[0].pVar;
	void *object = game.Scripts.pScriptParams[1].pVar;
	int func_result;
	unsigned int popsize = game.Scripts.pScriptParams[3].nVar * 4;
	for(int i = 0; i < game.Scripts.pScriptParams[2].nVar; i++)
	{
		script->Collect(1);
		unsigned int param = game.Scripts.pScriptParams[0].nVar;
		__asm push param
	}
	__asm mov ecx, object
	__asm call func
	__asm mov func_result, eax
	__asm add esp, popsize
	game.Scripts.pScriptParams[0].nVar = func_result;
	script->Store(1);
	while((*(ScriptParamType *)(&game.Scripts.pScriptSpace[script->m_nIp])).type)
		script->Collect(1);
	script->m_nIp++;
	return OR_CONTINUE;
}

eOpcodeResult
CustomOpcodes::GET_GAME_VERSION(Script* script)
{
		int result = (game.Version == GAME_GTAVC_V1_0 || game.Version == GAME_GTA3_V1_0) ? 0 :
					 (game.Version == GAME_GTAVC_V1_1 || game.Version == GAME_GTA3_V1_1) ? 1 : 2;

		game.Scripts.pScriptParams[0].nVar = result;
		script->Store(1);

		return OR_CONTINUE;
}

eOpcodeResult
CustomOpcodes::GET_CHAR_STRUCT(Script* script)
{
		script->Collect(1);

		game.Scripts.pScriptParams[0].pVar = game.Pools.pfPedPoolGetAt(*game.Pools.ppPedPool, game.Scripts.pScriptParams[0].nVar);
		script->Store(1);

		return OR_CONTINUE;
}

eOpcodeResult
CustomOpcodes::GET_CAR_STRUCT(Script* script)
{
		script->Collect(1);

		game.Scripts.pScriptParams[0].pVar = game.Pools.pfVehiclePoolGetAt(*game.Pools.ppVehiclePool, game.Scripts.pScriptParams[0].nVar);
		script->Store(1);

		return OR_CONTINUE;
}

eOpcodeResult
CustomOpcodes::GET_OBJECT_STRUCT(Script* script)
{
		script->Collect(1);

		game.Scripts.pScriptParams[0].pVar = game.Pools.pfObjectPoolGetAt(*game.Pools.ppObjectPool, game.Scripts.pScriptParams[0].nVar);
		script->Store(1);

		return OR_CONTINUE;
}

eOpcodeResult
CustomOpcodes::GET_CHAR_HANDLE(Script* script)
{
		script->Collect(1);

		game.Scripts.pScriptParams[0].nVar = game.Pools.pfPedPoolGetIndex(*game.Pools.ppPedPool, game.Scripts.pScriptParams[0].pVar);
		script->Store(1);

		return OR_CONTINUE;
}

eOpcodeResult
CustomOpcodes::GET_CAR_HANDLE(Script* script)
{
		script->Collect(1);

		game.Scripts.pScriptParams[0].nVar = game.Pools.pfVehiclePoolGetIndex(*game.Pools.ppVehiclePool, game.Scripts.pScriptParams[0].pVar);
		script->Store(1);

		return OR_CONTINUE;
}

eOpcodeResult
CustomOpcodes::GET_OBJECT_HANDLE(Script* script)
{
		script->Collect(1);

		game.Scripts.pScriptParams[0].nVar = game.Pools.pfObjectPoolGetIndex(*game.Pools.ppObjectPool, game.Scripts.pScriptParams[0].pVar);
		script->Store(1);

		return OR_CONTINUE;
}

eOpcodeResult
CustomOpcodes::GET_THREAD_POINTER(Script* script)
{
		game.Scripts.pScriptParams[0].pVar = script;
		script->Store(1);

		return OR_CONTINUE;
}

eOpcodeResult
CustomOpcodes::GET_NAMED_THREAD_POINTER(Script* script)
{
		char name[KEY_LENGTH_IN_SCRIPT];
		script->ReadShortString(name);

		Script* found = scriptMgr::FindScriptNamed(&name, true);
		script->UpdateCompareFlag(found);
		game.Scripts.pScriptParams[0].pVar = found;
		script->Store(1);

		return OR_CONTINUE;
}

eOpcodeResult CustomOpcodes::IS_KEY_PRESSED(Script *script)
{
	script->Collect(1);
#if CLEO_VC
	switch (game.Scripts.pScriptParams[0].nVar)
	{
	case OVK_F1:
		game.Scripts.pScriptParams[0].nVar = VK_F1;
		break;
	case OVK_F2:
		game.Scripts.pScriptParams[0].nVar = VK_F2;
		break;
	case OVK_F3:
		game.Scripts.pScriptParams[0].nVar = VK_F3;
		break;
	case OVK_F4:
		game.Scripts.pScriptParams[0].nVar = VK_F4;
		break;
	case OVK_F5:
		game.Scripts.pScriptParams[0].nVar = VK_F5;
		break;
	case OVK_F6:
		game.Scripts.pScriptParams[0].nVar = VK_F6;
		break;
	case OVK_F7:
		game.Scripts.pScriptParams[0].nVar = VK_F7;
		break;
	case OVK_F8:
		game.Scripts.pScriptParams[0].nVar = VK_F8;
		break;
	case OVK_F9:
		game.Scripts.pScriptParams[0].nVar = VK_F9;
		break;
	case OVK_F10:
		game.Scripts.pScriptParams[0].nVar = VK_F10;
		break;
	case OVK_F11:
		game.Scripts.pScriptParams[0].nVar = VK_F11;
		break;
	case OVK_F12:
		game.Scripts.pScriptParams[0].nVar = VK_F12;
		break;
	case OVK_INSERT:
		game.Scripts.pScriptParams[0].nVar = VK_INSERT;
		break;
	case OVK_DELETE:
		game.Scripts.pScriptParams[0].nVar = VK_DELETE;
		break;
	case OVK_HOME:
		game.Scripts.pScriptParams[0].nVar = VK_HOME;
		break;
	case OVK_END:
		game.Scripts.pScriptParams[0].nVar = VK_END;
		break;
	case OVK_PRIOR:
		game.Scripts.pScriptParams[0].nVar = VK_PRIOR;
		break;
	case OVK_NEXT:
		game.Scripts.pScriptParams[0].nVar = VK_NEXT;
		break;
	case OVK_UP:
		game.Scripts.pScriptParams[0].nVar = VK_UP;
		break;
	case OVK_DOWN:
		game.Scripts.pScriptParams[0].nVar = VK_DOWN;
		break;
	case OVK_LEFT:
		game.Scripts.pScriptParams[0].nVar = VK_LEFT;
		break;
	case OVK_RIGHT:
		game.Scripts.pScriptParams[0].nVar = VK_RIGHT;
		break;
	case OVK_DIVIDE:
		game.Scripts.pScriptParams[0].nVar = VK_DIVIDE;
		break;
	case OVK_MULTIPLY:
		game.Scripts.pScriptParams[0].nVar = VK_MULTIPLY;
		break;
	case OVK_ADD:
		game.Scripts.pScriptParams[0].nVar = VK_ADD;
		break;
	case OVK_SUBTRACT:
		game.Scripts.pScriptParams[0].nVar = VK_SUBTRACT;
		break;
	case OVK_DECIMAL:
		game.Scripts.pScriptParams[0].nVar = VK_DECIMAL;
		break;
	case OVK_NUMPAD1:
		game.Scripts.pScriptParams[0].nVar = VK_NUMPAD1;
		break;
	case OVK_NUMPAD2:
		game.Scripts.pScriptParams[0].nVar = VK_NUMPAD2;
		break;
	case OVK_NUMPAD3:
		game.Scripts.pScriptParams[0].nVar = VK_NUMPAD3;
		break;
	case OVK_NUMPAD4:
		game.Scripts.pScriptParams[0].nVar = VK_NUMPAD4;
		break;
	case OVK_NUMPAD5:
		game.Scripts.pScriptParams[0].nVar = VK_NUMPAD5;
		break;
	case OVK_NUMLOCK:
		game.Scripts.pScriptParams[0].nVar = VK_NUMLOCK;
		break;
	case OVK_NUMPAD6:
		game.Scripts.pScriptParams[0].nVar = VK_NUMPAD6;
		break;
	case OVK_NUMPAD7:
		game.Scripts.pScriptParams[0].nVar = VK_NUMPAD7;
		break;
	case OVK_NUMPAD8:
		game.Scripts.pScriptParams[0].nVar = VK_NUMPAD8;
		break;
	case OVK_NUMPAD9:
		game.Scripts.pScriptParams[0].nVar = VK_NUMPAD9;
		break;
	case OVK_NUMPAD0:
		game.Scripts.pScriptParams[0].nVar = VK_NUMPAD0;
		break;
	case OVK_SEPARATOR:
		game.Scripts.pScriptParams[0].nVar = VK_SEPARATOR;
		break;
	case OVK_SCROLL:
		game.Scripts.pScriptParams[0].nVar = VK_SCROLL;
		break;
	case OVK_PAUSE:
		game.Scripts.pScriptParams[0].nVar = VK_PAUSE;
		break;
	case OVK_BACK:
		game.Scripts.pScriptParams[0].nVar = VK_BACK;
		break;
	case OVK_TAB:
		game.Scripts.pScriptParams[0].nVar = VK_TAB;
		break;
	case OVK_CAPITAL:
		game.Scripts.pScriptParams[0].nVar = VK_CAPITAL;
		break;
	case OVK_RETURN:
		game.Scripts.pScriptParams[0].nVar = VK_RETURN;
		break;
	case OVK_LSHIFT:
		game.Scripts.pScriptParams[0].nVar = VK_LSHIFT;
		break;
	case OVK_RSHIFT:
		game.Scripts.pScriptParams[0].nVar = VK_RSHIFT;
		break;
	case OVK_ESC:
		game.Scripts.pScriptParams[0].nVar = VK_ESCAPE;
		break;
	case OVK_LCONTROL:
		game.Scripts.pScriptParams[0].nVar = VK_LCONTROL;
		break;
	case OVK_RCONTROL:
		game.Scripts.pScriptParams[0].nVar = VK_RCONTROL;
		break;
	case OVK_LMENU:
		game.Scripts.pScriptParams[0].nVar = VK_LMENU;
		break;
	case OVK_RMENU:
		game.Scripts.pScriptParams[0].nVar = VK_RMENU;
		break;
	case OVK_LWIN:
		game.Scripts.pScriptParams[0].nVar = VK_LWIN;
		break;
	case OVK_RWIN:
		game.Scripts.pScriptParams[0].nVar = VK_RWIN;
		break;
	case OVK_APPS:
		game.Scripts.pScriptParams[0].nVar = VK_APPS;
		break;
	default:
		break;
	}
#endif
	script->UpdateCompareFlag(GetKeyState(game.Scripts.pScriptParams[0].nVar) & 0x8000);
	return OR_CONTINUE;
}

eOpcodeResult CustomOpcodes::FIND_RANDOM_CHAR(Script *script)
{
	script->Collect(6);
	uint objcount;
	if(game.Scripts.pScriptParams[4].nVar)
	{
		if(!script->m_nLastPedSearchIndex)
		{
			game.Scripts.pScriptParams[0].nVar = 0;
			script->Store(1);
			script->UpdateCompareFlag(false);
			return OR_CONTINUE;
		}
		objcount = script->m_nLastPedSearchIndex;
	}
	else
	{
		if(!(*game.Pools.ppPedPool)->m_size)
		{
			game.Scripts.pScriptParams[0].nVar = 0;
			script->Store(1);
			script->UpdateCompareFlag(false);
			return OR_CONTINUE;
		}
		objcount = (*game.Pools.ppPedPool)->m_size;
	}

#if CLEO_VC
	static constexpr auto off6D8 = 0x6D8;
	static constexpr auto off3D4 = 0x3D4;
	static constexpr auto off244 = 0x244;
#else
	static constexpr auto off6D8 = 0x5F0;
	static constexpr auto off3D4 = 0x32C;
	static constexpr auto off244 = 0x224;
#endif

	uint poolobj = (uint)(*game.Pools.ppPedPool)->m_entries + objcount * off6D8;
	bool found = 0;
	float maxsq = game.Scripts.pScriptParams[3].fVar * game.Scripts.pScriptParams[3].fVar;
	for(int i = objcount - 1; i >= 0; i--)
	{
		poolobj -= off6D8;
		if(!((*game.Pools.ppPedPool)->m_flags[i] & 0x80))
		{
			if (*(unsigned int *)(poolobj + off3D4))
			{
				if(!game.Scripts.pScriptParams[5].nVar || (*(unsigned int *)(poolobj + off244) != 48 && *(unsigned int *)(poolobj + off244) != 49))
				{
					float xd = *(float *)(poolobj + 0x34) - game.Scripts.pScriptParams[0].fVar;
					float yd = *(float *)(poolobj + 0x38) - game.Scripts.pScriptParams[1].fVar;
					float zd = *(float *)(poolobj + 0x3C) - game.Scripts.pScriptParams[2].fVar;
					float distsq = xd * xd + yd * yd + zd * zd;
					if(distsq <= maxsq)
					{
						found = true;
						script->m_nLastPedSearchIndex = i;
						game.Scripts.pScriptParams[0].nVar = game.Pools.pfPedPoolGetIndex(*game.Pools.ppPedPool, (void *)poolobj);
						script->UpdateCompareFlag(true);
						break;
					}
				}
			}
		}
	}
	if(!found)
	{
		game.Scripts.pScriptParams[0].nVar = -1;
		script->UpdateCompareFlag(false);
		script->m_nLastPedSearchIndex = 0;
	}
	script->Store(1);
	return OR_CONTINUE;
}

eOpcodeResult CustomOpcodes::FIND_RANDOM_CAR(Script *script)
{
	script->Collect(6);
	uint objcount;
	if(game.Scripts.pScriptParams[4].nVar)
	{
		if(!script->m_nLastVehicleSearchIndex)
		{
			game.Scripts.pScriptParams[0].nVar = 0;
			script->Store(1);
			script->UpdateCompareFlag(false);
			return OR_CONTINUE;
		}
		objcount = script->m_nLastVehicleSearchIndex;
	}
	else
	{
		if(!(*game.Pools.ppVehiclePool)->m_size)
		{
			game.Scripts.pScriptParams[0].nVar = 0;
			script->Store(1);
			script->UpdateCompareFlag(false);
			return OR_CONTINUE;
		}
		objcount = (*game.Pools.ppVehiclePool)->m_size;
	}

#if CLEO_VC
	static constexpr auto off5DC = 0x5DC;
	static constexpr auto off29C = 0x29C;
	static constexpr auto off11A = 0x11A;
#else
	static constexpr auto off5DC = 0x5A8;
	static constexpr auto off29C = 0x284;
	static constexpr auto off11A = 0x122;
#endif

	uint poolobj = (uint)(*game.Pools.ppVehiclePool)->m_entries + objcount * off5DC;
	bool found = 0;
	float maxsq = game.Scripts.pScriptParams[3].fVar * game.Scripts.pScriptParams[3].fVar;
	for(int i = objcount - 1; i >= 0; i--)
	{
		poolobj -= off5DC;
		if(!((*game.Pools.ppVehiclePool)->m_flags[i] & 0x80))
		{
			if (!game.Scripts.pScriptParams[5].nVar || ((*(unsigned char *)(poolobj + 0x50) & 0xF8) != 40 && *(unsigned int *)(poolobj + off29C) != 1 && !(*(unsigned char *)(poolobj + off11A) & 8)))
			{
				float xd = *(float *)(poolobj + 0x34) - game.Scripts.pScriptParams[0].fVar;
				float yd = *(float *)(poolobj + 0x38) - game.Scripts.pScriptParams[1].fVar;
				float zd = *(float *)(poolobj + 0x3C) - game.Scripts.pScriptParams[2].fVar;
				float distsq = xd * xd + yd * yd + zd * zd;
				if(distsq <= maxsq)
				{
					found = true;
					script->m_nLastVehicleSearchIndex = i;
					game.Scripts.pScriptParams[0].nVar = game.Pools.pfVehiclePoolGetIndex(*game.Pools.ppVehiclePool, (void *)poolobj);
					script->UpdateCompareFlag(true);
					break;
				}
			}
		}
	}
	if(!found)
	{
		game.Scripts.pScriptParams[0].nVar = -1;
		script->UpdateCompareFlag(false);
		script->m_nLastVehicleSearchIndex = 0;
	}
	script->Store(1);
	return OR_CONTINUE;
}

eOpcodeResult CustomOpcodes::FIND_RANDOM_OBJECT(Script *script)
{
	script->Collect(5);
	uint objcount;
	if(game.Scripts.pScriptParams[4].nVar)
	{
		if(!script->m_nLastObjectSearchIndex)
		{
			game.Scripts.pScriptParams[0].nVar = 0;
			script->Store(1);
			script->UpdateCompareFlag(false);
			return OR_CONTINUE;
		}
		objcount = script->m_nLastObjectSearchIndex;
	}
	else
	{
		if(!(*game.Pools.ppObjectPool)->m_size)
		{
			game.Scripts.pScriptParams[0].nVar = 0;
			script->Store(1);
			script->UpdateCompareFlag(false);
			return OR_CONTINUE;
		}
		objcount = (*game.Pools.ppObjectPool)->m_size;
	}

#if CLEO_VC
	static constexpr auto off1A0 = 0x1A0;
#else
	static constexpr auto off1A0 = 0x19C;
#endif

	uint poolobj = (uint)(*game.Pools.ppObjectPool)->m_entries + objcount * off1A0;
	bool found = 0;
	float maxsq = game.Scripts.pScriptParams[3].fVar * game.Scripts.pScriptParams[3].fVar;
	for(int i = objcount - 1; i >= 0; i--)
	{
		poolobj -= off1A0;
		if(!((*game.Pools.ppObjectPool)->m_flags[i] & 0x80))
		{
			float xd = *(float *)(poolobj + 0x34) - game.Scripts.pScriptParams[0].fVar;
			float yd = *(float *)(poolobj + 0x38) - game.Scripts.pScriptParams[1].fVar;
			float zd = *(float *)(poolobj + 0x3C) - game.Scripts.pScriptParams[2].fVar;
			float distsq = xd * xd + yd * yd + zd * zd;
			if(distsq <= maxsq)
			{
				found = true;
				script->m_nLastObjectSearchIndex = i;
				game.Scripts.pScriptParams[0].nVar = game.Pools.pfObjectPoolGetIndex(*game.Pools.ppObjectPool, (void *)poolobj);
				script->UpdateCompareFlag(true);
				break;
			}
		}
	}
	if(!found)
	{
		game.Scripts.pScriptParams[0].nVar = -1;
		script->UpdateCompareFlag(false);
		script->m_nLastObjectSearchIndex = 0;
	}
	script->Store(1);
	return OR_CONTINUE;
}

eOpcodeResult CustomOpcodes::CALL_POP_FLOAT(Script *script)
{
	float *pParam = &game.Scripts.pScriptParams[0].fVar;
	__asm mov eax, pParam
	__asm fstp [eax]
	script->Store(1);
	return OR_CONTINUE;
}

eOpcodeResult CustomOpcodes::MATH_EXP(Script *script)
{
	script->Collect(2);
	game.Scripts.pScriptParams[0].fVar = powf(game.Scripts.pScriptParams[0].fVar, game.Scripts.pScriptParams[1].fVar);
	script->Store(1);
	return OR_CONTINUE;
}

eOpcodeResult CustomOpcodes::MATH_LOG(Script *script)
{
	script->Collect(2);
	game.Scripts.pScriptParams[0].fVar =  logf(game.Scripts.pScriptParams[0].fVar) / logf(game.Scripts.pScriptParams[1].fVar);
	script->Store(1);
	return OR_CONTINUE;
}

eOpcodeResult
CustomOpcodes::CALL_SCM_FUNCTION(Script* script)
{
		script->Collect(2);
		int addr = game.Scripts.pScriptParams[0].nVar;
		int paramCount = game.Scripts.pScriptParams[1].nVar;

		script->Collect(paramCount);

		/*
			We didn't actually read all params this opcode provides just yet: after we read values that caller 
			passes to callee with script->Collect(), there are indexes of m_aLVars where callee's retvals should be stored. 
			We will continue reading them after returning from callee.
		*/
		script->PushStackFrame();
		script->JumpTo(addr);

		std::memset(&script->m_aLVars, 0, sizeof(script->m_aLVars));
		std::memcpy(&script->m_aLVars, &game.Scripts.pScriptParams, paramCount * sizeof(ScriptParam));

		return OR_CONTINUE;
}

eOpcodeResult
CustomOpcodes::SCM_FUNCTION_RET(Script* script)
{
		script->Collect(1);
		int paramCount = game.Scripts.pScriptParams[0].nVar;

		// collect callee's retvals
		script->Collect(paramCount);

		// return to caller
		script->PopStackFrame();

		// continue reading indexes of m_aLVars to store callee's retvals
		script->Store(paramCount);

		// variadic opcodes like 0AB1: CLEO_CALL end with PARAM_TYPE_END_OF_PARAMS; we skip it manually here.
		script->m_nIp += 1;

		return OR_CONTINUE;
}

eOpcodeResult
CustomOpcodes::GET_LABEL_OFFSET(Script* script)
{
		script->Collect(1);
		int address = game.Scripts.pScriptParams[0].nVar;

		// negated address is a hack that lets us tell custom and mission scripts from regular ones
		void* result;
		if (address >= 0)
				result = &game.Scripts.pScriptSpace[address];
		else {
				if (script->m_bIsCustom)
						result = &script->m_pCodeData[-address];
				else
						result = &game.Scripts.pScriptSpace[game.kMainSize + (-address)];
		}

		game.Scripts.pScriptParams[0].pVar = result;
		script->Store(1);

		return OR_CONTINUE;
}

eOpcodeResult
CustomOpcodes::GET_VAR_OFFSET(Script* script)
{
		// we don't collect params here, because we simply call native function 
		game.Scripts.pScriptParams[0].pVar = script->GetPointerToScriptVariable();
		script->Store(1);

		return OR_CONTINUE;
}

eOpcodeResult CustomOpcodes::BIT_AND(Script *script)
{
	script->Collect(2);
	game.Scripts.pScriptParams[0].nVar = game.Scripts.pScriptParams[0].nVar & game.Scripts.pScriptParams[1].nVar;
	script->Store(1);
	return OR_CONTINUE;
}

eOpcodeResult CustomOpcodes::BIT_OR(Script *script)
{
	script->Collect(2);
	game.Scripts.pScriptParams[0].nVar = game.Scripts.pScriptParams[0].nVar | game.Scripts.pScriptParams[1].nVar;
	script->Store(1);
	return OR_CONTINUE;
}

eOpcodeResult CustomOpcodes::BIT_XOR(Script *script)
{
	script->Collect(2);
	game.Scripts.pScriptParams[0].nVar = game.Scripts.pScriptParams[0].nVar ^ game.Scripts.pScriptParams[1].nVar;
	script->Store(1);
	return OR_CONTINUE;
}

eOpcodeResult CustomOpcodes::BIT_NOT(Script *script)
{
	script->Collect(1);
	game.Scripts.pScriptParams[0].nVar = ~game.Scripts.pScriptParams[0].nVar;
	script->Store(1);
	return OR_CONTINUE;
}

eOpcodeResult CustomOpcodes::BIT_MOD(Script *script)
{
	script->Collect(2);
	game.Scripts.pScriptParams[0].nVar = game.Scripts.pScriptParams[0].nVar % game.Scripts.pScriptParams[1].nVar;
	script->Store(1);
	return OR_CONTINUE;
}

eOpcodeResult CustomOpcodes::BIT_SHR(Script *script)
{
	script->Collect(2);
	game.Scripts.pScriptParams[0].nVar = game.Scripts.pScriptParams[0].nVar >> game.Scripts.pScriptParams[1].nVar;
	script->Store(1);
	return OR_CONTINUE;
}

eOpcodeResult CustomOpcodes::BIT_SHL(Script *script)
{
	script->Collect(2);
	game.Scripts.pScriptParams[0].nVar = game.Scripts.pScriptParams[0].nVar << game.Scripts.pScriptParams[1].nVar;
	script->Store(1);
	return OR_CONTINUE;
}

//0400=7,store_coords_to %5d% %6d% %7d% from_object %1d% with_offset %2d% %3d% %4d%
eOpcodeResult CustomOpcodes::STORE_COORDS_FROM_OBJECT_WITH_OFFSET(Script *script)
{
	script->Collect(4);
	void* object = game.Pools.pfObjectPoolGetAt(*game.Pools.ppObjectPool, game.Scripts.pScriptParams[0].nVar);

	CVector offset;
	offset.x = game.Scripts.pScriptParams[1].fVar;
	offset.y = game.Scripts.pScriptParams[2].fVar;
	offset.z = game.Scripts.pScriptParams[3].fVar;

	game.Misc.pfRwV3dTransformPoints(&offset, &offset, 1, (uintptr_t*)((uintptr_t)object + 4));

	game.Scripts.pScriptParams[0].fVar = offset.x;
	game.Scripts.pScriptParams[1].fVar = offset.y;
	game.Scripts.pScriptParams[2].fVar = offset.z;

	script->Store(3);
	return OR_CONTINUE;
}

//0407=7,store_coords_to %5d% %6d% %7d% from_car %1d% with_offset %2d% %3d% %4d%
eOpcodeResult CustomOpcodes::STORE_COORDS_FROM_CAR_WITH_OFFSET(Script *script)
{
	script->Collect(4);
	void* car = game.Pools.pfVehiclePoolGetAt(*game.Pools.ppVehiclePool, game.Scripts.pScriptParams[0].nVar);

	CVector offset;
	offset.x = game.Scripts.pScriptParams[1].fVar;
	offset.y = game.Scripts.pScriptParams[2].fVar;
	offset.z = game.Scripts.pScriptParams[3].fVar;

	game.Misc.pfRwV3dTransformPoints(&offset, &offset, 1, (uintptr_t*)((uintptr_t)car + 4));

	game.Scripts.pScriptParams[0].fVar = offset.x;
	game.Scripts.pScriptParams[1].fVar = offset.y;
	game.Scripts.pScriptParams[2].fVar = offset.z;

	script->Store(3);
	return OR_CONTINUE;
}

//04C4=7,store_coords_to %5d% %6d% %7d% from_actor %1d% with_offset %2d% %3d% %4d%
eOpcodeResult CustomOpcodes::STORE_COORDS_FROM_ACTOR_WITH_OFFSET(Script *script)
{
	script->Collect(4);
	void* actor = game.Pools.pfPedPoolGetAt(*game.Pools.ppPedPool, game.Scripts.pScriptParams[0].nVar);

	CVector offset;
	offset.x = game.Scripts.pScriptParams[1].fVar;
	offset.y = game.Scripts.pScriptParams[2].fVar;
	offset.z = game.Scripts.pScriptParams[3].fVar;

	game.Misc.pfRwV3dTransformPoints(&offset, &offset, 1, (uintptr_t*)((uintptr_t)actor + 4));

	game.Scripts.pScriptParams[0].fVar = offset.x;
	game.Scripts.pScriptParams[1].fVar = offset.y;
	game.Scripts.pScriptParams[2].fVar = offset.z;

	script->Store(3);
	return OR_CONTINUE;
}

eOpcodeResult CustomOpcodes::STORE_PLAYER_CURRENTLY_ARMED_WEAPON(Script *script)
{
	script->Collect(1);
	game.Scripts.pScriptParams[0].nVar = *(DWORD *)(game.Pools.pPlayers[79 * game.Scripts.pScriptParams[0].nVar] + 24 * *(BYTE *)(game.Pools.pPlayers[79 * game.Scripts.pScriptParams[0].nVar] + 1176) + 860);
	script->Store(1);
	return OR_CONTINUE;
}

eOpcodeResult CustomOpcodes::GET_CHAR_ARMOUR(Script *script)
{
	script->Collect(1);
	void* actor = game.Pools.pfPedPoolGetAt(*game.Pools.ppPedPool, game.Scripts.pScriptParams[0].nVar);
	game.Scripts.pScriptParams[0].nVar = static_cast<int>(*(float*)((uintptr_t)actor + 0x2C4));
	script->Store(1);
	return OR_CONTINUE;
}

eOpcodeResult
CustomOpcodes::PLAYER_DRIVING_PLANE(Script* script)
{
		bool gta3 = game.IsGta3();
		short miDodo = gta3 ? 126 : 190; // skimmer for VC
		uint CPlayerInfoSize = gta3 ? 0x13C : 0x170;
		uint offset_bInVehicle = gta3 ? 0x314 : 0x3AC; // CPed::bInVehicle
		uint offset_pMyVehicle = gta3 ? 0x310 : 0x3A8; // CPed::pMyVehicle
		uint offset_pHandling = gta3 ? 0x128 : 0x120; // CVehicle::pHandling
		uint offset_Flags = gta3 ? 0xC8 : 0xCC; // tHandlingData::Flags

		script->Collect(1);

		// get CPlayerInfo::m_pPed as uchar*
		uint offset = CPlayerInfoSize * game.Scripts.pScriptParams[0].nVar); // game technically supports only 1 player...
		uchar* player = reinterpret_cast<uchar*>(*(uint*)(game.Pools.pPlayers + offset); // we use 2 casts here! read m_pPed as uint and cast it to uchar*

		bool result = false;
		if (*(bool*)(player + offset_bInVehicle)) {
				uchar* vehicle = reinterpret_cast<uchar*>(*(uint*)(player + offset_pMyVehicle));
				short mi = *(short*)(vehicle + 0x5C); // CEntity::m_modelIndex; same offset for both games

				uchar* handling = reinterpret_cast<uchar*>(*(uint*)(vehicle + offset_pHandling));
				uint flags = *(uint*)(handling + offset_Flags);

				result = (mi == miDodo || flags & 0x40000) ? true : false;
		}

		script->UpdateCompareFlag(result);

		return OR_CONTINUE;
}

eOpcodeResult
CustomOpcodes::PLAYER_DRIVING_BOAT(Script* script)
{
		bool gta3 = game.IsGta3();
		uint CPlayerInfoSize = gta3 ? 0x13C : 0x170;
		uint offset_bInVehicle = gta3 ? 0x314 : 0x3AC; // CPed::bInVehicle
		uint offset_pMyVehicle = gta3 ? 0x310 : 0x3A8; // CPed::pMyVehicle
		uint offset_vehType = gta3 ? 0x284 : 0x29C; // CVehicle::m_vehType

		script->Collect(1);

		// get CPlayerInfo::m_pPed as uchar*
		uint offset = CPlayerInfoSize * game.Scripts.pScriptParams[0].nVar); // game technically supports only 1 player...
		uchar* player = reinterpret_cast<uchar*>(*(uint*)(game.Pools.pPlayers + offset); // we use 2 casts here! read m_pPed as uint and cast it to uchar*

		bool result = false;
		if (*(bool*)(player + offset_bInVehicle)) {
				uchar* vehicle = reinterpret_cast<uchar*>(*(uint*)(player + offset_pMyVehicle));
				result = *(int*)(vehicle + offset_vehType) == 1 ? true : false;
		}

		script->UpdateCompareFlag(result);

		return OR_CONTINUE;
}

eOpcodeResult
CustomOpcodes::PLAYER_DRIVING_HELI(Script* script)
{
		bool gta3 = game.IsGta3();
		uint CPlayerInfoSize = gta3 ? 0x13C : 0x170;
		uint offset_bInVehicle = gta3 ? 0x314 : 0x3AC; // CPed::bInVehicle
		uint offset_pMyVehicle = gta3 ? 0x310 : 0x3A8; // CPed::pMyVehicle
		uint offset_pHandling = gta3 ? 0x128 : 0x120; // CVehicle::pHandling
		uint offset_Flags = gta3 ? 0xC8 : 0xCC; // tHandlingData::Flags

		script->Collect(1);

		// get CPlayerInfo::m_pPed as uchar*
		uint offset = CPlayerInfoSize * game.Scripts.pScriptParams[0].nVar); // game technically supports only 1 player...
		uchar* player = reinterpret_cast<uchar*>(*(uint*)(game.Pools.pPlayers + offset); // we use 2 casts here! read m_pPed as uint and cast it to uchar*

		bool result = false;
		if (*(bool*)(player + offset_bInVehicle)) {
				uchar* vehicle = reinterpret_cast<uchar*>(*(uint*)(player + offset_pMyVehicle));

				uchar* handling = reinterpret_cast<uchar*>(*(uint*)(vehicle + offset_pHandling));
				uint flags = *(uint*)(handling + offset_Flags);

				result = (flags & 0x20000) ? true : false;
		}

		script->UpdateCompareFlag(result);

		return OR_CONTINUE;
}

eOpcodeResult
CustomOpcodes::PLAYER_DRIVING_A_MOTORBIKE(Script* script)
{
		bool gta3 = game.IsGta3();
		uint CPlayerInfoSize = gta3 ? 0x13C : 0x170;
		uint offset_bInVehicle = gta3 ? 0x314 : 0x3AC; // CPed::bInVehicle
		uint offset_pMyVehicle = gta3 ? 0x310 : 0x3A8; // CPed::pMyVehicle
		uint offset_vehType = gta3 ? 0x284 : 0x29C; // CVehicle::m_vehType

		script->Collect(1);

		// get CPlayerInfo::m_pPed as uchar*
		uint offset = CPlayerInfoSize * game.Scripts.pScriptParams[0].nVar); // game technically supports only 1 player...
		uchar* player = reinterpret_cast<uchar*>(*(uint*)(game.Pools.pPlayers + offset); // we use 2 casts here! read m_pPed as uint and cast it to uchar*

		bool result = false;
		if (*(bool*)(player + offset_bInVehicle)) {
				uchar* vehicle = reinterpret_cast<uchar*>(*(uint*)(player + offset_pMyVehicle));
				result = *(int*)(vehicle + offset_vehType) == 5 ? true : false;
		}

		script->UpdateCompareFlag(result);

		return OR_CONTINUE;
}

eOpcodeResult
CustomOpcodes::IS_PC_VERSION(Script* script)
{
		script->UpdateCompareFlag(true);

		return OR_CONTINUE;
}

eOpcodeResult
CustomOpcodes::IS_AUSTRALIAN_GAME(Script* script)
{
		script->UpdateCompareFlag(false);

		return OR_CONTINUE;
}

eOpcodeResult
CustomOpcodes::START_CUSTOM_THREAD_VSTRING(Script* script)
{
		script->Collect(1);

		fs::path filepath = fs::path(game.Misc.szRootDirName) / "CLEO" / game.Scripts.pScriptParams[0].szVar;

		Script* new_script = new Script(filepath.c_str());

		try {
				LOGL(LOG_PRIORITY_OPCODE, "START_CUSTOM_THREAD_VSTRING: Starting new script \"%s\"", filepath.c_str());
				Script* new_script = scriptMgr::StartScript(filepath.c_str());

				for (int i = 0; script->GetNextParamType(); ++i) {
						script->Collect(1);
						new_script->m_aLVars[i].nVar = game.Scripts.pScriptParams[0].nVar;
				}
				script->m_nIp++;

				script->UpdateCompareFlag(true);

				return OR_CONTINUE;
		} catch (const char* e) {
				LOGL(LOG_PRIORITY_OPCODE, "START_CUSTOM_THREAD_VSTRING: Script loading failed, \"%s\"", e);
		} catch (...) {
				LOGL(LOG_PRIORITY_OPCODE, "START_CUSTOM_THREAD_VSTRING: Script loading failed, \"%s\"", "Unknown exception");
		}

		// just skip all params if we fail
		while (script->GetNextParamType()) {
				script->Collect(1);
		}
		script->m_nIp++;

		script->UpdateCompareFlag(false);

		return OR_CONTINUE;
}

//0601=2, is_button_pressed_on_pad %1d% with_sensitivity %2d%
eOpcodeResult
CustomOpcodes::IS_BUTTON_PRESSED_ON_PAD(Script* script)
{
		script->Collect(2);

		script->UpdateCompareFlag(*(game.Misc.pPadNewState + game.Scripts.pScriptParams[0].nVar) == (short)game.Scripts.pScriptParams[1].nVar);

		return OR_CONTINUE;
}

//0602=2, emulate_button_press_on_pad %1d% with_sensitivity %2d%
eOpcodeResult
CustomOpcodes::EMULATE_BUTTON_PRESS_ON_PAD(Script* script)
{
		script->Collect(2);

		*(game.Misc.pPadNewState + game.Scripts.pScriptParams[0].nVar) = (short)game.Scripts.pScriptParams[1].nVar;

		return OR_CONTINUE;
}

//0603=0, is_camera_in_widescreen_mode
eOpcodeResult
CustomOpcodes::IS_CAMERA_IN_WIDESCREEN_MODE(Script* script)
{
		script->UpdateCompareFlag(*game.Misc.pWideScreenOn);

		return OR_CONTINUE;
}

//0604=2, %2d% = weapon %1d% model
eOpcodeResult
CustomOpcodes::GET_MODEL_ID_FROM_WEAPON_ID(Script* script)
{
		script->Collect(1);

		int weaponMI = game.Misc.pfModelForWeapon(game.Scripts.pScriptParams[0].nVar);
		game.Scripts.pScriptParams[0].nVar = weaponMI ? weaponMI : -1;
		script->Store(1);

		return OR_CONTINUE;
}

//0605=2, %2d% = model %1d% weapon id
eOpcodeResult
CustomOpcodes::GET_WEAPON_ID_FROM_MODEL_ID(Script* script)
{
		script->Collect(1);
		int weaponMI = game.Scripts.pScriptParams[0].nVar;

		if (weaponMI < 0) 
				weaponMI = game.Scripts.pUsedObjectArray[-weaponMI].index;

		// CPickups::WeaponForModel() exits only in III, so we do this manually for VC compatability
		int result = -1;
		for (size_t i = 0; i < 37; ++i) {
				if (weaponMI == game.Misc.pfModelForWeapon(i))
						break;
		}

		game.Scripts.pScriptParams[0].nVar = result;
		script->Store(1);

		return OR_CONTINUE;
}

//0606=3, set_memory_offset memory_pointer %1d% memory_to_point %2d% virtual_protect %3d%
eOpcodeResult
CustomOpcodes::SET_MEM_OFFSET(Script* script)
{
		script->Collect(3);

		memory::Write(game.Scripts.pScriptParams[0].pVar, game.Scripts.pScriptParams[1].nVar - (game.Scripts.pScriptParams[0].nVar + 4));

		return OR_CONTINUE;
}

//0607=1, %1d% = get_current_weather
eOpcodeResult
CustomOpcodes::GET_CURRENT_WEATHER(Script* script)
{
		game.Scripts.pScriptParams[0].nVar = *game.Misc.pOldWeatherType;
		script->Store(1);

		return OR_CONTINUE;
}

//0608=3, show_text_position %1d% %2d% text %3d%
eOpcodeResult CustomOpcodes::SHOW_TEXT_POSITION(Script *script)
{
	script->Collect(3);
	game.Text.pIntroTextLines[*game.Text.pNumberOfIntroTextLinesThisFrame].x = game.Scripts.pScriptParams[0].fVar;
	game.Text.pIntroTextLines[*game.Text.pNumberOfIntroTextLinesThisFrame].y = game.Scripts.pScriptParams[1].fVar;
	const char *text = game.Scripts.pScriptParams[2].szVar;
	swprintf((wchar_t*)&game.Text.pIntroTextLines[*game.Text.pNumberOfIntroTextLinesThisFrame].text, 100, L"%hs", text);
	*game.Text.pNumberOfIntroTextLinesThisFrame = *game.Text.pNumberOfIntroTextLinesThisFrame + 1;
	return OR_CONTINUE;
};

//0609=-1, show_formatted_text_position %1d% %2d% text %3d%
eOpcodeResult CustomOpcodes::SHOW_FORMATTED_TEXT_POSITION(Script *script)
{
	script->Collect(3);
	game.Text.pIntroTextLines[*game.Text.pNumberOfIntroTextLinesThisFrame].x = game.Scripts.pScriptParams[0].fVar;
	game.Text.pIntroTextLines[*game.Text.pNumberOfIntroTextLinesThisFrame].y = game.Scripts.pScriptParams[1].fVar;
	char fmt[100]; char text[100]; static wchar_t message_buf[0x80];
	strcpy(fmt, game.Scripts.pScriptParams[2].szVar);
	format(script, text, sizeof(text), fmt);
	swprintf((wchar_t*)&game.Text.pIntroTextLines[*game.Text.pNumberOfIntroTextLinesThisFrame].text, 100, L"%hs", text);
	while ((*(ScriptParamType *)(&game.Scripts.pScriptSpace[script->m_nIp])).type)
		script->Collect(1);
	script->m_nIp++;
	*game.Text.pNumberOfIntroTextLinesThisFrame = *game.Text.pNumberOfIntroTextLinesThisFrame + 1;
	return OR_CONTINUE;
};

//0673=4,play_animation on actor %1d% animgroup %2d% anim %3d% blendfactor %4f%
eOpcodeResult WINAPI CustomOpcodes::PLAY_ANIMATION(Script *script)
{
	script->Collect(4);
	void* actor = game.Pools.pfPedPoolGetAt(*game.Pools.ppPedPool, game.Scripts.pScriptParams[0].nVar);
	game.Misc.pfBlendAnimation(*(DWORD *)((uintptr_t)actor + 0x4C), game.Scripts.pScriptParams[1].nVar, game.Scripts.pScriptParams[2].nVar, game.Scripts.pScriptParams[3].fVar);
	return OR_CONTINUE;
}

eOpcodeResult CustomOpcodes::DRAW_SHADOW(Script *script)
{
	script->Collect(10);
	int type = 2;	
	CVector pos;
	pos.x = game.Scripts.pScriptParams[1].fVar;
	pos.y = game.Scripts.pScriptParams[2].fVar;
	pos.z = game.Scripts.pScriptParams[3].fVar;
	float angle = game.Scripts.pScriptParams[4].fVar;
	float length = game.Scripts.pScriptParams[5].fVar;

	void* pShadowTex;
	switch (game.Scripts.pScriptParams[0].nVar)
	{
		case 1:
			pShadowTex = *game.Shadows.ppShadowCarTex;
			type = 1;
			break;
		case 2:
			pShadowTex = *game.Shadows.ppShadowPedTex;
			break;
		case 3:
			pShadowTex = *game.Shadows.ppShadowExplosionTex;
			break;
		case 4:
			pShadowTex = *game.Shadows.ppShadowHeliTex;
			type = 1;
			break;
		case 5:
			pShadowTex = *game.Shadows.ppShadowHeadLightsTex;
			break;
		case 6:
			pShadowTex = *game.Shadows.ppBloodPoolTex;
			break;
		case 7:
			pShadowTex = *game.Shadows.ppShadowBikeTex;
			break;
		case 8:
			pShadowTex = *game.Shadows.ppShadowBaronTex;
			break;
		default:
			return OR_CONTINUE;
	}

	float x, y;
	if (angle != 0.0f) {
			y = std::cos(angle) * length;
			x = std::sin(angle) * length;
	} else {
			y = length;
			x = 0.0f;
	}
	float frontX = -x;
	float frontY = y;
	float sideX = y;
	float sideY = x;

	game.Shadows.pfStoreShadowToBeRendered(type, pShadowTex, &pos, frontX, frontY, sideX, sideY, game.Scripts.pScriptParams[6].nVar, game.Scripts.pScriptParams[7].nVar, game.Scripts.pScriptParams[8].nVar, game.Scripts.pScriptParams[9].nVar, 150.0f, true, 1.0f, nullptr, false);
	return OR_CONTINUE;
}

eOpcodeResult CustomOpcodes::SET_TEXT_DRAW_FONT(Script *script)
{
	script->Collect(1);
	game.Text.pIntroTextLines[*game.Text.pNumberOfIntroTextLinesThisFrame].fontStyle = game.Scripts.pScriptParams[0].nVar;
	return OR_CONTINUE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
/*****************************************************************************************************/
////////////////////////////////CLEO4 SA opcodes///////////////////////////////////////////////////////
//0A8C=4,write_memory %1d% size %2d% value %3d% virtual_protect %4d% //dup
//0A8D=4,%4d% = read_memory %1d% size %2d% virtual_protect %3d% //dup

//0A8E=3,%3d% = %1d% + %2d% ; int
eOpcodeResult
CustomOpcodes::OPCODE_0A8E(Script* script)
{
		script->Collect(2);

		game.Scripts.pScriptParams[0].nVar = game.Scripts.pScriptParams[0].nVar + game.Scripts.pScriptParams[1].nVar;
		script->Store(1);

		return OR_CONTINUE;
}

//0A8F=3,%3d% = %1d% - %2d% ; int
eOpcodeResult
CustomOpcodes::OPCODE_0A8F(Script* script)
{
		script->Collect(2);

		game.Scripts.pScriptParams[0].nVar = game.Scripts.pScriptParams[0].nVar - game.Scripts.pScriptParams[1].nVar;
		script->Store(1);

		return OR_CONTINUE;
}

//0A90=3,%3d% = %1d% * %2d% ; int
eOpcodeResult
CustomOpcodes::OPCODE_0A90(Script* script)
{
		script->Collect(2);

		game.Scripts.pScriptParams[0].nVar = game.Scripts.pScriptParams[0].nVar * game.Scripts.pScriptParams[1].nVar;
		script->Store(1);

		return OR_CONTINUE;
}

//0A91=3,%3d% = %1d% / %2d% ; int
eOpcodeResult
CustomOpcodes::OPCODE_0A91(Script* script)
{
		script->Collect(2);

		game.Scripts.pScriptParams[0].nVar = game.Scripts.pScriptParams[0].nVar / game.Scripts.pScriptParams[1].nVar;
		script->Store(1);

		return OR_CONTINUE;
}

//0A92=-1,create_custom_thread %1s% //dup
//0A93=0,end_custom_thread //dup
//0A94=-1,start_custom_mission %1s% //not supported
//0A95=0,enable_thread_saving ////not supported
//0A96=2,%2d% = actor %1d% struct //dup
//0A97=2,%2d% = car %1d% struct //dup
//0A98=2,%2d% = object %1d% struct //dup

//0A99=1,chdir %1buserdir/rootdir%
eOpcodeResult
CustomOpcodes::OPCODE_0A99(Script* script)
{
		script->Collect(1);

		fs::current_path(game.Scripts.pScriptParams[0].nVar == 0 ? game.Misc.szRootDirName : 
						 game.Scripts.pScriptParams[0].nVar == 1 ? game.Misc.pfGetUserFilesFolder() : game.Scripts.pScriptParams[0].szVar);

		return OR_CONTINUE;
}

//0A9A=3,%3d% = openfile %1s% mode %2d% ; IF and SET
eOpcodeResult
CustomOpcodes::OPCODE_0A9A(Script* script)
{
		// cppreference.com/w/cpp/io/basic_filebuf/open
		auto openmode_string_to_bitmask = [](const char* openmode) -> std::ios::openmode {
				std::ios::openmode bitmask(0);
				for (int i = 0; openmode[i]; ++i) {
						if (openmode[i] == 'r') {
								bitmask |= std::ios::in;
						} else if (openmode[i] == 'w') {
								bitmask |= std::ios::out | std::ios::trunc;
						} else if (openmode[i] == 'a') {
								bitmask |= std::ios::out | std::ios::app;
						} else if (openmode[i] == '+') {
								bitmask |= std::ios::in | std::ios::out;
						} else if (openmode[i] == 'b') {
								bitmask |= std::ios::binary;
						} else if (openmode[i] == 'x') {
								bitmask |= std::ios::noreplace;
						}
				}
				return bitmask;
		};

		script->Collect(2);

		try {
				script->StoreCache(&m_pOpenedFiles, new std::fstream(game.Scripts.pScriptParams[0].szVar, openmode_string_to_bitmask(game.Scripts.pScriptParams[1].szVar)));

				script->UpdateCompareFlag(*(std::fstream*)m_pOpenedFiles->data); // check for ill-formed fstream with bool()
				game.Scripts.pScriptParams[0].pVar = m_pOpenedFiles->data;
		} catch (...) {
				script->UpdateCompareFlag(false);
				game.Scripts.pScriptParams[0].pVar = nullptr;
		}
		script->Store(1);

		return OR_CONTINUE;
}

//0A9B=1,closefile %1d%
eOpcodeResult
CustomOpcodes::OPCODE_0A9B(Script* script)
{
		script->Collect(1);

		script->ClearCahce(&m_pOpenedFiles, game.Scripts.pScriptParams[0].pVar);

		return OR_CONTINUE;
}

//0A9C=2,%2d% = file %1d% size
eOpcodeResult
CustomOpcodes::OPCODE_0A9C(Script* script)
{
		script->Collect(1);
		std::fstream* file = (std::fstream*)game.Scripts.pScriptParams[0].pVar;

		auto saved_pos = file->tellg();
		size_t filesize = file->seekg(0, std::ios::beg).ignore(std::numeric_limits<std::streamsize>::max()).gcount();
		file->clear();
		file->seekg(saved_pos);

		game.Scripts.pScriptParams[0].nVar = filesize;
		script->Store(1);

		return OR_CONTINUE;
}

//0A9D=3,readfile %1d% size %2d% to %3d%
eOpcodeResult
CustomOpcodes::OPCODE_0A9D(Script* script)
{
		script->Collect(3);
		std::fstream* file = (std::fstream*)game.Scripts.pScriptParams[0].pVar;

		file->read(game.Scripts.pScriptParams[2].pVar, game.Scripts.pScriptParams[1].nVar);

		return OR_CONTINUE;
}

//0A9E=3,writefile %1d% size %2d% from %3d%
eOpcodeResult
CustomOpcodes::OPCODE_0A9E(Script* script)
{
		script->Collect(3);
		std::fstream* file = (std::fstream*)game.Scripts.pScriptParams[0].pVar;

		file->write(game.Scripts.pScriptParams[2].pVar, game.Scripts.pScriptParams[1].nVar);
		file->flush();

		return OR_CONTINUE;
}

//0A9F=1,%1d% = current_thread_pointer //dup


//0AA0=1,gosub_if_false %1p%
eOpcodeResult CustomOpcodes::OPCODE_0AA0(Script *script)
{
	script->Collect(1);
	script->m_anGosubStack[script->m_nGosubStackPointer++] = script->m_nIp;
	if (!script->m_bCondResult)
		script->JumpTo(game.Scripts.pScriptParams[0].nVar);
	return OR_CONTINUE;
}

//0AA1=0,return_if_false
eOpcodeResult CustomOpcodes::OPCODE_0AA1(Script *script)
{
	if (script->m_bCondResult) return OR_CONTINUE;
	script->m_nIp = script->m_anGosubStack[--script->m_nGosubStackPointer];
	return OR_CONTINUE;
}

//0AA2=2,%2h% = load_library %1s% ; IF and SET
eOpcodeResult CustomOpcodes::OPCODE_0AA2(Script *script)
{
	script->Collect(1);
	auto libHandle = LoadLibrary(game.Scripts.pScriptParams[0].szVar);
	game.Scripts.pScriptParams[0].pVar = libHandle;
	script->Store(1);
	script->UpdateCompareFlag(libHandle);
	return OR_CONTINUE;
}

//0AA3=1,free_library %1h%
eOpcodeResult CustomOpcodes::OPCODE_0AA3(Script *script)
{
	script->Collect(1);
	HMODULE libHandle;
	libHandle = (HMODULE)game.Scripts.pScriptParams[0].pVar;
	FreeLibrary(libHandle);
	return OR_CONTINUE;
}

//0AA4=3,%3d% = get_proc_address %1s% library %2d% ; IF and SET
eOpcodeResult CustomOpcodes::OPCODE_0AA4(Script *script)
{
	script->Collect(2);
	char *funcName = game.Scripts.pScriptParams[0].szVar;
	HMODULE libHandle;
	libHandle = (HMODULE)game.Scripts.pScriptParams[1].pVar;
	void *funcAddr = (void *)GetProcAddress(libHandle, funcName);
	game.Scripts.pScriptParams[0].pVar = funcAddr;
	script->Store(1);
	script->UpdateCompareFlag(funcAddr);
	return OR_CONTINUE;
}

//0AA5=-1,call %1d% num_params %2h% pop %3h% //dup
//0AA6=-1,call_method %1d% struct %2d% num_params %3h% pop %4h% //dup
//0AA7=-1,call_function %1d% num_params %2h% pop %3h% //dup
//0AA8=-1,call_function_method %1d% struct %2d% num_params %3h% pop %4h% //dup


//0AA9=0,  is_game_version_original
eOpcodeResult CustomOpcodes::OPCODE_0AA9(Script *script)
{
	script->UpdateCompareFlag(game.GetGameVersion() == GAME_V1_0);
	return OR_CONTINUE;
}
//0AAA=2,%2d% = thread %1s% pointer //dup


//0AAB=1,   file_exists %1s%
eOpcodeResult CustomOpcodes::OPCODE_0AAB(Script *script)
{
	script->Collect(1);
	DWORD fAttr = GetFileAttributes(game.Scripts.pScriptParams[0].szVar);
	script->UpdateCompareFlag((fAttr != INVALID_FILE_ATTRIBUTES) &&
		!(fAttr & FILE_ATTRIBUTE_DIRECTORY));
	return OR_CONTINUE;
}

//0AAC=2,%2d% = load_audiostream %1d%
//0AAD=2,set_mp3 %1d% perform_action %2d%
//0AAE=1,release_mp3 %1d%
//0AAF=2,%2d% = get_mp3_length %1d%

//0AB0=1,  key_pressed %1d% // dup

//0AB1=-1,call_scm_func %1p% //dup
//0AB2=-1,ret  //dup

//0AB3=2,var %1d% = %2d%
eOpcodeResult CustomOpcodes::OPCODE_0AB3(Script *script)
{
	script->Collect(2);
	SHARED_VAR[game.Scripts.pScriptParams[0].nVar].nVar = game.Scripts.pScriptParams[1].nVar;
	return OR_CONTINUE;
}

//0AB4=2,%2d% = var %1d%
eOpcodeResult CustomOpcodes::OPCODE_0AB4(Script *script)
{
	script->Collect(1);
	game.Scripts.pScriptParams[0].nVar = SHARED_VAR[game.Scripts.pScriptParams[0].nVar].nVar;
	script->Store(1);
	return OR_CONTINUE;
}

//0AB5=3,store_actor %1d% closest_vehicle_to %2d% closest_ped_to %3d%
//0AB6=3,store_target_marker_coords_to %1d% %2d% %3d% // IF and SET //not supported

//0AB7=2,get_vehicle %1d% number_of_gears_to %2d%
eOpcodeResult __stdcall CustomOpcodes::OPCODE_0AB7(Script *script)
{
	script->Collect(1);
	uintptr_t vehicle = reinterpret_cast<uintptr_t>(game.Pools.pfVehiclePoolGetAt(*game.Pools.ppVehiclePool, game.Scripts.pScriptParams[0].nVar));
	if (vehicle)
#if CLEO_VC
		game.Scripts.pScriptParams[0].nVar = *reinterpret_cast<int*>(*reinterpret_cast<uintptr_t*>(vehicle + 0x120) + 0x34 + 0x4A);
#else
		game.Scripts.pScriptParams[0].nVar = *reinterpret_cast<int*>(*reinterpret_cast<uintptr_t*>(vehicle + 0x128) + 0x34 + 0x4A);
#endif
	else
		game.Scripts.pScriptParams[0].nVar = 0;
	script->Store(1);
	return OR_CONTINUE;
}

//0AB8=2,get_vehicle %1d% current_gear_to %2d%
eOpcodeResult __stdcall CustomOpcodes::OPCODE_0AB8(Script *script)
{
	script->Collect(1);
	uintptr_t vehicle = reinterpret_cast<uintptr_t>(game.Pools.pfVehiclePoolGetAt(*game.Pools.ppVehiclePool, game.Scripts.pScriptParams[0].nVar));
	if (vehicle)
#if CLEO_VC
		game.Scripts.pScriptParams[0].nVar = *reinterpret_cast<uint8_t*>(vehicle + 0x208);
#else
		game.Scripts.pScriptParams[0].nVar = *reinterpret_cast<uint8_t*>(vehicle + 0x204);
#endif
	else 
		game.Scripts.pScriptParams[0].nVar = 0;
	script->Store(1);
	return OR_CONTINUE;
}

//0AB9=2,get_mp3 %1d% state_to %2d%
//0ABA=1,end_custom_thread_named %1s% //dup
//0ABB=2,%2d% = audiostream %1d% volume
//0ABC=2,set_audiostream %1d% volume %2d%

//0ABD=1,  vehicle %1d% lights_on ( //0ABD=1,  vehicle %1d% siren_on // dup see 0383 )
eOpcodeResult __stdcall CustomOpcodes::OPCODE_0ABD(Script *script)
{
	script->Collect(1);
	uintptr_t vehicle = reinterpret_cast<uintptr_t>(game.Pools.pfVehiclePoolGetAt(*game.Pools.ppVehiclePool, game.Scripts.pScriptParams[0].nVar));
	if (vehicle)
#if CLEO_VC
		script->UpdateCompareFlag(reinterpret_cast<GtaGame::bVehicleFlags*>(vehicle + 0x1F9)->bLightsOn);
#else
		script->UpdateCompareFlag(reinterpret_cast<GtaGame::bVehicleFlags*>(vehicle + 0x1F5)->bLightsOn);
#endif
	else
		script->UpdateCompareFlag(false);
	return OR_CONTINUE;
}

//0ABE=1,  vehicle %1d% engine_on
eOpcodeResult __stdcall CustomOpcodes::OPCODE_0ABE(Script *script)
{
	script->Collect(1);
	uintptr_t vehicle = reinterpret_cast<uintptr_t>(game.Pools.pfVehiclePoolGetAt(*game.Pools.ppVehiclePool, game.Scripts.pScriptParams[0].nVar));
	if (vehicle) 
#if CLEO_VC
		script->UpdateCompareFlag(reinterpret_cast<GtaGame::bVehicleFlags*>(vehicle + 0x1F9)->bEngineOn);
#else
		script->UpdateCompareFlag(reinterpret_cast<GtaGame::bVehicleFlags*>(vehicle + 0x1F5)->bEngineOn);
#endif
	else 
		script->UpdateCompareFlag(false);
	return OR_CONTINUE;
}

//0ABF=2,set_vehicle %1d% engine_state_to %2d%
eOpcodeResult __stdcall CustomOpcodes::OPCODE_0ABF(Script *script)
{
	script->Collect(2);
	uintptr_t vehicle = reinterpret_cast<uintptr_t>(game.Pools.pfVehiclePoolGetAt(*game.Pools.ppVehiclePool, game.Scripts.pScriptParams[0].nVar));
	if (vehicle)
	{
#if CLEO_VC
		reinterpret_cast<GtaGame::bVehicleFlags*>(vehicle + 0x1F9)->bEngineOn = game.Scripts.pScriptParams[1].nVar != false;
#else
		reinterpret_cast<GtaGame::bVehicleFlags*>(vehicle + 0x1F5)->bEngineOn = game.Scripts.pScriptParams[1].nVar != false;
#endif
	}
	return OR_CONTINUE;
}

//0AC0=2,audiostream %1d% loop %2d%
//0AC1=2,%2d% = load_audiostream_with_3d_support %1d% ; IF and SET
//0AC2=4,set_audiostream %1d% 3d_position %2d% %3d% %4d%
//0AC3=2,link_3d_audiostream %1d% to_object %2d%
//0AC4=2,link_3d_audiostream %1d% to_actor %2d%
//0AC5=2,link_3d_audiostream %1d% to_vehicle %2d%


//0AC6=2,%2d% = label %1p% offset //dup
//0AC7=2,%2d% = var %1d% offset //dup

//0AC8=2,%2d% = allocate_memory_size %1d%
eOpcodeResult
CustomOpcodes::OPCODE_0AC8(Script* script)
{
		script->Collect(1);

		try {
				script->StoreCache(&m_pAllocatedMemory, new(game.Scripts.pScriptParams[0].nVar));

				script->UpdateCompareFlag(true);
				game.Scripts.pScriptParams[0].pVar = m_pAllocatedMemory->data;
		} catch (...) {
				script->UpdateCompareFlag(false);
				game.Scripts.pScriptParams[0].pVar = nullptr;
		}
		script->Store(1);

		return OR_CONTINUE;
};

//0AC9=1,free_allocated_memory %1d%
eOpcodeResult
CustomOpcodes::OPCODE_0AC9(Script* script)
{
		script->Collect(1);

		script->ClearCahce(&m_pAllocatedMemory, game.Scripts.pScriptParams[0].pVar);

		return OR_CONTINUE;
};

//0ACA=1,show_text_box %1s%
eOpcodeResult CustomOpcodes::OPCODE_0ACA(Script *script)
{
	static wchar_t message_buf[HELP_MSG_LENGTH];
	script->Collect(1);
	swprintf(message_buf, HELP_MSG_LENGTH, L"%hs", game.Scripts.pScriptParams[0].szVar);
	game.Text.pfSetHelpMessage(message_buf, false, false);
	return OR_CONTINUE;
};

//0ACB=3,show_styled_text %1s% time %2d% style %3d%
eOpcodeResult CustomOpcodes::OPCODE_0ACB(Script *script)
{
	static wchar_t message_buf[HELP_MSG_LENGTH];
	script->Collect(3);
	const char *text = game.Scripts.pScriptParams[0].szVar;
	swprintf(message_buf, HELP_MSG_LENGTH, L"%hs", text);
	game.Text.pfAddBigMessageQ(message_buf, game.Scripts.pScriptParams[1].nVar, game.Scripts.pScriptParams[2].nVar - 1);
	return OR_CONTINUE;
};

//0ACC=2,show_text_lowpriority %1s% time %2d%
eOpcodeResult CustomOpcodes::OPCODE_0ACC(Script *script)
{
	static wchar_t message_buf[HELP_MSG_LENGTH];
	script->Collect(2);
	const char *text = game.Scripts.pScriptParams[0].szVar;
	swprintf(message_buf, HELP_MSG_LENGTH, L"%hs", text);
	game.Text.pfAddMessage(message_buf, game.Scripts.pScriptParams[1].nVar, false, false);
	return OR_CONTINUE;
};

//0ACD=2,show_text_highpriority %1s% time %2d%
eOpcodeResult CustomOpcodes::OPCODE_0ACD(Script *script)
{
	static wchar_t message_buf[HELP_MSG_LENGTH];
	script->Collect(2);
	const char *text = game.Scripts.pScriptParams[0].szVar;
	swprintf(message_buf, HELP_MSG_LENGTH, L"%hs", text);
	game.Text.pfAddMessageJumpQ(message_buf, game.Scripts.pScriptParams[1].nVar, false, false);
	return OR_CONTINUE;
};

//0ACE=-1,show_formatted_text_box %1s%
eOpcodeResult CustomOpcodes::OPCODE_0ACE(Script *script)
{
	static wchar_t message_buf[HELP_MSG_LENGTH];
	script->Collect(1);
	char fmt[HELP_MSG_LENGTH]; char text[HELP_MSG_LENGTH];
	strcpy(fmt, game.Scripts.pScriptParams[0].szVar);
	format(script, text, sizeof(text), fmt);

	swprintf(message_buf, HELP_MSG_LENGTH, L"%hs", text);
	game.Text.pfSetHelpMessage(message_buf, false, false);

	while ((*(ScriptParamType *)(&game.Scripts.pScriptSpace[script->m_nIp])).type)
		script->Collect(1);
	script->m_nIp++;
	return OR_CONTINUE;
};

//0ACF=-1,show_formatted_styled_text %1s% time %2d% style %3d%
eOpcodeResult CustomOpcodes::OPCODE_0ACF(Script *script)
{
	script->Collect(3);
	char fmt[HELP_MSG_LENGTH]; char text[HELP_MSG_LENGTH]; static wchar_t message_buf[HELP_MSG_LENGTH];
	unsigned time, style;
	strcpy(fmt, game.Scripts.pScriptParams[0].szVar);
	time = game.Scripts.pScriptParams[1].nVar;
	style = game.Scripts.pScriptParams[2].nVar;
	format(script, text, sizeof(text), fmt);
	swprintf(message_buf, HELP_MSG_LENGTH, L"%hs", text);
	game.Text.pfAddBigMessageQ(message_buf, time, style - 1);
	while ((*(ScriptParamType *)(&game.Scripts.pScriptSpace[script->m_nIp])).type)
		script->Collect(1);
	script->m_nIp++;
	return OR_CONTINUE;
};

//0AD0=-1,show_formatted_text_lowpriority %1s% time %2s%
eOpcodeResult CustomOpcodes::OPCODE_0AD0(Script *script)
{
	script->Collect(2);
	char fmt[HELP_MSG_LENGTH]; char text[HELP_MSG_LENGTH]; static wchar_t message_buf[HELP_MSG_LENGTH];
	unsigned time;
	strcpy(fmt, game.Scripts.pScriptParams[0].szVar);
	time = game.Scripts.pScriptParams[1].nVar;
	format(script, text, sizeof(text), fmt);
	swprintf(message_buf, HELP_MSG_LENGTH, L"%hs", text);
	game.Text.pfAddMessage(message_buf, time, false, false);
	while ((*(ScriptParamType *)(&game.Scripts.pScriptSpace[script->m_nIp])).type)
		script->Collect(1);
	script->m_nIp++;
	return OR_CONTINUE;
};

//0AD1=-1,show_formatted_text_highpriority %1s% time %2s%
eOpcodeResult CustomOpcodes::OPCODE_0AD1(Script *script)
{
	script->Collect(2);
	char fmt[HELP_MSG_LENGTH]; char text[HELP_MSG_LENGTH]; static wchar_t message_buf[HELP_MSG_LENGTH];
	unsigned time;
	strcpy(fmt, game.Scripts.pScriptParams[0].szVar);
	time = game.Scripts.pScriptParams[1].nVar;
	format(script, text, sizeof(text), fmt);
	swprintf(message_buf, HELP_MSG_LENGTH, L"%hs", text);
	game.Text.pfAddMessageJumpQ(message_buf, time, false, false);
	while ((*(ScriptParamType *)(&game.Scripts.pScriptSpace[script->m_nIp])).type)
		script->Collect(1);
	script->m_nIp++;
	return OR_CONTINUE;
};

//0AD2=2,%2d% = player %1d% targeted_actor //IF and SET

//0AD3=-1,string %1d% format %2d%
eOpcodeResult CustomOpcodes::OPCODE_0AD3(Script *script)
{
	script->Collect(2);
	char fmt[HELP_MSG_LENGTH], *dst;
	dst = (char*)game.Scripts.pScriptParams[0].pVar;
	strcpy(fmt, game.Scripts.pScriptParams[1].szVar);
	format(script, dst, static_cast<size_t>(-1), fmt);
	while ((*(ScriptParamType *)(&game.Scripts.pScriptSpace[script->m_nIp])).type)
		script->Collect(1);
	script->m_nIp++;
	return OR_CONTINUE;
};

//0AD4=-1,  scan_string %1d% format %2s% store_num_results_to %3d%
eOpcodeResult CustomOpcodes::OPCODE_0AD4(Script *script)
{
	script->Collect(2);
	char fmt[HELP_MSG_LENGTH], *src;
	src = game.Scripts.pScriptParams[0].szVar;
	strcpy(fmt, game.Scripts.pScriptParams[1].szVar);
	size_t cExParams = 0;
	int *result = (int *)script->GetPointerToScriptVariable();
	ScriptParam *ExParams[35];
	memset(ExParams, 0, 35 * sizeof(ScriptParam*));
	// read extra params
	while ((*(ScriptParamType *)(&game.Scripts.pScriptSpace[script->m_nIp])).type)
	{
		ExParams[cExParams++] = (ScriptParam *)script->GetPointerToScriptVariable();
	}
	script->m_nIp++;

	*result = sscanf(src, fmt,
		ExParams[0], ExParams[1], ExParams[2], ExParams[3], ExParams[4], ExParams[5],
		ExParams[6], ExParams[7], ExParams[8], ExParams[9], ExParams[10], ExParams[11],
		ExParams[12], ExParams[13], ExParams[14], ExParams[15], ExParams[16], ExParams[17],
		ExParams[18], ExParams[19], ExParams[20], ExParams[21], ExParams[22], ExParams[23],
		ExParams[24], ExParams[25], ExParams[26], ExParams[27], ExParams[28], ExParams[29],
		ExParams[30], ExParams[31], ExParams[32], ExParams[33], ExParams[34]);

	if (*result)
		script->UpdateCompareFlag(true);
	return OR_CONTINUE;
};

//0AD5=3,file %1d% seek %2d% from_origin %3d% //IF and SET
eOpcodeResult CustomOpcodes::OPCODE_0AD5(Script *script)
{
	script->Collect(3);
	std::FILE* file = (std::FILE*)game.Scripts.pScriptParams[0].pVar;
	int seek = game.Scripts.pScriptParams[1].nVar;
	int origin = game.Scripts.pScriptParams[2].nVar;
	script->UpdateCompareFlag(fseek(file, seek, origin) == 0);
	return OR_CONTINUE;
};

//0AD6=1, end_of_file %1d% reached
eOpcodeResult CustomOpcodes::OPCODE_0AD6(Script *script)
{
	script->Collect(1);
	std::FILE* file = (std::FILE*)game.Scripts.pScriptParams[0].pVar;
	script->UpdateCompareFlag(feof(file) != 0);
	return OR_CONTINUE;
};

//0AD7=3,read_string_from_file %1d% to %2d% size %3d% // IF and SET
eOpcodeResult CustomOpcodes::OPCODE_0AD7(Script *script)
{
	script->Collect(3);
	std::FILE* file = (std::FILE*)game.Scripts.pScriptParams[0].pVar;
	char* buf = (char*)game.Scripts.pScriptParams[1].pVar;
	unsigned size = game.Scripts.pScriptParams[2].nVar;
	script->UpdateCompareFlag(fgets(buf, size, file) == buf);
	return OR_CONTINUE;
};

//0AD8=2,write_string_to_file %1d% from %2d% //IF and SET
eOpcodeResult CustomOpcodes::OPCODE_0AD8(Script *script)
{
	script->Collect(2);
	std::FILE* file = (std::FILE*)game.Scripts.pScriptParams[0].pVar;
	char* buf = (char*)game.Scripts.pScriptParams[1].pVar;
	script->UpdateCompareFlag(fputs(buf, file) > 0);
	fflush(file);
	return OR_CONTINUE;
};

//0AD9=-1,write_formatted_text %2d% in_file %1d%
eOpcodeResult CustomOpcodes::OPCODE_0AD9(Script *script)
{
	script->Collect(2);
	char fmt[HELP_MSG_LENGTH]; char text[HELP_MSG_LENGTH];
	std::FILE* file = (std::FILE*)game.Scripts.pScriptParams[0].pVar;
	strcpy(fmt, game.Scripts.pScriptParams[1].szVar);
	format(script, text, sizeof(text), fmt);
	fputs(text, file);
	fflush(file);
	while ((*(ScriptParamType *)(&game.Scripts.pScriptSpace[script->m_nIp])).type)
		script->Collect(1);
	script->m_nIp++;
	return OR_CONTINUE;
};

//0ADA=-1,%3d% = scan_file %1d% format %2d% //IF and SET
eOpcodeResult CustomOpcodes::OPCODE_0ADA(Script *script)
{
	script->Collect(2);
	char fmt[HELP_MSG_LENGTH];
	std::FILE* file = (std::FILE*)game.Scripts.pScriptParams[0].pVar;
	strcpy(fmt, game.Scripts.pScriptParams[1].szVar);
	size_t cExParams = 0;
	int *result = (int *)script->GetPointerToScriptVariable();
	ScriptParam *ExParams[35];
	memset(ExParams, 0, 35 * sizeof(ScriptParam*));
	// read extra params
	while ((*(ScriptParamType *)(&game.Scripts.pScriptSpace[script->m_nIp])).type)
	{
		ExParams[cExParams++] = (ScriptParam *)script->GetPointerToScriptVariable();
	}
	script->m_nIp++;

	*result = fscanf(file, fmt,
		ExParams[0], ExParams[1], ExParams[2], ExParams[3], ExParams[4], ExParams[5],
		ExParams[6], ExParams[7], ExParams[8], ExParams[9], ExParams[10], ExParams[11],
		ExParams[12], ExParams[13], ExParams[14], ExParams[15], ExParams[16], ExParams[17],
		ExParams[18], ExParams[19], ExParams[20], ExParams[21], ExParams[22], ExParams[23],
		ExParams[24], ExParams[25], ExParams[26], ExParams[27], ExParams[28], ExParams[29],
		ExParams[30], ExParams[31], ExParams[32], ExParams[33], ExParams[34]);
	return OR_CONTINUE;
};

//0ADB=2,%2d% = car_model %1o% name
eOpcodeResult CustomOpcodes::OPCODE_0ADB(Script *script)
{
	script->Collect(1);
	auto modelIdx = game.Scripts.pScriptParams[0].nVar;

#if CLEO_VC
	char* gxt = (char*)((game.Misc.pVehicleModelStore + 0x32) + ((modelIdx - 130) * 0x174)); // pVehicleModelStore.m_gameName + (modelIdx - MI_FIRST_VEHICLE) * sizeof(CVehicleModelInfo)
#else
	char* gxt = (char*)((game.Misc.pVehicleModelStore + 0x36) + ((modelIdx - 90) * 0x1F8)); // pVehicleModelStore.m_gameName + (modelIdx - MI_FIRST_VEHICLE) * sizeof(CVehicleModelInfo)
#endif

	auto resultType = script->GetNextParamType();
	switch (resultType)
	{
	// pointer to target buffer
	case eParamType::PARAM_TYPE_LVAR:
	case eParamType::PARAM_TYPE_GVAR:
		script->Collect(1);
		strcpy(game.Scripts.pScriptParams[0].szVar, gxt);
		script->UpdateCompareFlag(true);
		return OR_CONTINUE;
	}

	// unsupported result param type
	script->Collect(1); // skip result param
	return OR_CONTINUE;
}

//0ADC=1,   test_cheat %1d%
eOpcodeResult CustomOpcodes::OPCODE_0ADC(Script *script)
{
	script->Collect(1);

	char *c = game.Text.szKeyboardCheatString;
	char buf[30];
	strcpy(buf, game.Scripts.pScriptParams[0].szVar);
	char *s = _strrev(buf);
	while (*s)
	{
		if (toupper(*s++) != *c++)
		{
			script->UpdateCompareFlag(false);
			return OR_CONTINUE;
		}
	}
	game.Text.szKeyboardCheatString[0] = 0;
	script->UpdateCompareFlag(true);

	return OR_CONTINUE;
}

//0ADD=1,spawn_car_with_model %1o% like_a_cheat
eOpcodeResult CustomOpcodes::OPCODE_0ADD(Script *script)
{
	script->Collect(1);

#if CLEO_VC
	game.Misc.pfSpawnCar(game.Scripts.pScriptParams[0].nVar);
#else
	int modelIdx = game.Scripts.pScriptParams[0].nVar;
	int fun = (int)game.Misc.pfSpawnCar;
	const char oriModelIdx = 122; // by default function spawns tank

	// pfSpawnCar checks in models info table if model was loaded
	// calculate new address of 'model loaded' byte
	int oriAddress = *(int*)(fun + 0x33);
	int newAddrres = oriAddress + (modelIdx - oriModelIdx) * 20; // 20 bytes peer model entry

	CPatch::SetChar(fun + 0x22, modelIdx);
	CPatch::SetInt(fun + 0x33, newAddrres);
	CPatch::SetChar(fun + 0xA5, modelIdx);

	game.Misc.pfSpawnCar(); // TODO: fix crash when model index is >= 128

	CPatch::SetChar(fun + 0x22, oriModelIdx);
	CPatch::SetInt(fun + 0x33, oriAddress);
	CPatch::SetChar(fun + 0xA5, oriModelIdx);
#endif
	return OR_CONTINUE;
}

//0ADE=2,%2d% = text_by_GXT_entry %1d%
eOpcodeResult CustomOpcodes::OPCODE_0ADE(Script *script)
{
	script->Collect(2);
	char *gxt = game.Scripts.pScriptParams[0].szVar;
	char *result = game.Scripts.pScriptParams[1].szVar;
	wchar_t *text = CustomText::GetText(game.Text.pTheText, 0, gxt);
	wcstombs(result, text, wcslen(text));
	result[wcslen(text)] = '\0';
	return OR_CONTINUE;
}

//0ADF=2,add_dynamic_GXT_entry %1d% text %2d%
eOpcodeResult
CustomOpcodes::OPCODE_0ADF(Script* script)
{
		script->Collect(2);

		fxt::Add(game.Scripts.pScriptParams[0].szVar, game.Scripts.pScriptParams[1].szVar);

		return OR_CONTINUE;
}

//0AE0=1,remove_dynamic_GXT_entry %1d%
eOpcodeResult
CustomOpcodes::OPCODE_0AE0(Script* script)
{
		script->Collect(1);

		fxt::Remove(game.Scripts.pScriptParams[0].szVar);

		return OR_CONTINUE;
}

//0AE1=7,%7d% = random_actor_near_point %1d% %2d% %3d% in_radius %4d% find_next %5h% pass_deads %6h% //IF and SET //dup
//0AE2=7,%7d% = random_vehicle_near_point %1d% %2d% %3d% in_radius %4d% find_next %5h% pass_wrecked %6h% //IF and SET //dup
//0AE3=6,%6d% = random_object_near_point %1d% %2d% %3d% in_radius %4d% find_next %5h% //IF and SET //dup

//0AE4=1,  directory_exists %1d%
eOpcodeResult
CustomOpcodes::OPCODE_0AE4(Script* script)
{
		script->Collect(1);

		script->UpdateCompareFlag(fs::is_directory(game.Scripts.pScriptParams[0].szVar));

		return OR_CONTINUE;
}

//0AE5=1,create_directory %1d% ; IF and SET
eOpcodeResult
CustomOpcodes::OPCODE_0AE5(Script* script)
{
		script->Collect(1);

		script->UpdateCompareFlag(fs::create_directories(game.Scripts.pScriptParams[0].szVar));

		return OR_CONTINUE;
}

//0AE6=3,%2d% = find_first_file %1d% get_filename_to %3d% ; IF and SET
eOpcodeResult
CustomOpcodes::OPCODE_0AE6(Script* script)
{
		script->Collect(1);

		try {
				script->StoreCache(&m_pFileSearchHandles, new fs::directory_iterator(game.Scripts.pScriptParams[0].szVar));

				script->UpdateCompareFlag(*m_pFileSearchHandles->data != end(*m_pFileSearchHandles->data)); // check if directory is not empty
				game.Scripts.pScriptParams[0].pVar = m_pFileSearchHandles->data;
		} catch (...) {
				script->UpdateCompareFlag(false);
				game.Scripts.pScriptParams[0].pVar = nullptr;
		}
		script->Store(1);

		script->Collect(1);
		const char* src = script->m_bCondResult ? *(fs::directory_iterator*)(m_pFileSearchHandles->data)->path().filename().c_str() : "";
		std::strcpy(game.Scripts.pScriptParams[0].szVar, src);

		return OR_CONTINUE;
}

//0AE7=2,%2d% = find_next_file %1d% ; IF and SET
eOpcodeResult
CustomOpcodes::OPCODE_0AE7(Script* script)
{
		script->Collect(2);
		fs::directory_iterator* handle = (fs::directory_iterator*)game.Scripts.pScriptParams[0].pVar;

		(*handle)++;
		bool result = (*handle != end(*handle)) ? true : false;

		script->UpdateCompareFlag(result);
		const char* src = result ? (*handle)->path().filename().c_str() : "";
		std::strcpy(game.Scripts.pScriptParams[1].szVar, src);

		return OR_CONTINUE;
}

//0AE8=1,find_close %1d%
eOpcodeResult
CustomOpcodes::OPCODE_0AE8(Script* script)
{
		script->Collect(1);
		fs::directory_iterator* handle = (fs::directory_iterator*)game.Scripts.pScriptParams[0].pVar;

		delete handle;
		scriptMgr::DeleteFileSearchHandle(handle);

		return OR_CONTINUE;
}

//0AE9=1,pop_float %1d% //dup
//0AEA=2,%2d% = actor_struct %1d% handle //dup
//0AEB=2,%2d% = car_struct %1d% handle //dup
//0AEC=2,%2d% = object_struct %1d% handle //dup
//0AED=3,%3d% = float %1d% to_string_format %2d%
//0AEE=3,%3d% = exp %1d% base %2d% //all floats //dup
//0AEF=3,%3d% = log %1d% base %2d% //all floats //dup

//0AF8=2,cleo_array %1d% = %2d%
eOpcodeResult __stdcall CustomOpcodes::SET_CLEO_ARRAY(Script *script)
{
	script->Collect(2);
	script->m_pCleoArray[game.Scripts.pScriptParams[0].nVar].nVar = game.Scripts.pScriptParams[1].nVar;
	return OR_CONTINUE;
}

//0AF9=2,%2d% = cleo_array %1d%
eOpcodeResult __stdcall CustomOpcodes::GET_CLEO_ARRAY(Script *script)
{
	script->Collect(1);
	game.Scripts.pScriptParams[0].nVar = script->m_pCleoArray[game.Scripts.pScriptParams[0].nVar].nVar;
	script->Store(1);
	return OR_CONTINUE;
}

//0AFA=2,%2d% = cleo_array %1d% pointer
eOpcodeResult __stdcall CustomOpcodes::GET_CLEO_ARRAY_OFFSET(Script *script)
{
	script->Collect(1);
	game.Scripts.pScriptParams[0].pVar = &script->m_pCleoArray[game.Scripts.pScriptParams[0].nVar].nVar;
	script->Store(1);
	return OR_CONTINUE;
}

//0AFB=3,%3d% = script %1d% cleo_array %2d% pointer
eOpcodeResult __stdcall CustomOpcodes::GET_CLEO_ARRAY_SCRIPT(Script *script)
{
	script->Collect(2);
	Script *pScript = reinterpret_cast<Script*>(game.Scripts.pScriptParams[0].pVar);
	if (pScript)
	{
		game.Scripts.pScriptParams[0].pVar = &pScript->m_pCleoArray[game.Scripts.pScriptParams[1].nVar].nVar;
	}
	script->Store(1);
	return OR_CONTINUE;
}

//0DD5=1,%1d% = get_platform
eOpcodeResult
CustomOpcodes::GET_PLATFORM(Script* script)
{
		game.Scripts.pScriptParams[0].nVar = PLATFORM_WINDOWS;
		script->Store(1);

		return OR_CONTINUE;
}

// perform 'sprintf'-operation for parameters, passed through SCM
int format(Script *script, char *str, size_t len, const char *format)
{
	unsigned int written = 0;
	const char *iter = format;
	char bufa[256], fmtbufa[64], *fmta;

	while (*iter)
	{
		while (*iter && *iter != '%')
		{
			if (written++ >= len)
				return -1;
			*str++ = *iter++;
		}
		if (*iter == '%')
		{
			if (iter[1] == '%')
			{
				if (written++ >= len)
					return -1;
				*str++ = '%'; /* "%%"->'%' */
				iter += 2;
				continue;
			}

			//get flags and width specifier
			fmta = fmtbufa;
			*fmta++ = *iter++;
			while (*iter == '0' ||
				*iter == '+' ||
				*iter == '-' ||
				*iter == ' ' ||
				*iter == '*' ||
				*iter == '#')
			{
				if (*iter == '*')
				{
					char *buffiter = bufa;
					//get width
					script->Collect(1);
					_itoa(game.Scripts.pScriptParams[0].nVar, buffiter, 10);
					while (*buffiter)
						*fmta++ = *buffiter++;
				}
				else
					*fmta++ = *iter;
				iter++;
			}

			//get immidiate width value
			while (isdigit(*iter))
				*fmta++ = *iter++;

			//get precision
			if (*iter == '.')
			{
				*fmta++ = *iter++;
				if (*iter == '*')
				{
					char *buffiter = bufa;
					script->Collect(1);
					_itoa(game.Scripts.pScriptParams[0].nVar, buffiter, 10);
					while (*buffiter)
						*fmta++ = *buffiter++;
				}
				else
					while (isdigit(*iter))
						*fmta++ = *iter++;
			}
			//get size
			if (*iter == 'h' || *iter == 'l')
				*fmta++ = *iter++;

			switch (*iter)
			{
			case 's':
			{
				script->Collect(1);
				static const char none[] = "(null)";
				const char *astr = game.Scripts.pScriptParams[0].szVar;
				const char *striter = astr ? astr : none;
				while (*striter)
				{
					if (written++ >= len)
						return -1;
					*str++ = *striter++;
				}
				iter++;
				break;
			}

			case 'c':
				if (written++ >= len)
					return -1;
				script->Collect(1);
				*str++ = (char)game.Scripts.pScriptParams[0].nVar;
				iter++;
				break;

			default:
			{
				/* For non wc types, use system sprintf and append to wide char output */
				/* FIXME: for unrecognised types, should ignore % when printing */
				char *bufaiter = bufa;
				if (*iter == 'p' || *iter == 'P')
				{
					script->Collect(1);
					sprintf(bufaiter, "%08X", game.Scripts.pScriptParams[0].nVar);
				}
				else
				{
					*fmta++ = *iter;
					*fmta = '\0';
					if (*iter == 'a' || *iter == 'A' ||
						*iter == 'e' || *iter == 'E' ||
						*iter == 'f' || *iter == 'F' ||
						*iter == 'g' || *iter == 'G')
					{
						script->Collect(1);
						sprintf(bufaiter, fmtbufa, game.Scripts.pScriptParams[0].fVar);
					}
					else
					{
						script->Collect(1);
						sprintf(bufaiter, fmtbufa, game.Scripts.pScriptParams[0].pVar);
					}
				}
				while (*bufaiter)
				{
					if (written++ >= len)
						return -1;
					*str++ = *bufaiter++;
				}
				iter++;
				break;
			}
			}
		}
	}
	if (written >= len)
		return -1;
	*str++ = 0;
	return (int)written;
}
