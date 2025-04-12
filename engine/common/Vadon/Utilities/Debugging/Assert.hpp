#ifndef VADON_UTILITIES_DEBUGGING_ASSERT_HPP
#define VADON_UTILITIES_DEBUGGING_ASSERT_HPP
#include <Vadon/Common.hpp>
#include <format>
#include <source_location>

// Taken from https://github.com/foonathan/debug_assert
#ifndef VADON_UNREACHABLE
#    ifdef __GNUC__
#        define VADON_UNREACHABLE __builtin_unreachable()
#    elif defined(_MSC_VER)
#        define VADON_UNREACHABLE __assume(false)
#    else
#        define VADON_UNREACHABLE
#    endif
#endif

#ifndef VADON_FORCE_INLINE
#    ifdef __GNUC__
#        define VADON_FORCE_INLINE [[gnu::always_inline]] inline
#    elif defined(_MSC_VER)
#        define VADON_FORCE_INLINE __forceinline
#    else
/// Strong hint to the compiler to inline a function.
/// Define it yourself prior to including the header to override it.
#        define VADON_FORCE_INLINE inline
#    endif
#endif

// Taken from SDL3:
#if !defined(VADON_ASSERT_LEVEL)
    #ifdef VADON_DEFAULT_ASSERT_LEVEL
        #define VADON_ASSERT_LEVEL VADON_DEFAULT_ASSERT_LEVEL
    #elif defined(_DEBUG) || defined(DEBUG) || (defined(__GNUC__) && !defined(__OPTIMIZE__))
        #define VADON_ASSERT_LEVEL 2
    #else
        #define VADON_ASSERT_LEVEL 1
    #endif
#endif

#if defined(_MSC_VER) && _MSC_VER >= 1310
    /* Don't include intrin.h here because it contains C++ code */
    extern void __cdecl __debugbreak(void);
    #define VADON_TRIGGER_BREAKPOINT() __debugbreak()
#elif defined(_MSC_VER) && defined(_M_IX86)
    #define VADON_TRIGGER_BREAKPOINT() { _asm { int 0x03 }  }
#elif VADON_TRIGGER_BREAKPOINT(__builtin_debugtrap)
    #define VADON_TRIGGER_BREAKPOINT() __builtin_debugtrap()
#elif (defined(__GNUC__) || defined(__clang__)) && (defined(__i386__) || defined(__x86_64__))
    #define VADON_TRIGGER_BREAKPOINT() __asm__ __volatile__ ( "int $3\n\t" )
#elif (defined(__GNUC__) || defined(__clang__)) && defined(__riscv)
    #define VADON_TRIGGER_BREAKPOINT() __asm__ __volatile__ ( "ebreak\n\t" )
#elif ( defined(SDL_PLATFORM_APPLE) && (defined(__arm64__) || defined(__aarch64__)) )  /* this might work on other ARM targets, but this is a known quantity... */
    #define VADON_TRIGGER_BREAKPOINT() __asm__ __volatile__ ( "brk #22\n\t" )
#elif defined(SDL_PLATFORM_APPLE) && defined(__arm__)
    #define VADON_TRIGGER_BREAKPOINT() __asm__ __volatile__ ( "bkpt #22\n\t" )
#elif defined(_WIN32) && ((defined(__GNUC__) || defined(__clang__)) && (defined(__arm64__) || defined(__aarch64__)) )
    #define VADON_TRIGGER_BREAKPOINT() __asm__ __volatile__ ( "brk #0xF000\n\t" )
#elif defined(__386__) && defined(__WATCOMC__)
    #define VADON_TRIGGER_BREAKPOINT() { _asm { int 0x03 } }
#elif defined(HAVE_SIGNAL_H) && !defined(__WATCOMC__)
    #include <signal.h>
    #define VADON_TRIGGER_BREAKPOINT() raise(SIGTRAP)
#else
    /* VADON_TRIGGER_BREAKPOINT is intentionally left undefined on unknown platforms. */
#endif

#if defined(_MSC_VER)  /* Avoid /W4 warnings. */
/* "while (0,0)" fools Microsoft's compiler's /W4 warning level into thinking
    this condition isn't constant. And looks like an owl's face! */
#define VADON_NULL_WHILE_LOOP_CONDITION (0,0)
#else
#define VADON_NULL_WHILE_LOOP_CONDITION (0)
#endif

#define VADON_DISABLED_ASSERT(condition) do { (void) sizeof ((condition)); } while (VADON_NULL_WHILE_LOOP_CONDITION)
#define VADON_DISABLED_ERROR(message)

#if !defined(VADON_ASSERT_BREAKPOINT)
#  if defined(ANDROID) && defined(assert)
/* Define this as empty in case assert() is defined as VADON_ASSERT */
#    define VADON_ASSERT_BREAKPOINT()
#  else
#    define VADON_ASSERT_BREAKPOINT() VADON_TRIGGER_BREAKPOINT()
#  endif
#endif

namespace Vadon::Utilities
{
    VADONCOMMON_API void do_assert(const char* expression_string, std::string_view message, std::source_location location = std::source_location::current());
    VADONCOMMON_API void do_error(std::string_view message, std::source_location location = std::source_location::current());
}

#define VADON_ENABLED_ASSERT(condition, message) do { if ( !(condition) ) { Vadon::Utilities::do_assert(#condition, message); } } while (VADON_NULL_WHILE_LOOP_CONDITION)
#define VADON_ENABLED_ERROR(message) Vadon::Utilities::do_error(message)

/* Enable various levels of assertions. */
#if VADON_ASSERT_LEVEL == 0   /* assertions disabled */
#   define VADON_ASSERT(condition, message) VADON_DISABLED_ASSERT(condition)
#   define VADON_ASSERT_RELEASE(condition, message) VADON_DISABLED_ASSERT(condition)
#   define VADON_ERROR(message) VADON_DISABLED_ERROR(message)
#   define VADON_ERROR_RELEASE(message) VADON_DISABLED_ERROR(message)
#elif VADON_ASSERT_LEVEL == 1  /* release settings. */
#   define VADON_ASSERT(condition, message) VADON_DISABLED_ASSERT(condition)
#   define VADON_ASSERT_RELEASE(condition, message) VADON_ENABLED_ASSERT(condition, message)
#   define VADON_ERROR(message) VADON_DISABLED_ERROR(message)
#   define VADON_ERROR_RELEASE(message) VADON_ENABLED_ERROR(message)
#elif VADON_ASSERT_LEVEL == 2  /* debug settings. */
#   define VADON_ASSERT(condition, message) VADON_ENABLED_ASSERT(condition, message)
#   define VADON_ASSERT_RELEASE(condition, message) VADON_ENABLED_ASSERT(condition, message)
#   define VADON_ERROR(message) VADON_ENABLED_ERROR(message)
#   define VADON_ERROR_RELEASE(message) VADON_ENABLED_ERROR(message)
#else
#   error Unknown assertion level.
#endif

#endif