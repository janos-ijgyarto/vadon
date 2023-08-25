#ifndef VADON_GRAPHICSAPI_HPP
#define VADON_GRAPHICSAPI_HPP
	#if defined(VADON_PLATFORM_WIN32) && defined(VADON_LINK_DYNAMIC)
		#ifdef VADONGRAPHICSAPI_EXPORTS
			#define VADONGRAPHICSAPI_API __declspec(dllexport)
		#else
			#define VADONGRAPHICSAPI_API __declspec(dllimport)
		#endif
	#else
		#define VADONGRAPHICSAPI_API
	#endif
#endif