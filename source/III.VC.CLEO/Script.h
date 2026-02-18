#pragma once

#include "domain.h"

enum {
		KEY_LENGTH_IN_SCRIPT = 8,
		MAX_STACK_DEPTH = 6,
		NUM_LOCAL_VARS = 16,
		NUM_TIMERS = 2,
		CLEO_ARRAY_SIZE = 256
};

/*
	We "inherit" CRunningScript from base game, and add custom data in CCustomScript. 
	The order of class' members is same for both III and VC, except for 3 flags defined in unions.
*/
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
		union { bool m_bCondResultIII, m_bSkipWakeTimeVC; };
		union { bool m_bIsMissionScriptIII, m_bCondResultVC; };
		union { bool m_bSkipWakeTimeIII, m_bIsMissionScriptVC; };
		uint m_nWakeTime;
		ushort m_nAndOrState;
		bool m_bNotFlag;
		bool m_bDeatharrestEnabled;
		bool m_bDeatharrestExecuted;
		bool m_bMissionFlag;

		CRunningScript();
};

static_assert(sizeof(CRunningScript) == 0x88);

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
			ObjectReferences keep track of script's ownership of objects it 
			creates (allocated memory, opened files...) to avoid memory leaks: 
			in case of programmer's fault or premature termination.
		*/
		template <typename T>
		void AddReference(T* obj) {
				m_pObjectReferences = new ObjectReference{m_pObjectReferences, obj, [](void* self) { static_cast<T*>(self)->~T(); }};
		}
		void DeleteObject(void* obj);

private:
		struct StackFrame {
				StackFrame* next;
				uint ret_addr;
				ScriptParam vars[NUM_LOCAL_VARS];
		}* m_pCleoCallStack;

		struct ObjectReference {
				ObjectReference* next;
				void* obj;
				void (__thiscall* destruct)(void* self); // non-capturing, dtor-invoking lambda
		}* m_pObjectReferences;
};

// this is introduced to provide laconic, forward-declarable type alias for CCustomScript.
class Script : protected CCustomScript
{
public:
		Script() = default;
		Script(const char* filepath);

		void Init(); // this is a hook

		eOpcodeResult ProcessOneCommand();

		__declspec(dllexport) void CollectParameters(uint* pIp, short numParams);
		__declspec(dllexport) int CollectNextParameterWithoutIncreasingPC(uint ip);
		__declspec(dllexport) void CollectParameters(short numParams) { CollectParameters(&m_nIp, numParams); }
		__declspec(dllexport) void StoreParameters(short numParams);

		__declspec(dllexport) ScriptParamType GetNextParamType();
		__declspec(dllexport) void* GetPointerToScriptVariable();
		__declspec(dllexport) void UpdateCompareFlag(bool result);
		__declspec(dllexport) void ReadShortString(char* out);
		__declspec(dllexport) void JumpTo(int address);
};
