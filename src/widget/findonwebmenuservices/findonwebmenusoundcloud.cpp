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
        QMenu* pFindOnWebMenu, const Track& track)
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
            const QUrl SoundcloudUrlArtistWithTrackTitle =
                    composeSoundcloudUrl(kSearchUrlTitle, artistWithTrackTitle);
            addActionToServiceMenu(
                    composeActionText(
                            tr("Artist + Title"), artistWithTrackTitle),
                    SoundcloudUrlArtistWithTrackTitle);
        }
        if (!album.isEmpty()) {
            const QString artistWithAlbum = composeSearchQuery(artist, album);
            const QUrl SoundcloudUrlArtistWithAlbum =
                    composeSoundcloudUrl(kSearchUrlAlbum, artistWithAlbum);
            addActionToServiceMenu(
                    composeActionText(tr("Artist + Album"), artistWithAlbum),
                    SoundcloudUrlArtistWithAlbum);
        }
        const QUrl SoundcloudUrlArtist = composeSoundcloudUrl(kSearchUrlArtist, artist);
        addActionToServiceMenu(
                composeActionText(tr("Artist"), artist),
                SoundcloudUrlArtist);
    }
    if (!album.isEmpty() && artist.isEmpty()) {
        const QUrl SoundcloudUrlAlbum = composeSoundcloudUrl(kSearchUrlAlbum, album);
        addActionToServiceMenu(
                composeActionText(tr("Album"), album),
                SoundcloudUrlAlbum);
    }
    if (!trackTitle.isEmpty()) {
        const QUrl SoundcloudUrlTrackTitle = composeSoundcloudUrl(kSearchUrlTitle, trackTitle);
        addActionToServiceMenu(
                composeActionText(tr("Title"), trackTitle),
                SoundcloudUrlTrackTitle);
    }
}
