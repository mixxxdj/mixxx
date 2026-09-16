#include "findonwebmenuyoutubemusic.h"

#include <QMenu>
#include <QUrlQuery>

#include "moc_findonwebmenuyoutubemusic.cpp"
#include "track/track.h"
#include "util/parented_ptr.h"

namespace {
const QString kServiceTitle = QStringLiteral("YouTube Music");

const QString kSearchUrl = QStringLiteral("https://music.youtube.com/search");

const QUrl composeYouTubeMusicUrl(const QString& query) {
    QUrlQuery urlQuery;
    urlQuery.addQueryItem("q", query);
    QUrl url(kSearchUrl);
    url.setQuery(urlQuery);
    return url;
}
} // namespace

FindOnWebMenuYouTubeMusic::FindOnWebMenuYouTubeMusic(const QPointer<QMenu>& pFindOnWebMenu,
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
            const QUrl YouTubeMusicUrlArtistWithTrackTitle =
                    composeYouTubeMusicUrl(composeSearchQuery(artist, trackTitle));
            addActionToServiceMenu(
                    kServiceTitle + QStringLiteral(",Artist,Title"),
                    tr("Artist + Title"),
                    YouTubeMusicUrlArtistWithTrackTitle);
        }
        if (!album.isEmpty()) {
            const QUrl YouTubeMusicUrlArtistWithAlbum =
                    composeYouTubeMusicUrl(composeSearchQuery(artist, album));
            addActionToServiceMenu(
                    kServiceTitle + QStringLiteral(",Artist,Album"),
                    tr("Artist + Album"),
                    YouTubeMusicUrlArtistWithAlbum);
        }
        const QUrl YouTubeMusicUrlArtist = composeYouTubeMusicUrl(artist);
        addActionToServiceMenu(
                kServiceTitle + QStringLiteral(",Artist"),
                tr("Artist"),
                YouTubeMusicUrlArtist);
    }
    if (!album.isEmpty() && artist.isEmpty()) {
        const QUrl YouTubeMusicUrlAlbum = composeYouTubeMusicUrl(album);
        addActionToServiceMenu(
                kServiceTitle + QStringLiteral(",Album"),
                tr("Album"),
                YouTubeMusicUrlAlbum);
    }
    if (!trackTitle.isEmpty()) {
        const QUrl YouTubeMusicUrlTrackTitle = composeYouTubeMusicUrl(trackTitle);
        addActionToServiceMenu(
                kServiceTitle + QStringLiteral(",Title"),
                tr("Title"),
                YouTubeMusicUrlTrackTitle);
    }
}
