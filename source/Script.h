#pragma once

#include "CLEO.h"
#include "domain.h"

/*
	We "inherit" CRunningScript from base game, and add custom data in Script. 
	III and VC have equal number and types of class' members. Order of members differs 
	only for 3 flags; they are defined in unions below.
*/
class CLEO_API CRunningScript
{
public:
		static constexpr int KEY_LENGTH_IN_SCRIPT = 8;
		static constexpr int MAX_STACK_DEPTH = 6;
		static constexpr int NUM_LOCAL_VARS = 16;
		static constexpr int NUM_TIMERS = 2;

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
};

class CLEO_API Script : public CRunningScript
{
private:
		struct CallFrame {
				CallFrame* prev;
				uint ip;
				uint gosub_stack[MAX_STACK_DEPTH];
				ushort gosub_stack_pointer;
				ScriptParam local_vars[NUM_LOCAL_VARS];
				bool cond_result;
				ushort and_or_state;
				bool not_flag;
		};

		struct RegData {
				RegData* prev;
				void* obj;
				void(*destruct)(void* self); // non-capturing, dtor-invoking lambda
		};

public:
		static constexpr int CLEO_ARRAY_SIZE = 256;

		uchar* code_data_;
		bool is_III_;
		bool is_custom_;
		bool is_persistent_;
		int last_ped_search_index_;
		int last_vehicle_search_index_;
		int last_object_search_index_;
		CallFrame* call_stack_;
		RegData* register_;
		ScriptParam* cleo_array_;

		Script() = default;
		Script(const char* filepath);
		~Script();

		// this is a hook, as game never calls ctors for CRunningScript
		void Init();

		// getters for flags defined in CRunningScript's unions
		bool& cond_result() { return is_III_ ? cond_result_III_ : cond_result_VC_; }
		bool& is_mission_script() { return is_III_ ? is_mission_script_III_ : is_mission_script_VC_; }
		bool& skip_wake_time() { return is_III_ ? skip_wake_time_III_ : skip_wake_time_VC_; }

		void jump(int address);

		// used by 0AB1: CLEO_CALL and 0AB2: CLEO_RETURN
		void push_call_frame();
		void pop_call_frame();

		// keep track of dynamically allocated memory
		template <typename T>
		void register_object(T* obj) { register_ = new RegData{register_, obj, [](void* self) { ((T*)self)->~T(); }}; }
		void delete_registered_object(void* obj);

		eOpcodeResult ProcessOneCommand();

		ScriptParam* GetPointerToScriptVariable(int a = 0, int n = 0); // both params are unused by game
		int CollectParameters(uint* p_ip, short num_params);
		int CollectParameters(short num_params) { return CollectParameters(&ip_, num_params); }
		int CollectNextParameterWithoutIncreasingPC(uint ip);
		void StoreParameters(short num_params);
		void UpdateCompareFlag(bool result);

		// returns address of member function; very dangerous, only MSVC returns actual function address
		template <typename Ret, typename... Args>
		static void* gaddr(Ret(Script::* func)(Args...)) {
				return (void*&)func;
		}

		// this is, hopefully, only temporary: proper formatting needs to be implemented for CLEO2 opcodes
		int format_string(char* out, const char* format);
		int scan_string(const char* in, const char* format, bool return_packed_data = false);
};

// make sure CRunningScript's composition wasn't changed; as long as member order is correct...
static_assert(sizeof(CRunningScript) == 0x88);
