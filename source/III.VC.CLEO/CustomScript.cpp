#include "CustomScript.h"
#include "Game.h"
#include "Log.h"
#include "OpcodesSystem.h"
#include "ScriptManager.h"

#include <cstring>
#include <ifstream>

CScript::CScript(char* filepath) : m_pNext(nullptr), m_pPrev(nullptr), m_acName(0), m_dwIp(0), m_aGosubAddr(0), m_nCurrentGosub(0), m_bIsCustom(true), _pad(0),
								   m_aLVars(0), m_bIsActive(false), m_bCondResult(false), m_bIsMission(false), m_bAwake(false), m_dwWakeTime(0), m_wIfOp(0),
								   m_bNotFlag(false), m_bDeathArrestCheckEnabled(true), m_bWastedOrBusted(false), m_bMissionFlag(false), m_bIsPersistent(false), m_bToBeReloaded(false),
								   m_nLastPedSearchIndex(0), m_nLastVehicleSearchIndex(0), m_nLastObjectSearchIndex(0), _padd(0),
								   m_pCodeData(nullptr), m_dwBaseIp(0), m_pScmFunction(nullptr), m_pNextCustom(nullptr), m_pPrevCustom(nullptr), m_pLocalArray(nullptr)
{
		std::ifstream file(filepath, std::ios_base::in || std::ios_base::binary)
		if (!file)
				throw std::invalid_argument("File not found.");

		size_t filesize = file.seekg(0, std::ios::end).tellg();
		if (!file || !filesize)
				throw std::length_error("File is empty or corrupt.");

		m_pCodeData = new char[filesize];
		m_dwIp = m_dwBaseIp = (uint)m_pCodeData - (uint)game.Scripts.Space;

		file.seekg(0, std::ios::beg).read(m_pCodeData, filesize);
		if (!file) {
				delete[] m_pCodeData;
				throw std::runtime_error("File is corrupt.");
		}

		std::strncpy(m_acName, &std::strrchr(filepath, '\\')[1], KEY_LENGTH_IN_SCRIPT - 1); // keep '\0' from initializer
		m_pLocalArray = new tScriptVar[0xFF];
}

CScript::~CScript()
{
		ScmFunction* scmf = m_pScmFunction;
		while(scmf) {
				ScmFunction* prev = scmf->prev;
				delete scmf;
				scmf = prev;
		}

		delete[] m_pLocalArray;
}

void
CScript::AddToCustomList(CScript** list)
{
		// push_front()
		CScript* list_head = *list;
		m_pNextCustom = list_head;
		m_pPrevCustom = nullptr;

		if (list_head)
				list_head->m_pPrevCustom = this;
		list_head = this;
}

void
CScript::RemoveFromCustomList(CScript** list)
{
		if (m_pPrevCustom)
				m_pPrevCustom->m_pNextCustom = m_pNextCustom;
		else // head of a list
				*list = m_pNextCustom;
		if (m_pNextCustom)
				m_pNextCustom->m_pPrevCustom = m_pPrevCustom;
}

eOpcodeResult
CScript::ProcessOneCommand()
{
		// highest bit of opcode denotes notFlag: reversing conditional result
		ushort op = *(ushort*)&game.Scripts.Space[m_dwIp];
		m_bNotFlag = (op & 0x8000) ? true : false;
		op &= 0x7FFF;
		m_dwIp += 2;

		if (Opcodes::functions[op]) { // call opcode registered as custom
				LOGL(LOG_PRIORITY_OPCODE_ID, "%s custom opcode %04X", m_acName, op);
				eOpcodeResult result = Opcodes::functions[op](this);
				*game.Scripts.pNumOpcodesExecuted += 1;
				return result;
		} else if (op >= CUSTOM_OPCODE_START_ID) { // if opcode isn't registered as custom, but has custom opcode's ID
				LOGL(LOG_PRIORITY_ALWAYS, "Error (incorrect opcode): %s, %04X", m_acName, op);
				Error("Incorrect opcode ID: %04X", op);
				return OR_UNDEFINED;
		} else { // call default opcode
				LOGL(LOG_PRIORITY_OPCODE_ID, "%s opcode %04X", m_acName, op);
				eOpcodeResult result = game.Scripts.OpcodeHandlers[op / 100](this, op);
				*game.Scripts.pNumOpcodesExecuted += 1;
				return result;
		}
}

void
CScript::PushStackFrame()
{
		// push_front()
		StackFrame* frame = new StackFrame();
		frame->prev = m_pStackFrameHead;
		m_pStackFrameHead = frame;

		std::memcpy(frame->vars, m_aLVars, sizeof(frame->vars));

		frame->retAddr = m_dwIp;
}

void
CScript::PopStackFrame()
{
		m_dwIp = m_pStackFrameHead->retAddr;

		std::memcpy(m_aLVars, m_pStackFrameHead->vars, sizeof(m_aLVars));

		// pop_front()
		StackFrame* prev = m_pStackFrameHead->prev;
		delete m_pStackFrameHead;
		m_pStackFrameHead = prev;
}

eParamType
CScript::GetNextParamType()
{
		return ((tParamType*)&game.Scripts.Space[m_dwIp])->type;
}

void*
CScript::GetPointerToScriptVariable()
{
		return game.Scripts.GetPointerToScriptVariable(this, &m_dwIp, 1);
}

void
CScript::UpdateCompareFlag(bool result)
{
		game.Scripts.UpdateCompareFlag(this, result);
}

void
CScript::ReadShortString(char* out)
{
		std::strncpy(out, &game.Scripts.Space[m_dwIp], KEY_LENGTH_IN_SCRIPT);
		m_dwIp += KEY_LENGTH_IN_SCRIPT;
}

void
CScript::JumpTo(int address)
{
		// negated address is a hack that lets us tell custom and mission scripts from regular ones
		if (address >= 0)
				m_dwIp = address;
		else {
				if (m_bIsCustom)
						m_dwIp = m_dwBaseIp + (-address);
				else { // mission script; loaded straight after main script space
					#if CLEO_VC
						m_dwIp = 0x370E8 + (-address);
					#else
						m_dwIp = 0x20000 + (-address);
					#endif
				}
		}
}

void
CScript::Collect(uint numParams)
{
		Collect(&m_dwIp, numParams);
}

void
CScript::Collect(uint* pIp, short numParams)
{
		for (short i = 0; i < numParams; i++) {
				tParamType* paramType = (tParamType*)&game.Scripts.Space[*pIp];
				*pIp += 1;

				switch (paramType->type) {
					case PARAM_TYPE_INT32:
						game.Scripts.Params[i].nVar = *(int*)&game.Scripts.Space[*pIp];
						*pIp += 4;
						break;
					case PARAM_TYPE_GVAR:
						game.Scripts.Params[i].nVar = *(int*)&game.Scripts.Space[*(ushort*)&game.Scripts.Space[*pIp]];
						*pIp += 2;
						break;
					case PARAM_TYPE_LVAR:
						game.Scripts.Params[i].nVar = m_aLVars[*(ushort*)&game.Scripts.Space[*pIp]].nVar;
						*pIp += 2;
						break;
					case PARAM_TYPE_INT8:
						game.Scripts.Params[i].nVar = *(char*)&game.Scripts.Space[*pIp];
						*pIp += 1;
						break;
					case PARAM_TYPE_INT16:
						game.Scripts.Params[i].nVar = *(short*)&game.Scripts.Space[*pIp];
						*pIp += 2;
						break;
					case PARAM_TYPE_FLOAT:
					#if CLEO_VC
						game.Scripts.Params[i].nVar = *(int*)&game.Scripts.Space[*pIp];
						*pIp += 4;
						break;
					#else
						game.Scripts.Params[i].fVar = (float)(*(short*)&game.Scripts.Space[*pIp]) / 16.0f;
						*pIp += 2;
						break;
					#endif
					case PARAM_TYPE_STRING:
						if (!paramType->processed) {
								uchar length = *(uchar*)&game.Scripts.Space[*pIp];
								std::memcpy(&game.Scripts.Space[*pIp + 1], &game.Scripts.Space[*pIp], length);
								*((char*)&game.Scripts.Space[*pIp] + length) = '\0';
								paramType->processed = true;
						}

						game.Scripts.Params[i].cVar = &game.Scripts.Space[*pIp];
						*pIp += std::strlen(&game.Scripts.Space[*pIp]) + 1;
						break;
					default:
						*pIp -= 1;
						game.Scripts.Params[i].cVar = &game.Scripts.Space[*pIp];
						*pIp += KEY_LENGTH_IN_SCRIPT;
						break;
				}
		}
}

int
CScript::CollectNextWithoutIncreasingPC(uint ip)
{
		tParamType* paramType = (tParamType*)&game.Scripts.Space[ip];
		ip += 1;

		switch (paramType->type) {
			case PARAM_TYPE_INT32:
				return *(int*)&game.Scripts.Space[ip];
			case PARAM_TYPE_GVAR:
				return *(int*)&game.Scripts.Space[*(ushort*)&game.Scripts.Space[ip]];
			case PARAM_TYPE_LVAR:
				return m_aLVars[*(ushort*)&game.Scripts.Space[ip]].nVar;
			case PARAM_TYPE_INT8:
				return *(char*)&game.Scripts.Space[ip];
			case PARAM_TYPE_INT16:
				return *(short*)&game.Scripts.Space[ip];
			case PARAM_TYPE_FLOAT:
			#if CLEO_VC
				return *(int*)&game.Scripts.Space[ip];
			#else
				float fParam = ((float)(*(short*)&game.Scripts.Space[ip]) / 16.0f);
				return static_cast<int>(fParam);
			#endif
			case PARAM_TYPE_STRING:
				if (!paramType->processed) {
						uchar length = *(uchar*)&game.Scripts.Space[ip];
						std::memcpy(&game.Scripts.Space[ip + 1], &game.Scripts.Space[ip], length);
						*((char*)&game.Scripts.Space[ip] + length) = '\0';
						paramType->processed = true;
				}

				return (int)&game.Scripts.Space[ip]; // pointer to string
			default:
				return -1;
		}
}

void
CScript::Store(uint numParams)
{
		game.Scripts.StoreParameters(this, &m_dwIp, numParams);
}
