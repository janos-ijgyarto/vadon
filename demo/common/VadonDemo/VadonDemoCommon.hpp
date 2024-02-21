#ifndef VADONDEMOCOMMON_VADONDEMOCOMMON_HPP
#define VADONDEMOCOMMON_VADONDEMOCOMMON_HPP
#if defined(VADONDEMO_PLATFORM_WIN32) && defined(VADON_LINK_DYNAMIC)
#ifdef VADONDEMO_EXPORTS
#define VADONDEMO_API __declspec(dllexport)
#else
#define VADONDEMO_API __declspec(dllimport)
#endif
#else
#define VADONDEMO_API
#endif
#endif