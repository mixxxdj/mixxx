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
        QMenu* pFindOnWebMenu, const Track& track) {
    const QString artist = track.getArtist();
    const QString trackTitle = track.getTitle();
    const QString album = track.getAlbum();
    auto pSoundcloudMenu = make_parented<QMenu>(pFindOnWebMenu);
    pSoundcloudMenu->setTitle(kServiceTitle);
    pFindOnWebMenu->addMenu(pSoundcloudMenu);
    pSoundcloudMenu->addSeparator();
    if (!artist.isEmpty()) {
        const QUrl SoundcloudUrlArtist = composeSoundcloudUrl(kSearchUrlArtist, artist);
        addActionToServiceMenu(pSoundcloudMenu,
                composeActionText(tr("Artist"), artist),
                SoundcloudUrlArtist);
    }
    if (!trackTitle.isEmpty()) {
        if (!artist.isEmpty()) {
            const QString artistWithTrackTitle = composeSearchQuery(artist, trackTitle);
            const QUrl SoundcloudUrlArtistWithTrackTitle =
                    composeSoundcloudUrl(kSearchUrlTitle, artistWithTrackTitle);
            addActionToServiceMenu(pSoundcloudMenu,
                    composeActionText(
                            tr("Artist + Title"), artistWithTrackTitle),
                    SoundcloudUrlArtistWithTrackTitle);
        }
        const QUrl SoundcloudUrlTrackTitle = composeSoundcloudUrl(kSearchUrlTitle, trackTitle);
        addActionToServiceMenu(pSoundcloudMenu,
                composeActionText(tr("Title"), trackTitle),
                SoundcloudUrlTrackTitle);
    }
    if (!album.isEmpty()) {
        if (!artist.isEmpty()) {
            const QString artistWithAlbum = composeSearchQuery(artist, album);
            const QUrl SoundcloudUrlArtistWithAlbum =
                    composeSoundcloudUrl(kSearchUrlAlbum, artistWithAlbum);
            addActionToServiceMenu(pSoundcloudMenu,
                    composeActionText(tr("Artist + Album"), artistWithAlbum),
                    SoundcloudUrlArtistWithAlbum);
        } else {
            const QUrl SoundcloudUrlAlbum = composeSoundcloudUrl(kSearchUrlAlbum, album);
            addActionToServiceMenu(pSoundcloudMenu,
                    composeActionText(tr("Album"), album),
                    SoundcloudUrlAlbum);
        }
    }
}
