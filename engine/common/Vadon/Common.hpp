#ifndef VADON_COMMON_HPP
#define VADON_COMMON_HPP
	#if defined(VADON_PLATFORM_WIN32) && defined(VADON_LINK_DYNAMIC)
		#ifdef VADONCOMMON_EXPORTS
			#define VADONCOMMON_API __declspec(dllexport)
		#else
			#define VADONCOMMON_API __declspec(dllimport)
		#endif
	#else
		#define VADONCOMMON_API
	#endif
#endif