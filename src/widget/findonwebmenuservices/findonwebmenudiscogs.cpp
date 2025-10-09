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

FindOnWebMenuDiscogs::FindOnWebMenuDiscogs(QMenu* pFindOnWebMenu, const Track& track)
        : WFindOnWebMenu(pFindOnWebMenu) {
    const QString artist = track.getArtist();
    const QString trackTitle = track.getTitle();
    const QString album = track.getAlbum();
    setTitle(kServiceTitle);
    pFindOnWebMenu->addMenu(this);
    addSeparator();
    if (!artist.isEmpty()) {
        if (!trackTitle.isEmpty()) {
            const QString artistWithTrackTitle = composeSearchQuery(artist, trackTitle);
            const QUrl discogsUrlArtistWithTrackTitle = composeDiscogsUrl(
                    kSearchUrl, artistWithTrackTitle, kQueryTypeRelease);
            addActionToServiceMenu(
                    composeActionText(
                            tr("Artist + Title"), artistWithTrackTitle),
                    discogsUrlArtistWithTrackTitle);
        }

        if (!album.isEmpty()) {
            const QString artistWithAlbum = composeSearchQuery(artist, album);
            const QUrl discogsUrlArtistWithAlbum = composeDiscogsUrl(
                    kSearchUrl, artistWithAlbum, kQueryTypeRelease);
            addActionToServiceMenu(
                    composeActionText(tr("Artist + Album"), artistWithAlbum),
                    discogsUrlArtistWithAlbum);
        }

        const QUrl discogsUrlArtist = composeDiscogsUrl(kSearchUrl, artist, kQueryTypeArtist);
        addActionToServiceMenu(
                composeActionText(tr("Artist"), artist),
                discogsUrlArtist);
    }
    if (!album.isEmpty() && artist.isEmpty()) {
        const QUrl discogsUrlAlbum = composeDiscogsUrl(kSearchUrl, album, kQueryTypeRelease);
        addActionToServiceMenu(
                composeActionText(tr("Album"), album),
                discogsUrlAlbum);
    }
    if (!trackTitle.isEmpty()) {
        const QUrl discogsUrlTrackTitle =
                composeDiscogsUrl(kSearchUrl, trackTitle, kQueryTypeRelease);
        addActionToServiceMenu(
                composeActionText(tr("Title"), trackTitle),
                discogsUrlTrackTitle);
    }
}
