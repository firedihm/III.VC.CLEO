#include "Fxt.h"
#include "Game.h"
#include "Log.h"
#include "Script.h"
#include "ScriptManager.h"

#include <cstring>
#include <filesystem>
#include <set>

namespace fs = std::filesystem;

std::set<const void*> AllocatedMemory;
std::set<const std::FILE*> FileStreams;
std::set<const fs::directory_iterator*> FileSearchHandles;

// CLEO sripts lists; c-styled to match mission scripts lists style
Script* pCustomScripts;
Script* pPersistentScripts;

Script*
scriptMgr::StartScript(const char* filepath)
{
		Script* script = new Script(filepath);

		game.Scripts.pfAddScriptToList(script, game.Scripts.ppActiveScripts);
		script->AddToCustomList(script->m_bIsPersistent ? &pPersistentScripts : &pCustomScripts);
		script->m_bIsActive = true;

		return script;
}

void
scriptMgr::TerminateScript(Script* script)
{
		script->RemoveFromCustomList(script->m_bIsPersistent ? &pPersistentScripts : &pCustomScripts);
		game.Scripts.pfRemoveScriptFromList(script, game.Scripts.ppActiveScripts);

		delete script;
}

void
scriptMgr::LoadScripts()
{
		fs::path dir = fs::path(game.Misc.szRootDirName) / "CLEO";

		for (const auto& entry : fs::directory_iterator(dir)) {
				if (entry.is_regular_file() && (entry.path().extension().string() == ".cs" || entry.path().extension().string() == ".csp")) {
						try {
								StartScript(entry.path().c_str());

								LOGL(LOG_PRIORITY_MEMORY_ALLOCATION, "Loaded custom script \"%s\"", &script->m_acName);
						} catch (const char* e) {
								LOGL(LOG_PRIORITY_MEMORY_ALLOCATION, "Failed to load custom script \"%s\". %s", entry.filename().c_str(), e);
						} catch (const std::bad_alloc& e) {
								LOGL(LOG_PRIORITY_MEMORY_ALLOCATION, "Failed to load custom script \"%s\". %s", entry.filename().c_str(), e.what());
						} catch (...) {
								LOGL(LOG_PRIORITY_MEMORY_ALLOCATION, "Failed to load custom script \"%s\". %s", entry.filename().c_str(), "Unknown exception");
						}
				}
		}
}

void
scriptMgr::UnloadScripts()
{
		Script* script = pCustomScripts;
		while (script) {
				game.Scripts.pfRemoveScriptFromList(script, game.Scripts.ppActiveScripts);

				LOGL(LOG_PRIORITY_SCRIPT_LOADING, "Unloading custom script \"%s\"", &script->m_acName);

				Script* next = script->m_pNextCustom;
				delete script;
				script = next;
		}
		pCustomScripts = nullptr;
}

void
ScriptManager::EnableScripts()
{
		Script* script = pCustomScripts;
		while (script) {
				game.Scripts.pfAddScriptToList(script, game.Scripts.ppActiveScripts);
				LOGL(LOG_PRIORITY_SCRIPT_LOADING, "Enabled script \"%s\"", &script->m_acName);
				script = script->m_pNextCustom;
		}
}

void
ScriptManager::DisableScripts()
{
		Script* script = pCustomScripts;
		while (script) {
				game.Scripts.pfRemoveScriptFromList(script, game.Scripts.ppActiveScripts);
				LOGL(LOG_PRIORITY_SCRIPT_LOADING, "Disabled script \"%s\"", &script->m_acName);
				script = script->m_pNextCustom;
		}
}

Script*
scriptMgr::FindScriptNamed(char* name, bool search_mission)
{
		Script* script = pCustomScripts;
		while (script) {
				if (!std::strncmp(&script->m_acName, name, KEY_LENGTH_IN_SCRIPT))
						return script;

				script = script->m_pNextCustom;
		}

		script = pPersistentScripts;
		while (script) {
				if (!std::strncmp(&script->m_acName, name, KEY_LENGTH_IN_SCRIPT))
						return script;

				script = script->m_pNextCustom;
		}

		if (search_mission) {
				script = *game.Scripts.ppActiveScripts;
				while (script) {
						if (!std::strncmp(&script->m_acName, name, KEY_LENGTH_IN_SCRIPT))
								return script;

						script = script->m_pNext;
				}
		}

		return nullptr;
}

void
ScriptManager::OnGameStart()
{
		LOGL(LOG_PRIORITY_GAME_EVENT, "--Game Start--");

		UnloadScripts();
		LOGL(LOG_PRIORITY_CUSTOM_TEXT, "Unloading fxt entries");
		fxt::UnloadEntries();

		game.Events.pfInitScripts();

		LoadScripts();
		LOGL(LOG_PRIORITY_CUSTOM_TEXT, "Loading fxt entries");
		fxt::LoadEntries();
}

void
ScriptManager::OnGameLoad()
{
		LOGL(LOG_PRIORITY_GAME_EVENT, "--Game Load--");

		UnloadScripts();
		LOGL(LOG_PRIORITY_CUSTOM_TEXT, "Unloading fxt entries");
		fxt::UnloadEntries();

		game.Events.pfInitScripts();

		LoadScripts();
		LOGL(LOG_PRIORITY_CUSTOM_TEXT, "Loading fxt entries");
		fxt::LoadEntries();

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
		LOGL(LOG_PRIORITY_CUSTOM_TEXT, "Unloading fxt entries");
		fxt::UnloadEntries();

		game.Events.pfInitScripts();

		LoadScripts();
		LOGL(LOG_PRIORITY_CUSTOM_TEXT, "Loading fxt entries");
		fxt::LoadEntries();

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

		UnloadScripts();
		LOGL(LOG_PRIORITY_CUSTOM_TEXT, "Unloading fxt entries");
		fxt::UnloadEntries();

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
ScriptManager::SaveFileStream(const std::FILE* stream)
{
		FileStreams.insert(stream);
}

void
ScriptManager::DeleteFileStream(const std::FILE* stream)
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
