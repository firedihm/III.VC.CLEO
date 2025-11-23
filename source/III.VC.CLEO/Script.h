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
		CRunningScript* m_pNext;
		CRunningScript* m_pPrev;
		char m_acName[KEY_LENGTH_IN_SCRIPT];
		uint m_dwIp;
		uint m_aGosubAddr[MAX_STACK_DEPTH];
		ushort m_nCurrentGosub;
		ScriptParam m_aLVars[NUM_LOCAL_VARS];
		ScriptParam m_aTimers[NUM_TIMERS];
#ifdef CLEO_VC
		bool m_bIsActive; 
		bool m_bCondResult; 
		bool m_bIsMission;
#else
		bool m_bCondResult;
		bool m_bIsMission;
		bool m_bIsActive;
#endif
		uint m_dwWakeTime;
		ushort m_wIfOp;
		bool m_bNotFlag;
		bool m_bDeathArrestCheckEnabled;
		bool m_bWastedOrBusted;
		bool m_bMissionFlag;

		CRunningScript();
};

class CCustomScript : protected CRunningScript
{
protected:
		struct StackFrame {
				StackFrame* prev;
				ScriptParam vars[NUM_LOCAL_VARS];
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
		ScriptParam* m_pLocalArray;

		CCustomScript();
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

// CRunningScript's data structure must not be altered!
static_assert(offsetof(CRunningScript, m_pNext) == 0x00);
static_assert(offsetof(CRunningScript, m_pPrev) == 0x04);
static_assert(offsetof(CRunningScript, m_acName) == 0x08);
static_assert(offsetof(CRunningScript, m_dwIp) == 0x10);
static_assert(offsetof(CRunningScript, m_aGosubAddr) == 0x14);
static_assert(offsetof(CRunningScript, m_nCurrentGosub) == 0x2C);
static_assert(offsetof(CRunningScript, m_aLVars) == 0x30);
static_assert(offsetof(CRunningScript, m_aTimers) == 0x70);
#ifdef CLEO_VC
	static_assert(offsetof(CRunningScript, m_bIsActive) == 0x78);
	static_assert(offsetof(CRunningScript, m_bCondResult) == 0x79);
	static_assert(offsetof(CRunningScript, m_bIsMission) == 0x7A);
#else
	static_assert(offsetof(CRunningScript, m_bCondResult) == 0x78);
	static_assert(offsetof(CRunningScript, m_bIsMission) == 0x79);
	static_assert(offsetof(CRunningScript, m_bIsActive) == 0x7A);
#endif
static_assert(offsetof(CRunningScript, m_dwWakeTime) == 0x7C);
static_assert(offsetof(CRunningScript, m_wIfOp) == 0x80);
static_assert(offsetof(CRunningScript, m_bNotFlag) == 0x82);
static_assert(offsetof(CRunningScript, m_bDeathArrestCheckEnabled) == 0x83);
static_assert(offsetof(CRunningScript, m_bWastedOrBusted) == 0x84);
static_assert(offsetof(CRunningScript, m_bMissionFlag) == 0x85);
static_assert(sizeof(CRunningScript) == 0x88);
