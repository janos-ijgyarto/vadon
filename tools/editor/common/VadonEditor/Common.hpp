#ifndef VADONEDITOR_COMMON_HPP
#define VADONEDITOR_COMMON_HPP
	#if defined(VADON_PLATFORM_WIN32) && defined(VADON_LINK_DYNAMIC)
	#ifdef VADONEDITORCOMMON_EXPORTS
	#define VADONEDITORCOMMON_API __declspec(dllexport)
	#else
	#define VADONEDITORCOMMON_API __declspec(dllimport)
	#endif
	#else
	#define VADONEDITORCOMMON_API
	#endif
#endif