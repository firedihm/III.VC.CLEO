#pragma once

#include "domain.h"

#ifndef CLEOAPI
#define CLEOAPI __declspec(dllexport)
#endif

enum {
		KEY_LENGTH_IN_SCRIPT = 8,
		MAX_STACK_DEPTH = 6,
		NUM_LOCAL_VARS = 16,
		NUM_TIMERS = 2,
		CLEO_ARRAY_SIZE = 256
};

class CRunningScript
{
protected:
		CRunningScript* m_pNext;
		CRunningScript* m_pPrev;
		char m_acName[KEY_LENGTH_IN_SCRIPT];
		uint m_nIp;
		uint m_anGosubStack[MAX_STACK_DEPTH];
		ushort m_nGosubStackPointer;
		ScriptParam m_aLVars[NUM_LOCAL_VARS];
		ScriptParam m_aTimers[NUM_TIMERS];
#ifdef CLEO_VC
		bool m_bSkipWakeTime; 
		bool m_bCondResult; 
		bool m_bIsMissionScript;
#else
		bool m_bCondResult;
		bool m_bIsMissionScript;
		bool m_bSkipWakeTime;
#endif
		uint m_nWakeTime;
		ushort m_nAndOrState;
		bool m_bNotFlag;
		bool m_bDeatharrestEnabled;
		bool m_bDeatharrestExecuted;
		bool m_bMissionFlag;

		CRunningScript();
};

class CCustomScript : protected CRunningScript
{
protected:
		uchar* m_pCodeData;
		bool m_bIsCustom;
		bool m_bIsPersistent;
		uint m_nLastPedSearchIndex;
		uint m_nLastVehicleSearchIndex;
		uint m_nLastObjectSearchIndex;
		ScriptParam* m_pCleoArray;

		CCustomScript();
		~CCustomScript();

		void PushStackFrame();
		void PopStackFrame();

		/*
			ObjectReferences keep track of Script's ownership of objects it 
			creates (allocated memory, opened files...) to avoid memory leaks: 
			in case of programmer's fault or Script being prematurely terminated.
		*/
		void* AddReference(ObjectReference&& ref);
		void DestroyObject(void* obj);

		template <typename T>
		T* AddReference(T* obj) {
				return (T*)AddReference({ nullptr, obj, [](void* self) { static_cast<T*>(self)->~T(); } });
		}

private:
		struct StackFrame {
				StackFrame* next;
				uint ret_addr;
				ScriptParam vars[NUM_LOCAL_VARS];
		}* m_pCleoCallStack;

		struct ObjectReference {
				ObjectReference* next;
				void* object;
				void (__stdcall* destruct)(void* self); // non-capturing, dtor-invoking lambda
		}* m_pObjectReferences;
};

class Script : protected CCustomScript
{
public:
		Script() = default;
		Script(const char* filepath);

		void Init(); // this is a hook

		eOpcodeResult ProcessOneCommand();

		// exports
		CLEOAPI void CollectParameters(uint* pIp, short numParams);
		CLEOAPI int CollectNextParameterWithoutIncreasingPC(uint ip);
		CLEOAPI void Collect(short numParams) { CollectParameters(&m_nIp, numParams); }
		CLEOAPI void Store(short numParams);

		CLEOAPI ScriptParamType GetNextParamType();
		CLEOAPI void* GetPointerToScriptVariable();
		CLEOAPI void UpdateCompareFlag(bool result);
		CLEOAPI void ReadShortString(char* out);
		CLEOAPI void JumpTo(int address);
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
