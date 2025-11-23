#pragma once

// Script parameters have their data types compiled as a byte prefix...
struct tParamType
{
        enum eParamType : char {
                PARAM_TYPE_END_OF_PARAMS = 0,
                PARAM_TYPE_INT32 = 1,
                PARAM_TYPE_GVAR = 2,
                PARAM_TYPE_LVAR = 3,
                PARAM_TYPE_INT8 = 4,
                PARAM_TYPE_INT16 = 5,
                PARAM_TYPE_FLOAT = 6,
                PARAM_TYPE_STRING = 14
        } type : 7;
        bool processed : 1; // strings are compiled as (size byte + char arr); we convert them to c-string at runtime
};

// ...but ScriptParams array, the virtual call stack, is type-agnostic; so we'll treat it as union
union tScriptVar
{
        int nVar;
        float fVar;
        char* szVar;
        void* pVar;
};

/*
    Script is being read while it's opcodes return OR_CONTINUE; like
            while (!ProcessOneCommand())
                    ;
            return;

    Opcodes that yield script's execution will return OR_TERMINATE.
    Some opcodes return OR_UNDEFINED in unexpected cases, but this value is never processed by game (cf. snippet above).
*/
enum eOpcodeResult : char
{
        OR_CONTINUE = 0,
        OR_TERMINATE = 1,
        OR_UNDEFINED = -1
};
