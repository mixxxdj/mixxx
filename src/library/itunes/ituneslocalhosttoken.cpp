#include "library/itunes/ituneslocalhosttoken.h"

#include <QString>

QString iTunesLocalhostToken() {
#if defined(__WINDOWS__)
    return "//localhost/";
#else
    return "//localhost";
#endif
}
