#ifndef VADONAPP_VADONAPP_HPP
#define VADONAPP_VADONAPP_HPP
	#if defined(VADONAPP_PLATFORM_WIN32) && defined(VADON_LINK_DYNAMIC)
		#ifdef VADONAPP_EXPORTS
			#define VADONAPP_API __declspec(dllexport)
		#else
			#define VADONAPP_API __declspec(dllimport)
		#endif
	#else
		#define VADONAPP_API
	#endif
#endif