#include "Log.h"
#include "Memory.h"
#include "Plugins.h"

#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

std::vector<void*> g_plugins;

void
plugins::load()
{
		fs::path dir = fs::path(game::RootDirName) / "CLEO" / "CLEO_PLUGINS";

		for (const auto& entry : fs::directory_iterator(dir)) {
				if (entry.is_regular_file() && entry.path().extension().string() == ".cleo") {
						g_plugins.push_back(memory::LoadLibrary(entry.path.c_str()));
						LOGL(LOG_PRIORITY_SCRIPT_LOADING, "Loaded plugin %s", entry.filename().c_str());
				}
		}
}

void
plugins::unload()
{
		for (void* handle : g_plugins)
				memory::FreeLibrary(handle);

		g_plugins.clear();
}
