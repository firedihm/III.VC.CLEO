#pragma once

#ifndef CLEOAPI
#define CLEOAPI __declspec(dllexport)
#endif

#include "ScmFunction.h"

#define KEY_LENGTH_IN_SCRIPT (8)

enum eOpcodeResult : char
{
	OR_CONTINUE = 0,
	OR_TERMINATE = 1,
	OR_UNDEFINED = -1
};

typedef unsigned long DWORD;

union tScriptVar
{
	int nVar;
	float fVar;
	char* cVar;
	void* pVar;
	DWORD dVar;
};

enum eParamType : unsigned char
{
	PARAM_TYPE_END_OF_PARAMS = 0,
	PARAM_TYPE_INT32 = 1,
	PARAM_TYPE_GVAR = 2,
	PARAM_TYPE_LVAR = 3,
	PARAM_TYPE_INT8 = 4,
	PARAM_TYPE_INT16 = 5,
	PARAM_TYPE_FLOAT = 6,
	PARAM_TYPE_STRING = 14
};

struct tParamType
{
	eParamType type : 7; // eParamType
	bool processed : 1; // did we process long string already
};

class CScript
{
	public:
		/* 0x00 */ CScript* m_pNext;
		/* 0x04 */ CScript* m_pPrev;
		/* 0x08 */ char m_acName[KEY_LENGTH_IN_SCRIPT];
		/* 0x10 */ uint m_dwIp;
		/* 0x14 */ uint m_aGosubAddr[6];
		/* 0x2C */ ushort m_nCurrentGosub;
		/* 0x2E */ bool m_bIsCustom;
		/* 0x2F */ bool m_bIsPersistent;
		/* 0x30 */ tScriptVar m_aLVars[18];
	#if CLEO_VC
		/* 0x78 */ bool m_bIsActive; 
		/* 0x79 */ bool m_bCondResult; 
		/* 0x7A */ bool m_bIsMission;
	#else
		/* 0x78 */ bool m_bCondResult;
		/* 0x79 */ bool m_bIsMission;
		/* 0x7A */ bool m_bIsActive;
	#endif
		/* 0x7B */ bool m_bAwake;
		/* 0x7C */ uint m_dwWakeTime;
		/* 0x80 */ ushort m_wIfOp;
		/* 0x82 */ bool m_bNotFlag;
		/* 0x83 */ bool m_bDeathArrestCheckEnabled;
		/* 0x84 */ bool m_bWastedOrBusted;
		/* 0x85 */ bool m_bMissionFlag;
		/* 0x86 */ short _pad;

		/* Custom data */
		/* 0x88 */ uint m_nLastPedSearchIndex;
		/* 0x8C */ uint m_nLastVehicleSearchIndex;
		/* 0x90 */ uint m_nLastObjectSearchIndex;
		/* 0x94 */ uint _padd;
		/* 0x98 */ char* m_pCodeData;
		/* 0x9C */ uint m_dwBaseIp;
		/* 0xA0 */ ScmFunction* m_pScmFunction;
		/* 0xA4 */ CScript* m_pNextCustom;
		/* 0xA8 */ CScript* m_pPrevCustom;
		/* 0xAC */ tScriptVar* m_pLocalArray;

		CScript(char* filepath);
		~CScript();

		void AddToCustomList(CScript** list);
		void RemoveFromCustomList(CScript** list);

	// exports
	CLEOAPI void Collect(uint numParams);

	CLEOAPI void Collect(uint* pIp, unsigned int numParams);

	CLEOAPI int CollectNextWithoutIncreasingPC(uint ip);

	CLEOAPI eParamType GetNextParamType() { return ((tParamType*)&game.Scripts.Space[m_dwIp])->type; }

	CLEOAPI void Store(uint numParams) { game.Scripts.StoreParameters(this, &m_dwIp, numParams); }

	CLEOAPI void ReadShortString(char* out);

	CLEOAPI void UpdateCompareFlag(bool result) { game.Scripts.UpdateCompareFlag(this, result); }

	CLEOAPI void* GetPointerToScriptVariable() { return game.Scripts.GetPointerToScriptVariable(this, &m_dwIp, 1); }

	CLEOAPI void JumpTo(int address);

	eOpcodeResult ProcessOneCommand();
};

static_assert(sizeof(tScriptVar) == 0x04, "tScriptVar size mismatch");
static_assert(sizeof(CScript) == 0xB0, "CScript size mismatch");
