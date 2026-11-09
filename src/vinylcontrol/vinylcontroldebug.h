#pragma once

#include <QString>
#include <cmath>

// Colors a drift in seconds for the console: green < 10 ms, magenta < 50 ms,
// red otherwise.
inline QString colorDrift(double drift) {
    const double absDrift = std::fabs(drift);
    const char* color;

    if (absDrift < 0.01) {
        color = "\033[32m"; // green
    } else if (absDrift < 0.05) {
        color = "\033[33m"; // magenta
    } else {
        color = "\033[31m"; // red
    }

    return QString::fromLatin1(color) + QString::asprintf("%+9.6f", drift) +
            QStringLiteral("\033[0m");
}
