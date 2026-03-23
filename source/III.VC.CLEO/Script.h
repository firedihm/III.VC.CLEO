#pragma once

#include "domain.h"

/*
	We "inherit" CRunningScript from base game, and add custom data in Script. 
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
};

// make sure CRunningScript's composition wasn't changed; as long as member order is correct...
static_assert(sizeof(CRunningScript) == 0x88);

class __declspec(dllexport) Script : protected CRunningScript
{
public:
		constexpr int CLEO_ARRAY_SIZE = 256;

		uchar* code_data_;
		bool is_III_; // lets opcode extensions know game version without importing game namespace
		bool is_custom_;
		bool is_persistent_;
		int last_ped_search_index_;
		int last_vehicle_search_index_;
		int last_object_search_index_;
		ScriptParam* cleo_array_;

		Script() = default;
		Script(const char* filepath);
		~Script();

		// this is a hook; game never calls ctors
		void Init();

		// getters for flags defined in CRunningScript's unions
		bool cond_result() { return is_III_ ? cond_result_III_ : cond_result_VC_; }
		bool is_mission_script() { return is_III_ ? is_mission_script_III_ : is_mission_script_VC_; }
		bool skip_wake_time() { return is_III_ ? skip_wake_time_III_ : skip_wake_time_VC_; }

		// used by 0AB1: CLEO_CALL and 0AB2: CLEO_RETURN
		void push_stack_frame();
		void pop_stack_frame();

		// keep track of dynamically allocated memory
		template <typename T>
		void register_object(T* obj) { register_ = new RegData{register_, obj, [](void* self) { static_cast<T*>(self)->~T(); }}; }
		void delete_registered_object(void* obj);

		eOpcodeResult ProcessOneCommand();

		ScriptParam* GetPointerToScriptVariable();
		int CollectParameters(uint* p_ip, short num_params);
		int CollectParameters(short num_params) { return CollectParameters(&m_nIp, num_params); }
		int CollectNextParameterWithoutIncreasingPC(uint ip);
		void StoreParameters(short num_params);
		void UpdateCompareFlag(bool result);

		void jump(int address);
		int format_string(char* out, const char* format);
		int scan_string(const char* in, const char* format);

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
