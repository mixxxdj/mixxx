#include "findonwebmenulastfm.h"

#include <QMenu>
#include <QUrlQuery>

#include "moc_findonwebmenulastfm.cpp"
#include "track/track.h"
#include "util/parented_ptr.h"

namespace {
const QString kServiceTitle = QStringLiteral("LastFm");

const QString kSearchUrlArtist = QStringLiteral("https://www.last.fm/search/artists?");

const QString kSearchUrlTitle = QStringLiteral("https://www.last.fm/search/tracks?");

const QString kSearchUrlAlbum = QStringLiteral("https://www.last.fm/search/albums?");

const QUrl composeLastfmUrl(const QString& serviceSearchUrl,
        const QString& query) {
    QUrlQuery urlQuery;
    urlQuery.addQueryItem("q", query);
    QUrl url(serviceSearchUrl);
    url.setQuery(urlQuery);
    return url;
}

} //namespace

FindOnWebMenuLastfm::FindOnWebMenuLastfm(QMenu* pFindOnWebMenu, const Track& track)
        : WFindOnWebMenu(pFindOnWebMenu) {
    const QString artist = track.getArtist();
    const QString trackTitle = track.getTitle();
    const QString album = track.getAlbum();
    setTitle(kServiceTitle);
    pFindOnWebMenu->addMenu(this);
    addSeparator();
    if (!artist.isEmpty()) {
        if (!artist.isEmpty()) {
            const QUrl lastfmUrlArtistWithTrackTitle =
                    composeLastfmUrl(kSearchUrlTitle, composeSearchQuery(artist, trackTitle));
            addActionToServiceMenu(
                    kServiceTitle + QStringLiteral(",Artist,Title"),
                    tr("Artist + Title"),
                    lastfmUrlArtistWithTrackTitle);
        }

        const QUrl lastfmUrlArtist = composeLastfmUrl(kSearchUrlArtist, artist);
        addActionToServiceMenu(
                kServiceTitle + QStringLiteral(",Artist"),
                tr("Artist"),
                lastfmUrlArtist);
    }

    if (!album.isEmpty()) {
        const QUrl lastfmUrlAlbum = composeLastfmUrl(kSearchUrlAlbum, album);
        addActionToServiceMenu(
                kServiceTitle + QStringLiteral(",Album"),
                tr("Album"),
                lastfmUrlAlbum);
    }
    if (!trackTitle.isEmpty()) {
        const QUrl lastfmUrlTrackTitle = composeLastfmUrl(kSearchUrlTitle, trackTitle);
        addActionToServiceMenu(
                kServiceTitle + QStringLiteral(",Title"),
                tr("Title"),
                lastfmUrlTrackTitle);
    }
}
