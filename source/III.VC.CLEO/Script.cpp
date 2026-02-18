#include "Game.h"
#include "Log.h"
#include "Opcodes.h"
#include "Script.h"

#include <cstring>
#include <fstream>

CRunningScript::CRunningScript() : m_pNext(nullptr), m_pPrev(nullptr), m_acName({'n', 'o', 'n', 'a', 'm', 'e', '\0'}),
								   m_nIp(0), m_anGosubStack({0}), m_nGosubStackPointer(0), m_aLVars({0}), m_aTimers({0}),
								   m_bCondResultIII(false), m_bIsMissionScriptIII(false), m_bSkipWakeTimeIII(false), m_nWakeTime(0),
								   m_nAndOrState(0), m_bNotFlag(false), m_bDeatharrestEnabled(true), m_bDeatharrestExecuted(false), m_bMissionFlag(false) {}

CCustomScript::CCustomScript() : m_pCodeData(nullptr), m_bIsCustom(true), m_bIsPersistent(false), m_nLastPedSearchIndex(0), m_nLastVehicleSearchIndex(0), m_nLastObjectSearchIndex(0),
								 CLEO_array_(new ScriptParam[CLEO_ARRAY_SIZE]), call_stack_(nullptr), register_(nullptr) {}

CCustomScript::~CCustomScript()
{
		while (register_)
				DeleteRegisteredObject(register_->obj);

		while (call_stack_)
				PopStackFrame();

		delete[] CLEO_array_;
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
		for (RegData* prev = nullptr, curr = register_; curr; prev = curr, curr = curr->next) {
				if (curr->object == obj) {
						if (prev)
								prev->next = curr->next;
						else
								register_ = curr->next;

						curr->destruct(curr->obj); // call dtor
						delete curr->obj; // release memory

						delete curr;
						return;
				}
		}
}

Script::Script(const char* filepath)
{
		std::ifstream file(filepath, std::ios::binary);

		size_t filesize = file.ignore(size_t(-1) >> 1).gcount();
		if (!file || !filesize)
				throw "File is empty or corrupt";

		/*
			We have to remember that game will always add pScriptSpace to m_nIp when processing scripts, because 
			the latter is an offset. Since custom scripts store code data on heap, and game can't account for this, 
			we'll have to initialise m_nIp to a difference of heap address and pScriptSpace, so it will even out 
			when game will be processing this script.
		*/
		m_pCodeData = new uchar[filesize];
		m_nIp = (uint)(m_pCodeData - game.Scripts.pScriptSpace);
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
		CLEO_array_ = new ScriptParam[CLEO_ARRAY_SIZE];
}

eOpcodeResult
Script::ProcessOneCommand()
{
		// highest bit of opcode denotes notFlag: reversing conditional result
		ushort op = *(ushort*)&game.Scripts.pScriptSpace[m_nIp];
		m_bNotFlag = (op & 0x8000) ? true : false;
		op &= 0x7FFF;
		m_nIp += 2;

		if (opcodes::Definitions[op]) {
				// call opcode registered as custom
				LOGL(LOG_PRIORITY_OPCODE_ID, "%s custom opcode %04X", &m_acName, op);
				eOpcodeResult result = opcodes::Definitions[op](this);
				*game.Scripts.pNumOpcodesExecuted += 1;
				return result;
		} else if (op >= opcodes::CUSTOM_START_ID) {
				// if opcode isn't registered as custom, but has custom opcode's ID
				LOGL(LOG_PRIORITY_ALWAYS, "Error (incorrect opcode): %s, %04X", &m_acName, op);
				return OR_UNDEFINED;
		} else {
				// call default opcode
				LOGL(LOG_PRIORITY_OPCODE_ID, "%s opcode %04X", &m_acName, op);
				eOpcodeResult result = game.Scripts.apfOpcodeHandlers[op / 100](this, op);
				*game.Scripts.pNumOpcodesExecuted += 1;
				return result;
		}
}

void
Script::CollectParameters(uint* pIp, short numParams)
{
		for (short i = 0; i < numParams; i++) {
				ScriptParamType* paramType = (ScriptParamType*)&game.Scripts.pScriptSpace[*pIp];
				*pIp += 1;

				switch (paramType->type) {
				case PARAM_TYPE_INT32:
						game.Scripts.pScriptParams[i].nVar = *(int*)&game.Scripts.pScriptSpace[*pIp];
						*pIp += 4;
						break;
				case PARAM_TYPE_GVAR:
						game.Scripts.pScriptParams[i].nVar = *(int*)&game.Scripts.pScriptSpace[*(ushort*)&game.Scripts.pScriptSpace[*pIp]];
						*pIp += 2;
						break;
				case PARAM_TYPE_LVAR:
						game.Scripts.pScriptParams[i].nVar = m_aLVars[*(ushort*)&game.Scripts.pScriptSpace[*pIp]].nVar;
						*pIp += 2;
						break;
				case PARAM_TYPE_INT8:
						game.Scripts.pScriptParams[i].nVar = *(char*)&game.Scripts.pScriptSpace[*pIp];
						*pIp += 1;
						break;
				case PARAM_TYPE_INT16:
						game.Scripts.pScriptParams[i].nVar = *(short*)&game.Scripts.pScriptSpace[*pIp];
						*pIp += 2;
						break;
				case PARAM_TYPE_FLOAT:
						if (game.IsGta3()) {
								game.Scripts.pScriptParams[i].fVar = *(short*)&game.Scripts.pScriptSpace[*pIp] / 16.0f;
								*pIp += 2;
								break;
						} else {
								game.Scripts.pScriptParams[i].fVar = *(float*)&game.Scripts.pScriptSpace[*pIp];
								*pIp += 4;
								break;
						}
				case PARAM_TYPE_STRING:
						if (!paramType->processed) {
								uchar length = game.Scripts.pScriptSpace[*pIp];
								std::memcpy(&game.Scripts.pScriptSpace[*pIp + 1], &game.Scripts.pScriptSpace[*pIp], length);
								(game.Scripts.pScriptSpace[*pIp] + length) = '\0';
								paramType->processed = true;
						}

						game.Scripts.pScriptParams[i].szVar = &game.Scripts.pScriptSpace[*pIp];
						*pIp += std::strlen(&game.Scripts.pScriptSpace[*pIp]) + 1;
						break;
				default:
						*pIp -= 1;
						game.Scripts.pScriptParams[i].szVar = &game.Scripts.pScriptSpace[*pIp];
						*pIp += KEY_LENGTH_IN_SCRIPT;
						break;
				}
		}
}

int
Script::CollectNextParameterWithoutIncreasingPC(uint ip)
{
		ScriptParamType* paramType = (ScriptParamType*)&game.Scripts.pScriptSpace[ip];
		ip += 1;

		switch (paramType->type) {
		case PARAM_TYPE_INT32:
				return *(int*)&game.Scripts.pScriptSpace[ip];
		case PARAM_TYPE_GVAR:
				return *(int*)&game.Scripts.pScriptSpace[*(ushort*)&game.Scripts.pScriptSpace[ip]];
		case PARAM_TYPE_LVAR:
				return m_aLVars[*(ushort*)&game.Scripts.pScriptSpace[ip]].nVar;
		case PARAM_TYPE_INT8:
				return *(char*)&game.Scripts.pScriptSpace[ip];
		case PARAM_TYPE_INT16:
				return *(short*)&game.Scripts.pScriptSpace[ip];
		case PARAM_TYPE_FLOAT:
				if (game.IsGta3())
						return (int)(*(short*)&game.Scripts.pScriptSpace[ip] / 16.0f);
				else
						return *(int*)&game.Scripts.pScriptSpace[ip];
		case PARAM_TYPE_STRING:
				if (!paramType->processed) {
						uchar length = game.Scripts.pScriptSpace[ip];
						std::memcpy(&game.Scripts.pScriptSpace[ip + 1], &game.Scripts.pScriptSpace[ip], length);
						(game.Scripts.pScriptSpace[ip] + length) = '\0';
						paramType->processed = true;
				}

				return (int)&game.Scripts.pScriptSpace[ip]; // string address
		default:
				return -1;
		}
}

void
Script::StoreParameters(short numParams)
{
		game.Scripts.pfStoreParameters(this, &m_nIp, numParams);
}

ScriptParamType
Script::GetNextParamType()
{
		return ((ScriptParamType*)&game.Scripts.pScriptSpace[m_nIp])->type;
}

void*
Script::GetPointerToScriptVariable()
{
		return game.Scripts.pfGetPointerToScriptVariable(this, &m_nIp, 1);
}

void
Script::UpdateCompareFlag(bool result)
{
		game.Scripts.pfUpdateCompareFlag(this, result);
}

void
Script::ReadShortString(char* out)
{
		std::strncpy(out, &game.Scripts.pScriptSpace[m_nIp], KEY_LENGTH_IN_SCRIPT);
		m_nIp += KEY_LENGTH_IN_SCRIPT;
}

void
Script::JumpTo(int address)
{
		// negated address is a hack that lets us tell custom and mission scripts from regular ones
		if (address >= 0) {
				m_nIp = address;
		} else {
				if (m_bIsCustom)
						m_nIp = (uint)(m_pCodeData - game.Scripts.pScriptSpace) + (-address); // see Script ctor for details
				else
						m_nIp = game.kMainSize + (-address);
		}
}
