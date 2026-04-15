#include "findonwebmenudiscogs.h"

#include <QMenu>
#include <QUrlQuery>

#include "moc_findonwebmenudiscogs.cpp"
#include "track/track.h"
#include "util/parented_ptr.h"
#include "widget/findonweblast.h"

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

FindOnWebMenuDiscogs::FindOnWebMenuDiscogs(const QPointer<QMenu>& pFindOnWebMenu,
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
            const QUrl discogsUrlArtistWithTrackTitle = composeDiscogsUrl(
                    kSearchUrl, composeSearchQuery(artist, trackTitle), kQueryTypeRelease);
            addActionToServiceMenu(
                    kServiceTitle + QStringLiteral(",Artist,Title"),
                    tr("Artist + Title"),
                    discogsUrlArtistWithTrackTitle);
        }

        if (!album.isEmpty()) {
            const QUrl discogsUrlArtistWithAlbum = composeDiscogsUrl(
                    kSearchUrl, composeSearchQuery(artist, album), kQueryTypeRelease);
            addActionToServiceMenu(
                    kServiceTitle + QStringLiteral(",Artist,Album"),
                    tr("Artist + Album"),
                    discogsUrlArtistWithAlbum);
        }

        const QUrl discogsUrlArtist = composeDiscogsUrl(kSearchUrl, artist, kQueryTypeArtist);
        addActionToServiceMenu(
                kServiceTitle + QStringLiteral(",Artist"),
                tr("Artist"),
                discogsUrlArtist);
    }
    if (!album.isEmpty() && artist.isEmpty()) {
        const QUrl discogsUrlAlbum = composeDiscogsUrl(kSearchUrl, album, kQueryTypeRelease);
        addActionToServiceMenu(
                kServiceTitle + QStringLiteral(",Artist,Album"),
                tr("Album"),
                discogsUrlAlbum);
    }
    if (!trackTitle.isEmpty()) {
        const QUrl discogsUrlTrackTitle =
                composeDiscogsUrl(kSearchUrl, trackTitle, kQueryTypeRelease);
        addActionToServiceMenu(
                kServiceTitle + QStringLiteral(",Title"),
                tr("Title"),
                discogsUrlTrackTitle);
    }
}
