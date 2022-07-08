#include "widget/wfindonwebmenu.h"

#include <QDesktopServices>
#include <QScreen>
#include <QUrl>
#include <QUrlQuery>
#include <QtDebug>

#include "track/track.h"
#include "util/math.h"
#include "util/parented_ptr.h"
#include "util/qt.h"
#include "util/widgethelper.h"

namespace {

QString composeActionText(const QString& prefix, const QString& trackProperty) {
    return prefix + QStringLiteral(" | ") + trackProperty;
}

QString composeSearchQuery(const QString& trackAlbumOrTitle, const QString& artist) {
    return trackAlbumOrTitle + QStringLiteral(" ") + artist;
}

QString composePrefixAction(WFindOnWebMenu::TrackSearchProperties trackSearchProperties) {
    switch (trackSearchProperties) {
    case WFindOnWebMenu::TrackSearchProperties::Title:
        return QObject::tr("Title");
    case WFindOnWebMenu::TrackSearchProperties::Artist:
        return QObject::tr("Artist");
    case WFindOnWebMenu::TrackSearchProperties::ArtistAndTitle:
        return QObject::tr("Title + Artist");
    case WFindOnWebMenu::TrackSearchProperties::ArtistAndAlbum:
        return QObject::tr("Album + Artist");
    default:
        return QObject::tr("Album");
    }
}

const QString kSearchUrlSoundCloudArtist = QStringLiteral("https://soundcloud.com/search/people?");

const QString kSearchUrlSoundCloudTitle = QStringLiteral("https://soundcloud.com/search/sounds?");

const QString kSearchUrlSoundCloudAlbum = QStringLiteral("https://soundcloud.com/search/albums?");

const QString kSearchUrlDiscogsGen = QStringLiteral("https://www.discogs.com/search/?");

const QString kSearchUrlLastFmArtist = QStringLiteral("https://www.last.fm/search/artists?");

const QString kSearchUrlLastFmTitle = QStringLiteral("https://www.last.fm/search/tracks?");

const QString kSearchUrlLastFmAlbum = QStringLiteral("https://www.last.fm/search/albums?");

const QString kSearchUrlDefault = QStringLiteral("https://soundcloud.com/search?");

QString getServiceUrl(WFindOnWebMenu::Service service,
        WFindOnWebMenu::TrackSearchProperties trackSearchProperties) {
    switch (service) {
    case WFindOnWebMenu::Service::Discogs:
        return kSearchUrlDiscogsGen;
    case WFindOnWebMenu::Service::LastFm:
        if (trackSearchProperties == WFindOnWebMenu::TrackSearchProperties::Title ||
                trackSearchProperties == WFindOnWebMenu::TrackSearchProperties::ArtistAndTitle) {
            return kSearchUrlLastFmTitle;
        } else if (trackSearchProperties == WFindOnWebMenu::TrackSearchProperties::Artist) {
            return kSearchUrlLastFmArtist;
        } else {
            return kSearchUrlLastFmAlbum;
        }
    case WFindOnWebMenu::Service::SoundCloud:
        if (trackSearchProperties == WFindOnWebMenu::TrackSearchProperties::Title ||
                trackSearchProperties == WFindOnWebMenu::TrackSearchProperties::ArtistAndTitle) {
            return kSearchUrlSoundCloudTitle;
        } else if (trackSearchProperties == WFindOnWebMenu::TrackSearchProperties::Artist) {
            return kSearchUrlSoundCloudArtist;
        } else {
            return kSearchUrlSoundCloudAlbum;
        }
    default:
        return kSearchUrlDefault;
    }
}
} // namespace

WFindOnWebMenu::WFindOnWebMenu(QWidget* parent)
        : QMenu(tr("Find on Web"), parent) {
}

void WFindOnWebMenu::addSubmenusForServices(const Track& track) {
    WFindOnWebMenu::populateFromTrackProperties(
            track,
            QStringLiteral("SoundCloud"),
            Service::SoundCloud);
    WFindOnWebMenu::populateFromTrackProperties(
            track, QStringLiteral("LastFm"), Service::LastFm);
    WFindOnWebMenu::populateFromTrackProperties(
            track, QStringLiteral("Discogs"), Service::Discogs);
}

void WFindOnWebMenu::addActions(Service service,
        const QString& queryValue,
        QMenu* pServiceMenu,
        TrackSearchProperties trackSearchProperties) {
    const QString prefixAction = composePrefixAction(trackSearchProperties);
    pServiceMenu->addAction(
            composeActionText(prefixAction, queryValue),
            this,
            [this, service, trackSearchProperties, queryValue]() {
                openInBrowser(service, trackSearchProperties, queryValue);
            });
}

void WFindOnWebMenu::openInBrowser(Service service,
        TrackSearchProperties trackSearchProperties,
        const QString& queryValue) {
    const QString serviceUrl = getServiceUrl(service, trackSearchProperties);
    QUrlQuery urlQuery;
    urlQuery.addQueryItem("q", queryValue);
    QUrl url(serviceUrl);
    url.setQuery(urlQuery);
    if (!QDesktopServices::openUrl(url)) {
        qWarning() << "QDesktopServices::openUrl() failed for " << url;
        DEBUG_ASSERT(false);
    }
}

bool WFindOnWebMenu::hasEntriesForTrack(const Track& track) {
    return !(track.getArtist().isEmpty() &&
            track.getAlbum().isEmpty() &&
            track.getTitle().isEmpty());
}

void WFindOnWebMenu::populateFromTrackProperties(
        const Track& track,
        const QString& serviceTitle,
        Service service) {
    const QString artist = track.getArtist();
    const QString trackTitle = track.getTitle();
    const QString album = track.getAlbum();
    auto pServiceMenu = make_parented<QMenu>(this);
    pServiceMenu->setTitle(serviceTitle);
    addMenu(pServiceMenu);
    addSeparator();
    if (!artist.isEmpty()) {
        addActions(service,
                artist,
                pServiceMenu,
                WFindOnWebMenu::TrackSearchProperties::Artist);
    }
    if (!trackTitle.isEmpty()) {
        if (!artist.isEmpty()) {
            const QString artistWithTrackTitle = composeSearchQuery(trackTitle, artist);
            addActions(service,
                    artistWithTrackTitle,
                    pServiceMenu,
                    WFindOnWebMenu::TrackSearchProperties::ArtistAndTitle);
        }
        addActions(service,
                trackTitle,
                pServiceMenu,
                WFindOnWebMenu::TrackSearchProperties::Title);
    }
    if (!album.isEmpty()) {
        if (!artist.isEmpty()) {
            const QString artistWithAlbum = composeSearchQuery(album, artist);
            addActions(service,
                    artistWithAlbum,
                    pServiceMenu,
                    WFindOnWebMenu::TrackSearchProperties::ArtistAndAlbum);
        } else {
            addActions(service,
                    album,
                    pServiceMenu,
                    WFindOnWebMenu::TrackSearchProperties::Album);
        }
    }
}
