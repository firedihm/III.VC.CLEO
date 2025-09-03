#pragma once

#define CLEO_VERSION_MAIN	2
#define CLEO_VERSION_MAJOR	2
#define CLEO_VERSION_MINOR	0

#ifdef RC_INVOKED
		#define STR_HELPER(x) #x
		#define STR(x) STR_HELPER(x)

		#define CLEO_VERSION CLEO_VERSION_MAIN.CLEO_VERSION_MAJOR.CLEO_VERSION_MINOR
		#define CLEO_VERSION_STR STR(CLEO_VERSION)
#else
		#ifndef CLEO_API
				#ifdef CLEO_IMPORT
						#define CLEO_API __declspec(dllimport)
				#else
						#define CLEO_API __declspec(dllexport)
				#endif
		#endif

		namespace cleo
		{
				constexpr uint version() { return CLEO_VERSION_MAIN << 24 | CLEO_VERSION_MAJOR << 16 | CLEO_VERSION_MINOR << 8; }
				constexpr uint version(uint main, uint major, uint minor) { return main << 24 | major << 16 | minor << 8; }
		}
#endif
