#pragma once

#include <QString>

namespace mixxx {
namespace rekordboxconstants {
const QString beatsSubversion = QStringLiteral("Rekordbox USB drive");
// Stable key for column layout; independent of table/playlist lifecycle.
const QString kRekordboxHeaderStateKey =
        QStringLiteral("mixxx.rekordbox.header_state_pb");
}
} // namespace mixxx
