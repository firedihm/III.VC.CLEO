#include "Game.h"
#include "Log.h"
#include "Script.h"
#include "ScriptManager.h"

#include <filesystem>
#include <set>

namespace fs = std::filesystem;

std::set<const void*> AllocatedMemory;
std::set<const FILE*> FileStreams;
std::set<const fs::directory_iterator*> FileSearchHandles;

ScriptManager scriptMgr;

ScriptManager::ScriptManager() : pCusomScripts(nullptr), numLoadedCustomScripts(0) {}

void
ScriptManager::LoadScripts()
{
		fs::path dir(game.szRootPath);
		dir /= "CLEO";

		for (const auto& entry : fs::directory_iterator(dir)) {
				if (entry.is_regular_file() && entry.path().extension().string() == ".cs") {
						try {
								CScript* script = new CScript(entry.path().c_str());

								game.Scripts.pfAddScriptToList(script, game.Scripts.ppActiveScripts);
								script->AddToCustomList(&pCusomScripts);
								script->m_bIsActive = true;
								numLoadedCustomScripts++;

								LOGL(LOG_PRIORITY_MEMORY_ALLOCATION, "Loaded custom script \"%s\"", &script->m_acName);
						} catch (const char* e) {
								LOGL(LOG_PRIORITY_MEMORY_ALLOCATION, "Failed to allocate custom script \"%s\". %s", entry.filename().c_str(), e);
						} catch (const std::bad_alloc& e) {
								LOGL(LOG_PRIORITY_MEMORY_ALLOCATION, "Failed to allocate custom script \"%s\". %s", entry.filename().c_str(), e.what());
						} catch (...) {
								LOGL(LOG_PRIORITY_MEMORY_ALLOCATION, "Failed to allocate custom script \"%s\". %s", entry.filename().c_str(), "Unhandled exception");
						}
				}
		}
}

void
ScriptManager::UnloadScripts()
{
		CScript* script = pCusomScripts;
		while (script) {
				game.Scripts.pfRemoveScriptFromList(script, game.Scripts.ppActiveScripts);
				numLoadedCustomScripts--;

				LOGL(LOG_PRIORITY_SCRIPT_LOADING, "Unloaded custom script \"%s\"", &script->m_acName);

				CScript* next = script->m_pNextCustom;
				delete script;
				script = next;
		}
		pCusomScripts = nullptr;
}

void
ScriptManager::EnableAllScripts()
{
		CScript* script = pCusomScripts;
		while (script) {
				game.Scripts.pfAddScriptToList(script, game.Scripts.ppActiveScripts);
				LOGL(LOG_PRIORITY_SCRIPT_LOADING, "Enabled script \"%s\"", &script->m_acName);
				script = script->m_pNextCustom;
		}
}

void
ScriptManager::DisableAllScripts()
{
		CScript* script = pCusomScripts;
		while (script) {
				game.Scripts.pfRemoveScriptFromList(script, game.Scripts.ppActiveScripts);
				LOGL(LOG_PRIORITY_SCRIPT_LOADING, "Disabled script \"%s\"", &script->m_acName);
				script = script->m_pNextCustom;
		}
}

void
ScriptManager::SaveMemoryAddress(const void* memory)
{
		AllocatedMemory.insert(memory);
}

void
ScriptManager::DeleteMemoryAddress(const void* memory)
{
		AllocatedMemory.erase(memory);
}

void
ScriptManager::SaveFileStream(const FILE* stream)
{
		FileStreams.insert(stream);
}

void
ScriptManager::DeleteFileStream(const FILE* stream)
{
		FileStreams.erase(stream);
}

void
ScriptManager::SaveFileSearchHandle(const fs::directory_iterator* handle)
{
		FileSearchHandles.insert(handle);
}

void
ScriptManager::DeleteFileSearchHandle(const fs::directory_iterator* handle)
{
		FileSearchHandles.erase(handle);
}

void
GtaGame::InitScripts_OnGameInit()
{
		LOGL(LOG_PRIORITY_GAME_EVENT, "--Game Init--");

		scriptMgr.UnloadScripts();
		CustomText::Unload();

		Events.pfInitScripts_OnGameInit();

		scriptMgr.LoadScripts();
		CustomText::Load();
}

void
GtaGame::InitScripts_OnGameReinit()
{
		LOGL(LOG_PRIORITY_GAME_EVENT, "--Game Re-Init--");

		scriptMgr.UnloadScripts();
		CustomText::Unload();

		Events.pfInitScripts_OnGameReinit();

		scriptMgr.LoadScripts();
		CustomText::Load();

		std::for_each(Misc.openedFiles->begin(), Misc.openedFiles->end(), fclose);
		Misc.openedFiles->clear();
		std::for_each(Misc.allocatedMemory->begin(), Misc.allocatedMemory->end(), free);
		Misc.allocatedMemory->clear();
		std::for_each(Misc.openedHandles->begin(), Misc.openedHandles->end(), CloseHandle);
		Misc.openedHandles->clear();
}

void
GtaGame::InitScripts_OnGameSaveLoad()
{
		LOGL(LOG_PRIORITY_GAME_EVENT, "--Game Load Save--");

		scriptMgr.UnloadScripts();
		CustomText::Unload();

		Events.pfInitScripts_OnGameSaveLoad();

		scriptMgr.LoadScripts();
		CustomText::Load();

		std::for_each(Misc.openedFiles->begin(), Misc.openedFiles->end(), fclose);
		Misc.openedFiles->clear();
		std::for_each(Misc.allocatedMemory->begin(), Misc.allocatedMemory->end(), free);
		Misc.allocatedMemory->clear();
		std::for_each(Misc.openedHandles->begin(), Misc.openedHandles->end(), CloseHandle);
		Misc.openedHandles->clear();
}

void
GtaGame::OnGameSaveScripts(int a, int b)
{
		LOGL(LOG_PRIORITY_GAME_EVENT, "--Game Save Scripts--");

		scriptMgr.DisableAllScripts();

		Events.pfGameSaveScripts(a, b);

		scriptMgr.EnableAllScripts();
}

void
GtaGame::OnShutdownGame()
{
		LOGL(LOG_PRIORITY_GAME_EVENT, "--Game Shutdown--");

		Events.pfShutdownGame();

		scriptMgr.UnloadScripts();
		CustomText::Unload();

		std::for_each(Misc.openedFiles->begin(),.Misc.openedFiles->end(), fclose);
		Misc.openedFiles->clear();
		std::for_each(Misc.allocatedMemory->begin(), Misc.allocatedMemory->end(), free);
		Misc.allocatedMemory->clear();
		std::for_each(Misc.openedHandles->begin(), Misc.openedHandles->end(), CloseHandle);
		Misc.openedHandles->clear();
}

float
ScreenCoord(float a)
{
		return a * (*Screen.pHeight / 900.0f);
}

void
GtaGame::OnMenuDrawing(float x, float y, wchar_t* text)
{
	game.Events.pfDrawInMenu(x, y, text);

	CRGBA color;
	wchar_t line[128];
	if (game.IsGta3())
			color = { 0xEB, 0xAA, 0x32, 0xFF };
	else
			color = { 0xFF, 0x96, 0xE1, 0xFF };

	game.Font.pfSetColor(&color);
	game.Font.pfSetDropShadowPosition(0);
	game.Font.pfSetPropOn();
	game.Font.pfSetFontStyle(0);
	game.Font.pfSetScale(ScreenCoord(0.45f), ScreenCoord(0.7f));
	game.Font.pfSetJustifyOn();
	
	swprintf(line, L"CLEO v%d.%d.%d", CLEO_VERSION_MAIN, CLEO_VERSION_MAJOR, CLEO_VERSION_MINOR);
	game.Font.pfPrintString(ScreenCoord(30.0f), (float)*game.Screen.pHeight - ScreenCoord(34.0f), line);

	scriptMgr.numLoadedCustomScripts ?
	swprintf(line, L"%d %s, %d %s loaded", scriptMgr.numLoadedCustomScripts, scriptMgr.numLoadedCustomScripts == 1? L"script" : L"scripts",
		CleoPlugins::numLoadedPlugins, CleoPlugins::numLoadedPlugins == 1? L"plugin" : L"plugins") :
	swprintf(line, L"%d %s loaded", CleoPlugins::numLoadedPlugins, CleoPlugins::numLoadedPlugins == 1 ? L"plugin" : L"plugins");
	game.Font.pfPrintString(ScreenCoord(30.0f), (float)*game.Screen.pHeight - ScreenCoord(20.0f), line);
}
