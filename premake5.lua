workspace "CLEO"
	configurations { "Release", "Debug" }
	platforms "Win32"
	architecture "x32"
	location "build"
	objdir "build/obj"
	libdirs "bin"
	buildlog "build/log/%{prj.name}.log"

	kind "SharedLib"
	language "C++"
	cdialect "C17"
	cppdialect "C++latest"
	characterset "MBCS"
	staticruntime "On"

	defines {
		"uchar=unsigned char",
		"ushort=unsigned short",
		"uint=unsigned int",
		"ulong=unsigned long",

		"RC_COMPANY_NAME=\"CLEO\"",
		"RC_FILE_DESCRIPTION=\"https://cleo.li\"",
		"RC_INTERNAL_NAME=\"%{prj.name}\"",
		"RC_LEGAL_COPYRIGHT=\"MIT License\"",
		"RC_ORIGINAL_FILENAME=\"%{prj.name}.asi\"",
		"RC_PRODUCT_NAME=\"%{prj.name}\"",
		"RC_UPDATE_URL=\"https://github.com/cleolibrary/III.VC.CLEO\"",
	}

	pbcommands = { 
		"setlocal EnableDelayedExpansion",
		-- "set \"path=" .. (gamepath) .. "\"",
		"set file=$(TargetPath)",
		"FOR %%i IN (\"%file%\") DO (",
		"set filename=%%~ni",
		"set fileextension=%%~xi",
		"set target=!path!!filename!!fileextension!",
		"if exist \"!target!\" copy /y \"!file!\" \"!target!\"",
		")"
	}

	function setpaths (gamepath, exepath, scriptspath)
		scriptspath = scriptspath or "scripts/"
		if (gamepath) then
			cmdcopy = { "set \"path=" .. gamepath .. scriptspath .. "\"" }
			table.insert(cmdcopy, pbcommands)
			postbuildcommands (cmdcopy)
			debugdir (gamepath)
			if (exepath) then
				debugcommand (gamepath .. exepath)
				dir, file = exepath:match'(.*/)(.*)'
				debugdir (gamepath .. (dir or ""))
			end
		end
		targetdir ("bin" .. scriptspath)
	end

	--[[ filter "configurations:Release_xp"
		toolset "v141_xp"
		buildoptions { "/Zc:threadSafeInit-" }
	]]

	filter "configurations:Debug"
		defines "DEBUG"
		symbols "On"
		optimize "On"

	filter "configurations:Release"
		defines "NDEBUG"
		symbols "Off"
		optimize "Full"

	project "CLEO"
		targetdir "bin"
		targetextension ".asi"

		files {
			"source/*.h",
			"source/*.cpp",
			"resources/*.rc"
		}

		-- includedirs { "external/injector/include" }

		setpaths("Z:/WGTA/gta3sc_test/Grand Theft Auto III/", "gta3.exe", "")

	project "ClipboardControl"
		targetdir "bin/CLEO_PLUGINS"
		targetextension ".cleo"

		files {
			"source/CLEO_SDK/ClipboardControl/*.h",
			"source/CLEO_SDK/ClipboardControl/*.cpp",
			"resources/*.rc"
		}

	project "FileSystemOperations"
		targetdir "bin/CLEO_PLUGINS"
		targetextension ".cleo"

		files {
			"source/CLEO_SDK/FileSystemOperations/*.h",
			"source/CLEO_SDK/FileSystemOperations/*.cpp",
			"resources/*.rc"
		}

	project "IniFiles"
		targetdir "bin/CLEO_PLUGINS"
		targetextension ".cleo"

		files {
			"source/CLEO_SDK/IniFiles/*.h",
			"source/CLEO_SDK/IniFiles/*.cpp",
			"resources/*.rc"
		}

	project "IntOperations"
		targetdir "bin/CLEO_PLUGINS"
		targetextension ".cleo"

		files {
			"source/CLEO_SDK/IntOperations/*.h",
			"source/CLEO_SDK/IntOperations/*.cpp",
			"resources/*.rc"
		}

	project "MemoryModule"
		targetdir "bin/CLEO_PLUGINS"
		targetextension ".cleo"

		files {
			"source/CLEO_SDK/MemoryModule/*.h",
			"source/CLEO_SDK/MemoryModule/*.cpp",
			"source/CLEO_SDK/MemoryModule/*.c",
			"resources/*.rc"
		}