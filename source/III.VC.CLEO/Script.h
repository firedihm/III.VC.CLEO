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
		constexpr int KEY_LENGTH_IN_SCRIPT = 8;
		constexpr int MAX_STACK_DEPTH = 6;
		constexpr int NUM_LOCAL_VARS = 16;
		constexpr int NUM_TIMERS = 2;

		CRunningScript* next_;
		CRunningScript* prev_;
		char name_[KEY_LENGTH_IN_SCRIPT];
		uint ip_;
		uint gosub_stack_[MAX_STACK_DEPTH];
		ushort gosub_stack_pointer_;
		ScriptParam local_vars_[NUM_LOCAL_VARS];
		ScriptParam local_timers_[NUM_TIMERS];
		union { bool cond_result_III_, skip_wake_time_VC_; };
		union { bool is_mission_script_III_, cond_result_VC_; };
		union { bool skip_wake_time_III_, is_mission_script_VC_; };
		uint wake_time_;
		ushort and_or_state_;
		bool not_flag_;
		bool deatharrest_enabled_;
		bool deatharrest_executed_;
		bool mission_flag_;

		CRunningScript();

		// getters for flags defined in unions above
		bool cond_result();
		bool is_mission_script();
		bool skip_wake_time();
};

// make sure CRunningScript's composition wasn't changed; as long as member order is correct...
static_assert(sizeof(CRunningScript) == 0x88);

class CCustomScript : protected CRunningScript
{
protected:
		constexpr int CLEO_ARRAY_SIZE = 256;

		uchar* code_data_;
		bool is_custom_;
		bool is_persistent_;
		int last_ped_search_index_;
		int last_vehicle_search_index_;
		int last_object_search_index_;
		ScriptParam* cleo_array_;

		CCustomScript();
		~CCustomScript();

		void push_stack_frame();
		void pop_stack_frame();

		/*
			We register objects script creates (allocated memory, opened files...) and their dtors 
			to avoid memory leaks: in case of programmer's fault or premature termination.
		*/
		template <typename T>
		void register_object(T* obj) { register_ = new RegData{register_, obj, [](void* self) { static_cast<T*>(self)->~T(); }}; }
		void delete_registered_object(void* obj);

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

		__declspec(dllexport) void CollectParameters(uint* p_ip, short num_params);
		__declspec(dllexport) int CollectNextParameterWithoutIncreasingPC(uint ip);
		__declspec(dllexport) void CollectParameters(short num_params) { CollectParameters(&m_nIp, num_params); }
		__declspec(dllexport) void StoreParameters(short num_params);

		__declspec(dllexport) ScriptParamType GetNextParamType();
		__declspec(dllexport) void* GetPointerToScriptVariable();
		__declspec(dllexport) void UpdateCompareFlag(bool result);
		__declspec(dllexport) void JumpTo(int address);
		__declspec(dllexport) void FormatString(char* out, const char* format);
};
