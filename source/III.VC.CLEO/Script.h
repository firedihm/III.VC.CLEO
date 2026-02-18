#pragma once

#include "domain.h"

/*
	We "inherit" CRunningScript from base game, and add custom data in CCustomScript. 
	III and VC have equal number and types of class' members. Order of members differs 
	only for 3 flags; they are defined in unions below.
*/
class CRunningScript
{
protected:
		static constexpr int KEY_LENGTH_IN_SCRIPT = 8;
		static constexpr int MAX_STACK_DEPTH = 6;
		static constexpr int NUM_LOCAL_VARS = 16;
		static constexpr int NUM_TIMERS = 2;

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

// make sure CRunningScript's composition wasn't changed; as long as member order is correct...
static_assert(sizeof(CRunningScript) == 0x88);

class CCustomScript : protected CRunningScript
{
protected:
		static constexpr int CLEO_ARRAY_SIZE = 256;

		uchar* m_pCodeData;
		bool m_bIsCustom;
		bool m_bIsPersistent;
		uint m_nLastPedSearchIndex;
		uint m_nLastVehicleSearchIndex;
		uint m_nLastObjectSearchIndex;
		ScriptParam* CLEO_array_;

		CCustomScript();
		~CCustomScript();

		void PushStackFrame();
		void PopStackFrame();

		/*
			We register objects script creates (allocated memory, opened files...) and their dtors 
			to avoid memory leaks: in case of programmer's fault or premature termination.
		*/
		template <typename T>
		void RegisterObject(T* obj) { register_ = new RegData{register_, obj, [](void* self) { static_cast<T*>(self)->~T(); }}; }
		void DeleteRegisteredObject(void* obj);

private:
		struct StackFrame {
				StackFrame* next;
				uint ret_addr;
				ScriptParam vars[NUM_LOCAL_VARS];
		}* call_stack_;

		struct RegData {
				RegData* next;
				void* obj;
				void (__thiscall* destruct)(void* self); // non-capturing, dtor-invoking lambda
		}* register_;
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
