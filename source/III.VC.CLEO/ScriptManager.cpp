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
Script* pCusomScripts;
Script* pPersistentScripts;

Script*
scriptMgr::StartScript(const char* filepath)
{
		Script* script = new Script(filepath);

		game.Scripts.pfAddScriptToList(script, game.Scripts.ppActiveScripts);
		script->AddToCustomList(&pCusomScripts);
		script->m_bIsActive = true;
}

void
scriptMgr::TerminateScript(Script* script)
{
		Script** list = script->m_bIsPersistent ? &pPersistentScripts : &pCusomScripts;
		script->RemoveFromCustomList(list);
		game.Scripts.pfRemoveScriptFromList(script, game.Scripts.ppActiveScripts);
		LOGL(LOG_PRIORITY_OPCODE, "TERMINATE_CUSTOM_THREAD: Terminating custom script \"%s\"", script->m_acName);
		delete script;
}

void
ScriptManager::LoadScripts()
{
		fs::path dir(game.Misc.szRootDirName);
		dir /= "CLEO";

		for (const auto& entry : fs::directory_iterator(dir)) {
				if (entry.is_regular_file() && entry.path().extension().string() == ".cs") {
						try {
								StartScript(entry.path().c_str());

								LOGL(LOG_PRIORITY_MEMORY_ALLOCATION, "Loaded custom script \"%s\"", &script->m_acName);
						} catch (const char* e) {
								LOGL(LOG_PRIORITY_MEMORY_ALLOCATION, "Failed to allocate custom script \"%s\". %s", entry.filename().c_str(), e);
						} catch (const std::bad_alloc& e) {
								LOGL(LOG_PRIORITY_MEMORY_ALLOCATION, "Failed to allocate custom script \"%s\". %s", entry.filename().c_str(), e.what());
						} catch (...) {
								LOGL(LOG_PRIORITY_MEMORY_ALLOCATION, "Failed to allocate custom script \"%s\". %s", entry.filename().c_str(), "Unknown exception");
						}
				}
		}
}

void
ScriptManager::UnloadScripts()
{
		Script* script = pCusomScripts;
		while (script) {
				game.Scripts.pfRemoveScriptFromList(script, game.Scripts.ppActiveScripts);

				LOGL(LOG_PRIORITY_SCRIPT_LOADING, "Unloading custom script \"%s\"", &script->m_acName);

				Script* next = script->m_pNextCustom;
				delete script;
				script = next;
		}
		pCusomScripts = nullptr;
}

void
ScriptManager::EnableScripts()
{
		Script* script = pCusomScripts;
		while (script) {
				game.Scripts.pfAddScriptToList(script, game.Scripts.ppActiveScripts);
				LOGL(LOG_PRIORITY_SCRIPT_LOADING, "Enabled script \"%s\"", &script->m_acName);
				script = script->m_pNextCustom;
		}
}

void
ScriptManager::DisableScripts()
{
		Script* script = pCusomScripts;
		while (script) {
				game.Scripts.pfRemoveScriptFromList(script, game.Scripts.ppActiveScripts);
				LOGL(LOG_PRIORITY_SCRIPT_LOADING, "Disabled script \"%s\"", &script->m_acName);
				script = script->m_pNextCustom;
		}
}

Script*
scriptMgr::FindScriptNamed(char* name)
{
		Script* script = pCusomScripts;
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

		return nullptr;
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

		UnloadScripts();
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
