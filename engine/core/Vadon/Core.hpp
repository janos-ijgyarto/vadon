#ifndef VADON_CORE_HPP
#define VADON_CORE_HPP
	#if defined(VADON_PLATFORM_WIN32) && defined(VADON_LINK_DYNAMIC)
		#ifdef VADONCORE_EXPORTS
			#define VADONCORE_API __declspec(dllexport)
		#else
			#define VADONCORE_API __declspec(dllimport)
		#endif
	#else
		#define VADONCORE_API
	#endif
#endif