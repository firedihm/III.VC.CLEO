#include "Game.h"
#include "Log.h"
#include "OpcodesSystem.h"
#include "Script.h"

#include <cstring>
#include <fstream>

CCustomScript::CCustomScript() : m_pCodeData(nullptr), m_nBaseIp(0), m_bIsCustom(true), m_bIsPersistent(false), m_nLastPedSearchIndex(0), m_nLastVehicleSearchIndex(0), m_nLastObjectSearchIndex(0),
								 m_pCleoArray(new ScriptParam[CLEO_ARRAY_SIZE]), m_pCleoCallStack(nullptr) {}

CCustomScript::~CCustomScript()
{
		while (m_pCleoCallStack)
				PopStackFrame();

		delete[] m_pCleoArray;
		delete[] m_pCodeData;
}

void*
CCustomScript::StoreCache(std::any&& obj)
{
		(ObjectCache*)(m_pObjectCache)->push_front(std::move(obj));
}

void
CCustomScript::ClearCache(void* obj)
{
		for (Cache* previous = nullptr, current = *head; current; previous = current, current = current->next) {
				if (current->data == data) {
						if (previous)
								previous->next = current->next;
						else
								*head = current->next;

						if (head == &m_pFileSearchHandles)
								delete (fs::directory_iterator*)current->data;
						else if (head == &m_pOpenedFiles)
								delete (std::fstream*)current->data;
						else
								delete current->data;

						delete current;
						return;
				}
		}
}

void
CCustomScript::PushStackFrame()
{
		StackFrame* frame = new StackFrame();
		frame->next = m_pCleoCallStack;
		m_pCleoCallStack = frame;

		std::memcpy(&frame->vars, &m_aLVars, sizeof(frame->vars));

		frame->retAddr = m_nIp;
}

void
CCustomScript::PopStackFrame()
{
		m_nIp = m_pCleoCallStack->retAddr;

		std::memcpy(&m_aLVars, &m_pCleoCallStack->vars, sizeof(m_aLVars));

		StackFrame* head_next = m_pCleoCallStack->next;
		delete m_pCleoCallStack;
		m_pCleoCallStack = head_next;
}

Script::Script(const char* filepath)
{
		std::ifstream file(filepath, std::ios::binary);

		size_t filesize = file.ignore(size_t(-1) >> 1).gcount();
		if (!file || !filesize)
				throw "File is empty or corrupt";

		m_pCodeData = new uchar[filesize];
		m_nIp = m_nBaseIp = (uint)(m_pCodeData - game.Scripts.pScriptSpace);
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
		m_pCleoArray = new ScriptParam[CLEO_ARRAY_SIZE];
}

eOpcodeResult
Script::ProcessOneCommand()
{
		// highest bit of opcode denotes notFlag: reversing conditional result
		ushort op = *(ushort*)&game.Scripts.pScriptSpace[m_nIp];
		m_bNotFlag = (op & 0x8000) ? true : false;
		op &= 0x7FFF;
		m_nIp += 2;

		if (Opcodes::functions[op]) { // call opcode registered as custom
				LOGL(LOG_PRIORITY_OPCODE_ID, "%s custom opcode %04X", &m_acName, op);
				eOpcodeResult result = Opcodes::functions[op](this);
				*game.Scripts.pNumOpcodesExecuted += 1;
				return result;
		} else if (op >= CUSTOM_OPCODE_START_ID) { // if opcode isn't registered as custom, but has custom opcode's ID
				LOGL(LOG_PRIORITY_ALWAYS, "Error (incorrect opcode): %s, %04X", &m_acName, op);
				Error("Incorrect opcode ID: %04X", op);
				return OR_UNDEFINED;
		} else { // call default opcode
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
Script::Store(short numParams)
{
		game.Scripts.pfStoreParameters(this, &m_nIp, numParams);
}

eParamType
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
						m_nIp = m_nBaseIp + (-address);
				else
						m_nIp = game.kMainSize + (-address);
		}
}
