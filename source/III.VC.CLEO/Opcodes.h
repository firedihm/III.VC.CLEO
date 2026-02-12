#pragma once

constexpr ushort CUSTOM_OPCODE_START_ID = 0x05DC;
constexpr ushort MAX_NUMBER_OF_OPCODES = 0x8000;

class Script;

namespace opcodes
{
        extern eOpcodeResult (__stdcall* Functions[MAX_NUMBER_OF_OPCODES])(Script*);

        __declspec(dllexport) bool Register(ushort id, eOpcodeResult (__stdcall*)(Script*) function);
}
