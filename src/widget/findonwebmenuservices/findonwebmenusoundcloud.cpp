#include "findonwebmenusoundcloud.h"

#include <QMenu>
#include <QUrlQuery>

#include "moc_findonwebmenusoundcloud.cpp"
#include "track/track.h"
#include "util/parented_ptr.h"

namespace {
const QString kServiceTitle = QStringLiteral("Soundcloud");

const QString kSearchUrlArtist = QStringLiteral("https://soundcloud.com/search/people?");

const QString kSearchUrlTitle = QStringLiteral("https://soundcloud.com/search/sounds?");

const QString kSearchUrlAlbum = QStringLiteral("https://soundcloud.com/search/albums?");

const QUrl composeSoundcloudUrl(const QString& serviceSearchUrl,
        const QString& query) {
    QUrlQuery urlQuery;
    urlQuery.addQueryItem("q", query);
    QUrl url(serviceSearchUrl);
    url.setQuery(urlQuery);
    return url;
}
} // namespace

FindOnWebMenuSoundcloud::FindOnWebMenuSoundcloud(
        QMenu* pFindOnWebMenu, FindOnWebLast* pFindOnWebLast, const Track& track)
        : WFindOnWebMenu(pFindOnWebMenu, pFindOnWebLast) {
    const QString artist = track.getArtist();
    const QString trackTitle = track.getTitle();
    const QString album = track.getAlbum();
    setTitle(kServiceTitle);
    pFindOnWebMenu->addMenu(this);
    addSeparator();
    if (!artist.isEmpty()) {
        if (!trackTitle.isEmpty()) {
            const QUrl SoundcloudUrlArtistWithTrackTitle =
                    composeSoundcloudUrl(kSearchUrlTitle, composeSearchQuery(artist, trackTitle));
            addActionToServiceMenu(
                    kServiceTitle + QStringLiteral(",Artist,Title"),
                    tr("Artist + Title"),
                    SoundcloudUrlArtistWithTrackTitle);
        }
        if (!album.isEmpty()) {
            const QUrl SoundcloudUrlArtistWithAlbum =
                    composeSoundcloudUrl(kSearchUrlAlbum, composeSearchQuery(artist, album));
            addActionToServiceMenu(
                    kServiceTitle + QStringLiteral(",Artist,Album"),
                    tr("Artist + Album"),
                    SoundcloudUrlArtistWithAlbum);
        }
        const QUrl SoundcloudUrlArtist = composeSoundcloudUrl(kSearchUrlArtist, artist);
        addActionToServiceMenu(
                kServiceTitle + QStringLiteral(",Artist"),
                tr("Artist"),
                SoundcloudUrlArtist);
    }
    if (!album.isEmpty() && artist.isEmpty()) {
        const QUrl SoundcloudUrlAlbum = composeSoundcloudUrl(kSearchUrlAlbum, album);
        addActionToServiceMenu(
                kServiceTitle + QStringLiteral(",Artist,Album"),
                tr("Album"),
                SoundcloudUrlAlbum);
    }
    if (!trackTitle.isEmpty()) {
        const QUrl SoundcloudUrlTrackTitle = composeSoundcloudUrl(kSearchUrlTitle, trackTitle);
        addActionToServiceMenu(
                kServiceTitle + QStringLiteral(",Title"),
                tr("Title"),
                SoundcloudUrlTrackTitle);
    }
}
