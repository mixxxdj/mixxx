#include "findonwebmenuspotify.h"

#include <QMenu>
#include <QUrl>

#include "moc_findonwebmenuspotify.cpp"
#include "track/track.h"
#include "util/parented_ptr.h"

namespace {
const QString kServiceTitle = QStringLiteral("Spotify");
const QString kSearchUrl = QStringLiteral("https://open.spotify.com/search/");

QString quote(const QString& text) {
    // Escapes double quotes inside the text for Spotify's query syntax.
    QString quoted = text;
    quoted.replace('"', QStringLiteral("\\\""));
    return QStringLiteral("\"%1\"").arg(quoted);
}

QString composeSpotifySearchQuery(
        const QString& artist, const QString& title, const QString& album) {
    QStringList queryParts;
    if (!artist.isEmpty()) {
        queryParts << QStringLiteral("artist:") + quote(artist);
    }
    if (!title.isEmpty()) {
        queryParts << QStringLiteral("track:") + quote(title);
    }
    if (!album.isEmpty()) {
        queryParts << QStringLiteral("album:") + quote(album);
    }
    return queryParts.join(' ');
}

QUrl composeSpotifyUrl(const QString& query) {
    // Spotify expects the query as path, URL-encoded.
    return QUrl(kSearchUrl + QUrl::toPercentEncoding(query));
}
} // namespace

FindOnWebMenuSpotify::FindOnWebMenuSpotify(const QPointer<QMenu>& pFindOnWebMenu,
        QPointer<FindOnWebLast> pFindOnWebLast,
        const Track& track)
        : WFindOnWebMenu(pFindOnWebMenu, std::move(pFindOnWebLast)) {
    const QString artist = track.getArtist();
    const QString trackTitle = track.getTitle();
    const QString album = track.getAlbum();
    setTitle(kServiceTitle);
    pFindOnWebMenu->addMenu(this);
    addSeparator();
    if (!artist.isEmpty()) {
        if (!trackTitle.isEmpty()) {
            const QString query = composeSpotifySearchQuery(artist, trackTitle, QString());
            addActionToServiceMenu(
                    kServiceTitle + QStringLiteral(",Artist,Title"),
                    tr("Artist + Title"),
                    composeSpotifyUrl(query));
        }
        if (!album.isEmpty()) {
            const QString query = composeSpotifySearchQuery(artist, QString(), album);
            addActionToServiceMenu(
                    kServiceTitle + QStringLiteral(",Artist,Album"),
                    tr("Artist + Album"),
                    composeSpotifyUrl(query));
        }
        {
            const QString query = composeSpotifySearchQuery(artist, QString(), QString());
            addActionToServiceMenu(
                    kServiceTitle + QStringLiteral(",Artist"),
                    tr("Artist"),
                    composeSpotifyUrl(query));
        }
    }
    if (!album.isEmpty() && artist.isEmpty()) {
        const QString query = composeSpotifySearchQuery(QString(), QString(), album);
        addActionToServiceMenu(
                kServiceTitle + QStringLiteral(",Album"),
                tr("Album"),
                composeSpotifyUrl(query));
    }
    if (!trackTitle.isEmpty()) {
        const QString query = composeSpotifySearchQuery(QString(), trackTitle, QString());
        addActionToServiceMenu(
                kServiceTitle + QStringLiteral(",Title"),
                tr("Title"),
                composeSpotifyUrl(query));
    }
}
