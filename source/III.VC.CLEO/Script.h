#pragma once

#include "domain.h"

#ifndef CLEOAPI
#define CLEOAPI __declspec(dllexport)
#endif

#define KEY_LENGTH_IN_SCRIPT 8
#define MAX_STACK_DEPTH 6
#define NUM_LOCAL_VARS 16
#define NUM_TIMERS 2

class CRunningScript
{
protected:
		/* 0x00 */ CRunningScript* m_pNext;
		/* 0x04 */ CRunningScript* m_pPrev;
		/* 0x08 */ char m_acName[KEY_LENGTH_IN_SCRIPT];
		/* 0x10 */ uint m_dwIp;
		/* 0x14 */ uint m_aGosubAddr[MAX_STACK_DEPTH];
		/* 0x2C */ ushort m_nCurrentGosub;
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
		/* 0x7C */ uint m_dwWakeTime;
		/* 0x80 */ ushort m_wIfOp;
		/* 0x82 */ bool m_bNotFlag;
		/* 0x83 */ bool m_bDeathArrestCheckEnabled;
		/* 0x84 */ bool m_bWastedOrBusted;
		/* 0x85 */ bool m_bMissionFlag;

		CRunningScript() = default;
};

class CCustomScript : protected CRunningScript
{
protected:
		struct StackFrame {
				StackFrame* prev;
				tScriptVar vars[NUM_LOCAL_VARS];
				uint retAddr;
		};

		CCustomScript* m_pNextCustom;
		CCustomScript* m_pPrevCustom;
		uchar* m_pCodeData;
		uint m_dwBaseIp;
		bool m_bIsCustom;
		bool m_bIsPersistent;
		uint m_nLastPedSearchIndex;
		uint m_nLastVehicleSearchIndex;
		uint m_nLastObjectSearchIndex;
		StackFrame* m_pCleoCallStack;
		tScriptVar* m_pLocalArray;

		CCustomScript() = default;
};

class Script : protected CCustomScript
{
public:
		Script();
		Script(const char* filepath);
		~Script();

		void Init();

		void AddToCustomList(Script** list);
		void RemoveFromCustomList(Script** list);

		void PushStackFrame();
		void PopStackFrame();

		eOpcodeResult ProcessOneCommand();

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
};

static_assert(sizeof(CRunningScript) == 0x88, "CRunningScript size mismatch");
static_assert(offsetof(CRunningScript, m_aTimers) == offsetof(CRunningScript, m_aLVars) + sizeof(CRunningScript::m_aLVars), "CRunningScript::m_aTimers must follow Script::m_aLVars")
