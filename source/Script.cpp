#include "Game.h"
#include "Opcodes.h"
#include "Script.h"
#include "Trace.h"

#include <cctype>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <cstdio>

CRunningScript::CRunningScript() : next_(nullptr), prev_(nullptr), name_{'n', 'o', 'n', 'a', 'm', 'e', '\0'},
								   ip_(0), gosub_stack_{0}, gosub_stack_pointer_(0), local_vars_{0}, local_timers_{0},
								   cond_result_III_(false), use_mission_cleanup_III_(false), skip_wake_time_III_(false), is_active_III_(false), wake_time_(0),
								   and_or_state_(0), not_flag_(false), deatharrest_enabled_(true), deatharrest_executed_(false), mission_flag_(false) {}

Script::Script(int label) : code_data_(nullptr), is_III_(game::is_III()), is_custom_(false), is_persistent_(false),
							last_ped_search_index_(0), last_vehicle_search_index_(0), last_object_search_index_(0),
							call_stack_(nullptr), register_(nullptr), cleo_array_(new ScriptParam[CLEO_ARRAY_SIZE])
{
		ip_ = label;
		is_active() = true;
}

Script::Script(const char* filepath) : code_data_(nullptr), is_III_(game::is_III()), is_custom_(true), is_persistent_(false),
									   last_ped_search_index_(0), last_vehicle_search_index_(0), last_object_search_index_(0),
									   call_stack_(nullptr), register_(nullptr), cleo_array_(new ScriptParam[CLEO_ARRAY_SIZE])
{
		std::ifstream file(filepath, std::ios::binary);

		size_t filesize = file.seekg(0, std::ios::end).tellg(); // ok for binary mode
		if (!file || filesize == -1)
				throw "File is empty or corrupt";

		/*
		 *  We have to remember that game will always add ScriptSpace to ip_ when processing scripts, because 
		 *  the latter is an offset. Since custom scripts store code data on heap, and game can't account for this, 
		 *  we'll have to initialise ip_ to a difference of heap address and ScriptSpace, so it will even out 
		 *  when game will be processing this script.
		 */
		code_data_ = new uchar[filesize];
		file.seekg(0, std::ios::beg).read((char*)code_data_, filesize);

		ip_ = (uint)(code_data_ - game::ScriptSpace);
		is_active() = true;

		if (const char* ext = std::strrchr(filepath, '.'); !std::strcmp(ext, ".csp"))
				is_persistent_ = true;
}

Script::~Script()
{
		while (register_)
				delete_registered_object(register_->obj);

		while (call_stack_)
				pop_call_frame();

		delete[] cleo_array_;

		if (code_data_)
				delete[] code_data_;

		is_active() = false;
}

void
Script::Init()
{
		// game inits generic scripts on game::ScriptArray (game::ScriptArray[i].Init()), so "this" points to valid memory
		new (this) Script(0);
}

void
Script::jump(int label)
{
		// negated label is a hack that lets us tell apart custom and mission scripts
		if (label >= 0) {
				ip_ = label;
		} else {
				if (is_custom_)
						ip_ = (uint)(code_data_ - game::ScriptSpace) + (-label); // see Script ctor for details
				else
						ip_ = game::main_size + (-label);
		}
}

void
Script::push_call_frame()
{
		CallFrame* frame = new CallFrame();
		frame->prev = call_stack_;
		call_stack_ = frame;

		frame->ip = ip_;

		std::memcpy(frame->gosub_stack, gosub_stack_, sizeof(frame->gosub_stack));
		frame->gosub_stack_pointer = gosub_stack_pointer_;

		std::memcpy(frame->local_vars, local_vars_, sizeof(frame->local_vars));

		// save and reset logical flags, so they won't "leak" between caller and callee
		frame->cond_result = cond_result();
		frame->and_or_state = and_or_state_;
		frame->not_flag = not_flag_;
		and_or_state_ = false; 
		not_flag_ = 0;
}

void
Script::pop_call_frame()
{
		CallFrame* frame = call_stack_;

		// callee's new cond_result_ must be evaluated against caller's logical flags
		bool new_cond_result = cond_result();
		not_flag_ = frame->not_flag;
		and_or_state_ = frame->and_or_state;
		cond_result() = frame->cond_result;
		UpdateCompareFlag(new_cond_result);

		std::memcpy(local_vars_, frame->local_vars, sizeof(local_vars_));

		gosub_stack_pointer_ = frame->gosub_stack_pointer;
		std::memcpy(gosub_stack_, frame->gosub_stack, sizeof(gosub_stack_));

		ip_ = frame->ip;

		CallFrame* prev = frame->prev;
		delete frame;
		call_stack_ = prev;
}

void
Script::delete_registered_object(void* obj)
{
		RegData* target = register_;
		RegData* pentarget = nullptr;

		while (target) {
				if (target->obj == obj) {
						if (pentarget)
								pentarget->prev = target->prev;
						else
								register_ = target->prev;

						target->destruct(target->obj); // call dtor
						delete target->obj; // release memory

						delete target;
						return;
				} else {
						pentarget = target;
						target = target->prev;
				}
		}
}

void
Script::Process()
{
		if (use_mission_cleanup())
				game::DoDeathArrestCheck(this, 0);

		if (mission_flag_ && *game::pFailCurrentMission == 1 && gosub_stack_pointer_ == 1)
				ip_ = gosub_stack_[--gosub_stack_pointer_];

		if (is_persistent_ && *game::pTimeInMillisecondsPauseMode >= wake_time_ || !is_persistent_ && *game::pTimeInMilliseconds >= wake_time_) {
				while (!ProcessOneCommand());
		} else if (skip_wake_time() && *(game::pPadNewState + 0x20) && !(*game::pPadOldState + 0x20)) { // GetCrossJustDown()
				wake_time_ = 0;

				// reset all screen messages
				for (int style = 0; style < 6; ++style)
						game::AddBigMessageQ(L"", 0, style);

				game::AddMessageJumpQ(L"", 0, 0);
		}
}

eOpcodeResult
Script::ProcessOneCommand()
{
		// highest bit of opcode denotes not_flag_: reversing conditional result
		ushort op = *(ushort*)&game::ScriptSpace[ip_];
		not_flag_ = op & 0x8000;
		op &= 0x7FFF;
		ip_ += 2;

		eOpcodeResult result = OR_UNDEFINED;
		if (opcodes::definition(op)) {
				result = opcodes::definition(op)(this);
				(*game::pCommandsExecuted)++;
		} else if (op < opcodes::CUSTOM_START_ID) {
				result = game::OpcodeHandlers[op / 100](this, 0, op);
				(*game::pCommandsExecuted)++;
		} else {
				// if opcode isn't registered as custom, but has custom opcode's ID
				trace::line("&s: incorect call %X", name_, op);
				throw "Error has occured when parsing script";
		}

		trace::opcode(this, op);

		return result;
}

ScriptParam*
Script::GetPointerToScriptVariable(int, int)
{
		auto* data_type = (ScriptParam::Type*)&game::ScriptSpace[ip_];
		ip_++;

		switch (*data_type) {
		case ScriptParam::Type::GVar: {
				auto* addr = (ScriptParam*)&game::ScriptSpace[*(ushort*)&game::ScriptSpace[ip_]];
				ip_ += 2;
				return addr;
		}
		case ScriptParam::Type::LVar: {
				auto* addr = &local_vars_[*(ushort*)&game::ScriptSpace[ip_]];
				ip_ += 2;
				return addr;
		}
		default:
				return nullptr;
		}
}

int
Script::CollectParameters(uint* p_ip, short num_params)
{
		// -1 is a symbolic value to read params of variadic opcodes: read until ScriptParam::Type::EOP
		if (num_params == -1)
				num_params = game::MAX_NUM_SCRIPT_PARAMS;

		int collected = 0;
		for (short i = 0; i < num_params; i++) {
				auto* data_type = (ScriptParam::Type*)&game::ScriptSpace[*p_ip];
				(*p_ip)++;

				switch (*data_type) {
				case ScriptParam::Type::EOP:
						return collected;
				case ScriptParam::Type::Int32:
						game::ScriptParams[i].nVar = *(int*)&game::ScriptSpace[*p_ip];
						*p_ip += 4;
						break;
				case ScriptParam::Type::GVar:
						game::ScriptParams[i].nVar = *(int*)&game::ScriptSpace[*(ushort*)&game::ScriptSpace[*p_ip]];
						*p_ip += 2;
						break;
				case ScriptParam::Type::LVar:
						game::ScriptParams[i].nVar = local_vars_[*(ushort*)&game::ScriptSpace[*p_ip]].nVar;
						*p_ip += 2;
						break;
				case ScriptParam::Type::Int8:
						game::ScriptParams[i].nVar = *(char*)&game::ScriptSpace[*p_ip];
						*p_ip += 1;
						break;
				case ScriptParam::Type::Int16:
						game::ScriptParams[i].nVar = *(short*)&game::ScriptSpace[*p_ip];
						*p_ip += 2;
						break;
				case ScriptParam::Type::Float:
						if (is_III_) {
								game::ScriptParams[i].fVar = *(short*)&game::ScriptSpace[*p_ip] / 16.0f;
								*p_ip += 2;
						} else {
								game::ScriptParams[i].fVar = *(float*)&game::ScriptSpace[*p_ip];
								*p_ip += 4;
						}
						break;
				case ScriptParam::Type::StringUnproc: {
						uchar length = game::ScriptSpace[*p_ip];
						std::memcpy(&game::ScriptSpace[*p_ip], &game::ScriptSpace[*p_ip] + 1, length);
						*(&game::ScriptSpace[*p_ip] + length) = '\0';

						*data_type = ScriptParam::Type::String;
						[[fallthrough]];
				}
				case ScriptParam::Type::String:
						game::ScriptParams[i].szVar = (char*)&game::ScriptSpace[*p_ip];
						*p_ip += std::strlen((char*)&game::ScriptSpace[*p_ip]) + 1;
						break;
				default:
						// text labels have no data type prefix
						(*p_ip)--;
						game::ScriptParams[i].szVar = (char*)&game::ScriptSpace[*p_ip];
						*p_ip += KEY_LENGTH_IN_SCRIPT;
						break;
				}
				collected++;
		}

		return collected;
}

int
Script::CollectNextParameterWithoutIncreasingPC(uint ip)
{
		auto* data_type = (ScriptParam::Type*)&game::ScriptSpace[ip];
		ip++;

		switch (*data_type) {
		case ScriptParam::Type::Int32:
				return *(int*)&game::ScriptSpace[ip];
		case ScriptParam::Type::GVar:
				return *(int*)&game::ScriptSpace[*(ushort*)&game::ScriptSpace[ip]];
		case ScriptParam::Type::LVar:
				return local_vars_[*(ushort*)&game::ScriptSpace[ip]].nVar;
		case ScriptParam::Type::Int8:
				return *(char*)&game::ScriptSpace[ip];
		case ScriptParam::Type::Int16:
				return *(short*)&game::ScriptSpace[ip];
		case ScriptParam::Type::Float:
				if (is_III_)
						return (int)(*(short*)&game::ScriptSpace[ip] / 16.0f);
				else
						return *(int*)&game::ScriptSpace[ip];
		case ScriptParam::Type::StringUnproc: {
				uchar length = game::ScriptSpace[ip];
				std::memcpy(&game::ScriptSpace[ip], &game::ScriptSpace[ip] + 1, length);
				*(&game::ScriptSpace[ip] + length) = '\0';

				*data_type = ScriptParam::Type::String;
				[[fallthrough]];
		}
		case ScriptParam::Type::String:
				return (int)&game::ScriptSpace[ip]; // string address
		default:
				return -1;
		}
}

void
Script::StoreParameters(short num_params)
{
		game::StoreParameters(this, 0, &ip_, num_params);
}

void
Script::UpdateCompareFlag(bool result)
{
		game::UpdateCompareFlag(this, 0, result);
}

int
Script::format_string(char* out, const char* format)
{
		char* init_pos = out;

		// https://en.cppreference.com/w/cpp/io/c/printf.html
		while (*format) {
				if (*format != '%') {
						*(out++) = *(format++);
				} else {
						// read conversion specification (flags + length modifiers + specifier) and resolve it
						char conv_spec[32];
						int i = 0;

						conv_spec[i++] = *(format++); // '%'

						// flags
						while (*format == '-' || *format == '+' || *format == ' ' || *format == '#' || *format == '0' ||
							   *format == '*' || *format == '.' || std::isdigit(*format)) {
								if (*format == '*') {
										CollectParameters(1);
										i += std::sprintf(&conv_spec[i], "%u", game::ScriptParams[0].nVar);
										format++;
								} else {
										conv_spec[i++] = *(format++);
								}
						}

						// length modifiers
						while (*format == 'h' || *format == 'l' || *format == 'j' || *format == 'z' || *format == 't' || *format == 'L')
								conv_spec[i++] = *(format++);

						// conversion specifier
						conv_spec[i++] = *(format++);
						conv_spec[i] = '\0';

						// "%%" needs no parameters: it formats to '%'
						if (!(conv_spec[0] == '%' && conv_spec[1] == '%'))
								CollectParameters(1);

						bool is_float = false;
						for (int j = 0; j < i && !is_float; ++j) {
								is_float = conv_spec[j] == 'f' || conv_spec[j] == 'F' || conv_spec[j] == 'e' || conv_spec[j] == 'E' ||
										   conv_spec[j] == 'a' || conv_spec[j] == 'A' || conv_spec[j] == 'g' || conv_spec[j] == 'G';
						}

						if (is_float)
								out += std::sprintf(out, conv_spec, game::ScriptParams[0].fVar);
						else
								out += std::sprintf(out, conv_spec, game::ScriptParams[0].nVar);
				}
		}
		*out = '\0';

		// skip redundant params and consume ScriptParam::Type::EOP
		CollectParameters(-1);

		return out - init_pos;
}

int
Script::scan_string(const char* in, const char* format, bool return_packed_data)
{
		int num_assigned = 0;
		const char* init_pos = in;

		// https://en.cppreference.com/w/cpp/io/c/fscanf
		while (*format) {
				if (std::isspace(*format)) {
						format++;
						while (std::isspace(*in))
								in++;
				} else if (*format != '%') {
						if (*format == *in) {
								format++;
								in++;
						} else {
								break;
						}
				} else {
						// read conversion specification (flags + length modifiers + specifier) and resolve it
						char conv_spec[32];
						int i = 0;
						int chars_read = 0;

						conv_spec[i++] = *(format++); // '%'

						// assignment suppression flag
						if (*format == '*')
								conv_spec[i++] = *(format++);

						// field width
						while (std::isdigit(*format))
								conv_spec[i++] = *(format++);

						// length modifiers
						while (*format == 'h' || *format == 'l' || *format == 'j' || *format == 'z' || *format == 't' || *format == 'L')
								conv_spec[i++] = *(format++);

						// conversion specifier; can be a charset "[...]"
						if (char c = conv_spec[i++] = *(format++); c == '[') {
								// charset beggining with ']' will include ']' into set: "[]]"
								int num_closing_brackets_to_read = (*format == ']' || *format == '^' && *(format + 1) == ']') ? 2 : 1;
								while (num_closing_brackets_to_read) {
										char read = conv_spec[i++] = *(format++);
										num_closing_brackets_to_read = (read == ']') ? --num_closing_brackets_to_read : num_closing_brackets_to_read;
								}
						}

						// append "%n" to advance in*
						conv_spec[i++] = '%';
						conv_spec[i++] = 'n';
						conv_spec[i] = '\0';

						// sscanf() expects no arg if suppression flag is set or "%%" format is passed
						bool ignore_first_arg = conv_spec[1] == '*' || (conv_spec[0] == '%' && conv_spec[1] == '%');

						// args are passed as binaries: no type-checking; second arg is redundant if check above passes
						num_assigned += std::sscanf(in, conv_spec, (ignore_first_arg ? &chars_read : (int*)GetPointerToScriptVariable()), &chars_read);
						in += chars_read;
				}
		}

		// skip redundant args and consume ScriptParam::Type::EOP
		while (ScriptParam* ptr = GetPointerToScriptVariable());

		// TODO: get rid of this monstrosity by implementing proper parsers with scnlib and fmt
		if (return_packed_data) {
				return num_assigned | ((in - init_pos) << 8);
		} else {
				return num_assigned;
		}
}
