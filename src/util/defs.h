#ifndef DEFS_H
#define DEFS_H

// Support for 'constexpr' is available since Visual Studio 2015
// NOTE(uklotzde) This workaround is ugly, but could easily be
// removed as soon as Visual Studio 2015 is required for building
// Mixxx on Windows.
#if defined(_MSC_VER) && _MSC_VER < 1900
#undef MIXXX_HAS_CONSTEXPR
#define mixxx_constexpr const
#else
#define MIXXX_HAS_CONSTEXPR 1
#define mixxx_constexpr constexpr
#endif

// Used for returning errors from functions.
enum Result {
    OK = 0,
    ERR = -1
};

// Maximum buffer length to each EngineObject::process call.
const unsigned int MAX_BUFFER_LEN = 160000;

#endif /* DEFS_H */
