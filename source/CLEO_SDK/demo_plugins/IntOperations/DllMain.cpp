#pragma comment (lib, "CLEO.lib")
#include "CLEO.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

ScriptParam* ScriptParams = game::script_params();

eOpcodeResult
__stdcall BIT_AND(Script* script)
{
		script->CollectParameters(2);

		ScriptParams[0].nVar = ScriptParams[0].nVar & ScriptParams[1].nVar;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall BIT_OR(Script* script)
{
		script->CollectParameters(2);

		ScriptParams[0].nVar = ScriptParams[0].nVar | ScriptParams[1].nVar;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall BIT_XOR(Script* script)
{
		script->CollectParameters(2);

		ScriptParams[0].nVar = ScriptParams[0].nVar ^ ScriptParams[1].nVar;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall BIT_NOT(Script* script)
{
		script->CollectParameters(1);

		ScriptParams[0].nVar = ~ScriptParams[0].nVar;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall MOD(Script* script)
{
		script->CollectParameters(2);

		ScriptParams[0].nVar = ScriptParams[0].nVar % ScriptParams[1].nVar;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall BIT_SHR(Script* script)
{
		script->CollectParameters(2);

		ScriptParams[0].nVar = ScriptParams[0].nVar >> ScriptParams[1].nVar;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall BIT_SHL(Script* script)
{
		script->CollectParameters(2);

		ScriptParams[0].nVar = ScriptParams[0].nVar << ScriptParams[1].nVar;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

/****************************************************************
Now do them as real operators...
*****************************************************************/

eOpcodeResult
__stdcall BIT_AND_COMPOUND(Script* script)
{
		ScriptParam* param = script->GetPointerToScriptVariable();
		script->CollectParameters(1);

		param->nVar &= ScriptParams[0].nVar;

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall BIT_OR_COMPOUND(Script* script)
{
		ScriptParam* param = script->GetPointerToScriptVariable();
		script->CollectParameters(1);

		param->nVar |= ScriptParams[0].nVar;

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall BIT_XOR_COMPOUND(Script* script)
{
		ScriptParam* param = script->GetPointerToScriptVariable();
		script->CollectParameters(1);

		param->nVar ^= ScriptParams[0].nVar;

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall BIT_NOT_COMPOUND(Script* script)
{
		ScriptParam* param = script->GetPointerToScriptVariable();

		param->nVar = ~param->nVar;

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall MOD_COMPOUND(Script* script)
{
		ScriptParam* param = script->GetPointerToScriptVariable();
		script->CollectParameters(1);

		param->nVar %= ScriptParams[0].nVar;

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall BIT_SHR_COMPOUND(Script* script)
{
		ScriptParam* param = script->GetPointerToScriptVariable();
		script->CollectParameters(1);

		param->nVar >>= ScriptParams[0].nVar;

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall BIT_SHL_COMPOUND(Script* script)
{
		ScriptParam* param = script->GetPointerToScriptVariable();
		script->CollectParameters(1);

		param->nVar <<= ScriptParams[0].nVar;

		return OR_CONTINUE;
}

BOOL
APIENTRY DllMain(HMODULE hModule, DWORD reason_for_call, LPVOID reserved)
{
		if (reason_for_call == DLL_PROCESS_ATTACH) {
				if (cleo::version() < cleo::version(2, 1, 0)) {
						::MessageBox(HWND_DESKTOP, "An incorrect version of CLEO was loaded.", "IntOperations.cleo", MB_ICONERROR);
						return FALSE;
				}

				opcodes::register(0x0B10, BIT_AND);
				opcodes::register(0x0B11, BIT_OR);
				opcodes::register(0x0B12, BIT_XOR);
				opcodes::register(0x0B13, BIT_NOT);
				opcodes::register(0x0B14, MOD);
				opcodes::register(0x0B15, BIT_SHR);
				opcodes::register(0x0B16, BIT_SHL);
				opcodes::register(0x0B17, BIT_AND_COMPOUND);
				opcodes::register(0x0B18, BIT_OR_COMPOUND);
				opcodes::register(0x0B19, BIT_XOR_COMPOUND);
				opcodes::register(0x0B1A, BIT_NOT_COMPOUND);
				opcodes::register(0x0B1B, MOD_COMPOUND);
				opcodes::register(0x0B1C, BIT_SHR_COMPOUND);
				opcodes::register(0x0B1D, BIT_SHL_COMPOUND);
		}

		return TRUE;
}
