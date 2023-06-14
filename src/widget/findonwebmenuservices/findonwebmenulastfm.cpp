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

FindOnWebMenuLastfm::FindOnWebMenuLastfm(QMenu* pFindOnWebMenu, const Track& track) {
    const QString artist = track.getArtist();
    const QString trackTitle = track.getTitle();
    const QString album = track.getAlbum();
    auto pLastfmMenu = make_parented<QMenu>(pFindOnWebMenu);
    pLastfmMenu->setTitle(kServiceTitle);
    pFindOnWebMenu->addMenu(pLastfmMenu);
    pLastfmMenu->addSeparator();
    if (!artist.isEmpty()) {
        const QUrl lastfmUrlArtist = composeLastfmUrl(kSearchUrlArtist, artist);
        addActionToServiceMenu(pLastfmMenu,
                composeActionText(tr("Artist"), artist),
                lastfmUrlArtist);
    }
    if (!trackTitle.isEmpty()) {
        if (!artist.isEmpty()) {
            const QString artistWithTrackTitle = composeSearchQuery(artist, trackTitle);
            const QUrl lastfmUrlArtistWithTrackTitle =
                    composeLastfmUrl(kSearchUrlTitle, artistWithTrackTitle);
            addActionToServiceMenu(pLastfmMenu,
                    composeActionText(
                            tr("Artist + Title"), artistWithTrackTitle),
                    lastfmUrlArtistWithTrackTitle);
        }
        const QUrl lastfmUrlTrackTitle = composeLastfmUrl(kSearchUrlTitle, trackTitle);
        addActionToServiceMenu(pLastfmMenu,
                composeActionText(tr("Title"), trackTitle),
                lastfmUrlTrackTitle);
    }
    if (!album.isEmpty()) {
        if (!artist.isEmpty()) {
            const QString artistWithAlbum = composeSearchQuery(artist, album);
            const QUrl lastfmUrlArtistWithAlbum =
                    composeLastfmUrl(kSearchUrlAlbum, artistWithAlbum);
            addActionToServiceMenu(pLastfmMenu,
                    composeActionText(tr("Artist + Album"), artistWithAlbum),
                    lastfmUrlArtistWithAlbum);
        } else {
            const QUrl lastfmUrlAlbum = composeLastfmUrl(kSearchUrlAlbum, album);
            addActionToServiceMenu(pLastfmMenu,
                    composeActionText(tr("Album"), album),
                    lastfmUrlAlbum);
        }
    }
}
