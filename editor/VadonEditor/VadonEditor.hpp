#ifndef VADONEDITOR_VADONEDITOR_HPP
#define VADONEDITOR_VADONEDITOR_HPP
#if defined(VADON_PLATFORM_WIN32) && defined(VADON_LINK_DYNAMIC)
#ifdef VADONEDITOR_EXPORTS
#define VADONEDITOR_API __declspec(dllexport)
#else
#define VADONEDITOR_API __declspec(dllimport)
#endif
#else
#define VADONEDITOR_API
#endif
#endif