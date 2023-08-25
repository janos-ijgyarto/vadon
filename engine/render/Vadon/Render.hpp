#ifndef VADON_RENDER_HPP
#define VADON_RENDER_HPP
	#if defined(VADON_PLATFORM_WIN32) && defined(VADON_LINK_DYNAMIC)
		#ifdef VADONRENDER_EXPORTS
			#define VADONRENDER_API __declspec(dllexport)
		#else
			#define VADONRENDER_API __declspec(dllimport)
		#endif
	#else
		#define VADONRENDER_API
	#endif
#endif