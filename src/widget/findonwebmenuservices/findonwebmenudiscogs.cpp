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
            const QUrl discogsUrlArtistWithTrackTitle = composeDiscogsUrl(
                    kSearchUrl, composeSearchQuery(artist, trackTitle), kQueryTypeRelease);
            addActionToServiceMenu(
                    tr("Artist + Title"),
                    discogsUrlArtistWithTrackTitle);
        }

        if (!album.isEmpty()) {
            const QUrl discogsUrlArtistWithAlbum = composeDiscogsUrl(
                    kSearchUrl, composeSearchQuery(artist, album), kQueryTypeRelease);
            addActionToServiceMenu(
                    tr("Artist + Album"),
                    discogsUrlArtistWithAlbum);
        }

        const QUrl discogsUrlArtist = composeDiscogsUrl(kSearchUrl, artist, kQueryTypeArtist);
        addActionToServiceMenu(
                tr("Artist"), discogsUrlArtist);
    }
    if (!album.isEmpty() && artist.isEmpty()) {
        const QUrl discogsUrlAlbum = composeDiscogsUrl(kSearchUrl, album, kQueryTypeRelease);
        addActionToServiceMenu(
                tr("Album"), discogsUrlAlbum);
    }
    if (!trackTitle.isEmpty()) {
        const QUrl discogsUrlTrackTitle =
                composeDiscogsUrl(kSearchUrl, trackTitle, kQueryTypeRelease);
        addActionToServiceMenu(
                tr("Title"), discogsUrlTrackTitle);
    }
}
