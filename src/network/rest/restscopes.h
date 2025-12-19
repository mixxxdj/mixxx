#pragma once

#ifdef MIXXX_HAS_HTTP_SERVER

#include <QString>
#include <QStringList>

namespace mixxx::network::rest::scopes {

const QString kStatusRead = QStringLiteral("status:read");
const QString kDecksRead = QStringLiteral("decks:read");
const QString kAutoDjRead = QStringLiteral("autodj:read");
const QString kAutoDjWrite = QStringLiteral("autodj:write");
const QString kPlaylistsRead = QStringLiteral("playlists:read");
const QString kPlaylistsWrite = QStringLiteral("playlists:write");
const QString kControlWrite = QStringLiteral("control:write");

inline QStringList defaultReadScopes() {
    return QStringList{
            kStatusRead,
            kDecksRead,
            kAutoDjRead,
            kPlaylistsRead,
    };
}

inline QStringList allScopes() {
    return QStringList{
            kStatusRead,
            kDecksRead,
            kAutoDjRead,
            kAutoDjWrite,
            kPlaylistsRead,
            kPlaylistsWrite,
            kControlWrite,
    };
}

} // namespace mixxx::network::rest::scopes

#endif // MIXXX_HAS_HTTP_SERVER
