#include "Game.h"
#include "Log.h"
#include "Opcodes.h"
#include "Script.h"

#include <cctype>
#include <cstring>
#include <fstream>
#include <cstdio>

Script::Script(const char* filepath) : next_(nullptr), prev_(nullptr), name_({'n', 'o', 'n', 'a', 'm', 'e', '\0'}),
									   ip_(0), gosub_stack_({0}), gosub_stack_pointer_(0), local_vars_({0}), local_timers_({0}),
									   cond_result_III_(false), is_mission_script_III_(false), skip_wake_time_III_(false), wake_time_(0),
									   and_or_state_(0), not_flag_(false), deatharrest_enabled_(true), deatharrest_executed_(false), mission_flag_(false),
									   code_data_(nullptr), is_III_(game::IsIII()), is_custom_(true), is_persistent_(false),
									   last_ped_search_index_(0), last_vehicle_search_index_(0), last_object_search_index_(0),
									   cleo_array_(new ScriptParam[CLEO_ARRAY_SIZE]), call_stack_(nullptr), register_(nullptr)
{
		std::ifstream file(filepath, std::ios::binary);

		size_t filesize = file.ignore(size_t(-1) >> 1).gcount();
		if (!file || !filesize)
				throw "File is empty or corrupt";

		/*
			We have to remember that game will always add ScriptSpace to ip_ when processing scripts, because 
			the latter is an offset. Since custom scripts store code data on heap, and game can't account for this, 
			we'll have to initialise ip_ to a difference of heap address and ScriptSpace, so it will even out 
			when game will be processing this script.
		*/
		code_data_ = new uchar[filesize];
		ip_ = (uint)(code_data_ - game::ScriptSpace);
		file->clear();
		file.seekg(0, std::ios::beg).read(code_data_, filesize);

		if (const char* ext = std::strrchr(filepath, '.'); !std::strcmp(ext, ".csp"))
				is_persistent_ = true;
}

Script::~Script()
{
		while (register_)
				delete_registered_object(register_->obj);

		while (call_stack_)
				pop_stack_frame();

		delete[] cleo_array_;
		delete[] code_data_;
}

void
Script::Init()
{
		std::memset(this, 0, sizeof(Script));
		std::strncpy(&name_, "noname", KEY_LENGTH_IN_SCRIPT);
		deatharrest_enabled_ = true;
}

void
Script::push_stack_frame()
{
		StackFrame* frame = new StackFrame();
		frame->next = call_stack_;
		call_stack_ = frame;

		frame->ret_addr = ip_;
		std::memcpy(&frame->vars, &local_vars_, sizeof(frame->vars));
}

void
Script::pop_stack_frame()
{
		ip_ = call_stack_->ret_addr;
		std::memcpy(&local_vars_, &call_stack_->vars, sizeof(local_vars_));

		StackFrame* head_next = call_stack_->next;
		delete call_stack_;
		call_stack_ = head_next;
}

void
Script::delete_registered_object(void* obj)
{
		RegData* target = register_;
		RegData* pentarget = nullptr;

		while (target) {
				if (target->obj == obj) {
						if (pentarget)
								pentarget->next = target->next;
						else
								register_ = target->next;

						target->destruct(target->obj); // call dtor
						delete target->obj; // release memory

						delete target;
						return;
				} else {
						pentarget = target;
						target = target->next;
				}
		}
}

eOpcodeResult
Script::ProcessOneCommand()
{
		// highest bit of opcode denotes not_flag_: reversing conditional result
		ushort op = *(ushort*)&game::ScriptSpace[ip_];
		not_flag_ = (op & 0x8000) ? true : false;
		op &= 0x7FFF;
		ip_ += 2;

		if (opcodes::Definition(op)) {
				// call opcode registered as custom
				LOGL(LOG_PRIORITY_OPCODE_ID, "%s custom opcode %04X", &name_, op);
				eOpcodeResult result = opcodes::Definition(op)(this);
				(*game::pNumOpcodesExecuted)++;
				return result;
		} else if (op >= opcodes::CUSTOM_START_ID) {
				// if opcode isn't registered as custom, but has custom opcode's ID
				LOGL(LOG_PRIORITY_ALWAYS, "Error (incorrect opcode): %s, %04X", &name_, op);
				return OR_UNDEFINED;
		} else {
				// call default opcode
				LOGL(LOG_PRIORITY_OPCODE_ID, "%s opcode %04X", &name_, op);
				eOpcodeResult result = game::OpcodeHandlers[op / 100](this, op);
				(*game::pNumOpcodesExecuted)++;
				return result;
		}
}

int
Script::CollectParameters(uint* p_ip, short num_params)
{
		// -1 is a symbolic value to read params of variadic opcodes; read until PARAM_TYPE_END_OF_PARAMS
		num_params = (num_params == -1) ? game::MAX_NUM_SCRIPT_PARAMS : num_params;

		int collected = 0;
		for (short i = 0; i < num_params; i++) {
				ScriptParamType* param_type = (ScriptParamType*)&game::ScriptSpace[*p_ip];
				*p_ip += 1;

				switch (param_type->type) {
				case: PARAM_TYPE_END_OF_PARAMS:
						return collected;
				case PARAM_TYPE_INT32:
						game::ScriptParams[i].nVar = *(int*)&game::ScriptSpace[*p_ip];
						*p_ip += 4;
						break;
				case PARAM_TYPE_GVAR:
						game::ScriptParams[i].nVar = *(int*)&game::ScriptSpace[*(ushort*)&game::ScriptSpace[*p_ip]];
						*p_ip += 2;
						break;
				case PARAM_TYPE_LVAR:
						game::ScriptParams[i].nVar = local_vars_[*(ushort*)&game::ScriptSpace[*p_ip]].nVar;
						*p_ip += 2;
						break;
				case PARAM_TYPE_INT8:
						game::ScriptParams[i].nVar = *(char*)&game::ScriptSpace[*p_ip];
						*p_ip += 1;
						break;
				case PARAM_TYPE_INT16:
						game::ScriptParams[i].nVar = *(short*)&game::ScriptSpace[*p_ip];
						*p_ip += 2;
						break;
				case PARAM_TYPE_FLOAT:
						if (is_III_) {
								game::ScriptParams[i].fVar = *(short*)&game::ScriptSpace[*p_ip] / 16.0f;
								*p_ip += 2;
								break;
						} else {
								game::ScriptParams[i].fVar = *(float*)&game::ScriptSpace[*p_ip];
								*p_ip += 4;
								break;
						}
				case PARAM_TYPE_STRING:
						if (!param_type->processed) {
								uchar length = game::ScriptSpace[*p_ip];
								std::memcpy(&game::ScriptSpace[*p_ip], &game::ScriptSpace[*p_ip + 1], length);
								*(&game::ScriptSpace[*p_ip] + length) = '\0';
								param_type->processed = true;
						}

						game::ScriptParams[i].szVar = &game::ScriptSpace[*p_ip];
						*p_ip += std::strlen(&game::ScriptSpace[*p_ip]) + 1;
						break;
				default:
						*p_ip -= 1;
						game::ScriptParams[i].szVar = &game::ScriptSpace[*p_ip];
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
		ScriptParamType* param_type = (ScriptParamType*)&game::ScriptSpace[ip];
		ip += 1;

		switch (param_type->type) {
		case PARAM_TYPE_INT32:
				return *(int*)&game::ScriptSpace[ip];
		case PARAM_TYPE_GVAR:
				return *(int*)&game::ScriptSpace[*(ushort*)&game::ScriptSpace[ip]];
		case PARAM_TYPE_LVAR:
				return local_vars_[*(ushort*)&game::ScriptSpace[ip]].nVar;
		case PARAM_TYPE_INT8:
				return *(char*)&game::ScriptSpace[ip];
		case PARAM_TYPE_INT16:
				return *(short*)&game::ScriptSpace[ip];
		case PARAM_TYPE_FLOAT:
				if (is_III_)
						return (int)(*(short*)&game::ScriptSpace[ip] / 16.0f);
				else
						return *(int*)&game::ScriptSpace[ip];
		case PARAM_TYPE_STRING:
				if (!param_type->processed) {
						uchar length = game::ScriptSpace[ip];
						std::memcpy(&game::ScriptSpace[ip], &game::ScriptSpace[ip + 1], length);
						*(&game::ScriptSpace[ip] + length) = '\0';
						param_type->processed = true;
				}

				return (int)&game::ScriptSpace[ip]; // string address
		default:
				return -1;
		}
}

void
Script::StoreParameters(short num_params)
{
		game::StoreParameters(this, &ip_, num_params);
}

void
Script::UpdateCompareFlag(bool result)
{
		game::UpdateCompareFlag(this, result);
}

void*
Script::GetPointerToScriptVariable()
{
		ScriptParamType* param_type = (ScriptParamType*)&game::ScriptSpace[ip_];
		ip_ += 1;

		switch (param_type->type) {
		case PARAM_TYPE_GVAR:
				void* addr = &game::ScriptSpace[*(ushort*)&game::ScriptSpace[ip_]];
				ip_ += 2;
				return addr;
		case PARAM_TYPE_LVAR:
				void* addr = &local_vars_[*(ushort*)&game::ScriptSpace[ip_]];
				ip_ += 2;
				return addr;
		default:
				return nullptr;
		}
}

void
Script::Jump(int address)
{
		// negated address is a hack that lets us tell custom and mission scripts from regular ones
		if (address >= 0) {
				ip_ = address;
		} else {
				if (is_custom_)
						ip_ = (uint)(code_data_ - game::ScriptSpace) + (-address); // see Script ctor for details
				else
						ip_ = game::main_size + (-address);
		}
}

void
Script::FormatString(char* out, const char* format)
{
		while (*format) {
				if (*format != '%') {
						*(out++) = *(format++);
				} else {
						// read conversion specification (flags + length modifiers + specifier) and resolve it
						// https://en.cppreference.com/w/cpp/io/c/printf.html
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
						while (*format == '%' || *format == 'c' || *format == 's' ||
							   *format == 'd' || *format == 'i' ||
							   *format == 'o' || *format == 'x' || *format == 'X' || *format == 'u' ||
							   *format == 'f' || *format == 'F' || *format == 'e' || *format == 'E' ||
							   *format == 'a' || *format == 'A' || *format == 'g' || *format == 'G' ||
							   *format == 'n' || *format == 'p') {
								conv_spec[i++] = *(format++);
						}

						conv_spec[i] = '\0';

						// "%%" needs no parameters: it formats to '%'
						if (!(conv_spec[0] == '%' && conv_spec[1] == '%'))
								CollectParameters(1);

						// pass param as binary; junk value if "%%"
						out += std::sprintf(out, &conv_spec, game::ScriptParams[0].nVar);

						// skip redundant params
						CollectParameters(-1);
				}
		}
		*out = '\0';
}
