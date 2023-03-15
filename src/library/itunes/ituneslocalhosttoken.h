#pragma once

#include <QString>

static const QString kiTunesLocalhostToken = QStringLiteral(
        "//localhost"
#if defined(__WINDOWS__)
        "/"
#endif
);
