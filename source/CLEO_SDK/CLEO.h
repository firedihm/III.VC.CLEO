#pragma once

struct ScriptParamType
{
        enum : char {
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

union ScriptParam
{
        int nVar;
        float fVar;
        char* szVar;
        void* pVar;
};

enum eOpcodeResult : char
{
        OR_CONTINUE = 0,
        OR_TERMINATE = 1,
        OR_UNDEFINED = -1
};

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

class __declspec(dllimport) Script : protected CRunningScript
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

		ScriptParam* GetPointerToScriptVariable();
		int CollectParameters(uint* p_ip, short num_params);
		int CollectParameters(short num_params) { return CollectParameters(&m_nIp, num_params); }
		int CollectNextParameterWithoutIncreasingPC(uint ip);
		void StoreParameters(short num_params);
		void UpdateCompareFlag(bool result);

		void jump(int address);

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

namespace opcodes
{
        constexpr ushort CUSTOM_START_ID = 0x05DC;
        constexpr ushort MAX_ID = 0x8000;

        using Definition = eOpcodeResult __stdcall(Script*);

        __declspec(dllimport) bool register(ushort id, Definition* def);
        __declspec(dllimport) Definition* definition(ushort id);
}

namespace game
{
		__declspec(dllimport) uchar* script_space();
		__declspec(dllimport) ScriptParam* script_params();
}

namespace cleo
{
		__declspec(dllimport) uint version(); // returns current version
		__declspec(dllimport) uint version(uint main, uint major, uint minor);
}
