#include "Fxt.h"
#include "Game.h"
#include "Memory.h"
#include "Opcodes.h"
#include "Script.h"
#include "ScriptManager.h"

#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

// game prints messages through wchar_t*, so it must be static
wchar_t g_message[HELP_MSG_LENGTH];

ScriptParam g_cleo_shared_vars[0xFFFF];

eOpcodeResult __stdcall
STREAM_CUSTOM_SCRIPT(Script* script)
{
		script->CollectParameters(1);

		fs::path filepath = fs::path(game::RootDirName) / "CLEO" / game::ScriptParams[0].szVar;

		Script* new_script = script_mgr::start_script(filepath.string().c_str());

		int collected = script->CollectParameters(-1);
		std::memcpy(new_script->local_vars_, game::ScriptParams, collected * sizeof(ScriptParam));

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
IS_BUTTON_PRESSED_WITH_SENSITIVITY(Script* script)
{
		script->CollectParameters(2);

		script->UpdateCompareFlag(*(game::pPadNewState + game::ScriptParams[0].nVar) == (short)game::ScriptParams[1].nVar);

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
EMULATE_BUTTON_PRESS_WITH_SENSITIVITY(Script* script)
{
		script->CollectParameters(2);

		*(game::pPadNewState + game::ScriptParams[0].nVar) = (short)game::ScriptParams[1].nVar;

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
IS_CAMERA_IN_WIDESCREEN_MODE(Script* script)
{
		script->UpdateCompareFlag(*game::pWideScreenOn);

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
GET_WEAPONTYPE_MODEL(Script* script)
{
		script->CollectParameters(1);

		game::ScriptParams[0].nVar = game::ModelForWeapon(game::ScriptParams[0].nVar);
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
GET_WEAPONTYPE_FOR_MODEL(Script* script)
{
		script->CollectParameters(1);
		int weapon_mi = game::ScriptParams[0].nVar;

		if (weapon_mi < 0)
			weapon_mi = game::UsedObjectArray[-weapon_mi].index;

		// CPickups::WeaponForModel() exits only in III, so we do this manually for VC compatability
		int result = -1;
		for (size_t i = 0; i < 37; ++i) {
				if (weapon_mi == game::ModelForWeapon(i))
						break;
		}

		game::ScriptParams[0].nVar = result;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
SET_MEMORY_OFFSET(Script* script)
{
		script->CollectParameters(3);

		int value = game::ScriptParams[1].nVar - (game::ScriptParams[0].nVar + 4);
		memory::write(game::ScriptParams[0].pVar, &value, sizeof(value), game::ScriptParams[2].nVar);

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
GET_CURRENT_WEATHER(Script* script)
{
		game::ScriptParams[0].nVar = *game::pOldWeatherType;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
DISPLAY_TEXT_STRING(Script* script)
{
		script->CollectParameters(3);

		game::IntroTextLines[*game::pNumberOfIntroTextLinesThisFrame].m_fAtX = game::ScriptParams[0].fVar;
		game::IntroTextLines[*game::pNumberOfIntroTextLinesThisFrame].m_fAtY = game::ScriptParams[1].fVar;
		std::swprintf(game::IntroTextLines[*game::pNumberOfIntroTextLinesThisFrame].text, INTRO_TEXT_LENGTH, L"%hs", game::ScriptParams[2].szVar);
		(*game::pNumberOfIntroTextLinesThisFrame)++;

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
DISPLAY_TEXT_FORMATTED(Script* script)
{
		script->CollectParameters(3);
		float x = game::ScriptParams[0].fVar;
		float y = game::ScriptParams[1].fVar;

		char fmt[INTRO_TEXT_LENGTH];
		script->format_string(fmt, game::ScriptParams[2].szVar);

		game::IntroTextLines[*game::pNumberOfIntroTextLinesThisFrame].m_fAtX = x;
		game::IntroTextLines[*game::pNumberOfIntroTextLinesThisFrame].m_fAtY = y;
		std::swprintf(game::IntroTextLines[*game::pNumberOfIntroTextLinesThisFrame].text, INTRO_TEXT_LENGTH, L"%hs", fmt);
		(*game::pNumberOfIntroTextLinesThisFrame)++;

		return OR_CONTINUE;
};

eOpcodeResult __stdcall
PLAY_ANIMATION(Script* script)
{
		script->CollectParameters(4);
		uchar* ped = (uchar*)game::PedPoolGetAt(*game::ppPedPool, 0, game::ScriptParams[0].nVar);
		int anim_group = game::ScriptParams[1].nVar;
		int anim = game::ScriptParams[2].nVar;
		float blend = game::ScriptParams[3].fVar;

		game::BlendAnimation(*(uchar**)(ped + 0x4C) /* CEntity::m_rwObject */, anim_group, anim, blend);

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
INT_ADD(Script* script)
{
		script->CollectParameters(2);

		game::ScriptParams[0].nVar = game::ScriptParams[0].nVar + game::ScriptParams[1].nVar;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
INT_SUB(Script* script)
{
		script->CollectParameters(2);

		game::ScriptParams[0].nVar = game::ScriptParams[0].nVar - game::ScriptParams[1].nVar;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
INT_MUL(Script* script)
{
		script->CollectParameters(2);

		game::ScriptParams[0].nVar = game::ScriptParams[0].nVar * game::ScriptParams[1].nVar;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
INT_DIV(Script* script)
{
		script->CollectParameters(2);

		game::ScriptParams[0].nVar = game::ScriptParams[0].nVar / game::ScriptParams[1].nVar;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
SET_CURRENT_DIRECTORY(Script* script)
{
		script->CollectParameters(1);

		fs::current_path(game::ScriptParams[0].nVar == 0 ? game::RootDirName :
				game::ScriptParams[0].nVar == 1 ? game::GetUserFilesFolder() : game::ScriptParams[0].szVar);

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
OPEN_FILE(Script* script)
{
		script->CollectParameters(2);

		// cppreference.com/w/cpp/io/basic_filebuf/open
		std::ios::openmode openmode(0);
		for (char* str = game::ScriptParams[1].szVar; *str != '\0'; ++str) {
				if (*str == 'r') {
						openmode |= std::ios::in;
				} else if (*str == 'w') {
						openmode |= std::ios::out | std::ios::trunc;
				} else if (*str == 'a') {
						openmode |= std::ios::out | std::ios::app;
				} else if (*str == '+') {
						openmode |= std::ios::in | std::ios::out;
				} else if (*str == 'b') {
						openmode |= std::ios::binary;
				} else if (*str == 'x') {
						openmode |= std::ios::noreplace;
				}
		}

		auto* file = new std::fstream(game::ScriptParams[0].szVar, openmode);
		script->register_object(file);

		script->UpdateCompareFlag(!file->fail());

		game::ScriptParams[0].pVar = file;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
CLOSE_FILE(Script* script)
{
		script->CollectParameters(1);

		script->delete_registered_object(game::ScriptParams[0].pVar);

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
GET_FILE_SIZE(Script* script)
{
		script->CollectParameters(1);
		auto* file = (std::fstream*)game::ScriptParams[0].pVar;
		auto saved_pos = file->tellg();

		game::ScriptParams[0].nVar = file->seekg(0, std::ios::beg).ignore(size_t(-1) >> 1).gcount();
		script->StoreParameters(1);

		file->seekg(saved_pos);

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
READ_FROM_FILE(Script* script)
{
		script->CollectParameters(3);
		auto* file = (std::fstream*)game::ScriptParams[0].pVar;

		file->read(game::ScriptParams[2].szVar, game::ScriptParams[1].nVar);

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
WRITE_TO_FILE(Script* script)
{
		script->CollectParameters(3);
		auto* file = (std::fstream*)game::ScriptParams[0].pVar;

		file->write(game::ScriptParams[2].szVar, game::ScriptParams[1].nVar);
		// script->UpdateCompareFlag(*file);

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
GOSUB_IF_FALSE(Script* script)
{
		script->CollectParameters(1);

		if (!script->cond_result()) {
				script->gosub_stack_[script->gosub_stack_pointer_++] = script->ip_;
				script->jump(game::ScriptParams[0].nVar);
		}

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
RETURN_IF_FALSE(Script* script)
{
		if (!script->cond_result())
				script->ip_ = script->gosub_stack_[--script->gosub_stack_pointer_];

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
LOAD_DYNAMIC_LIBRARY(Script* script)
{
		script->CollectParameters(1);

		game::ScriptParams[0].pVar = memory::load_library(game::ScriptParams[0].szVar);
		script->StoreParameters(1);

		script->UpdateCompareFlag(game::ScriptParams[0].pVar);

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
FREE_DYNAMIC_LIBRARY(Script* script)
{
		script->CollectParameters(1);

		memory::free_library(game::ScriptParams[0].pVar);

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
GET_DYNAMIC_LIBRARY_PROCEDURE(Script* script)
{
		script->CollectParameters(2);

		game::ScriptParams[0].pVar = memory::get_proc_address(game::ScriptParams[1].pVar, game::ScriptParams[0].szVar);
		script->StoreParameters(1);

		script->UpdateCompareFlag(game::ScriptParams[0].pVar);

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
IS_GAME_VERSION_ORIGINAL(Script* script)
{
		script->UpdateCompareFlag(game::version == game::Version::VC_1_0 || game::version == game::Version::III_1_0);

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
DOES_FILE_EXIST(Script* script)
{
		script->CollectParameters(1);

		script->UpdateCompareFlag(fs::exists(game::ScriptParams[0].szVar));

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
SET_CLEO_SHARED_VAR(Script* script)
{
		script->CollectParameters(2);

		g_cleo_shared_vars[game::ScriptParams[0].nVar].nVar = game::ScriptParams[1].nVar;

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
GET_CLEO_SHARED_VAR(Script* script)
{
		script->CollectParameters(1);

		game::ScriptParams[0].nVar = g_cleo_shared_vars[game::ScriptParams[0].nVar].nVar;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
GET_CAR_NUMBER_OF_GEARS(Script* script)
{
		uint offset_pHandling = script->is_III_ ? 0x128 : 0x120; // CVehicle::pHandling
		uint offset_Transmission = 0x34; // tHandlingData::Transmission
		uint offset_nNumberOfGears = 0x4A; // cTransmission::nNumberOfGears

		script->CollectParameters(1);

		uchar* vehicle = (uchar*)(game::VehiclePoolGetAt(*game::ppVehiclePool, 0, game::ScriptParams[0].nVar));
		uchar* handling = *(uchar**)(vehicle + offset_pHandling);
		uchar num_gears = *(uchar*)(handling + offset_Transmission + offset_nNumberOfGears);

		game::ScriptParams[0].nVar = num_gears;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
GET_CAR_CURRENT_GEAR(Script* script)
{
		uint offset_nCurrentGear = script->is_III_ ? 0x204 : 0x208; // CVehicle::m_nCurrentGear

		script->CollectParameters(1);

		uchar* vehicle = (uchar*)(game::VehiclePoolGetAt(*game::ppVehiclePool, 0, game::ScriptParams[0].nVar));
		uchar curr_gear = *(uchar*)(vehicle + offset_nCurrentGear);

		game::ScriptParams[0].nVar = curr_gear;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
IS_CAR_LIGHTS_ON(Script* script)
{
		uint offset_flags = script->is_III_ ? 0x1F5 : 0x1F9; // CVehicle::flags

		script->CollectParameters(1);

		uchar* vehicle = (uchar*)(game::VehiclePoolGetAt(*game::ppVehiclePool, 0, game::ScriptParams[0].nVar));
		uchar flags = *(uchar*)(vehicle + offset_flags);

		script->UpdateCompareFlag(flags & 0x40); // bLightsOn

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
IS_CAR_ENGINE_ON(Script* script)
{
		uint offset_flags = script->is_III_ ? 0x1F5 : 0x1F9; // CVehicle::flags

		script->CollectParameters(1);

		uchar* vehicle = (uchar*)(game::VehiclePoolGetAt(*game::ppVehiclePool, 0, game::ScriptParams[0].nVar));
		uchar flags = *(uchar*)(vehicle + offset_flags);

		script->UpdateCompareFlag(flags & 0x10); // bEngineOn

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
SET_CAR_ENGINE_ON(Script* script)
{
		uint offset_flags = script->is_III_ ? 0x1F5 : 0x1F9; // CVehicle::flags

		script->CollectParameters(2);

		uchar* vehicle = (uchar*)(game::VehiclePoolGetAt(*game::ppVehiclePool, 0, game::ScriptParams[0].nVar));
		uchar* flags = (uchar*)(vehicle + offset_flags);

		uchar flag_mask = (-(uchar)game::ScriptParams[1].nVar); // convert supposed bool to all 0 or 1
		*flags = (*flags & ~0x10) | (flag_mask & 0x10);

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
ALLOCATE_MEMORY(Script* script)
{
		script->CollectParameters(1);

		void* mem = new uchar[game::ScriptParams[0].nVar];
		script->register_object(mem);

		script->UpdateCompareFlag(true); // will throw otherwise

		game::ScriptParams[0].pVar = mem;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
FREE_MEMORY(Script* script)
{
		script->CollectParameters(1);

		script->delete_registered_object(game::ScriptParams[0].pVar);

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
PRINT_HELP_STRING(Script* script)
{
		script->CollectParameters(1);

		std::swprintf(g_message, HELP_MSG_LENGTH, L"%hs", game::ScriptParams[0].szVar);
		game::SetHelpMessage(g_message, false, false);

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
PRINT_BIG_STRING(Script* script)
{
		script->CollectParameters(3);

		std::swprintf(g_message, HELP_MSG_LENGTH, L"%hs", game::ScriptParams[0].szVar);
		game::AddBigMessageQ(g_message, game::ScriptParams[1].nVar, game::ScriptParams[2].nVar - 1);

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
PRINT_STRING(Script* script)
{
		script->CollectParameters(2);

		std::swprintf(g_message, HELP_MSG_LENGTH, L"%hs", game::ScriptParams[0].szVar);
		game::AddMessage(g_message, game::ScriptParams[1].nVar, 0);

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
PRINT_STRING_NOW(Script* script)
{
		script->CollectParameters(2);

		std::swprintf(g_message, HELP_MSG_LENGTH, L"%hs", game::ScriptParams[0].szVar);
		game::AddMessageJumpQ(g_message, game::ScriptParams[1].nVar, 0);

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
PRINT_HELP_FORMATTED(Script* script)
{
		script->CollectParameters(1);

		char fmt[HELP_MSG_LENGTH];
		script->format_string(fmt, game::ScriptParams[0].szVar);

		std::swprintf(g_message, HELP_MSG_LENGTH, L"%hs", fmt);
		game::SetHelpMessage(g_message, false, false);

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
PRINT_BIG_FORMATTED(Script* script)
{
		script->CollectParameters(3);
		int time = game::ScriptParams[1].nVar;
		int style = game::ScriptParams[2].nVar;

		char fmt[HELP_MSG_LENGTH];
		script->format_string(fmt, game::ScriptParams[0].szVar);

		std::swprintf(g_message, HELP_MSG_LENGTH, L"%hs", fmt);
		game::AddBigMessageQ(g_message, time, style - 1);

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
PRINT_FORMATTED(Script* script)
{
		script->CollectParameters(2);
		int time = game::ScriptParams[1].nVar;

		char fmt[HELP_MSG_LENGTH];
		script->format_string(fmt, game::ScriptParams[0].szVar);

		std::swprintf(g_message, HELP_MSG_LENGTH, L"%hs", fmt);
		game::AddMessage(g_message, time, 0);

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
PRINT_FORMATTED_NOW(Script* script)
{
		script->CollectParameters(2);
		int time = game::ScriptParams[1].nVar;

		char fmt[HELP_MSG_LENGTH];
		script->format_string(fmt, game::ScriptParams[0].szVar);

		std::swprintf(g_message, HELP_MSG_LENGTH, L"%hs", fmt);
		game::AddMessageJumpQ(g_message, time, 0);

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
STRING_FORMAT(Script* script)
{
		script->CollectParameters(2);

		script->format_string(game::ScriptParams[0].szVar, game::ScriptParams[1].szVar);

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
SCAN_STRING(Script* script)
{
		script->CollectParameters(2);
		ScriptParam* num_assigned = script->GetPointerToScriptVariable();

		num_assigned->nVar = script->scan_string(game::ScriptParams[0].szVar, game::ScriptParams[1].szVar);
		script->UpdateCompareFlag(num_assigned->nVar);

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
FILE_SEEK(Script* script)
{
		script->CollectParameters(3);
		auto* file = (std::fstream*)game::ScriptParams[0].pVar;

		file->seekg(game::ScriptParams[1].nVar, game::ScriptParams[2].nVar);
		script->UpdateCompareFlag(!file->fail());

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
IS_END_OF_FILE_REACHED(Script* script)
{
		script->CollectParameters(1);
		auto* file = (std::fstream*)game::ScriptParams[0].pVar;

		script->UpdateCompareFlag(file->fail());

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
READ_STRING_FROM_FILE(Script* script)
{
		script->CollectParameters(3);
		auto* file = (std::fstream*)game::ScriptParams[0].pVar;

		file->getline(game::ScriptParams[1].szVar, game::ScriptParams[2].nVar);
		script->UpdateCompareFlag(!file->fail());

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
WRITE_STRING_TO_FILE(Script* script)
{
		script->CollectParameters(2);
		auto* file = (std::fstream*)game::ScriptParams[0].pVar;

		file->write(game::ScriptParams[1].szVar, std::strlen(game::ScriptParams[1].szVar));
		script->UpdateCompareFlag(!file->fail());

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
WRITE_FORMATTED_STRING_TO_FILE(Script* script)
{
		script->CollectParameters(2);
		auto* file = (std::fstream*)game::ScriptParams[0].pVar;

		char fmt[HELP_MSG_LENGTH];
		int length = script->format_string(fmt, game::ScriptParams[1].szVar);

		file->write(fmt, length);
		// script->UpdateCompareFlag(!file->fail());

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
SCAN_FILE(Script* script)
{
		script->CollectParameters(2);
		auto* file = (std::fstream*)game::ScriptParams[0].pVar;
		ScriptParam* num_assigned = script->GetPointerToScriptVariable();

		// TODO: get rid of this monstrosity by implementing proper parsers with scnlib and fmt
		char buf[2048];
		auto saved_pos = file->tellg();

		file->read(buf, 2048);
		buf[file->gcount()] = '\0';

		int retval = script->scan_string(buf, game::ScriptParams[1].szVar, true);
		num_assigned->nVar = retval & 0xFF;
		script->UpdateCompareFlag(num_assigned->nVar);

		saved_pos += retval >> 8;
		file->seekg(saved_pos);

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
GET_NAME_OF_VEHICLE_MODEL(Script* script)
{
		uint sizeof_CVehicleModelInfo = script->is_III_ ? 0x1F8 : 0x174;
		uint offset_gameName = script->is_III_ ? 0x36 : 0x32; // CVehicleModelInfo::m_gameName
		uint MI_FIRST_VEHICLE = script->is_III_ ? 90 : 130;

		script->CollectParameters(2);
		int model_id = game::ScriptParams[0].nVar;

		uint element_offset = (model_id - MI_FIRST_VEHICLE) * sizeof_CVehicleModelInfo;
		char* name = (char*)(game::pVehicleModelStore + offset_gameName + element_offset);
		std::strcpy(game::ScriptParams[1].szVar, name);

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
TEST_CHEAT(Script* script)
{
		script->CollectParameters(1);
		char* input = game::ScriptParams[0].szVar;

		// KeyboardCheatString registers user input as LIFO, so we'll mirror user input
		bool result = true;
		for (int i = std::strlen(input) - 1, j = 0; result && i >= 0; --i, ++j)
				result = std::toupper(input[i]) == game::KeyboardCheatString[j];

		script->UpdateCompareFlag(result);

		// prevent circular execution
		game::KeyboardCheatString[0] = result ? '\0' : game::KeyboardCheatString[0];

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
SPAWN_VEHICLE_BY_CHEATING(Script* script)
{
		script->CollectParameters(1);

		// VC has VehicleCheat(model) we can call; III only has TankCheat() we have to edit at runtime
		if (!script->is_III_) {
				game::SpawnCar(game::ScriptParams[0].nVar);
		} else {
				int model = game::ScriptParams[0].nVar;
				uchar* func = (uchar*)game::SpawnCar;

				uchar orig_model = 122; // MI_RHINO
				uchar* orig_model_load_state = *(uchar**)(func + 0x33); // CStreaming::ms_aInfoForModel[MI_RHINO].m_loadState

				memory::write(func + 0x22, uchar(model)); // param for CStreaming::RequestModel()
				memory::write(func + 0x33, orig_model_load_state + (model - orig_model) * 0x14); // CStreaming::ms_aInfoForModel[model].m_loadState
				memory::write(func + 0xA5, uchar(model)); // param for CAutomobile::CAutomobile()

				game::SpawnCar(model); // TODO: fix crash when model is outside char bounds; 

				memory::write(func + 0x22, orig_model); // param for CStreaming::RequestModel()
				memory::write(func + 0x33, orig_model_load_state); // CStreaming::ms_aInfoForModel[MI_RHINO].m_loadState
				memory::write(func + 0xA5, orig_model);  // param for CAutomobile::CAutomobile()
		}

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
GET_TEXT_LABEL_STRING(Script* script)
{
		script->CollectParameters(2);

		const wchar_t* text = fxt::find(game::TheText, 0, game::ScriptParams[0].szVar);
		std::wcstombs(game::ScriptParams[1].szVar, text, std::wcslen(text) + 1);

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
ADD_TEXT_LABEL(Script* script)
{
		script->CollectParameters(2);

		fxt::add(game::ScriptParams[0].szVar, game::ScriptParams[1].szVar);

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
REMOVE_TEXT_LABEL(Script* script)
{
		script->CollectParameters(1);

		fxt::remove(game::ScriptParams[0].szVar);

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
DOES_DIRECTORY_EXIST(Script* script)
{
		script->CollectParameters(1);

		script->UpdateCompareFlag(fs::is_directory(game::ScriptParams[0].szVar));

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
CREATE_DIRECTORY(Script* script)
{
		script->CollectParameters(1);

		script->UpdateCompareFlag(fs::create_directories(game::ScriptParams[0].szVar));

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
FIND_FIRST_FILE(Script* script)
{
		script->CollectParameters(1);

		auto* handle = new fs::directory_iterator(game::ScriptParams[0].szVar);
		script->register_object(handle);

		script->UpdateCompareFlag(*handle != end(*handle)); // check if directory is not empty

		game::ScriptParams[0].pVar = handle;
		script->StoreParameters(1);

		script->CollectParameters(1);
		std::strcpy(game::ScriptParams[0].szVar, script->cond_result() ? (*handle)->path().filename().string().c_str() : "");

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
FIND_NEXT_FILE(Script* script)
{
		script->CollectParameters(2);
		auto* handle = (fs::directory_iterator*)game::ScriptParams[0].pVar;

		(*handle)++;
		script->UpdateCompareFlag(*handle != end(*handle));

		std::strcpy(game::ScriptParams[1].szVar, script->cond_result() ? (*handle)->path().filename().string().c_str() : "");

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
FIND_CLOSE(Script* script)
{
		script->CollectParameters(1);

		script->delete_registered_object(game::ScriptParams[0].pVar);

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
SET_CLEO_ARRAY(Script* script)
{
		script->CollectParameters(2);

		script->cleo_array_[game::ScriptParams[0].nVar].nVar = game::ScriptParams[1].nVar;

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
GET_CLEO_ARRAY(Script* script)
{
		script->CollectParameters(1);

		game::ScriptParams[0].nVar = script->cleo_array_[game::ScriptParams[0].nVar].nVar;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
GET_CLEO_ARRAY_OFFSET(Script* script)
{
		script->CollectParameters(1);

		game::ScriptParams[0].pVar = &script->cleo_array_[game::ScriptParams[0].nVar].nVar;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
GET_CLEO_ARRAY_SCRIPT(Script* script)
{
		script->CollectParameters(2);
		Script* param_script = (Script*)game::ScriptParams[0].pVar;

		game::ScriptParams[0].pVar = &param_script->cleo_array_[game::ScriptParams[1].nVar].nVar;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

void
opcodes::reg_CLEO2()
{
		// CLEO 2 opcodes
		reg(0x0600, &STREAM_CUSTOM_SCRIPT);
		reg(0x0601, &IS_BUTTON_PRESSED_WITH_SENSITIVITY);
		reg(0x0602, &EMULATE_BUTTON_PRESS_WITH_SENSITIVITY);
		reg(0x0603, &IS_CAMERA_IN_WIDESCREEN_MODE);
		reg(0x0604, &GET_WEAPONTYPE_MODEL);
		reg(0x0605, &GET_WEAPONTYPE_FOR_MODEL);
		reg(0x0606, &SET_MEMORY_OFFSET);
		reg(0x0607, &GET_CURRENT_WEATHER);
		reg(0x0608, &DISPLAY_TEXT_STRING);
		reg(0x0609, &DISPLAY_TEXT_FORMATTED);
		reg(0x0673, &PLAY_ANIMATION);

		// duplicate ids for some definitions to match SA's CLEO indexes (0A8C0AEF)
		reg(0x0A92, &STREAM_CUSTOM_SCRIPT);

		// SA's CLEO 3-4 unique opcodes (0A8C0AEF)
		reg(0x0A8E, &INT_ADD);
		reg(0x0A8F, &INT_SUB);
		reg(0x0A90, &INT_MUL);
		reg(0x0A91, &INT_DIV);
		// reg(0x0A94, &LOAD_AND_LAUNCH_CUSTOM_MISSION);
		// reg(0x0A95, &SAVE_THIS_CUSTOM_SCRIPT);
		reg(0x0A99, &SET_CURRENT_DIRECTORY);
		reg(0x0A9A, &OPEN_FILE);
		reg(0x0A9B, &CLOSE_FILE);
		reg(0x0A9C, &GET_FILE_SIZE);
		reg(0x0A9D, &READ_FROM_FILE);
		reg(0x0A9E, &WRITE_TO_FILE);
		reg(0x0AA0, &GOSUB_IF_FALSE);
		reg(0x0AA1, &RETURN_IF_FALSE);
		reg(0x0AA2, &LOAD_DYNAMIC_LIBRARY);
		reg(0x0AA3, &FREE_DYNAMIC_LIBRARY);
		reg(0x0AA4, &GET_DYNAMIC_LIBRARY_PROCEDURE);
		reg(0x0AA9, &IS_GAME_VERSION_ORIGINAL);
		reg(0x0AAB, &DOES_FILE_EXIST);
		// reg(0x0AAC, LOAD_AUDIO_STREAM);
		// reg(0x0AAD, SET_AUDIO_STREAM_STATE);
		// reg(0x0AAE, REMOVE_AUDIO_STREAM);
		// reg(0x0AAF, GET_AUDIO_STREAM_LENGTH);
		reg(0x0AB3, &SET_CLEO_SHARED_VAR);
		reg(0x0AB4, &GET_CLEO_SHARED_VAR);
		// reg(0x0AB5, STORE_CLOSEST_ENTITIES);
		// reg(0x0AB6, GET_TARGET_BLIP_COORDS);
		reg(0x0AB7, &GET_CAR_NUMBER_OF_GEARS);
		reg(0x0AB8, &GET_CAR_CURRENT_GEAR);
		// reg(0x0AB9, GET_AUDIO_STREAM_STATE);
		// reg(0x0ABB, GET_AUDIO_STREAM_VOLUME);
		// reg(0x0ABC, SET_AUDIO_STREAM_VOLUME);
		reg(0x0ABD, &IS_CAR_LIGHTS_ON);
		reg(0x0ABE, &IS_CAR_ENGINE_ON);
		reg(0x0ABF, &SET_CAR_ENGINE_ON);
		// reg(0x0AC0, SET_AUDIO_STREAM_LOOPED);
		// reg(0x0AC1, LOAD_3D_AUDIO_STREAM);
		// reg(0x0AC2, SET_PLAY_3D_AUDIO_STREAM_AT_COORDS);
		// reg(0x0AC3, SET_PLAY_3D_AUDIO_STREAM_AT_OBJECT);
		// reg(0x0AC4, SET_PLAY_3D_AUDIO_STREAM_AT_CHAR);
		// reg(0x0AC5, SET_PLAY_3D_AUDIO_STREAM_AT_CAR);
		reg(0x0AC8, &ALLOCATE_MEMORY);
		reg(0x0AC9, &FREE_MEMORY);
		reg(0x0ACA, &PRINT_HELP_STRING);
		reg(0x0ACB, &PRINT_BIG_STRING);
		reg(0x0ACC, &PRINT_STRING);
		reg(0x0ACD, &PRINT_STRING_NOW);
		reg(0x0ACE, &PRINT_HELP_FORMATTED);
		reg(0x0ACF, &PRINT_BIG_FORMATTED);
		reg(0x0AD0, &PRINT_FORMATTED);
		reg(0x0AD1, &PRINT_FORMATTED_NOW);
		// reg(0x0AD2, GET_CHAR_PLAYER_IS_TARGETING);
		reg(0x0AD3, &STRING_FORMAT);
		reg(0x0AD4, &SCAN_STRING);
		reg(0x0AD5, &FILE_SEEK);
		reg(0x0AD6, &IS_END_OF_FILE_REACHED);
		reg(0x0AD7, &READ_STRING_FROM_FILE);
		reg(0x0AD8, &WRITE_STRING_TO_FILE);
		reg(0x0AD9, &WRITE_FORMATTED_STRING_TO_FILE);
		reg(0x0ADA, &SCAN_FILE);
		reg(0x0ADB, &GET_NAME_OF_VEHICLE_MODEL);
		reg(0x0ADC, &TEST_CHEAT);
		reg(0x0ADD, &SPAWN_VEHICLE_BY_CHEATING);
		reg(0x0ADE, &GET_TEXT_LABEL_STRING);
		reg(0x0ADF, &ADD_TEXT_LABEL);
		reg(0x0AE0, &REMOVE_TEXT_LABEL);
		reg(0x0AE4, &DOES_DIRECTORY_EXIST);
		reg(0x0AE5, &CREATE_DIRECTORY);
		reg(0x0AE6, &FIND_FIRST_FILE);
		reg(0x0AE7, &FIND_NEXT_FILE);
		reg(0x0AE8, &FIND_CLOSE);
		// reg(0x0AED, STRING_FLOAT_FORMAT);

		// CLEO 2.1 opcodes
		reg(0x0AF8, &SET_CLEO_ARRAY);
		reg(0x0AF9, &GET_CLEO_ARRAY);
		reg(0x0AFA, &GET_CLEO_ARRAY_OFFSET);
		reg(0x0AFB, &GET_CLEO_ARRAY_SCRIPT);
}
