#include "Game.h"
#include "Log.h"
#include "Opcodes.h"
#include "Script.h"

#include <cctype>
#include <cstring>
#include <fstream>
#include <cstdio>

CRunningScript::CRunningScript() : m_pNext(nullptr), m_pPrev(nullptr), m_acName({'n', 'o', 'n', 'a', 'm', 'e', '\0'}),
								   m_nIp(0), m_anGosubStack({0}), m_nGosubStackPointer(0), m_aLVars({0}), m_aTimers({0}),
								   m_bCondResultIII(false), m_bIsMissionScriptIII(false), m_bSkipWakeTimeIII(false), m_nWakeTime(0),
								   m_nAndOrState(0), m_bNotFlag(false), m_bDeatharrestEnabled(true), m_bDeatharrestExecuted(false), m_bMissionFlag(false) {}

CCustomScript::CCustomScript() : m_pCodeData(nullptr), m_bIsCustom(true), m_bIsPersistent(false), m_nLastPedSearchIndex(0), m_nLastVehicleSearchIndex(0), m_nLastObjectSearchIndex(0),
								 cleo_array_(new ScriptParam[CLEO_ARRAY_SIZE]), call_stack_(nullptr), register_(nullptr) {}

CCustomScript::~CCustomScript()
{
		while (register_)
				DeleteRegisteredObject(register_->obj);

		while (call_stack_)
				PopStackFrame();

		delete[] cleo_array_;
		delete[] m_pCodeData;
}

void
CCustomScript::PushStackFrame()
{
		StackFrame* frame = new StackFrame();
		frame->next = call_stack_;
		call_stack_ = frame;

		frame->ret_addr = m_nIp;
		std::memcpy(&frame->vars, &m_aLVars, sizeof(frame->vars));
}

void
CCustomScript::PopStackFrame()
{
		m_nIp = call_stack_->ret_addr;
		std::memcpy(&m_aLVars, &call_stack_->vars, sizeof(m_aLVars));

		StackFrame* head_next = call_stack_->next;
		delete call_stack_;
		call_stack_ = head_next;
}

void
CCustomScript::DeleteRegisteredObject(void* obj)
{
		RegData* target = register_;
		RegData* pentarget = nullptr;

		while (target && target->obj != obj) {
				pentarget = target;
				target = target->next;
		}

		if (target) {
				if (pentarget)
						pentarget->next = target->next;
				else
						register_ = target->next;

				target->destruct(target->obj); // call dtor
				delete target->obj; // release memory

				delete target;
		}
}

Script::Script(const char* filepath)
{
		std::ifstream file(filepath, std::ios::binary);

		size_t filesize = file.ignore(size_t(-1) >> 1).gcount();
		if (!file || !filesize)
				throw "File is empty or corrupt";

		/*
			We have to remember that game will always add ScriptSpace to m_nIp when processing scripts, because 
			the latter is an offset. Since custom scripts store code data on heap, and game can't account for this, 
			we'll have to initialise m_nIp to a difference of heap address and ScriptSpace, so it will even out 
			when game will be processing this script.
		*/
		m_pCodeData = new uchar[filesize];
		m_nIp = (uint)(m_pCodeData - game::ScriptSpace);
		file->clear();
		file.seekg(0, std::ios::beg).read(m_pCodeData, filesize);

		if (const char* ext = std::strrchr(filepath, '.'); !std::strcmp(ext, ".csp"))
				m_bIsPersistent = true;
}

void
Script::Init()
{
		std::memset(this, 0, sizeof(Script));
		std::strncpy(&m_acName, "noname", KEY_LENGTH_IN_SCRIPT);
		m_bDeatharrestEnabled = true;
		cleo_array_ = new ScriptParam[CLEO_ARRAY_SIZE];
}

eOpcodeResult
Script::ProcessOneCommand()
{
		// highest bit of opcode denotes notFlag: reversing conditional result
		ushort op = *(ushort*)&game::ScriptSpace[m_nIp];
		m_bNotFlag = (op & 0x8000) ? true : false;
		op &= 0x7FFF;
		m_nIp += 2;

		if (opcodes::Definition(op)) {
				// call opcode registered as custom
				LOGL(LOG_PRIORITY_OPCODE_ID, "%s custom opcode %04X", &m_acName, op);
				eOpcodeResult result = opcodes::Definition(op)(this);
				(*game::pNumOpcodesExecuted)++;
				return result;
		} else if (op >= opcodes::CUSTOM_START_ID) {
				// if opcode isn't registered as custom, but has custom opcode's ID
				LOGL(LOG_PRIORITY_ALWAYS, "Error (incorrect opcode): %s, %04X", &m_acName, op);
				return OR_UNDEFINED;
		} else {
				// call default opcode
				LOGL(LOG_PRIORITY_OPCODE_ID, "%s opcode %04X", &m_acName, op);
				eOpcodeResult result = game::OpcodeHandlers[op / 100](this, op);
				(*game::pNumOpcodesExecuted)++;
				return result;
		}
}

void
Script::CollectParameters(uint* pIp, short num_params)
{
		for (short i = 0; i < num_params; i++) {
				ScriptParamType* paramType = (ScriptParamType*)&game::ScriptSpace[*pIp];
				*pIp += 1;

				switch (paramType->type) {
				case PARAM_TYPE_INT32:
						game::ScriptParams[i].nVar = *(int*)&game::ScriptSpace[*pIp];
						*pIp += 4;
						break;
				case PARAM_TYPE_GVAR:
						game::ScriptParams[i].nVar = *(int*)&game::ScriptSpace[*(ushort*)&game::ScriptSpace[*pIp]];
						*pIp += 2;
						break;
				case PARAM_TYPE_LVAR:
						game::ScriptParams[i].nVar = m_aLVars[*(ushort*)&game::ScriptSpace[*pIp]].nVar;
						*pIp += 2;
						break;
				case PARAM_TYPE_INT8:
						game::ScriptParams[i].nVar = *(char*)&game::ScriptSpace[*pIp];
						*pIp += 1;
						break;
				case PARAM_TYPE_INT16:
						game::ScriptParams[i].nVar = *(short*)&game::ScriptSpace[*pIp];
						*pIp += 2;
						break;
				case PARAM_TYPE_FLOAT:
						if (game::IsIII()) {
								game::ScriptParams[i].fVar = *(short*)&game::ScriptSpace[*pIp] / 16.0f;
								*pIp += 2;
								break;
						} else {
								game::ScriptParams[i].fVar = *(float*)&game::ScriptSpace[*pIp];
								*pIp += 4;
								break;
						}
				case PARAM_TYPE_STRING:
						if (!paramType->processed) {
								uchar length = game::ScriptSpace[*pIp];
								std::memcpy(&game::ScriptSpace[*pIp], &game::ScriptSpace[*pIp + 1], length);
								*(&game::ScriptSpace[*pIp] + length) = '\0';
								paramType->processed = true;
						}

						game::ScriptParams[i].szVar = &game::ScriptSpace[*pIp];
						*pIp += std::strlen(&game::ScriptSpace[*pIp]) + 1;
						break;
				default:
						*pIp -= 1;
						game::ScriptParams[i].szVar = &game::ScriptSpace[*pIp];
						*pIp += KEY_LENGTH_IN_SCRIPT;
						break;
				}
		}
}

int
Script::CollectNextParameterWithoutIncreasingPC(uint ip)
{
		ScriptParamType* paramType = (ScriptParamType*)&game::ScriptSpace[ip];
		ip += 1;

		switch (paramType->type) {
		case PARAM_TYPE_INT32:
				return *(int*)&game::ScriptSpace[ip];
		case PARAM_TYPE_GVAR:
				return *(int*)&game::ScriptSpace[*(ushort*)&game::ScriptSpace[ip]];
		case PARAM_TYPE_LVAR:
				return m_aLVars[*(ushort*)&game::ScriptSpace[ip]].nVar;
		case PARAM_TYPE_INT8:
				return *(char*)&game::ScriptSpace[ip];
		case PARAM_TYPE_INT16:
				return *(short*)&game::ScriptSpace[ip];
		case PARAM_TYPE_FLOAT:
				if (game::IsIII())
						return (int)(*(short*)&game::ScriptSpace[ip] / 16.0f);
				else
						return *(int*)&game::ScriptSpace[ip];
		case PARAM_TYPE_STRING:
				if (!paramType->processed) {
						uchar length = game::ScriptSpace[ip];
						std::memcpy(&game::ScriptSpace[ip], &game::ScriptSpace[ip + 1], length);
						*(&game::ScriptSpace[ip] + length) = '\0';
						paramType->processed = true;
				}

				return (int)&game::ScriptSpace[ip]; // string address
		default:
				return -1;
		}
}

void
Script::StoreParameters(short num_params)
{
		game::StoreParameters(this, &m_nIp, num_params);
}

ScriptParamType
Script::GetNextParamType()
{
		return ((ScriptParamType*)&game::ScriptSpace[m_nIp])->type;
}

void*
Script::GetPointerToScriptVariable()
{
		return game::GetPointerToScriptVariable(this, &m_nIp, 1);
}

void
Script::UpdateCompareFlag(bool result)
{
		game::UpdateCompareFlag(this, result);
}

void
Script::JumpTo(int address)
{
		// negated address is a hack that lets us tell custom and mission scripts from regular ones
		if (address >= 0) {
				m_nIp = address;
		} else {
				if (m_bIsCustom)
						m_nIp = (uint)(m_pCodeData - game::ScriptSpace) + (-address); // see Script ctor for details
				else
						m_nIp = game::MainSize + (-address);
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

						// "%%" needs no argument: it formats to '%'
						if (!(conv_spec[0] == '%' && conv_spec[1] == '%'))
								CollectParameters(1);

						out += std::sprintf(out, &conv_spec, game::ScriptParams[0].nVar); // pass argument as binary; junk value if "%%"
				}
		}

		*out = '\0';
}
