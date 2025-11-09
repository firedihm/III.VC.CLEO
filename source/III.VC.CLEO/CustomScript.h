#pragma once

#ifndef CLEOAPI
#define CLEOAPI __declspec(dllexport)
#endif

#define KEY_LENGTH_IN_SCRIPT 8
#define MAX_STACK_DEPTH 6
#define NUM_LOCAL_VARS 16
#define NUM_TIMERS 2

enum eOpcodeResult : char
{
		OR_CONTINUE = 0,
		OR_TERMINATE = 1,
		OR_UNDEFINED = -1
};

union tScriptVar
{
		int nVar;
		float fVar;
		char* cVar;
		void* pVar;
		ulong dVar; // DWORD
};

enum eParamType : uchar
{
		PARAM_TYPE_END_OF_PARAMS = 0,
		PARAM_TYPE_INT32 = 1,
		PARAM_TYPE_GVAR = 2,
		PARAM_TYPE_LVAR = 3,
		PARAM_TYPE_INT8 = 4,
		PARAM_TYPE_INT16 = 5,
		PARAM_TYPE_FLOAT = 6,
		PARAM_TYPE_STRING = 14
};

struct tParamType
{
		eParamType type : 7;
		bool processed : 1; // strings are compiled as (size byte + char arr); we convert them to c-string at runtime
};

class CScript
{
	public:
		/* 0x00 */ CScript* m_pNext;
		/* 0x04 */ CScript* m_pPrev;
		/* 0x08 */ char m_acName[KEY_LENGTH_IN_SCRIPT];
		/* 0x10 */ uint m_dwIp;
		/* 0x14 */ uint m_aGosubAddr[MAX_STACK_DEPTH];
		/* 0x2C */ ushort m_nCurrentGosub;
		/* 0x2E */ bool m_bIsCustom;
		/* 0x2F */ char _pad;
		/* 0x30 */ tScriptVar m_aLVars[NUM_LOCAL_VARS];
		/* 0x70 */ tScriptVar m_aTimers[NUM_TIMERS];
	#if CLEO_VC
		/* 0x78 */ bool m_bIsActive; 
		/* 0x79 */ bool m_bCondResult; 
		/* 0x7A */ bool m_bIsMission;
	#else
		/* 0x78 */ bool m_bCondResult;
		/* 0x79 */ bool m_bIsMission;
		/* 0x7A */ bool m_bIsActive;
	#endif
		/* 0x7B */ bool m_bAwake;
		/* 0x7C */ uint m_dwWakeTime;
		/* 0x80 */ ushort m_wIfOp;
		/* 0x82 */ bool m_bNotFlag;
		/* 0x83 */ bool m_bDeathArrestCheckEnabled;
		/* 0x84 */ bool m_bWastedOrBusted;
		/* 0x85 */ bool m_bMissionFlag;
		/* 0x86 */ bool m_bIsPersistent;
		/* 0x87 */ bool m_bToBeReloaded;

		/* Custom data */
		/* 0x88 */ uint m_nLastPedSearchIndex;
		/* 0x8C */ uint m_nLastVehicleSearchIndex;
		/* 0x90 */ uint m_nLastObjectSearchIndex;
		/* 0x94 */ uint _padd;
		/* 0x98 */ char* m_pCodeData;
		/* 0x9C */ uint m_dwBaseIp;
		/* 0xA0 */ StackFrame* m_pCleoCallStack;
		/* 0xA4 */ CScript* m_pNextCustom;
		/* 0xA8 */ CScript* m_pPrevCustom;
		/* 0xAC */ tScriptVar* m_pLocalArray;

		CScript(const char* filepath);
		~CScript();

		void AddToCustomList(CScript** list);
		void RemoveFromCustomList(CScript** list);
		eOpcodeResult ProcessOneCommand();

		// called by 0AB1: CLEO_CALL and 0AB2: CLEO_RETURN respectively
		void PushStackFrame();
		void PopStackFrame();

		// exports
		CLEOAPI eParamType GetNextParamType();
		CLEOAPI void* GetPointerToScriptVariable();
		CLEOAPI void UpdateCompareFlag(bool result);
		CLEOAPI void ReadShortString(char* out);
		CLEOAPI void JumpTo(int address);

		CLEOAPI void Collect(short numParams);
		CLEOAPI void CollectParameters(uint* pIp, short numParams);
		CLEOAPI int CollectNextParameterWithoutIncreasingPC(uint ip);
		CLEOAPI void Store(short numParams);

	private:
		struct StackFrame {
				StackFrame* prev;
				tScriptVar vars[NUM_LOCAL_VARS];
				int retAddr;
		};
};

static_assert(offsetof(CScript, m_aTimers) == offsetof(CScript, m_aLVars) + sizeof(CScript::m_aLVars), "CScript::m_aTimers must follow CScript::m_aLVars")
static_assert(sizeof(CScript) == 0xB0, "CScript size mismatch");
