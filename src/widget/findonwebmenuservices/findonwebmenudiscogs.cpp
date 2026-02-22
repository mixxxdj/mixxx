#include "findonwebmenudiscogs.h"

#include <QMenu>
#include <QUrlQuery>

#include "moc_findonwebmenudiscogs.cpp"
#include "track/track.h"
#include "util/parented_ptr.h"

namespace {
const QString kServiceTitle = QStringLiteral("Discogs");

const QString kQueryTypeRelease = QStringLiteral("release");

const QString kQueryTypeArtist = QStringLiteral("artist");

const QString kSearchUrl = QStringLiteral(
        "https://www.discogs.com/search/?");

const QUrl composeDiscogsUrl(const QString& serviceDefaultUrl,
        const QString& query,
        const QString& queryType) {
    QUrlQuery urlQuery;
    urlQuery.addQueryItem("q", query);
    urlQuery.addQueryItem("type", queryType);
    QUrl url(serviceDefaultUrl);
    url.setQuery(urlQuery);
    return url;
}
} //namespace

FindOnWebMenuDiscogs::FindOnWebMenuDiscogs(QMenu* pFindOnWebMenu, const Track& track) {
    const QString artist = track.getArtist();
    const QString trackTitle = track.getTitle();
    const QString album = track.getAlbum();
    auto pDiscogsMenu = make_parented<QMenu>(pFindOnWebMenu);
    pDiscogsMenu->setTitle(kServiceTitle);
    pFindOnWebMenu->addMenu(pDiscogsMenu);
    addSeparator();
    if (!artist.isEmpty()) {
        const QUrl discogsUrlArtist = composeDiscogsUrl(kSearchUrl, artist, kQueryTypeArtist);
        addActionToServiceMenu(pDiscogsMenu,
                composeActionText(tr("Artist"), artist),
                discogsUrlArtist);
    }
    if (!trackTitle.isEmpty()) {
        if (!artist.isEmpty()) {
            const QString artistWithTrackTitle = composeSearchQuery(artist, trackTitle);
            const QUrl discogsUrlArtistWithTrackTitle = composeDiscogsUrl(
                    kSearchUrl, artistWithTrackTitle, kQueryTypeRelease);
            addActionToServiceMenu(pDiscogsMenu,
                    composeActionText(
                            tr("Artist + Title"), artistWithTrackTitle),
                    discogsUrlArtistWithTrackTitle);
        }
        const QUrl discogsUrlTrackTitle =
                composeDiscogsUrl(kSearchUrl, trackTitle, kQueryTypeRelease);
        addActionToServiceMenu(pDiscogsMenu,
                composeActionText(tr("Title"), trackTitle),
                discogsUrlTrackTitle);
    }
    if (!album.isEmpty()) {
        if (!artist.isEmpty()) {
            const QString artistWithAlbum = composeSearchQuery(artist, album);
            const QUrl discogsUrlArtistWithAlbum = composeDiscogsUrl(
                    kSearchUrl, artistWithAlbum, kQueryTypeRelease);
            addActionToServiceMenu(pDiscogsMenu,
                    composeActionText(tr("Artist + Album"), artistWithAlbum),
                    discogsUrlArtistWithAlbum);
        } else {
            const QUrl discogsUrlAlbum = composeDiscogsUrl(kSearchUrl, album, kQueryTypeRelease);
            addActionToServiceMenu(pDiscogsMenu,
                    composeActionText(tr("Album"), album),
                    discogsUrlAlbum);
        }
    }
}
