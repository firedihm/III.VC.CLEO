#include "Game.h"
#include "Memory.h"
#include "Plugins.h"
#include "Trace.h"

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
						trace::line("loading plugin %s", entry.path().string().c_str());
						g_plugins.push_back(memory::load_library(entry.path().string().c_str()));
				}
		}
}

void
plugins::unload()
{
		for (void* handle : g_plugins)
				memory::free_library(handle);

		trace::line("unloaded all plugins");
		g_plugins.clear();
}
