#pragma once

#include <QString>

static const QString kiTunesLocalhostToken =
#if defined(__WINDOWS__)
        QStringLiteral("//localhost/");
#else
        QStringLiteral("//localhost");
#endif
