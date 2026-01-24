#include "Fxt.h"
#include "Game.h"
#include "Log.h"
#include "Script.h"
#include "ScriptManager.h"

#include <cstring>
#include <filesystem>
#include <list>
#include <set>

namespace fs = std::filesystem;

std::list<Script> CustomScripts;
std::list<Script> PersistentScripts;

std::set<const void*> AllocatedMemory;
std::set<const std::FILE*> FileStreams;
std::set<const fs::directory_iterator*> FileSearchHandles;

Script*
scriptMgr::StartScript(const char* filepath)
{
		Script script(filepath);

		std::list<Script>& list = script.m_bIsPersistent ? PersistentScripts : CustomScripts;

		list.push_front(std::move(script));

		game.Scripts.pfAddScriptToList(&list.front(), game.Scripts.ppActiveScripts);
		list.front().m_bIsActive = true;

		return &list.front();
}

void
scriptMgr::TerminateScript(Script* script)
{
		game.Scripts.pfRemoveScriptFromList(script, game.Scripts.ppActiveScripts);
		script->m_bIsActive = false;

		std::list<Script>& list = script->m_bIsPersistent ? PersistentScripts : CustomScripts;

		for (auto it = list.begin(); it != list.end(); ++it) {
				if (Script& current = *it; &current == script) {
						list.erase(it);
						break;
				}
		}
}

void
scriptMgr::LoadScripts(bool game_start)
{
		fs::path dir = fs::path(game.Misc.szRootDirName) / "CLEO";

		for (const auto& entry : fs::directory_iterator(dir)) {
				if (entry.is_regular_file() && (entry.path().extension().string() == ".cs" || 
												entry.path().extension().string() == ".csp" && game_start)) {
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
scriptMgr::UnloadScripts(bool game_shutdown)
{
		auto unload_list = [](std::list<Script>& list) {
				for (auto it = list.begin(); it != list.end(); ) {
						game.Scripts.pfRemoveScriptFromList(&(*it), game.Scripts.ppActiveScripts);
						it->m_bIsActive = false;

						it = list.erase(it);
				}
		};

		unload_list(CustomScripts);
		if (game_shutdown)
				unload_list(PersistentScripts);
}

void
scriptMgr::EnableScripts()
{
		for (Script& script : CustomScripts)
				game.Scripts.pfAddScriptToList(&script, game.Scripts.ppActiveScripts);

		for (Script& script : PersistentScripts)
				game.Scripts.pfAddScriptToList(&script, game.Scripts.ppActiveScripts);
}

void
scriptMgr::DisableScripts()
{
		for (Script& script : CustomScripts)
				game.Scripts.pfRemoveScriptFromList(&script, game.Scripts.ppActiveScripts);

		for (Script& script : PersistentScripts)
				game.Scripts.pfRemoveScriptFromList(&script, game.Scripts.ppActiveScripts);
}

Script*
scriptMgr::FindScriptNamed(char* name, bool search_generic)
{
		for (Script& script : CustomScripts) {
				if (!std::strncmp(&script.m_acName, name, KEY_LENGTH_IN_SCRIPT))
						return &script;
		}

		for (Script& script : PersistentScripts) {
				if (!std::strncmp(&script.m_acName, name, KEY_LENGTH_IN_SCRIPT))
						return &script;
		}

		if (search_generic) {
				for (Script* script = *game.Scripts.ppActiveScripts; script; script = script->m_pNext) {
						if (!std::strncmp(&script->m_acName, name, KEY_LENGTH_IN_SCRIPT))
								return script;
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
