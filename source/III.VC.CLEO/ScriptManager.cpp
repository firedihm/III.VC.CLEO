#include "CustomScript.h"
#include "Game.h"
#include "Log.h"
#include "ScriptManager.h"

#include <filesystem>

namespace fs = std::filesystem;

ScriptManager scriptMgr;

ScriptManager::ScriptManager() : gameScripts(), scriptMemory(), pCusomScripts(nullptr), numLoadedCustomScripts(0) {}

void
ScriptManager::LoadScripts()
{
		fs::path dir(game.szRootPath);
		dir /= "CLEO";

		for (const auto& entry : fs::directory_iterator(dir)) {
				if (entry.is_regular_file() && entry.path().extension().string() == ".cs") {
						try {
								CScript* script = new CScript(entry.path().c_str());

								game.Scripts.AddScriptToList(script, game.Scripts.pActiveScriptsList);
								script->AddToCustomList(&pCusomScripts);
								script->m_bIsActive = true;
								numLoadedCustomScripts++;
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
				game.Scripts.RemoveScriptFromList(script, game.Scripts.pActiveScriptsList);
				numLoadedCustomScripts--;

				LOGL(LOG_PRIORITY_SCRIPT_LOADING, "Unloading custom script \"%s\"", script->m_acName);

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
				game.Scripts.AddScriptToList(script, game.Scripts.pActiveScriptsList);
				LOGL(LOG_PRIORITY_SCRIPT_LOADING, "Enabled script \"%s\"", script->m_acName);
				script = script->m_pNextCustom;
		}
}

void
ScriptManager::DisableAllScripts()
{
		CScript* script = pCusomScripts;
		while (script) {
				game.Scripts.RemoveScriptFromList(script, game.Scripts.pActiveScriptsList);
				LOGL(LOG_PRIORITY_SCRIPT_LOADING, "Disabled script \"%s\"", script->m_acName);
				script = script->m_pNextCustom;
		}
}

void
ScriptManager::InitialiseScript(CScript* script)
{
		script->Init();
}

eOpcodeResult
ScriptManager::ProcessScriptCommand(CScript* script)
{
		return script->ProcessOneCommand();
}

void
ScriptManager::CollectScriptParameters(CScript* script, int, uint* pIp, uint numParams)
{
		script->Collect(pIp, numParams);
}

int
ScriptManager::CollectScriptNextParameterWithoutIncreasingPC(CScript* script, int, uint ip)
{
		return script->CollectNextWithoutIncreasingPC(ip);
}
