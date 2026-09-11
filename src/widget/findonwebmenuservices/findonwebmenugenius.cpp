#include "findonwebmenugenius.h"

#include <QMenu>
#include <QUrlQuery>

#include "moc_findonwebmenugenius.cpp"
#include "track/track.h"
#include "util/parented_ptr.h"

namespace {
const QString kServiceTitle = QStringLiteral("Genius");
const QString kSearchUrl = QStringLiteral("https://genius.com/search");

const QUrl composeGeniusUrl(const QString& query) {
    QUrlQuery urlQuery;
    urlQuery.addQueryItem("q", query);
    QUrl url(kSearchUrl);
    url.setQuery(urlQuery);
    return url;
}
} // namespace

FindOnWebMenuGenius::FindOnWebMenuGenius(const QPointer<QMenu>& pFindOnWebMenu,
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
            const QUrl GeniusUrlArtistWithTrackTitle =
                    composeGeniusUrl(artist + " " + trackTitle);
            addActionToServiceMenu(
                    kServiceTitle + QStringLiteral(",Artist,Title"),
                    tr("Artist + Title"),
                    GeniusUrlArtistWithTrackTitle);
        }
        if (!album.isEmpty()) {
            const QUrl GeniusUrlArtistWithAlbum =
                    composeGeniusUrl(artist + " " + album);
            addActionToServiceMenu(
                    kServiceTitle + QStringLiteral(",Artist,Album"),
                    tr("Artist + Album"),
                    GeniusUrlArtistWithAlbum);
        }
        const QUrl GeniusUrlArtist = composeGeniusUrl(artist);
        addActionToServiceMenu(
                kServiceTitle + QStringLiteral(",Artist"),
                tr("Artist"),
                GeniusUrlArtist);
    }
    if (!album.isEmpty() && artist.isEmpty()) {
        const QUrl GeniusUrlAlbum = composeGeniusUrl(album);
        addActionToServiceMenu(
                kServiceTitle + QStringLiteral(",Album"),
                tr("Album"),
                GeniusUrlAlbum);
    }
    if (!trackTitle.isEmpty()) {
        const QUrl GeniusUrlTrackTitle = composeGeniusUrl(trackTitle);
        addActionToServiceMenu(
                kServiceTitle + QStringLiteral(",Title"),
                tr("Title"),
                GeniusUrlTrackTitle);
    }
}
