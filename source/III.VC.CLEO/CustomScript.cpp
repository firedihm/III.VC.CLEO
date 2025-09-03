#include <ifstream>
#include <stdio.h>
#include <Windows.h>

#include "CustomScript.h"
#include "Game.h"
#include "Log.h"
#include "OpcodesSystem.h"
#include "ScriptManager.h"

CScript::CScript(char* filepath)
{
		memset(this, 0, sizeof(CScript));

		std::ifstream file(filepath, std::ios_base::in || std::ios_base::binary)
		if (!file)
				throw std::invalid_argument("File not found.");

		size_t filesize = file.seekg(0, std::ios::end).tellg();
		if (!file || !filesize)
				throw std::length_error("File is empty or corrupt.");

		m_pCodeData = new char[filesize];
		m_dwIp = m_dwBaseIp = (unsigned int)m_pCodeData - (unsigned int)game.Scripts.Space;

		file.seekg(0, std::ios::beg).read(m_pCodeData, filesize);
		if (!file) {
				delete[] m_pCodeData;
				throw std::runtime_error("File is corrupt.");
		}

		strncpy(m_acName, &strrchr(filepath, '\\')[1], 7);
		m_nScriptType = SCRIPT_TYPE_CUSTOM;
		m_bDeathArrestCheckEnabled = true;
		m_bMissionFlag = false;
		m_pLocalArray = new tScriptVar[0xFF];
}

CScript::~CScript()
{
	ScmFunction *scmf = this->m_pScmFunction;
	while(scmf)
	{
		ScmFunction *prev = scmf->prev;
		delete scmf;
		scmf = prev;
	}
	this->m_pScmFunction = nullptr;
	delete[] this->m_pLocalArray;
	this->m_pLocalArray = nullptr;
}

void CScript::ReadShortString(char *out)
{
	strncpy(out, &game.Scripts.Space[this->m_dwIp], 7);
	out[7] = '\0';
	this->m_dwIp += 8;
}

void CScript::AddToCustomList(CScript **list)
{
	this->m_pNextCustom = *list;
	this->m_pPrevCustom = 0;
	if(*list)
		(*list)->m_pPrevCustom = this;
	*list = this;
}

void CScript::RemoveFromCustomList(CScript **list)
{
	if(this->m_pPrevCustom)
		this->m_pPrevCustom->m_pNextCustom = this->m_pNextCustom;
	else
		*list = this->m_pNextCustom;
	if(this->m_pNextCustom)
		this->m_pNextCustom->m_pPrevCustom = this->m_pPrevCustom;
}

void CScript::JumpTo(int address)
{
	if(address >= 0)
		this->m_dwIp = address;
	else
	{
		if(this->m_nScriptType == SCRIPT_TYPE_CUSTOM)
			this->m_dwIp = this->m_dwBaseIp - address;
		else
		{
#if CLEO_VC
			this->m_dwIp = 0x370E8 - address;
#else
			this->m_dwIp = 0x20000 - address;
#endif
		}
	}
}

eParamType CScript::GetNextParamType()
{
	return ((tParamType *)&game.Scripts.Space[this->m_dwIp])->type;
}

void CScript::Collect(unsigned int numParams)
{
	this->Collect(&this->m_dwIp, numParams);
}

void CScript::Collect(unsigned int *pIp, unsigned int numParams)
{
	for(unsigned int i = 0; i < numParams; i++)
	{
		tParamType *paramType = (tParamType *)&game.Scripts.Space[*pIp];
		*pIp += 1;

		switch (paramType->type)
		{
		case PARAM_TYPE_FLOAT:
#if CLEO_VC
			game.Scripts.Params[i].nVar = *(int *)&game.Scripts.Space[*pIp];
			*pIp += 4;
			break;
#else
			game.Scripts.Params[i].fVar = (float)(*(short *)&game.Scripts.Space[*pIp]) / 16.0f;
			*pIp += 2;
			break;
#endif
		case PARAM_TYPE_INT32:
			game.Scripts.Params[i].nVar = *(int *)&game.Scripts.Space[*pIp];
			*pIp += 4;
			break;
		case PARAM_TYPE_GVAR:
			game.Scripts.Params[i].nVar = *(int *)&game.Scripts.Space[*(unsigned short *)&game.Scripts.Space[*pIp]];
			*pIp += 2;
			break;
		case PARAM_TYPE_LVAR:
			game.Scripts.Params[i].nVar = this->m_aLVars[*(unsigned short *)&game.Scripts.Space[*pIp]].nVar;
			*pIp += 2;
			break;
		case PARAM_TYPE_INT8:
			game.Scripts.Params[i].nVar = *(char *)&game.Scripts.Space[*pIp];
			*pIp += 1;
			break;
		case PARAM_TYPE_INT16:
			game.Scripts.Params[i].nVar = *(short *)&game.Scripts.Space[*pIp];
			*pIp += 2;
			break;
		case PARAM_TYPE_STRING:
			if (!paramType->processed)
			{
				unsigned char length = *(unsigned char *)&game.Scripts.Space[*pIp];
				*std::copy_n(&game.Scripts.Space[*pIp + 1], length, &game.Scripts.Space[*pIp]) = 0;
				paramType->processed = true;
			}

			game.Scripts.Params[i].cVar = &game.Scripts.Space[*pIp];
			*pIp += (strlen(&game.Scripts.Space[*pIp]) + 1);
			break;

		default:
			*pIp -= 1;
			game.Scripts.Params[i].cVar = &game.Scripts.Space[*pIp];
			*pIp += 8;
			break;
		}
	}
}

int CScript::CollectNextWithoutIncreasingPC(unsigned int ip)
{
	tParamType *paramType = (tParamType *)&game.Scripts.Space[ip];
	ip += 1;

	switch (paramType->type)
	{
	case PARAM_TYPE_FLOAT:
#if CLEO_VC
		return *(int *)&game.Scripts.Space[ip];
#else
		{
		auto fParam = ((float)(*(short *)&game.Scripts.Space[ip]) / 16.0f);
		return static_cast<int>(*(float*)&fParam);
		}
#endif
	case PARAM_TYPE_INT32:
		return *(int *)&game.Scripts.Space[ip];
	case PARAM_TYPE_GVAR:
		return *(int *)&game.Scripts.Space[*(unsigned short *)&game.Scripts.Space[ip]];
	case PARAM_TYPE_LVAR:
		return this->m_aLVars[*(unsigned short *)&game.Scripts.Space[ip]].nVar;
	case PARAM_TYPE_INT8:
		return *(char *)&game.Scripts.Space[ip];
	case PARAM_TYPE_INT16:
		return *(short *)&game.Scripts.Space[ip];
	case PARAM_TYPE_STRING:
		if (!paramType->processed)
		{
			unsigned char length = *(unsigned char *)&game.Scripts.Space[ip];
			*std::copy_n(&game.Scripts.Space[ip + 1], length, &game.Scripts.Space[ip]) = 0;
			paramType->processed = true;
		}

		return (int)&game.Scripts.Space[ip];
		break;

	default:
		return -1;
	}
}

void CScript::Store(unsigned int numParams)
{
	game.Scripts.StoreParameters(this, &this->m_dwIp, numParams);
}

void CScript::UpdateCompareFlag(bool result)
{
	game.Scripts.UpdateCompareFlag(this, result);
}

void *CScript::GetPointerToScriptVariable()
{
	return game.Scripts.GetPointerToScriptVariable(this, &this->m_dwIp, 1);
}

eOpcodeResult CScript::ProcessOneCommand()
{
	*game.Scripts.pNumOpcodesExecuted += 1;
	unsigned short id = *(unsigned short *)&game.Scripts.Space[this->m_dwIp] & 0x7FFF;
	if(*(unsigned short *)&game.Scripts.Space[this->m_dwIp] & 0x8000)
		this->m_bNotFlag = true;
	else
		this->m_bNotFlag = false;
	this->m_dwIp += 2;
	// check for custom opcodes here
	if(Opcodes::functions[id])
	{
		// call custom opcode
		LOGL(LOG_PRIORITY_OPCODE_ID, "%s custom opcode %04X", this->m_acName, id);
		return Opcodes::functions[id](this);
	}
	else if(id >= CUSTOM_OPCODE_START_ID)
	{
		LOGL(LOG_PRIORITY_ALWAYS, "Error (incorrect opcode): %s, %04X", this->m_acName, id);
		Error("Incorrect opcode ID: %04X", id);
		return OR_UNDEFINED;
	}
	// call default opcode
	LOGL(LOG_PRIORITY_OPCODE_ID, "%s opcode %04X", this->m_acName, id);
	eOpcodeResult result = game.Scripts.OpcodeHandlers[id / 100](this, id);
	return result;
}
