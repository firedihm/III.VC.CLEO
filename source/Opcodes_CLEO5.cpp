#include "Game.h"
#include "Opcodes.h"
#include "Script.h"

eOpcodeResult __stdcall
GET_PLATFORM(Script* script)
{
		game::ScriptParams[0].nVar = (int)game::Platform::Windows;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
GET_CLEO_ARG_COUNT(Script* script)
{
		auto* callee = script->call_stack_;

		game::ScriptParams[0].nVar = callee ? callee->argc : 0;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
HAS_GAME_LOADED(Script* script)
{
		script->UpdateCompareFlag(!(*game::pGameNotLoaded));

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
CLEO_RETURN_WITH(Script* script)
{
		script->CollectParameters(2);
		bool conditional_result = game::ScriptParams[0].nVar;
		int argc = game::ScriptParams[1].nVar;

		script->UpdateCompareFlag(conditional_result);

		// collect callee's retvals; ScriptParam::Type::EOP isn't consumed here, but it's no problem...
		script->CollectParameters(argc);

		// ...since we return to caller anyway
		script->pop_call_frame();

		// continue reading indexes of caller's local_vars_ to store callee's retvals
		script->StoreParameters(argc);

		// consume ScriptParam::Type::EOP
		script->ip_++;

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
CLEO_RETURN_FAIL(Script* script)
{
		script->UpdateCompareFlag(false);
		
		script->pop_call_frame();

		// we failed and have nothing to return: skip indexes of caller's local_vars_
		script->CollectParameters(-1);

		return OR_CONTINUE;
}

void
opcodes::reg_CLEO5()
{
		reg(0x0DD5, &GET_PLATFORM);
		reg(0x2000, &GET_CLEO_ARG_COUNT);
		reg(0x2001, &HAS_GAME_LOADED);
		reg(0x2002, &CLEO_RETURN_WITH);
		reg(0x2003, &CLEO_RETURN_FAIL);
}
