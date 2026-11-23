#ifndef VADON_RHI_HPP
#define VADON_RHI_HPP
#if defined(VADON_PLATFORM_WIN32) && defined(VADON_LINK_DYNAMIC)
#ifdef VADONRHI_EXPORTS
#define VADONRHI_API __declspec(dllexport)
#else
#define VADONRHI_API __declspec(dllimport)
#endif
#else
#define VADONRHI_API
#endif
#endif