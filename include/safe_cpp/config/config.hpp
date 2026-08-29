#pragma once

#if defined(__has_include)
#if __has_include(<version>)
#include <version>
#endif
#endif

#if defined(__cpp_lib_expected) && __cpp_lib_expected >= 202202L
#include <expected>
#define SAFE_CPP_HAS_STD_EXPECTED 1
#else
#define SAFE_CPP_HAS_STD_EXPECTED 0
#endif

#if defined(_MSVC_LANG)
#define SAFE_CPP_ACTIVE_STANDARD _MSVC_LANG
#else
#define SAFE_CPP_ACTIVE_STANDARD __cplusplus
#endif

#if SAFE_CPP_ACTIVE_STANDARD > 202002L
#define SAFE_CPP_LANGUAGE_LEVEL 23
#else
#define SAFE_CPP_LANGUAGE_LEVEL 20
#endif
