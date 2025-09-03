#include "Opcodes.h"

opcodes::Definition* g_opcode_defs[opcodes::MAX_ID];

opcodes::Definition*
opcodes::definition(ushort id)
{
		return g_opcode_defs[id];
}

bool
opcodes::reg(ushort id, Definition* def)
{
		if (id >= MAX_ID) {
				return false;
		} else {
				// opcode can already have registered definition, but we'll allow overloading
				g_opcode_defs[id] = def;
				return true;
		}
}