#include "Game.h"
#include "Log.h"
#include "OpcodesSystem.h"
#include "Script.h"

#include <cstring>
#include <ifstream>

Script::Script()
{
		Init();
}

Script::Script(const char* filepath)
{
		Init();

		std::ifstream file(filepath, std::ios_base::in | std::ios_base::binary);

		size_t filesize = file.seekg(0, std::ios::end).tellg();
		if (!file || !filesize)
				throw "File is empty or corrupt";

		m_pCodeData = new uchar[filesize];
		m_dwIp = m_dwBaseIp = (uint)m_pCodeData - (uint)game.Scripts.pScriptSpace;
		file.seekg(0, std::ios::beg).read(m_pCodeData, filesize);
}

Script::~Script()
{
		while (m_pCleoCallStack)
				PopStackFrame();

		delete[] m_pCodeData;
		delete[] m_pLocalArray;
}

void
Script::Init()
{
		std::memset(this, 0, sizeof(Script));
		std::strcpy(&m_acName, "noname");
		m_bDeathArrestCheckEnabled = true;

		m_pLocalArray = new ScriptParam[0xFF];
		std::memset(m_pLocalArray, 0, sizeof(ScriptParam) * 0xFF);
}

void
Script::AddToCustomList(Script** list)
{
		// push_front()
		Script* first = *list;
		m_pNextCustom = first;
		m_pPrevCustom = nullptr;

		if (first)
				first->m_pPrevCustom = (CCustomScript*)this;

		first = this;
}

void
Script::RemoveFromCustomList(Script** list)
{
		if (m_pPrevCustom)
				m_pPrevCustom->m_pNextCustom = m_pNextCustom;
		else
				*list = (Script*)m_pNextCustom;

		if (m_pNextCustom)
				m_pNextCustom->m_pPrevCustom = m_pPrevCustom;
}

void
Script::PushStackFrame()
{
		// push_front()
		StackFrame* frame = new StackFrame();
		frame->prev = m_pCleoCallStack;
		m_pCleoCallStack = frame;

		std::memcpy(&frame->vars, &m_aLVars, sizeof(frame->vars));

		frame->retAddr = m_dwIp;
}

void
Script::PopStackFrame()
{
		m_dwIp = m_pCleoCallStack->retAddr;

		std::memcpy(&m_aLVars, &m_pCleoCallStack->vars, sizeof(m_aLVars));

		// pop_front()
		StackFrame* prev = m_pCleoCallStack->prev;
		delete m_pCleoCallStack;
		m_pCleoCallStack = prev;
}

eOpcodeResult
Script::ProcessOneCommand()
{
		// highest bit of opcode denotes notFlag: reversing conditional result
		ushort op = *(ushort*)&game.Scripts.pScriptSpace[m_dwIp];
		m_bNotFlag = (op & 0x8000) ? true : false;
		op &= 0x7FFF;
		m_dwIp += 2;

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

eParamType
Script::GetNextParamType()
{
		return ((ScriptParamType*)&game.Scripts.pScriptSpace[m_dwIp])->type;
}

void*
Script::GetPointerToScriptVariable()
{
		return game.Scripts.pfGetPointerToScriptVariable(this, &m_dwIp, 1);
}

void
Script::UpdateCompareFlag(bool result)
{
		game.Scripts.pfUpdateCompareFlag(this, result);
}

void
Script::ReadShortString(char* out)
{
		std::strncpy(out, &game.Scripts.pScriptSpace[m_dwIp], KEY_LENGTH_IN_SCRIPT);
		m_dwIp += KEY_LENGTH_IN_SCRIPT;
}

void
Script::JumpTo(int address)
{
		// negated address is a hack that lets us tell custom and mission scripts from regular ones
		if (address >= 0)
				m_dwIp = address;
		else {
				if (m_bIsCustom)
						m_dwIp = m_dwBaseIp + (-address);
				else
						m_dwIp = game.kMainSize + (-address);
		}
}

void
Script::Collect(short numParams)
{
		CollectParameters(&m_dwIp, numParams);
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
		game.Scripts.pfStoreParameters(this, &m_dwIp, numParams);
}
