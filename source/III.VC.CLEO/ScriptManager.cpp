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
								script->m_bIsCustom = true;

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

				LOGL(LOG_PRIORITY_SCRIPT_LOADING, "Unloading custom script \"%s\"", &script->m_acName);

				CScript* next = script->m_pNextCustom;
				delete script;
				script = next;
		}
		pCusomScripts = nullptr;
}

void
ScriptManager::EnableScripts()
{
		CScript* script = pCusomScripts;
		while (script) {
				game.Scripts.pfAddScriptToList(script, game.Scripts.ppActiveScripts);
				LOGL(LOG_PRIORITY_SCRIPT_LOADING, "Enabled script \"%s\"", &script->m_acName);
				script = script->m_pNextCustom;
		}
}

void
ScriptManager::DisableScripts()
{
		CScript* script = pCusomScripts;
		while (script) {
				game.Scripts.pfRemoveScriptFromList(script, game.Scripts.ppActiveScripts);
				LOGL(LOG_PRIORITY_SCRIPT_LOADING, "Disabled script \"%s\"", &script->m_acName);
				script = script->m_pNextCustom;
		}
}

void
ScriptManager::OnGameStart()
{
		LOGL(LOG_PRIORITY_GAME_EVENT, "--Game Start--");

		UnloadScripts();
		CustomText::Unload();

		game.Events.pfInitScripts();

		LoadScripts();
		CustomText::Load();
}

void
ScriptManager::OnGameLoad()
{
		LOGL(LOG_PRIORITY_GAME_EVENT, "--Game Load--");

		UnloadScripts();
		CustomText::Unload();

		game.Events.pfInitScripts();

		LoadScripts();
		CustomText::Load();

		std::for_each(Misc.openedFiles->begin(), Misc.openedFiles->end(), fclose);
		Misc.openedFiles->clear();
		std::for_each(Misc.allocatedMemory->begin(), Misc.allocatedMemory->end(), free);
		Misc.allocatedMemory->clear();
		std::for_each(Misc.openedHandles->begin(), Misc.openedHandles->end(), CloseHandle);
		Misc.openedHandles->clear();
}

void
ScriptManager::OnGameReload()
{
		LOGL(LOG_PRIORITY_GAME_EVENT, "--Game Reload--");

		UnloadScripts();
		CustomText::Unload();

		game.Events.pfInitScripts();

		LoadScripts();
		CustomText::Load();

		std::for_each(Misc.openedFiles->begin(), Misc.openedFiles->end(), fclose);
		Misc.openedFiles->clear();
		std::for_each(Misc.allocatedMemory->begin(), Misc.allocatedMemory->end(), free);
		Misc.allocatedMemory->clear();
		std::for_each(Misc.openedHandles->begin(), Misc.openedHandles->end(), CloseHandle);
		Misc.openedHandles->clear();
}

void
ScriptManager::OnGameSaveAllScripts(uchar* buf, uint* size)
{
		LOGL(LOG_PRIORITY_GAME_EVENT, "--Game Save All Scripts--");

		DisableScripts();
		Events.pfSaveAllScripts(buf, size);
		EnableScripts();
}

void
ScriptManager::OnGameShutdown()
{
		LOGL(LOG_PRIORITY_GAME_EVENT, "--Game Shutdown--");

		game.Events.pfCdStreamRemoveImages();

		scriptMgr.UnloadScripts();
		CustomText::Unload();

		std::for_each(Misc.openedFiles->begin(),.Misc.openedFiles->end(), fclose);
		Misc.openedFiles->clear();
		std::for_each(Misc.allocatedMemory->begin(), Misc.allocatedMemory->end(), free);
		Misc.allocatedMemory->clear();
		std::for_each(Misc.openedHandles->begin(), Misc.openedHandles->end(), CloseHandle);
		Misc.openedHandles->clear();
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
