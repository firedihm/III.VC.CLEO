#include "CleoVersion.h"
#include "Fxt.h"
#include "Game.h"
#include "Log.h"
#include "Memory.h"
#include "Opcodes.h"
#include "Script.h"
#include "ScriptManager.h"

#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <memory>

namespace fs = std::filesystem;

constexpr int HELP_MSG_LENGTH = 256;

ScriptParam SharedVars[0xFFFF];

int format(Script *script, char *str, size_t len, const char *format);

/*
// Exports
CLEOAPI unsigned CLEO_GetVersion() { return CLEO_VERSION; }
CLEOAPI char* CLEO_GetScriptSpaceAddress() { return game::ScriptSpace; }
CLEOAPI ScriptParam* CLEO_GetParamsAddress() { return game::ScriptParams; }

#ifdef __cplusplus
extern "C" {
#endif
	unsigned __stdcall _CLEO_GetVersion() { return CLEO_GetVersion(); }
	char* __stdcall _CLEO_GetScriptSpaceAddress() { return CLEO_GetScriptSpaceAddress(); }
	ScriptParam* __stdcall _CLEO_GetParamsAddress() { return CLEO_GetParamsAddress(); }
	bool __stdcall CLEO_RegisterOpcode(unsigned short id, Opcode func) { return opcodes::Register(id, func); }

	// Script methods
	void __stdcall CLEO_Collect(Script* script, unsigned int numParams) { script->CollectParameters(numParams); }
	void __stdcall CLEO_CollectAt(Script* script, unsigned int* pIp, unsigned int numParams) { script->CollectParameters(pIp, numParams); }
	int __stdcall CLEO_CollectNextWithoutIncreasingPC(Script* script, unsigned int ip) { return script->CollectNextWithoutIncreasingPC(ip); }
	eParamType __stdcall CLEO_GetNextParamType(Script* script) { return script->GetNextParamType(); }
	void __stdcall CLEO_Store(Script* script, unsigned int numParams) { script->StoreParameters(numParams); }
	void __stdcall CLEO_ReadShortString(Script* script, char* out) { script->ReadShortString(out); }
	void __stdcall CLEO_UpdateCompareFlag(Script* script, bool result) { script->UpdateCompareFlag(result); }
	void* __stdcall CLEO_GetPointerToScriptVariable(Script* script) { return script->GetPointerToScriptVariable(); }
	void __stdcall CLEO_JumpTo(Script* script, int address) { script->JumpTo(address); }
#ifdef __cplusplus
}
#endif
*/

eOpcodeResult
__stdcall DUMMY(Script* script)
{
		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GOTO(Script* script)
{
		script->CollectParameters(1);

		script->JumpTo(game::ScriptParams[0].nVar);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GOTO_IF_TRUE(Script* script)
{
		script->CollectParameters(1);

		if (script->m_bCondResult)
				script->JumpTo(game::ScriptParams[0].nVar);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GOTO_IF_FALSE(Script* script)
{
		script->CollectParameters(1);

		if (!script->m_bCondResult)
				script->JumpTo(game::ScriptParams[0].nVar);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GOSUB(Script* script)
{
		script->CollectParameters(1);

		script->m_anGosubStack[script->m_nGosubStackPointer++] = script->m_nIp;
		script->JumpTo(game::ScriptParams[0].nVar);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall TERMINATE_THIS_CUSTOM_SCRIPT(Script* script)
{
		LOGL(LOG_PRIORITY_OPCODE, "TERMINATE_THIS_CUSTOM_SCRIPT: Terminating custom script \"%s\"", &script->m_acName);
		script_mgr::TerminateScript(script);

		return OR_TERMINATE;
}

eOpcodeResult
__stdcall TERMINATE_ALL_CUSTOM_SCRIPTS_WITH_THIS_NAME(Script* script)
{
		char name[KEY_LENGTH_IN_SCRIPT];
		script->ReadShortString(&name);

		bool terminate_self = false;
		while (Script* found = script_mgr::FindScriptNamed(&name)) {
				LOGL(LOG_PRIORITY_OPCODE, "TERMINATE_ALL_CUSTOM_SCRIPTS_WITH_THIS_NAME: Terminating custom script \"%s\"", &found->m_acName);
				script_mgr::TerminateScript(found);

				terminate_self = (found == script) ? true : false;
		}

		return terminate_self ? OR_TERMINATE : OR_CONTINUE;
}

eOpcodeResult
__stdcall START_CUSTOM_SCRIPT(Script* script)
{
		char filename[KEY_LENGTH_IN_SCRIPT];
		script->ReadShortString(&filename);

		fs::path filepath = fs::path(game::RootDirName) / "CLEO" / &filename;

		LOGL(LOG_PRIORITY_OPCODE, "START_CUSTOM_SCRIPT: Starting new script \"%s\"", filepath.c_str());
		Script* new_script = script_mgr::StartScript(filepath.c_str());

		for (int i = 0; i < Script::NUM_LOCAL_VARS && script->GetNextParamType(); ++i) {
				script->CollectParameters(1);
				new_script->m_aLVars[i].nVar = game::ScriptParams[0].nVar;
		}
		script->m_nIp++; // consume PARAM_TYPE_END_OF_PARAMS

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall WRITE_MEMORY(Script* script)
{
		script->CollectParameters(4);

		memory::Write(game::ScriptParams[0].pVar, &game::ScriptParams[2].nVar, game::ScriptParams[1].nVar, game::ScriptParams[3].nVar);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall READ_MEMORY(Script* script)
{
		script->CollectParameters(3);

		game::ScriptParams[0].nVar = memory::Read(game::ScriptParams[0].pVar, game::ScriptParams[1].nVar, game::ScriptParams[2].nVar);
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult CustomOpcodes::CALL(Script *script)
{
	script->CollectParameters(3);
	void *func = game::ScriptParams[0].pVar;
	unsigned int popsize = game::ScriptParams[2].nVar * 4;
	for(int i = 0; i < game::ScriptParams[1].nVar; i++)
	{
		script->CollectParameters(1);
		unsigned int param = game::ScriptParams[0].nVar;
		__asm push param
	}
	__asm call func
	__asm add esp, popsize
	while((*(ScriptParamType *)(&game::ScriptSpace[script->m_nIp])).type)
		script->CollectParameters(1);
	script->m_nIp++;
	return OR_CONTINUE;
}

eOpcodeResult CustomOpcodes::CALL_FUNCTION(Script *script)
{
	script->CollectParameters(3);
	void *func = game::ScriptParams[0].pVar;
	int func_result;
	unsigned int popsize = game::ScriptParams[2].nVar * 4;
	for(int i = 0; i < game::ScriptParams[1].nVar; i++)
	{
		script->CollectParameters(1);
		unsigned int param = game::ScriptParams[0].nVar;
		__asm push param
	}
	__asm call func
	__asm mov func_result, eax
	__asm add esp, popsize
	game::ScriptParams[0].nVar = func_result;
	script->StoreParameters(1);
	while((*(ScriptParamType *)(&game::ScriptSpace[script->m_nIp])).type)
		script->CollectParameters(1);
	script->m_nIp++;
	return OR_CONTINUE;
}

eOpcodeResult CustomOpcodes::CALL_METHOD(Script *script)
{
	script->CollectParameters(4);
	void *func = game::ScriptParams[0].pVar;
	void *object = game::ScriptParams[1].pVar;
	unsigned int popsize = game::ScriptParams[3].nVar * 4;
	for(int i = 0; i < game::ScriptParams[2].nVar; i++)
	{
		script->CollectParameters(1);
		unsigned int param = game::ScriptParams[0].nVar;
		__asm push param
	}
	__asm mov ecx, object
	__asm call func
	__asm add esp, popsize
	while((*(ScriptParamType *)(&game::ScriptSpace[script->m_nIp])).type)
		script->CollectParameters(1);
	script->m_nIp++;
	return OR_CONTINUE;
}

eOpcodeResult CustomOpcodes::CALL_FUNCTION_METHOD(Script *script)
{
	script->CollectParameters(4);
	void *func = game::ScriptParams[0].pVar;
	void *object = game::ScriptParams[1].pVar;
	int func_result;
	unsigned int popsize = game::ScriptParams[3].nVar * 4;
	for(int i = 0; i < game::ScriptParams[2].nVar; i++)
	{
		script->CollectParameters(1);
		unsigned int param = game::ScriptParams[0].nVar;
		__asm push param
	}
	__asm mov ecx, object
	__asm call func
	__asm mov func_result, eax
	__asm add esp, popsize
	game::ScriptParams[0].nVar = func_result;
	script->StoreParameters(1);
	while((*(ScriptParamType *)(&game::ScriptSpace[script->m_nIp])).type)
		script->CollectParameters(1);
	script->m_nIp++;
	return OR_CONTINUE;
}

eOpcodeResult
__stdcall GET_GAME_VERSION(Script* script)
{
		int result = (game::Version == game::Release::VC_1_0 || game::Version == game::Release::III_1_0) ? 0 :
					 (game::Version == game::Release::VC_1_1 || game::Version == game::Release::III_1_1) ? 1 : 2;

		game::ScriptParams[0].nVar = result;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GET_PED_POINTER(Script* script)
{
		script->CollectParameters(1);

		game::ScriptParams[0].pVar = game::PedPoolGetAt(*game::ppPedPool, game::ScriptParams[0].nVar);
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GET_VEHICLE_POINTER(Script* script)
{
		script->CollectParameters(1);

		game::ScriptParams[0].pVar = game::VehiclePoolGetAt(*game::ppVehiclePool, game::ScriptParams[0].nVar);
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GET_OBJECT_POINTER(Script* script)
{
		script->CollectParameters(1);

		game::ScriptParams[0].pVar = game::ObjectPoolGetAt(*game::ppObjectPool, game::ScriptParams[0].nVar);
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GET_PED_REF(Script* script)
{
		script->CollectParameters(1);

		game::ScriptParams[0].nVar = game::PedPoolGetIndex(*game::ppPedPool, game::ScriptParams[0].pVar);
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GET_VEHICLE_REF(Script* script)
{
		script->CollectParameters(1);

		game::ScriptParams[0].nVar = game::VehiclePoolGetIndex(*game::ppVehiclePool, game::ScriptParams[0].pVar);
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GET_OBJECT_REF(Script* script)
{
		script->CollectParameters(1);

		game::ScriptParams[0].nVar = game::ObjectPoolGetIndex(*game::ppObjectPool, game::ScriptParams[0].pVar);
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GET_THIS_SCRIPT_STRUCT(Script* script)
{
		game::ScriptParams[0].pVar = script;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GET_SCRIPT_STRUCT_NAMED(Script* script)
{
		char name[KEY_LENGTH_IN_SCRIPT];
		script->ReadShortString(&name);

		game::ScriptParams[0].pVar = script_mgr::FindScriptNamed(&name, true);
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult CustomOpcodes::IS_KEY_PRESSED(Script *script)
{
	script->CollectParameters(1);
#if CLEO_VC
	switch (game::ScriptParams[0].nVar)
	{
	case OVK_F1:
		game::ScriptParams[0].nVar = VK_F1;
		break;
	case OVK_F2:
		game::ScriptParams[0].nVar = VK_F2;
		break;
	case OVK_F3:
		game::ScriptParams[0].nVar = VK_F3;
		break;
	case OVK_F4:
		game::ScriptParams[0].nVar = VK_F4;
		break;
	case OVK_F5:
		game::ScriptParams[0].nVar = VK_F5;
		break;
	case OVK_F6:
		game::ScriptParams[0].nVar = VK_F6;
		break;
	case OVK_F7:
		game::ScriptParams[0].nVar = VK_F7;
		break;
	case OVK_F8:
		game::ScriptParams[0].nVar = VK_F8;
		break;
	case OVK_F9:
		game::ScriptParams[0].nVar = VK_F9;
		break;
	case OVK_F10:
		game::ScriptParams[0].nVar = VK_F10;
		break;
	case OVK_F11:
		game::ScriptParams[0].nVar = VK_F11;
		break;
	case OVK_F12:
		game::ScriptParams[0].nVar = VK_F12;
		break;
	case OVK_INSERT:
		game::ScriptParams[0].nVar = VK_INSERT;
		break;
	case OVK_DELETE:
		game::ScriptParams[0].nVar = VK_DELETE;
		break;
	case OVK_HOME:
		game::ScriptParams[0].nVar = VK_HOME;
		break;
	case OVK_END:
		game::ScriptParams[0].nVar = VK_END;
		break;
	case OVK_PRIOR:
		game::ScriptParams[0].nVar = VK_PRIOR;
		break;
	case OVK_NEXT:
		game::ScriptParams[0].nVar = VK_NEXT;
		break;
	case OVK_UP:
		game::ScriptParams[0].nVar = VK_UP;
		break;
	case OVK_DOWN:
		game::ScriptParams[0].nVar = VK_DOWN;
		break;
	case OVK_LEFT:
		game::ScriptParams[0].nVar = VK_LEFT;
		break;
	case OVK_RIGHT:
		game::ScriptParams[0].nVar = VK_RIGHT;
		break;
	case OVK_DIVIDE:
		game::ScriptParams[0].nVar = VK_DIVIDE;
		break;
	case OVK_MULTIPLY:
		game::ScriptParams[0].nVar = VK_MULTIPLY;
		break;
	case OVK_ADD:
		game::ScriptParams[0].nVar = VK_ADD;
		break;
	case OVK_SUBTRACT:
		game::ScriptParams[0].nVar = VK_SUBTRACT;
		break;
	case OVK_DECIMAL:
		game::ScriptParams[0].nVar = VK_DECIMAL;
		break;
	case OVK_NUMPAD1:
		game::ScriptParams[0].nVar = VK_NUMPAD1;
		break;
	case OVK_NUMPAD2:
		game::ScriptParams[0].nVar = VK_NUMPAD2;
		break;
	case OVK_NUMPAD3:
		game::ScriptParams[0].nVar = VK_NUMPAD3;
		break;
	case OVK_NUMPAD4:
		game::ScriptParams[0].nVar = VK_NUMPAD4;
		break;
	case OVK_NUMPAD5:
		game::ScriptParams[0].nVar = VK_NUMPAD5;
		break;
	case OVK_NUMLOCK:
		game::ScriptParams[0].nVar = VK_NUMLOCK;
		break;
	case OVK_NUMPAD6:
		game::ScriptParams[0].nVar = VK_NUMPAD6;
		break;
	case OVK_NUMPAD7:
		game::ScriptParams[0].nVar = VK_NUMPAD7;
		break;
	case OVK_NUMPAD8:
		game::ScriptParams[0].nVar = VK_NUMPAD8;
		break;
	case OVK_NUMPAD9:
		game::ScriptParams[0].nVar = VK_NUMPAD9;
		break;
	case OVK_NUMPAD0:
		game::ScriptParams[0].nVar = VK_NUMPAD0;
		break;
	case OVK_SEPARATOR:
		game::ScriptParams[0].nVar = VK_SEPARATOR;
		break;
	case OVK_SCROLL:
		game::ScriptParams[0].nVar = VK_SCROLL;
		break;
	case OVK_PAUSE:
		game::ScriptParams[0].nVar = VK_PAUSE;
		break;
	case OVK_BACK:
		game::ScriptParams[0].nVar = VK_BACK;
		break;
	case OVK_TAB:
		game::ScriptParams[0].nVar = VK_TAB;
		break;
	case OVK_CAPITAL:
		game::ScriptParams[0].nVar = VK_CAPITAL;
		break;
	case OVK_RETURN:
		game::ScriptParams[0].nVar = VK_RETURN;
		break;
	case OVK_LSHIFT:
		game::ScriptParams[0].nVar = VK_LSHIFT;
		break;
	case OVK_RSHIFT:
		game::ScriptParams[0].nVar = VK_RSHIFT;
		break;
	case OVK_ESC:
		game::ScriptParams[0].nVar = VK_ESCAPE;
		break;
	case OVK_LCONTROL:
		game::ScriptParams[0].nVar = VK_LCONTROL;
		break;
	case OVK_RCONTROL:
		game::ScriptParams[0].nVar = VK_RCONTROL;
		break;
	case OVK_LMENU:
		game::ScriptParams[0].nVar = VK_LMENU;
		break;
	case OVK_RMENU:
		game::ScriptParams[0].nVar = VK_RMENU;
		break;
	case OVK_LWIN:
		game::ScriptParams[0].nVar = VK_LWIN;
		break;
	case OVK_RWIN:
		game::ScriptParams[0].nVar = VK_RWIN;
		break;
	case OVK_APPS:
		game::ScriptParams[0].nVar = VK_APPS;
		break;
	default:
		break;
	}
#endif
	script->UpdateCompareFlag(GetKeyState(game::ScriptParams[0].nVar) & 0x8000);
	return OR_CONTINUE;
}

eOpcodeResult
__stdcall GET_RANDOM_CHAR_IN_SPHERE_NO_SAVE_RECURSIVE(Script* script)
{
#if CLEO_VC
	static constexpr auto off6D8 = 0x6D8;
	static constexpr auto off3D4 = 0x3D4;
	static constexpr auto off244 = 0x244;
#else
	static constexpr auto off6D8 = 0x5F0;
	static constexpr auto off3D4 = 0x32C;
	static constexpr auto off244 = 0x224;
#endif

		script->CollectParameters(6);
		float radius = game::ScriptParams[3].fVar;
		bool find_next = game::ScriptParams[4].nVar;
		bool skip_dead = game::ScriptParams[5].nVar;

		int search_index = find_next ? script->m_nLastPedSearchIndex : (*game::ppPedPool)->m_size;
		int obj_index = 0;

		for (int i = search_index - 1; i >= 0; --i) {
				uchar* obj = (uchar*)(*game::ppPedPool)->m_entries + i * off6D8;
				search_index = i;

				// POOLFLAG_ISFREE = 0x80
				if (!((*game::ppPedPool)->m_flags[i] & 0x80) && *(uint*)(obj + off3D4)) {
						if(!skip_dead || (*(uint*)(obj + off244) != 48 && *(uint*)(obj + off244) != 49)) {
								float xd = *(float*)(obj + 0x34) - game::ScriptParams[0].fVar;
								float yd = *(float*)(obj + 0x38) - game::ScriptParams[1].fVar;
								float zd = *(float*)(obj + 0x3C) - game::ScriptParams[2].fVar;
								float dist_sqr = xd * xd + yd * yd + zd * zd;
								if (dist_sqr <= std::pow(radius, 2)) {
										obj_index = game::PedPoolGetIndex(*game::ppPedPool, obj);
										break;
								}
						}
				}
		}

		script->m_nLastPedSearchIndex = search_index;

		script->UpdateCompareFlag(obj_index);

		game::ScriptParams[0].nVar = obj_index;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GET_RANDOM_CAR_IN_SPHERE_NO_SAVE_RECURSIVE(Script* script)
{
#if CLEO_VC
	static constexpr auto off5DC = 0x5DC;
	static constexpr auto off29C = 0x29C;
	static constexpr auto off11A = 0x11A;
#else
	static constexpr auto off5DC = 0x5A8;
	static constexpr auto off29C = 0x284;
	static constexpr auto off11A = 0x122;
#endif

		script->CollectParameters(6);
		float radius = game::ScriptParams[3].fVar;
		bool find_next = game::ScriptParams[4].nVar;
		bool skip_wrecked = game::ScriptParams[5].nVar;

		int search_index = find_next ? script->m_nLastVehicleSearchIndex : (*game::ppVehiclePool)->m_size;
		int obj_index = 0;

		for (int i = search_index - 1; i >= 0; --i) {
				uchar* obj = (uchar*)(*game::ppVehiclePool)->m_entries + i * off5DC;
				search_index = i;

				// POOLFLAG_ISFREE = 0x80
				if(!((*game::ppVehiclePool)->m_flags[i] & 0x80)) {
						if (!skip_wrecked || ((*(uchar*)(obj + 0x50) & 0xF8) != 40 && *(uint*)(obj + off29C) != 1 && !(*(uchar*)(obj + off11A) & 8))) {
								float xd = *(float*)(obj + 0x34) - game::ScriptParams[0].fVar;
								float yd = *(float*)(obj + 0x38) - game::ScriptParams[1].fVar;
								float zd = *(float*)(obj + 0x3C) - game::ScriptParams[2].fVar;
								float dist_sqr = xd * xd + yd * yd + zd * zd;
								if (dist_sqr <= std::pow(radius, 2)) {
										obj_index = game::VehiclePoolGetIndex(*game::ppVehiclePool, obj);
										break;
								}
						}
				}
		}

		script->m_nLastVehicleSearchIndex = search_index;

		script->UpdateCompareFlag(obj_index);

		game::ScriptParams[0].nVar = obj_index;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GET_RANDOM_OBJECT_IN_SPHERE_NO_SAVE_RECURSIVE(Script* script)
{
#if CLEO_VC
	static constexpr auto off1A0 = 0x1A0;
#else
	static constexpr auto off1A0 = 0x19C;
#endif

		script->CollectParameters(5);
		float radius = game::ScriptParams[3].fVar;
		bool find_next = game::ScriptParams[4].nVar;

		int search_index = find_next ? script->m_nLastObjectSearchIndex : (*game::ppObjectPool)->m_size;
		int obj_index = 0;

		for(int i = search_index - 1; i >= 0; --i) {
				uchar* obj = (uchar*)(*game::ppObjectPool)->m_entries + i * off1A0;
				search_index = i;

				// POOLFLAG_ISFREE = 0x80
				if (!((*game::ppObjectPool)->m_flags[i] & 0x80)) {
						float xd = *(float*)(obj + 0x34) - game::ScriptParams[0].fVar;
						float yd = *(float*)(obj + 0x38) - game::ScriptParams[1].fVar;
						float zd = *(float*)(obj + 0x3C) - game::ScriptParams[2].fVar;
						float dist_sqr = xd * xd + yd * yd + zd * zd;
						if (dist_sqr <= std::pow(radius, 2)) {
								obj_index = game::ObjectPoolGetIndex(*game::ppObjectPool, obj);
								break;
						}
				}
		}

		script->m_nLastObjectSearchIndex = search_index;

		script->UpdateCompareFlag(obj_index);

		game::ScriptParams[0].nVar = obj_index;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult CustomOpcodes::CALL_POP_FLOAT(Script *script)
{
	float *pParam = &game::ScriptParams[0].fVar;
	__asm mov eax, pParam
	__asm fstp [eax]
	script->StoreParameters(1);
	return OR_CONTINUE;
}

eOpcodeResult CustomOpcodes::MATH_EXP(Script *script)
{
	script->CollectParameters(2);
	game::ScriptParams[0].fVar = powf(game::ScriptParams[0].fVar, game::ScriptParams[1].fVar);
	script->StoreParameters(1);
	return OR_CONTINUE;
}

eOpcodeResult CustomOpcodes::MATH_LOG(Script *script)
{
	script->CollectParameters(2);
	game::ScriptParams[0].fVar =  logf(game::ScriptParams[0].fVar) / logf(game::ScriptParams[1].fVar);
	script->StoreParameters(1);
	return OR_CONTINUE;
}

eOpcodeResult
__stdcall CLEO_CALL(Script* script)
{
		script->CollectParameters(2);
		int addr = game::ScriptParams[0].nVar;
		int paramCount = game::ScriptParams[1].nVar;

		script->CollectParameters(paramCount);

		/*
			We didn't actually read all params this opcode provides just yet: after we read values that caller 
			passes to callee with script->CollectParameters(), there are indexes of caller's m_aLVars where callee's 
			retvals should be stored. We will continue reading them after returning from callee.
		*/
		script->PushStackFrame();

		std::memcpy(&script->m_aLVars, &game::ScriptParams, paramCount * sizeof(ScriptParam));
		script->JumpTo(addr);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall CLEO_RETURN(Script* script)
{
		script->CollectParameters(1);
		int paramCount = game::ScriptParams[0].nVar;

		// collect callee's retvals
		script->CollectParameters(paramCount);

		// return to caller
		script->PopStackFrame();

		// continue reading indexes of caller's m_aLVars to store callee's retvals
		script->StoreParameters(paramCount);

		// variadic opcodes like 0AB1: CLEO_CALL end with PARAM_TYPE_END_OF_PARAMS; consume it.
		script->m_nIp++;

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GET_LABEL_POINTER(Script* script)
{
		script->CollectParameters(1);
		int address = game::ScriptParams[0].nVar;

		// negated address is a hack that lets us tell custom and mission scripts from regular ones
		void* result;
		if (address >= 0)
				result = &game::ScriptSpace[address];
		else {
				if (script->m_bIsCustom)
						result = &script->m_pCodeData[-address];
				else
						result = &game::ScriptSpace[game::MainSize + (-address)];
		}

		game::ScriptParams[0].pVar = result;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GET_VAR_POINTER(Script* script)
{
		game::ScriptParams[0].pVar = script->GetPointerToScriptVariable();
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult CustomOpcodes::BIT_AND(Script *script)
{
	script->CollectParameters(2);
	game::ScriptParams[0].nVar = game::ScriptParams[0].nVar & game::ScriptParams[1].nVar;
	script->StoreParameters(1);
	return OR_CONTINUE;
}

eOpcodeResult CustomOpcodes::BIT_OR(Script *script)
{
	script->CollectParameters(2);
	game::ScriptParams[0].nVar = game::ScriptParams[0].nVar | game::ScriptParams[1].nVar;
	script->StoreParameters(1);
	return OR_CONTINUE;
}

eOpcodeResult CustomOpcodes::BIT_XOR(Script *script)
{
	script->CollectParameters(2);
	game::ScriptParams[0].nVar = game::ScriptParams[0].nVar ^ game::ScriptParams[1].nVar;
	script->StoreParameters(1);
	return OR_CONTINUE;
}

eOpcodeResult CustomOpcodes::BIT_NOT(Script *script)
{
	script->CollectParameters(1);
	game::ScriptParams[0].nVar = ~game::ScriptParams[0].nVar;
	script->StoreParameters(1);
	return OR_CONTINUE;
}

eOpcodeResult CustomOpcodes::BIT_MOD(Script *script)
{
	script->CollectParameters(2);
	game::ScriptParams[0].nVar = game::ScriptParams[0].nVar % game::ScriptParams[1].nVar;
	script->StoreParameters(1);
	return OR_CONTINUE;
}

eOpcodeResult CustomOpcodes::BIT_SHR(Script *script)
{
	script->CollectParameters(2);
	game::ScriptParams[0].nVar = game::ScriptParams[0].nVar >> game::ScriptParams[1].nVar;
	script->StoreParameters(1);
	return OR_CONTINUE;
}

eOpcodeResult CustomOpcodes::BIT_SHL(Script *script)
{
	script->CollectParameters(2);
	game::ScriptParams[0].nVar = game::ScriptParams[0].nVar << game::ScriptParams[1].nVar;
	script->StoreParameters(1);
	return OR_CONTINUE;
}

//0400=7,store_coords_to %5d% %6d% %7d% from_object %1d% with_offset %2d% %3d% %4d%
eOpcodeResult CustomOpcodes::STORE_COORDS_FROM_OBJECT_WITH_OFFSET(Script *script)
{
	script->CollectParameters(4);
	void* object = game::ObjectPoolGetAt(*game::ppObjectPool, game::ScriptParams[0].nVar);

	CVector offset;
	offset.x = game::ScriptParams[1].fVar;
	offset.y = game::ScriptParams[2].fVar;
	offset.z = game::ScriptParams[3].fVar;

	game::RwV3dTransformPoints(&offset, &offset, 1, (uintptr_t*)((uintptr_t)object + 4));

	game::ScriptParams[0].fVar = offset.x;
	game::ScriptParams[1].fVar = offset.y;
	game::ScriptParams[2].fVar = offset.z;

	script->StoreParameters(3);
	return OR_CONTINUE;
}

//0407=7,store_coords_to %5d% %6d% %7d% from_car %1d% with_offset %2d% %3d% %4d%
eOpcodeResult CustomOpcodes::STORE_COORDS_FROM_CAR_WITH_OFFSET(Script *script)
{
	script->CollectParameters(4);
	void* car = game::VehiclePoolGetAt(*game::ppVehiclePool, game::ScriptParams[0].nVar);

	CVector offset;
	offset.x = game::ScriptParams[1].fVar;
	offset.y = game::ScriptParams[2].fVar;
	offset.z = game::ScriptParams[3].fVar;

	game::RwV3dTransformPoints(&offset, &offset, 1, (uintptr_t*)((uintptr_t)car + 4));

	game::ScriptParams[0].fVar = offset.x;
	game::ScriptParams[1].fVar = offset.y;
	game::ScriptParams[2].fVar = offset.z;

	script->StoreParameters(3);
	return OR_CONTINUE;
}

//04C4=7,store_coords_to %5d% %6d% %7d% from_actor %1d% with_offset %2d% %3d% %4d%
eOpcodeResult CustomOpcodes::STORE_COORDS_FROM_ACTOR_WITH_OFFSET(Script *script)
{
	script->CollectParameters(4);
	void* actor = game::PedPoolGetAt(*game::ppPedPool, game::ScriptParams[0].nVar);

	CVector offset;
	offset.x = game::ScriptParams[1].fVar;
	offset.y = game::ScriptParams[2].fVar;
	offset.z = game::ScriptParams[3].fVar;

	game::RwV3dTransformPoints(&offset, &offset, 1, (uintptr_t*)((uintptr_t)actor + 4));

	game::ScriptParams[0].fVar = offset.x;
	game::ScriptParams[1].fVar = offset.y;
	game::ScriptParams[2].fVar = offset.z;

	script->StoreParameters(3);
	return OR_CONTINUE;
}

eOpcodeResult CustomOpcodes::STORE_PLAYER_CURRENTLY_ARMED_WEAPON(Script *script)
{
	script->CollectParameters(1);
	game::ScriptParams[0].nVar = *(DWORD *)(game::Players[79 * game::ScriptParams[0].nVar] + 24 * *(BYTE *)(game::Players[79 * game::ScriptParams[0].nVar] + 1176) + 860);
	script->StoreParameters(1);
	return OR_CONTINUE;
}

eOpcodeResult CustomOpcodes::GET_CHAR_ARMOUR(Script *script)
{
	script->CollectParameters(1);
	void* actor = game::PedPoolGetAt(*game::ppPedPool, game::ScriptParams[0].nVar);
	game::ScriptParams[0].nVar = static_cast<int>(*(float*)((uintptr_t)actor + 0x2C4));
	script->StoreParameters(1);
	return OR_CONTINUE;
}

eOpcodeResult
__stdcall IS_PLAYER_IN_FLYING_VEHICLE(Script* script)
{
		bool gta3 = game::IsIII();
		short miDodo = gta3 ? 126 : 190; // skimmer for VC
		uint CPlayerInfoSize = gta3 ? 0x13C : 0x170;
		uint offset_bInVehicle = gta3 ? 0x314 : 0x3AC; // CPed::bInVehicle
		uint offset_pMyVehicle = gta3 ? 0x310 : 0x3A8; // CPed::pMyVehicle
		uint offset_pHandling = gta3 ? 0x128 : 0x120; // CVehicle::pHandling
		uint offset_Flags = gta3 ? 0xC8 : 0xCC; // tHandlingData::Flags

		script->CollectParameters(1);

		// get CPlayerInfo::m_pPed as uchar*
		uint offset = CPlayerInfoSize * game::ScriptParams[0].nVar); // game technically supports only 1 player...
		uchar* player = reinterpret_cast<uchar*>(*(uint*)(game::Players + offset); // we use 2 casts here! read m_pPed as uint and cast it to uchar*

		/*
			Planes and helis have to be checked by handling flags, because game treats them as CAutomobile 
			instances; m_vehType = VEHICLE_TYPE_CAR.
		*/
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
__stdcall IS_PLAYER_IN_ANY_BOAT(Script* script)
{
		bool gta3 = game::IsIII();
		uint CPlayerInfoSize = gta3 ? 0x13C : 0x170;
		uint offset_bInVehicle = gta3 ? 0x314 : 0x3AC; // CPed::bInVehicle
		uint offset_pMyVehicle = gta3 ? 0x310 : 0x3A8; // CPed::pMyVehicle
		uint offset_vehType = gta3 ? 0x284 : 0x29C; // CVehicle::m_vehType

		script->CollectParameters(1);

		// get CPlayerInfo::m_pPed as uchar*
		uint offset = CPlayerInfoSize * game::ScriptParams[0].nVar); // game technically supports only 1 player...
		uchar* player = reinterpret_cast<uchar*>(*(uint*)(game::Players + offset); // we use 2 casts here! read m_pPed as uint and cast it to uchar*

		bool result = false;
		if (*(bool*)(player + offset_bInVehicle)) {
				uchar* vehicle = reinterpret_cast<uchar*>(*(uint*)(player + offset_pMyVehicle));
				result = *(int*)(vehicle + offset_vehType) == 1 ? true : false;
		}

		script->UpdateCompareFlag(result);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall IS_PLAYER_IN_ANY_HELI(Script* script)
{
		bool gta3 = game::IsIII();
		uint CPlayerInfoSize = gta3 ? 0x13C : 0x170;
		uint offset_bInVehicle = gta3 ? 0x314 : 0x3AC; // CPed::bInVehicle
		uint offset_pMyVehicle = gta3 ? 0x310 : 0x3A8; // CPed::pMyVehicle
		uint offset_pHandling = gta3 ? 0x128 : 0x120; // CVehicle::pHandling
		uint offset_Flags = gta3 ? 0xC8 : 0xCC; // tHandlingData::Flags

		script->CollectParameters(1);

		// get CPlayerInfo::m_pPed as uchar*
		uint offset = CPlayerInfoSize * game::ScriptParams[0].nVar); // game technically supports only 1 player...
		uchar* player = reinterpret_cast<uchar*>(*(uint*)(game::Players + offset); // we use 2 casts here! read m_pPed as uint and cast it to uchar*

		/*
			Planes and helis have to be checked by handling flags, because game treats them as CAutomobile 
			instances; m_vehType = VEHICLE_TYPE_CAR.
		*/
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
__stdcall IS_PLAYER_ON_ANY_BIKE(Script* script)
{
		bool gta3 = game::IsIII();
		uint CPlayerInfoSize = gta3 ? 0x13C : 0x170;
		uint offset_bInVehicle = gta3 ? 0x314 : 0x3AC; // CPed::bInVehicle
		uint offset_pMyVehicle = gta3 ? 0x310 : 0x3A8; // CPed::pMyVehicle
		uint offset_vehType = gta3 ? 0x284 : 0x29C; // CVehicle::m_vehType

		script->CollectParameters(1);

		// get CPlayerInfo::m_pPed as uchar*
		uint offset = CPlayerInfoSize * game::ScriptParams[0].nVar); // game technically supports only 1 player...
		uchar* player = reinterpret_cast<uchar*>(*(uint*)(game::Players + offset); // we use 2 casts here! read m_pPed as uint and cast it to uchar*

		bool result = false;
		if (*(bool*)(player + offset_bInVehicle)) {
				uchar* vehicle = reinterpret_cast<uchar*>(*(uint*)(player + offset_pMyVehicle));
				result = *(int*)(vehicle + offset_vehType) == 5 ? true : false;
		}

		script->UpdateCompareFlag(result);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall IS_PC_VERSION(Script* script)
{
		script->UpdateCompareFlag(true);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall IS_AUSTRALIAN_GAME(Script* script)
{
		script->UpdateCompareFlag(false);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall STREAM_CUSTOM_SCRIPT(Script* script)
{
		script->CollectParameters(1);

		fs::path filepath = fs::path(game::RootDirName) / "CLEO" / game::ScriptParams[0].szVar;

		LOGL(LOG_PRIORITY_OPCODE, "STREAM_CUSTOM_SCRIPT: Starting new script \"%s\"", filepath.c_str());
		Script* new_script = script_mgr::StartScript(filepath.c_str());

		for (int i = 0; script->GetNextParamType(); ++i) {
				script->CollectParameters(1);
				new_script->m_aLVars[i].nVar = game::ScriptParams[0].nVar;
		}
		script->m_nIp++; // consume PARAM_TYPE_END_OF_PARAMS

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall IS_BUTTON_PRESSED_WITH_SENSITIVITY(Script* script)
{
		script->CollectParameters(2);

		script->UpdateCompareFlag(*(game::pPadNewState + game::ScriptParams[0].nVar) == (short)game::ScriptParams[1].nVar);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall EMULATE_BUTTON_PRESS_WITH_SENSITIVITY(Script* script)
{
		script->CollectParameters(2);

		*(game::pPadNewState + game::ScriptParams[0].nVar) = (short)game::ScriptParams[1].nVar;

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall IS_CAMERA_IN_WIDESCREEN_MODE(Script* script)
{
		script->UpdateCompareFlag(*game::pWideScreenOn);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GET_WEAPONTYPE_MODEL(Script* script)
{
		script->CollectParameters(1);

		game::ScriptParams[0].nVar = game::ModelForWeapon(game::ScriptParams[0].nVar);
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GET_WEAPONTYPE_FOR_MODEL(Script* script)
{
		script->CollectParameters(1);
		int weaponMI = game::ScriptParams[0].nVar;

		if (weaponMI < 0) 
				weaponMI = game::UsedObjectArray[-weaponMI].index;

		// CPickups::WeaponForModel() exits only in III, so we do this manually for VC compatability
		int result = -1;
		for (size_t i = 0; i < 37; ++i) {
				if (weaponMI == game::ModelForWeapon(i))
						break;
		}

		game::ScriptParams[0].nVar = result;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall SET_MEMORY_OFFSET(Script* script)
{
		script->CollectParameters(3);

		memory::Write(game::ScriptParams[0].pVar, game::ScriptParams[1].nVar - (game::ScriptParams[0].nVar + 4));

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GET_CURRENT_WEATHER(Script* script)
{
		game::ScriptParams[0].nVar = *game::pOldWeatherType;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

//0608=3, show_text_position %1d% %2d% text %3d%
eOpcodeResult CustomOpcodes::SHOW_TEXT_POSITION(Script *script)
{
	script->CollectParameters(3);
	game::IntroTextLines[*game::pNumberOfIntroTextLinesThisFrame].x = game::ScriptParams[0].fVar;
	game::IntroTextLines[*game::pNumberOfIntroTextLinesThisFrame].y = game::ScriptParams[1].fVar;
	const char *text = game::ScriptParams[2].szVar;
	swprintf((wchar_t*)&game::IntroTextLines[*game::pNumberOfIntroTextLinesThisFrame].text, 100, L"%hs", text);
	*game::pNumberOfIntroTextLinesThisFrame = *game::pNumberOfIntroTextLinesThisFrame + 1;
	return OR_CONTINUE;
};

//0609=-1, show_formatted_text_position %1d% %2d% text %3d%
eOpcodeResult CustomOpcodes::SHOW_FORMATTED_TEXT_POSITION(Script *script)
{
	script->CollectParameters(3);
	game::IntroTextLines[*game::pNumberOfIntroTextLinesThisFrame].x = game::ScriptParams[0].fVar;
	game::IntroTextLines[*game::pNumberOfIntroTextLinesThisFrame].y = game::ScriptParams[1].fVar;
	char fmt[100]; char text[100]; static wchar_t message_buf[0x80];
	strcpy(fmt, game::ScriptParams[2].szVar);
	format(script, text, sizeof(text), fmt);
	swprintf((wchar_t*)&game::IntroTextLines[*game::pNumberOfIntroTextLinesThisFrame].text, 100, L"%hs", text);
	while ((*(ScriptParamType *)(&game::ScriptSpace[script->m_nIp])).type)
		script->CollectParameters(1);
	script->m_nIp++;
	*game::pNumberOfIntroTextLinesThisFrame = *game::pNumberOfIntroTextLinesThisFrame + 1;
	return OR_CONTINUE;
};

//0673=4,play_animation on actor %1d% animgroup %2d% anim %3d% blendfactor %4f%
eOpcodeResult WINAPI CustomOpcodes::PLAY_ANIMATION(Script *script)
{
	script->CollectParameters(4);
	void* actor = game::PedPoolGetAt(*game::ppPedPool, game::ScriptParams[0].nVar);
	game::BlendAnimation(*(DWORD *)((uintptr_t)actor + 0x4C), game::ScriptParams[1].nVar, game::ScriptParams[2].nVar, game::ScriptParams[3].fVar);
	return OR_CONTINUE;
}

eOpcodeResult CustomOpcodes::DRAW_SHADOW(Script *script)
{
	script->CollectParameters(10);
	int type = 2;	
	CVector pos;
	pos.x = game::ScriptParams[1].fVar;
	pos.y = game::ScriptParams[2].fVar;
	pos.z = game::ScriptParams[3].fVar;
	float angle = game::ScriptParams[4].fVar;
	float length = game::ScriptParams[5].fVar;

	void* pShadowTex;
	switch (game::ScriptParams[0].nVar)
	{
		case 1:
			pShadowTex = *game::ppShadowCarTex;
			type = 1;
			break;
		case 2:
			pShadowTex = *game::ppShadowPedTex;
			break;
		case 3:
			pShadowTex = *game::ppShadowExplosionTex;
			break;
		case 4:
			pShadowTex = *game::ppShadowHeliTex;
			type = 1;
			break;
		case 5:
			pShadowTex = *game::ppShadowHeadLightsTex;
			break;
		case 6:
			pShadowTex = *game::ppBloodPoolTex;
			break;
		case 7:
			pShadowTex = *game::ppShadowBikeTex;
			break;
		case 8:
			pShadowTex = *game::ppShadowBaronTex;
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

	game::StoreShadowToBeRendered(type, pShadowTex, &pos, frontX, frontY, sideX, sideY, game::ScriptParams[6].nVar, game::ScriptParams[7].nVar, game::ScriptParams[8].nVar, game::ScriptParams[9].nVar, 150.0f, true, 1.0f, nullptr, false);
	return OR_CONTINUE;
}

eOpcodeResult CustomOpcodes::SET_TEXT_DRAW_FONT(Script *script)
{
	script->CollectParameters(1);
	game::IntroTextLines[*game::pNumberOfIntroTextLinesThisFrame].fontStyle = game::ScriptParams[0].nVar;
	return OR_CONTINUE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
/*****************************************************************************************************/
////////////////////////////////CLEO4 SA opcodes///////////////////////////////////////////////////////
//0A8C=4,write_memory %1d% size %2d% value %3d% virtual_protect %4d% //dup
//0A8D=4,%4d% = read_memory %1d% size %2d% virtual_protect %3d% //dup

eOpcodeResult
__stdcall INT_ADD(Script* script)
{
		script->CollectParameters(2);

		game::ScriptParams[0].nVar = game::ScriptParams[0].nVar + game::ScriptParams[1].nVar;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall INT_SUB(Script* script)
{
		script->CollectParameters(2);

		game::ScriptParams[0].nVar = game::ScriptParams[0].nVar - game::ScriptParams[1].nVar;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall INT_MUL(Script* script)
{
		script->CollectParameters(2);

		game::ScriptParams[0].nVar = game::ScriptParams[0].nVar * game::ScriptParams[1].nVar;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall INT_DIV(Script* script)
{
		script->CollectParameters(2);

		game::ScriptParams[0].nVar = game::ScriptParams[0].nVar / game::ScriptParams[1].nVar;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall SET_CURRENT_DIRECTORY(Script* script)
{
		script->CollectParameters(1);

		fs::current_path(game::ScriptParams[0].nVar == 0 ? game::RootDirName : 
						 game::ScriptParams[0].nVar == 1 ? game::GetUserFilesFolder() : game::ScriptParams[0].szVar);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall OPEN_FILE(Script* script)
{
		script->CollectParameters(2);

		// cppreference.com/w/cpp/io/basic_filebuf/open
		std::ios::openmode openmode(0);
		for (char* str = game::ScriptParams[1].szVar; *str != '\0'; ++str) {
				if (*str == 'r') {
						openmode |= std::ios::in;
				} else if (*str == 'w') {
						openmode |= std::ios::out | std::ios::trunc;
				} else if (*str == 'a') {
						openmode |= std::ios::out | std::ios::app;
				} else if (*str == '+') {
						openmode |= std::ios::in | std::ios::out;
				} else if (*str == 'b') {
						openmode |= std::ios::binary;
				} else if (*str == 'x') {
						openmode |= std::ios::noreplace;
				}
		}

		auto file* = new std::fstream(game::ScriptParams[0].szVar, openmode);
		script->RegisterObject(file);

		script->UpdateCompareFlag(*file); // check for ill-formed fstream with bool()

		game::ScriptParams[0].pVar = file;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall CLOSE_FILE(Script* script)
{
		script->CollectParameters(1);

		script->DeleteRegisteredObject(game::ScriptParams[0].pVar);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GET_FILE_SIZE(Script* script)
{
		script->CollectParameters(1);
		auto* file = (std::fstream*)game::ScriptParams[0].pVar;
		auto saved_pos = file->tellg();

		game::ScriptParams[0].nVar = file->seekg(0, std::ios::beg).ignore(size_t(-1) >> 1).gcount();
		script->StoreParameters(1);

		file->clear();
		file->seekg(saved_pos);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall READ_FROM_FILE(Script* script)
{
		script->CollectParameters(3);
		auto* file = (std::fstream*)game::ScriptParams[0].pVar;

		file->read(game::ScriptParams[2].pVar, game::ScriptParams[1].nVar);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall WRITE_TO_FILE(Script* script)
{
		script->CollectParameters(3);
		auto* file = (std::fstream*)game::ScriptParams[0].pVar;

		file->write(game::ScriptParams[2].pVar, game::ScriptParams[1].nVar);
		file->flush();

		return OR_CONTINUE;
}

//0AA0=1,gosub_if_false %1p%
eOpcodeResult CustomOpcodes::OPCODE_0AA0(Script *script)
{
	script->CollectParameters(1);
	script->m_anGosubStack[script->m_nGosubStackPointer++] = script->m_nIp;
	if (!script->m_bCondResult)
		script->JumpTo(game::ScriptParams[0].nVar);
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
	script->CollectParameters(1);
	auto libHandle = LoadLibrary(game::ScriptParams[0].szVar);
	game::ScriptParams[0].pVar = libHandle;
	script->StoreParameters(1);
	script->UpdateCompareFlag(libHandle);
	return OR_CONTINUE;
}

//0AA3=1,free_library %1h%
eOpcodeResult CustomOpcodes::OPCODE_0AA3(Script *script)
{
	script->CollectParameters(1);
	HMODULE libHandle;
	libHandle = (HMODULE)game::ScriptParams[0].pVar;
	FreeLibrary(libHandle);
	return OR_CONTINUE;
}

//0AA4=3,%3d% = get_proc_address %1s% library %2d% ; IF and SET
eOpcodeResult CustomOpcodes::OPCODE_0AA4(Script *script)
{
	script->CollectParameters(2);
	char *funcName = game::ScriptParams[0].szVar;
	HMODULE libHandle;
	libHandle = (HMODULE)game::ScriptParams[1].pVar;
	void *funcAddr = (void *)GetProcAddress(libHandle, funcName);
	game::ScriptParams[0].pVar = funcAddr;
	script->StoreParameters(1);
	script->UpdateCompareFlag(funcAddr);
	return OR_CONTINUE;
}

//0AA5=-1,call %1d% num_params %2h% pop %3h% //dup
//0AA6=-1,call_method %1d% struct %2d% num_params %3h% pop %4h% //dup
//0AA7=-1,call_function %1d% num_params %2h% pop %3h% //dup
//0AA8=-1,call_function_method %1d% struct %2d% num_params %3h% pop %4h% //dup

eOpcodeResult
__stdcall IS_GAME_VERSION_ORIGINAL(Script* script)
{
		script->UpdateCompareFlag(game::Version == game::Release::VC_1_0 || game::Version == game::Release::III_1_0);

		return OR_CONTINUE;
}

//0AAB=1,   file_exists %1s%
eOpcodeResult CustomOpcodes::OPCODE_0AAB(Script *script)
{
	script->CollectParameters(1);
	DWORD fAttr = GetFileAttributes(game::ScriptParams[0].szVar);
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

eOpcodeResult
__stdcall SET_CLEO_SHARED_VAR(Script* script)
{
		script->CollectParameters(2);

		SharedVars[game::ScriptParams[0].nVar].nVar = game::ScriptParams[1].nVar;

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GET_CLEO_SHARED_VAR(Script* script)
{
		script->CollectParameters(1);

		game::ScriptParams[0].nVar = SharedVars[game::ScriptParams[0].nVar].nVar;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

//0AB5=3,store_actor %1d% closest_vehicle_to %2d% closest_ped_to %3d%
//0AB6=3,store_target_marker_coords_to %1d% %2d% %3d% // IF and SET //not supported

//0AB7=2,get_vehicle %1d% number_of_gears_to %2d%
eOpcodeResult __stdcall CustomOpcodes::OPCODE_0AB7(Script *script)
{
	script->CollectParameters(1);
	uintptr_t vehicle = reinterpret_cast<uintptr_t>(game::VehiclePoolGetAt(*game::ppVehiclePool, game::ScriptParams[0].nVar));
	if (vehicle)
#if CLEO_VC
		game::ScriptParams[0].nVar = *reinterpret_cast<int*>(*reinterpret_cast<uintptr_t*>(vehicle + 0x120) + 0x34 + 0x4A);
#else
		game::ScriptParams[0].nVar = *reinterpret_cast<int*>(*reinterpret_cast<uintptr_t*>(vehicle + 0x128) + 0x34 + 0x4A);
#endif
	else
		game::ScriptParams[0].nVar = 0;
	script->StoreParameters(1);
	return OR_CONTINUE;
}

//0AB8=2,get_vehicle %1d% current_gear_to %2d%
eOpcodeResult __stdcall CustomOpcodes::OPCODE_0AB8(Script *script)
{
	script->CollectParameters(1);
	uintptr_t vehicle = reinterpret_cast<uintptr_t>(game::VehiclePoolGetAt(*game::ppVehiclePool, game::ScriptParams[0].nVar));
	if (vehicle)
#if CLEO_VC
		game::ScriptParams[0].nVar = *reinterpret_cast<uint8_t*>(vehicle + 0x208);
#else
		game::ScriptParams[0].nVar = *reinterpret_cast<uint8_t*>(vehicle + 0x204);
#endif
	else 
		game::ScriptParams[0].nVar = 0;
	script->StoreParameters(1);
	return OR_CONTINUE;
}

//0AB9=2,get_mp3 %1d% state_to %2d%
//0ABA=1,end_custom_thread_named %1s% //dup
//0ABB=2,%2d% = audiostream %1d% volume
//0ABC=2,set_audiostream %1d% volume %2d%

//0ABD=1,  vehicle %1d% lights_on ( //0ABD=1,  vehicle %1d% siren_on // dup see 0383 )
eOpcodeResult __stdcall CustomOpcodes::OPCODE_0ABD(Script *script)
{
	script->CollectParameters(1);
	uintptr_t vehicle = reinterpret_cast<uintptr_t>(game::VehiclePoolGetAt(*game::ppVehiclePool, game::ScriptParams[0].nVar));
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
	script->CollectParameters(1);
	uintptr_t vehicle = reinterpret_cast<uintptr_t>(game::VehiclePoolGetAt(*game::ppVehiclePool, game::ScriptParams[0].nVar));
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
	script->CollectParameters(2);
	uintptr_t vehicle = reinterpret_cast<uintptr_t>(game::VehiclePoolGetAt(*game::ppVehiclePool, game::ScriptParams[0].nVar));
	if (vehicle)
	{
#if CLEO_VC
		reinterpret_cast<GtaGame::bVehicleFlags*>(vehicle + 0x1F9)->bEngineOn = game::ScriptParams[1].nVar != false;
#else
		reinterpret_cast<GtaGame::bVehicleFlags*>(vehicle + 0x1F5)->bEngineOn = game::ScriptParams[1].nVar != false;
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

eOpcodeResult
__stdcall ALLOCATE_MEMORY(Script* script)
{
		script->CollectParameters(1);

		auto* mem = new(game::ScriptParams[0].nVar);
		script->RegisterObject(mem);

		script->UpdateCompareFlag(true);

		game::ScriptParams[0].pVar = mem;
		script->StoreParameters(1);

		return OR_CONTINUE;
};

eOpcodeResult
__stdcall FREE_MEMORY(Script* script)
{
		script->CollectParameters(1);

		script->DeleteRegisteredObject(game::ScriptParams[0].pVar);

		return OR_CONTINUE;
};

//0ACA=1,show_text_box %1s%
eOpcodeResult CustomOpcodes::OPCODE_0ACA(Script *script)
{
	static wchar_t message_buf[HELP_MSG_LENGTH];
	script->CollectParameters(1);
	swprintf(message_buf, HELP_MSG_LENGTH, L"%hs", game::ScriptParams[0].szVar);
	game::SetHelpMessage(message_buf, false, false);
	return OR_CONTINUE;
};

//0ACB=3,show_styled_text %1s% time %2d% style %3d%
eOpcodeResult CustomOpcodes::OPCODE_0ACB(Script *script)
{
	static wchar_t message_buf[HELP_MSG_LENGTH];
	script->CollectParameters(3);
	const char *text = game::ScriptParams[0].szVar;
	swprintf(message_buf, HELP_MSG_LENGTH, L"%hs", text);
	game::AddBigMessageQ(message_buf, game::ScriptParams[1].nVar, game::ScriptParams[2].nVar - 1);
	return OR_CONTINUE;
};

//0ACC=2,show_text_lowpriority %1s% time %2d%
eOpcodeResult CustomOpcodes::OPCODE_0ACC(Script *script)
{
	static wchar_t message_buf[HELP_MSG_LENGTH];
	script->CollectParameters(2);
	const char *text = game::ScriptParams[0].szVar;
	swprintf(message_buf, HELP_MSG_LENGTH, L"%hs", text);
	game::AddMessage(message_buf, game::ScriptParams[1].nVar, false, false);
	return OR_CONTINUE;
};

//0ACD=2,show_text_highpriority %1s% time %2d%
eOpcodeResult CustomOpcodes::OPCODE_0ACD(Script *script)
{
	static wchar_t message_buf[HELP_MSG_LENGTH];
	script->CollectParameters(2);
	const char *text = game::ScriptParams[0].szVar;
	swprintf(message_buf, HELP_MSG_LENGTH, L"%hs", text);
	game::AddMessageJumpQ(message_buf, game::ScriptParams[1].nVar, false, false);
	return OR_CONTINUE;
};

//0ACE=-1,show_formatted_text_box %1s%
eOpcodeResult CustomOpcodes::OPCODE_0ACE(Script *script)
{
	static wchar_t message_buf[HELP_MSG_LENGTH];
	script->CollectParameters(1);
	char fmt[HELP_MSG_LENGTH]; char text[HELP_MSG_LENGTH];
	strcpy(fmt, game::ScriptParams[0].szVar);
	format(script, text, sizeof(text), fmt);

	swprintf(message_buf, HELP_MSG_LENGTH, L"%hs", text);
	game::SetHelpMessage(message_buf, false, false);

	while ((*(ScriptParamType *)(&game::ScriptSpace[script->m_nIp])).type)
		script->CollectParameters(1);
	script->m_nIp++;
	return OR_CONTINUE;
};

//0ACF=-1,show_formatted_styled_text %1s% time %2d% style %3d%
eOpcodeResult CustomOpcodes::OPCODE_0ACF(Script *script)
{
	script->CollectParameters(3);
	char fmt[HELP_MSG_LENGTH]; char text[HELP_MSG_LENGTH]; static wchar_t message_buf[HELP_MSG_LENGTH];
	unsigned time, style;
	strcpy(fmt, game::ScriptParams[0].szVar);
	time = game::ScriptParams[1].nVar;
	style = game::ScriptParams[2].nVar;
	format(script, text, sizeof(text), fmt);
	swprintf(message_buf, HELP_MSG_LENGTH, L"%hs", text);
	game::AddBigMessageQ(message_buf, time, style - 1);
	while ((*(ScriptParamType *)(&game::ScriptSpace[script->m_nIp])).type)
		script->CollectParameters(1);
	script->m_nIp++;
	return OR_CONTINUE;
};

//0AD0=-1,show_formatted_text_lowpriority %1s% time %2s%
eOpcodeResult CustomOpcodes::OPCODE_0AD0(Script *script)
{
	script->CollectParameters(2);
	char fmt[HELP_MSG_LENGTH]; char text[HELP_MSG_LENGTH]; static wchar_t message_buf[HELP_MSG_LENGTH];
	unsigned time;
	strcpy(fmt, game::ScriptParams[0].szVar);
	time = game::ScriptParams[1].nVar;
	format(script, text, sizeof(text), fmt);
	swprintf(message_buf, HELP_MSG_LENGTH, L"%hs", text);
	game::AddMessage(message_buf, time, false, false);
	while ((*(ScriptParamType *)(&game::ScriptSpace[script->m_nIp])).type)
		script->CollectParameters(1);
	script->m_nIp++;
	return OR_CONTINUE;
};

//0AD1=-1,show_formatted_text_highpriority %1s% time %2s%
eOpcodeResult CustomOpcodes::OPCODE_0AD1(Script *script)
{
	script->CollectParameters(2);
	char fmt[HELP_MSG_LENGTH]; char text[HELP_MSG_LENGTH]; static wchar_t message_buf[HELP_MSG_LENGTH];
	unsigned time;
	strcpy(fmt, game::ScriptParams[0].szVar);
	time = game::ScriptParams[1].nVar;
	format(script, text, sizeof(text), fmt);
	swprintf(message_buf, HELP_MSG_LENGTH, L"%hs", text);
	game::AddMessageJumpQ(message_buf, time, false, false);
	while ((*(ScriptParamType *)(&game::ScriptSpace[script->m_nIp])).type)
		script->CollectParameters(1);
	script->m_nIp++;
	return OR_CONTINUE;
};

//0AD2=2,%2d% = player %1d% targeted_actor //IF and SET

//0AD3=-1,string %1d% format %2d%
eOpcodeResult CustomOpcodes::OPCODE_0AD3(Script *script)
{
	script->CollectParameters(2);
	char fmt[HELP_MSG_LENGTH], *dst;
	dst = (char*)game::ScriptParams[0].pVar;
	strcpy(fmt, game::ScriptParams[1].szVar);
	format(script, dst, static_cast<size_t>(-1), fmt);
	while ((*(ScriptParamType *)(&game::ScriptSpace[script->m_nIp])).type)
		script->CollectParameters(1);
	script->m_nIp++;
	return OR_CONTINUE;
};

//0AD4=-1,  scan_string %1d% format %2s% store_num_results_to %3d%
eOpcodeResult CustomOpcodes::OPCODE_0AD4(Script *script)
{
	script->CollectParameters(2);
	char fmt[HELP_MSG_LENGTH], *src;
	src = game::ScriptParams[0].szVar;
	strcpy(fmt, game::ScriptParams[1].szVar);
	size_t cExParams = 0;
	int *result = (int *)script->GetPointerToScriptVariable();
	ScriptParam *ExParams[35];
	memset(ExParams, 0, 35 * sizeof(ScriptParam*));
	// read extra params
	while ((*(ScriptParamType *)(&game::ScriptSpace[script->m_nIp])).type)
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
	script->CollectParameters(3);
	std::FILE* file = (std::FILE*)game::ScriptParams[0].pVar;
	int seek = game::ScriptParams[1].nVar;
	int origin = game::ScriptParams[2].nVar;
	script->UpdateCompareFlag(fseek(file, seek, origin) == 0);
	return OR_CONTINUE;
};

//0AD6=1, end_of_file %1d% reached
eOpcodeResult CustomOpcodes::OPCODE_0AD6(Script *script)
{
	script->CollectParameters(1);
	std::FILE* file = (std::FILE*)game::ScriptParams[0].pVar;
	script->UpdateCompareFlag(feof(file) != 0);
	return OR_CONTINUE;
};

//0AD7=3,read_string_from_file %1d% to %2d% size %3d% // IF and SET
eOpcodeResult CustomOpcodes::OPCODE_0AD7(Script *script)
{
	script->CollectParameters(3);
	std::FILE* file = (std::FILE*)game::ScriptParams[0].pVar;
	char* buf = (char*)game::ScriptParams[1].pVar;
	unsigned size = game::ScriptParams[2].nVar;
	script->UpdateCompareFlag(fgets(buf, size, file) == buf);
	return OR_CONTINUE;
};

//0AD8=2,write_string_to_file %1d% from %2d% //IF and SET
eOpcodeResult CustomOpcodes::OPCODE_0AD8(Script *script)
{
	script->CollectParameters(2);
	std::FILE* file = (std::FILE*)game::ScriptParams[0].pVar;
	char* buf = (char*)game::ScriptParams[1].pVar;
	script->UpdateCompareFlag(fputs(buf, file) > 0);
	fflush(file);
	return OR_CONTINUE;
};

//0AD9=-1,write_formatted_text %2d% in_file %1d%
eOpcodeResult CustomOpcodes::OPCODE_0AD9(Script *script)
{
	script->CollectParameters(2);
	char fmt[HELP_MSG_LENGTH]; char text[HELP_MSG_LENGTH];
	std::FILE* file = (std::FILE*)game::ScriptParams[0].pVar;
	strcpy(fmt, game::ScriptParams[1].szVar);
	format(script, text, sizeof(text), fmt);
	fputs(text, file);
	fflush(file);
	while ((*(ScriptParamType *)(&game::ScriptSpace[script->m_nIp])).type)
		script->CollectParameters(1);
	script->m_nIp++;
	return OR_CONTINUE;
};

//0ADA=-1,%3d% = scan_file %1d% format %2d% //IF and SET
eOpcodeResult CustomOpcodes::OPCODE_0ADA(Script *script)
{
	script->CollectParameters(2);
	char fmt[HELP_MSG_LENGTH];
	std::FILE* file = (std::FILE*)game::ScriptParams[0].pVar;
	strcpy(fmt, game::ScriptParams[1].szVar);
	size_t cExParams = 0;
	int *result = (int *)script->GetPointerToScriptVariable();
	ScriptParam *ExParams[35];
	memset(ExParams, 0, 35 * sizeof(ScriptParam*));
	// read extra params
	while ((*(ScriptParamType *)(&game::ScriptSpace[script->m_nIp])).type)
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
	script->CollectParameters(1);
	auto modelIdx = game::ScriptParams[0].nVar;

#if CLEO_VC
	char* gxt = (char*)((game::pVehicleModelStore + 0x32) + ((modelIdx - 130) * 0x174)); // pVehicleModelStore.m_gameName + (modelIdx - MI_FIRST_VEHICLE) * sizeof(CVehicleModelInfo)
#else
	char* gxt = (char*)((game::pVehicleModelStore + 0x36) + ((modelIdx - 90) * 0x1F8)); // pVehicleModelStore.m_gameName + (modelIdx - MI_FIRST_VEHICLE) * sizeof(CVehicleModelInfo)
#endif

	auto resultType = script->GetNextParamType();
	switch (resultType)
	{
	// pointer to target buffer
	case eParamType::PARAM_TYPE_LVAR:
	case eParamType::PARAM_TYPE_GVAR:
		script->CollectParameters(1);
		strcpy(game::ScriptParams[0].szVar, gxt);
		script->UpdateCompareFlag(true);
		return OR_CONTINUE;
	}

	// unsupported result param type
	script->CollectParameters(1); // skip result param
	return OR_CONTINUE;
}

//0ADC=1,   test_cheat %1d%
eOpcodeResult CustomOpcodes::OPCODE_0ADC(Script *script)
{
	script->CollectParameters(1);

	char *c = game::KeyboardCheatString;
	char buf[30];
	strcpy(buf, game::ScriptParams[0].szVar);
	char *s = _strrev(buf);
	while (*s)
	{
		if (toupper(*s++) != *c++)
		{
			script->UpdateCompareFlag(false);
			return OR_CONTINUE;
		}
	}
	game::KeyboardCheatString[0] = 0;
	script->UpdateCompareFlag(true);

	return OR_CONTINUE;
}

//0ADD=1,spawn_car_with_model %1o% like_a_cheat
eOpcodeResult CustomOpcodes::OPCODE_0ADD(Script *script)
{
	script->CollectParameters(1);

#if CLEO_VC
	game::SpawnCar(game::ScriptParams[0].nVar);
#else
	int modelIdx = game::ScriptParams[0].nVar;
	int fun = (int)game::SpawnCar;
	const char oriModelIdx = 122; // by default function spawns tank

	// pfSpawnCar checks in models info table if model was loaded
	// calculate new address of 'model loaded' byte
	int oriAddress = *(int*)(fun + 0x33);
	int newAddrres = oriAddress + (modelIdx - oriModelIdx) * 20; // 20 bytes peer model entry

	CPatch::SetChar(fun + 0x22, modelIdx);
	CPatch::SetInt(fun + 0x33, newAddrres);
	CPatch::SetChar(fun + 0xA5, modelIdx);

	game::SpawnCar(); // TODO: fix crash when model index is >= 128

	CPatch::SetChar(fun + 0x22, oriModelIdx);
	CPatch::SetInt(fun + 0x33, oriAddress);
	CPatch::SetChar(fun + 0xA5, oriModelIdx);
#endif
	return OR_CONTINUE;
}

//0ADE=2,%2d% = text_by_GXT_entry %1d%
eOpcodeResult CustomOpcodes::OPCODE_0ADE(Script *script)
{
	script->CollectParameters(2);
	char *gxt = game::ScriptParams[0].szVar;
	char *result = game::ScriptParams[1].szVar;
	wchar_t *text = CustomText::GetText(game::TheText, 0, gxt);
	wcstombs(result, text, wcslen(text));
	result[wcslen(text)] = '\0';
	return OR_CONTINUE;
}

eOpcodeResult
__stdcall ADD_TEXT_LABEL(Script* script)
{
		script->CollectParameters(2);

		fxt::Add(game::ScriptParams[0].szVar, game::ScriptParams[1].szVar);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall REMOVE_TEXT_LABEL(Script* script)
{
		script->CollectParameters(1);

		fxt::Remove(game::ScriptParams[0].szVar);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall DOES_DIRECTORY_EXIST(Script* script)
{
		script->CollectParameters(1);

		script->UpdateCompareFlag(fs::is_directory(game::ScriptParams[0].szVar));

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall CREATE_DIRECTORY(Script* script)
{
		script->CollectParameters(1);

		script->UpdateCompareFlag(fs::create_directories(game::ScriptParams[0].szVar));

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall FIND_FIRST_FILE(Script* script)
{
		script->CollectParameters(1);

		auto* handle = new fs::directory_iterator(game::ScriptParams[0].szVar);
		script->RegisterObject(handle);

		script->UpdateCompareFlag(*handle != end(*handle)); // check if directory is not empty

		game::ScriptParams[0].pVar = handle;
		script->StoreParameters(1);

		script->CollectParameters(1);
		std::strcpy(game::ScriptParams[0].szVar, (*handle)->path().filename().c_str());

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall FIND_NEXT_FILE(Script* script)
{
		script->CollectParameters(2);
		auto* handle = (fs::directory_iterator*)game::ScriptParams[0].pVar;

		(*handle)++;
		script->UpdateCompareFlag(*handle != end(*handle));

		std::strcpy(game::ScriptParams[1].szVar, (*handle)->path().filename().c_str());

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall FIND_CLOSE(Script* script)
{
		script->CollectParameters(1);

		script->DeleteRegisteredObject(game::ScriptParams[0].pVar);

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
	script->CollectParameters(2);
	script->CLEO_array_[game::ScriptParams[0].nVar].nVar = game::ScriptParams[1].nVar;
	return OR_CONTINUE;
}

//0AF9=2,%2d% = cleo_array %1d%
eOpcodeResult __stdcall CustomOpcodes::GET_CLEO_ARRAY(Script *script)
{
	script->CollectParameters(1);
	game::ScriptParams[0].nVar = script->CLEO_array_[game::ScriptParams[0].nVar].nVar;
	script->StoreParameters(1);
	return OR_CONTINUE;
}

//0AFA=2,%2d% = cleo_array %1d% pointer
eOpcodeResult __stdcall CustomOpcodes::GET_CLEO_ARRAY_OFFSET(Script *script)
{
	script->CollectParameters(1);
	game::ScriptParams[0].pVar = &script->CLEO_array_[game::ScriptParams[0].nVar].nVar;
	script->StoreParameters(1);
	return OR_CONTINUE;
}

//0AFB=3,%3d% = script %1d% cleo_array %2d% pointer
eOpcodeResult __stdcall CustomOpcodes::GET_CLEO_ARRAY_SCRIPT(Script *script)
{
	script->CollectParameters(2);
	Script *pScript = reinterpret_cast<Script*>(game::ScriptParams[0].pVar);
	if (pScript)
	{
		game::ScriptParams[0].pVar = &pScript->CLEO_array_[game::ScriptParams[1].nVar].nVar;
	}
	script->StoreParameters(1);
	return OR_CONTINUE;
}

eOpcodeResult
__stdcall GET_PLATFORM(Script* script)
{
		game::ScriptParams[0].nVar = game::Platform::Windows;
		script->StoreParameters(1);

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
					script->CollectParameters(1);
					_itoa(game::ScriptParams[0].nVar, buffiter, 10);
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
					script->CollectParameters(1);
					_itoa(game::ScriptParams[0].nVar, buffiter, 10);
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
				script->CollectParameters(1);
				static const char none[] = "(null)";
				const char *astr = game::ScriptParams[0].szVar;
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
				script->CollectParameters(1);
				*str++ = (char)game::ScriptParams[0].nVar;
				iter++;
				break;

			default:
			{
				/* For non wc types, use system sprintf and append to wide char output */
				/* FIXME: for unrecognised types, should ignore % when printing */
				char *bufaiter = bufa;
				if (*iter == 'p' || *iter == 'P')
				{
					script->CollectParameters(1);
					sprintf(bufaiter, "%08X", game::ScriptParams[0].nVar);
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
						script->CollectParameters(1);
						sprintf(bufaiter, fmtbufa, game::ScriptParams[0].fVar);
					}
					else
					{
						script->CollectParameters(1);
						sprintf(bufaiter, fmtbufa, game::ScriptParams[0].pVar);
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

opcodes::Definition* g_opcode_defs[opcodes::MAX_ID] = []() {
		std::memset(&g_opcode_defs, 0, sizeof(g_opcode_defs));

		opcodes::Register(0x0002, GOTO);
		opcodes::Register(0x004C, GOTO_IF_TRUE);
		opcodes::Register(0x004D, GOTO_IF_FALSE);
		opcodes::Register(0x0050, GOSUB);
		opcodes::Register(0x05DC, TERMINATE_THIS_CUSTOM_SCRIPT);
		opcodes::Register(0x05DD, TERMINATE_ALL_CUSTOM_SCRIPTS_WITH_THIS_NAME);
		opcodes::Register(0x05DE, START_CUSTOM_SCRIPT);
		opcodes::Register(0x05DF, WRITE_MEMORY);
		opcodes::Register(0x05E0, READ_MEMORY);
		opcodes::Register(0x05E1, CALL);
		opcodes::Register(0x05E2, CALL_FUNCTION);
		opcodes::Register(0x05E3, CALL_METHOD);
		opcodes::Register(0x05E4, CALL_FUNCTION_METHOD);
		opcodes::Register(0x05E5, GET_GAME_VERSION);
		opcodes::Register(0x05E6, GET_PED_POINTER);
		opcodes::Register(0x05E7, GET_VEHICLE_POINTER);
		opcodes::Register(0x05E8, GET_OBJECT_POINTER);
		opcodes::Register(0x05E9, GET_PED_REF);
		opcodes::Register(0x05EA, GET_VEHICLE_REF);
		opcodes::Register(0x05EB, GET_OBJECT_REF);
		opcodes::Register(0x05EC, GET_THIS_SCRIPT_STRUCT);
		opcodes::Register(0x05ED, GET_SCRIPT_STRUCT_NAMED);
		opcodes::Register(0x05EE, IS_KEY_PRESSED);
		opcodes::Register(0x05EF, GET_RANDOM_CHAR_IN_SPHERE_NO_SAVE_RECURSIVE);
		opcodes::Register(0x05F0, GET_RANDOM_CAR_IN_SPHERE_NO_SAVE_RECURSIVE);
		opcodes::Register(0x05F1, GET_RANDOM_OBJECT_IN_SPHERE_NO_SAVE_RECURSIVE);
		opcodes::Register(0x05F2, CALL_POP_FLOAT);
		opcodes::Register(0x05F3, MATH_EXP);
		opcodes::Register(0x05F4, MATH_LOG);
		opcodes::Register(0x05F5, CLEO_CALL);
		opcodes::Register(0x05F6, CLEO_RETURN);
		opcodes::Register(0x05F7, GET_LABEL_POINTER);
		opcodes::Register(0x05F8, GET_VAR_POINTER);
		opcodes::Register(0x05F9, BIT_AND);
		opcodes::Register(0x05FA, BIT_OR);
		opcodes::Register(0x05FB, BIT_XOR);
		opcodes::Register(0x05FC, BIT_NOT);
		opcodes::Register(0x05FD, BIT_MOD);
		opcodes::Register(0x05FE, BIT_SHR);
		opcodes::Register(0x05FF, BIT_SHL);

		//CLEO4 SA opcodes including duplicates with new ids
		opcodes::Register(0x0A8C, WRITE_MEMORY);
		opcodes::Register(0x0A8D, READ_MEMORY);
		opcodes::Register(0x0A8E, INT_ADD);
		opcodes::Register(0x0A8F, INT_SUB);
		opcodes::Register(0x0A90, INT_MUL);
		opcodes::Register(0x0A91, INT_DIV);
		opcodes::Register(0x0A92, STREAM_CUSTOM_SCRIPT);
		opcodes::Register(0x0A93, TERMINATE_THIS_CUSTOM_SCRIPT);
		opcodes::Register(0x0A94, DUMMY);
		opcodes::Register(0x0A95, DUMMY);
		opcodes::Register(0x0A96, GET_PED_POINTER);
		opcodes::Register(0x0A97, GET_VEHICLE_POINTER);
		opcodes::Register(0x0A98, GET_OBJECT_POINTER);
		opcodes::Register(0x0A99, SET_CURRENT_DIRECTORY);
		opcodes::Register(0x0A9A, OPEN_FILE);
		opcodes::Register(0x0A9B, CLOSE_FILE);
		opcodes::Register(0x0A9C, GET_FILE_SIZE);
		opcodes::Register(0x0A9D, READ_FROM_FILE);
		opcodes::Register(0x0A9E, WRITE_TO_FILE);
		opcodes::Register(0x0A9F, GET_THIS_SCRIPT_STRUCT);
		opcodes::Register(0x0AA0, OPCODE_0AA0);
		opcodes::Register(0x0AA1, OPCODE_0AA1);
		opcodes::Register(0x0AA2, OPCODE_0AA2);
		opcodes::Register(0x0AA3, OPCODE_0AA3);
		opcodes::Register(0x0AA4, OPCODE_0AA4);
		opcodes::Register(0x0AA5, CALL);
		opcodes::Register(0x0AA6, CALL_METHOD);
		opcodes::Register(0x0AA7, CALL_FUNCTION);
		opcodes::Register(0x0AA8, CALL_FUNCTION_METHOD);
		opcodes::Register(0x0AA9, IS_GAME_VERSION_ORIGINAL);
		opcodes::Register(0x0AAA, GET_SCRIPT_STRUCT_NAMED);
		opcodes::Register(0x0AAB, OPCODE_0AAB);
		opcodes::Register(0x0AAC, DUMMY);
		opcodes::Register(0x0AAD, DUMMY);
		opcodes::Register(0x0AAE, DUMMY);
		opcodes::Register(0x0AAF, DUMMY);
		opcodes::Register(0x0AB0, IS_KEY_PRESSED);
		opcodes::Register(0x0AB1, CLEO_CALL);
		opcodes::Register(0x0AB2, CLEO_RETURN);
		opcodes::Register(0x0AB3, SET_CLEO_SHARED_VAR);
		opcodes::Register(0x0AB4, GET_CLEO_SHARED_VAR);
		opcodes::Register(0x0AB5, DUMMY);
		opcodes::Register(0x0AB6, DUMMY);
		opcodes::Register(0x0AB7, OPCODE_0AB7);
		opcodes::Register(0x0AB8, OPCODE_0AB8);
		opcodes::Register(0x0AB9, DUMMY);
		opcodes::Register(0x0ABA, TERMINATE_ALL_CUSTOM_SCRIPTS_WITH_THIS_NAME);
		opcodes::Register(0x0ABB, DUMMY);
		opcodes::Register(0x0ABC, DUMMY);
		opcodes::Register(0x0ABD, OPCODE_0ABD);
		opcodes::Register(0x0ABE, OPCODE_0ABE);
		opcodes::Register(0x0ABF, OPCODE_0ABF);
		opcodes::Register(0x0AC0, DUMMY);
		opcodes::Register(0x0AC1, DUMMY);
		opcodes::Register(0x0AC2, DUMMY);
		opcodes::Register(0x0AC3, DUMMY);
		opcodes::Register(0x0AC4, DUMMY);
		opcodes::Register(0x0AC5, DUMMY);
		opcodes::Register(0x0AC6, GET_LABEL_POINTER);
		opcodes::Register(0x0AC7, GET_VAR_POINTER);
		opcodes::Register(0x0AC8, ALLOCATE_MEMORY);
		opcodes::Register(0x0AC9, FREE_MEMORY);
		opcodes::Register(0x0ACA, OPCODE_0ACA);
		opcodes::Register(0x0ACB, OPCODE_0ACB);
		opcodes::Register(0x0ACC, OPCODE_0ACC);
		opcodes::Register(0x0ACD, OPCODE_0ACD);
		opcodes::Register(0x0ACE, OPCODE_0ACE);
		opcodes::Register(0x0ACF, OPCODE_0ACF);
		opcodes::Register(0x0AD0, OPCODE_0AD0);
		opcodes::Register(0x0AD1, OPCODE_0AD1);
		opcodes::Register(0x0AD2, DUMMY);
		opcodes::Register(0x0AD3, OPCODE_0AD3);
		opcodes::Register(0x0AD4, OPCODE_0AD4);
		opcodes::Register(0x0AD5, OPCODE_0AD5);
		opcodes::Register(0x0AD6, OPCODE_0AD6);
		opcodes::Register(0x0AD7, OPCODE_0AD7);
		opcodes::Register(0x0AD8, OPCODE_0AD8);
		opcodes::Register(0x0AD9, OPCODE_0AD9);
		opcodes::Register(0x0ADA, OPCODE_0ADA);
		opcodes::Register(0x0ADB, OPCODE_0ADB);
		opcodes::Register(0x0ADC, OPCODE_0ADC);
		opcodes::Register(0x0ADD, OPCODE_0ADD);
		opcodes::Register(0x0ADE, OPCODE_0ADE);
		opcodes::Register(0x0ADF, ADD_TEXT_LABEL);
		opcodes::Register(0x0AE0, REMOVE_TEXT_LABEL);
		opcodes::Register(0x0AE1, GET_RANDOM_CHAR_IN_SPHERE_NO_SAVE_RECURSIVE);
		opcodes::Register(0x0AE2, GET_RANDOM_CAR_IN_SPHERE_NO_SAVE_RECURSIVE);
		opcodes::Register(0x0AE3, GET_RANDOM_OBJECT_IN_SPHERE_NO_SAVE_RECURSIVE);
		opcodes::Register(0x0AE4, DOES_DIRECTORY_EXIST);
		opcodes::Register(0x0AE5, CREATE_DIRECTORY);
		opcodes::Register(0x0AE6, FIND_FIRST_FILE);
		opcodes::Register(0x0AE7, FIND_NEXT_FILE);
		opcodes::Register(0x0AE8, FIND_CLOSE);
		opcodes::Register(0x0AE9, CALL_POP_FLOAT);
		opcodes::Register(0x0AEA, GET_PED_REF);
		opcodes::Register(0x0AEB, GET_VEHICLE_REF);
		opcodes::Register(0x0AEC, GET_OBJECT_REF);
		opcodes::Register(0x0AED, DUMMY);
		opcodes::Register(0x0AEE, MATH_EXP);
		opcodes::Register(0x0AEF, MATH_LOG);

		//CLEO 2 opcodes
		opcodes::Register(0x0600, STREAM_CUSTOM_SCRIPT);
		opcodes::Register(0x0601, IS_BUTTON_PRESSED_WITH_SENSITIVITY);
		opcodes::Register(0x0602, EMULATE_BUTTON_PRESS_WITH_SENSITIVITY);
		opcodes::Register(0x0603, IS_CAMERA_IN_WIDESCREEN_MODE);
		opcodes::Register(0x0604, GET_WEAPONTYPE_MODEL);
		opcodes::Register(0x0605, GET_WEAPONTYPE_FOR_MODEL);
		opcodes::Register(0x0606, SET_MEMORY_OFFSET);
		opcodes::Register(0x0607, GET_CURRENT_WEATHER);
		opcodes::Register(0x0608, SHOW_TEXT_POSITION);
		opcodes::Register(0x0609, SHOW_FORMATTED_TEXT_POSITION);
		opcodes::Register(0x0673, PLAY_ANIMATION);

#if CLEO_VC
		//Scrapped opcodes (VC)
		opcodes::Register(0x016F, DRAW_SHADOW);
		opcodes::Register(0x0349, SET_TEXT_DRAW_FONT);
#else
		//Original opcodes added since VC
		opcodes::Register(0x04C2, STORE_COORDS_FROM_OBJECT_WITH_OFFSET); //0400
		opcodes::Register(0x04C3, STORE_COORDS_FROM_CAR_WITH_OFFSET); //0407
		opcodes::Register(0x04C4, STORE_COORDS_FROM_ACTOR_WITH_OFFSET);

		opcodes::Register(0x046F, STORE_PLAYER_CURRENTLY_ARMED_WEAPON);
		opcodes::Register(0x04DD, GET_CHAR_ARMOUR);

		opcodes::Register(0x04C9, IS_PLAYER_IN_FLYING_VEHICLE);
		opcodes::Register(0x04A8, IS_PLAYER_IN_ANY_BOAT);
		opcodes::Register(0x04AA, IS_PLAYER_IN_ANY_HELI);
		opcodes::Register(0x047E, IS_PLAYER_ON_ANY_BIKE);
		opcodes::Register(0x0485, IS_PC_VERSION);
		opcodes::Register(0x059A, IS_AUSTRALIAN_GAME);
#endif

		// CLEO 2.1 opcodes
		opcodes::Register(0x0AF8, SET_CLEO_ARRAY);
		opcodes::Register(0x0AF9, GET_CLEO_ARRAY);
		opcodes::Register(0x0AFA, GET_CLEO_ARRAY_OFFSET);
		opcodes::Register(0x0AFB, GET_CLEO_ARRAY_SCRIPT);
		opcodes::Register(0x0DD5, GET_PLATFORM);
}();

bool
opcodes::Register(ushort id, Definition* def)
{
		if (id >= MAX_ID) {
				LOGL(LOG_PRIORITY_REGISTER_OPCODE, "opcodes::Register: ID is out of range (%04X > %04X)", id, MAX_ID - 1);
				return false;
		}

		if (g_opcode_defs[id]) {
				// we don't return false here to allow opcode overloading
				LOGL(LOG_PRIORITY_REGISTER_OPCODE, "opcodes::Register: %04X was already registered", id);
		}

		g_opcode_defs[id] = def;
		return true;
}

opcodes::Definition*
opcodes::Definition(ushort id)
{
		return g_opcode_defs[id];
}
