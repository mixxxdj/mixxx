#include "widget/wfindonwebmenu.h"

#include <QDesktopServices>
#include <QScreen>
#include <QUrl>
#include <QUrlQuery>
#include <QtDebug>

#include "track/track.h"
#include "util/math.h"
#include "util/qt.h"
#include "util/widgethelper.h"

namespace {

QString composeActionText(const QString& prefix, const QString& trackProperty) {
    return prefix + QStringLiteral(" | ") + trackProperty;
}

QString composeSearchQuery(const QString& trackAlbumOrTitle, const QString& artist) {
    return trackAlbumOrTitle + QStringLiteral(" ") + artist;
}

QString composePrefixAction(WFindOnWebMenu::TrackSearchProperties trackSearchProperty) {
    if (trackSearchProperty == WFindOnWebMenu::TrackSearchProperties::Title) {
        return QObject::tr("Title");
    } else if (trackSearchProperty == WFindOnWebMenu::TrackSearchProperties::Artist) {
        return QObject::tr("Artist");
    } else if (trackSearchProperty == WFindOnWebMenu::TrackSearchProperties::ArtistAndTitle) {
        return QObject::tr("Title && Artist");
    } else if (trackSearchProperty == WFindOnWebMenu::TrackSearchProperties::ArtistAndAlbum) {
        return QObject::tr("Album && Artist");
    } else {
        return QObject::tr("Album");
    }
}

QString searchUrlSoundCloudArtist = QStringLiteral("https://soundcloud.com/search/people?");

QString searchUrlSoundCloudTitle = QStringLiteral("https://soundcloud.com/search/sounds?");

QString searchUrlSoundCloudAlbum = QStringLiteral("https://soundcloud.com/search/albums?");

QString searchUrlDiscogsGen = QStringLiteral("https://www.discogs.com/search/?");

QString searchUrlLastFmArtist = QStringLiteral("https://www.last.fm/search/artists?");

QString searchUrlLastFmTitle = QStringLiteral("https://www.last.fm/search/tracks?");

QString searchUrlLastFmAlbum = QStringLiteral("https://www.last.fm/search/albums?");

QString defaultUrl = QStringLiteral("https://soundcloud.com/search?");

QString getServiceUrl(WFindOnWebMenu::Service service,
        WFindOnWebMenu::TrackSearchProperties trackSearchProperty) {
    switch (service) {
        {
        case WFindOnWebMenu::Service::Discogs:
            return searchUrlDiscogsGen;
            break;
        }
        {
        case WFindOnWebMenu::Service::LastFm:
            if (trackSearchProperty == WFindOnWebMenu::TrackSearchProperties::Title ||
                    trackSearchProperty == WFindOnWebMenu::TrackSearchProperties::ArtistAndTitle) {
                return searchUrlLastFmTitle;
            } else if (trackSearchProperty == WFindOnWebMenu::TrackSearchProperties::Artist) {
                return searchUrlLastFmArtist;
            } else {
                return searchUrlLastFmAlbum;
            }
            break;
        }
        {
        case WFindOnWebMenu::Service::SoundCloud:
            if (trackSearchProperty == WFindOnWebMenu::TrackSearchProperties::Title ||
                    trackSearchProperty == WFindOnWebMenu::TrackSearchProperties::ArtistAndTitle) {
                return searchUrlSoundCloudTitle;
            } else if (trackSearchProperty == WFindOnWebMenu::TrackSearchProperties::Artist) {
                return searchUrlSoundCloudArtist;
            } else {
                return searchUrlSoundCloudAlbum;
            }
            break;
        }
        {
        default:
            return defaultUrl;
            break;
        }
    }
}
} // namespace

WFindOnWebMenu::WFindOnWebMenu(QWidget* parent)
        : QMenu(tr("Find on Web"), parent) {
}

void WFindOnWebMenu::addSubmenusForServices(const Track& track) {
    WFindOnWebMenu::populateFromTrackProperties(m_pFindOnSoundCloud,
            track,
            QString(tr("SoundCloud")),
            Service::SoundCloud);
    WFindOnWebMenu::populateFromTrackProperties(
            m_pFindOnLastFm, track, QString(tr("LastFm")), Service::LastFm);
    WFindOnWebMenu::populateFromTrackProperties(
            m_pFindOnDiscogs, track, QString(tr("Discogs")), Service::Discogs);
}

void WFindOnWebMenu::addActionsArtist(Service service,
        const QString& artist,
        QMenu* pServiceMenu,
        TrackSearchProperties trackSearchProperty) {
    const QString prefixAction = composePrefixAction(trackSearchProperty);
    switch (service) {
    case Service::SoundCloud: {
        pServiceMenu->addAction(
                composeActionText(prefixAction, artist),
                this,
                [this, service, trackSearchProperty, artist]() {
                    emit triggerBrowser(service, trackSearchProperty, artist);
                });
        break;
    }

    case Service::LastFm: {
        pServiceMenu->addAction(
                composeActionText(prefixAction, artist),
                this,
                [this, service, trackSearchProperty, artist]() {
                    emit triggerBrowser(service, trackSearchProperty, artist);
                });
        break;
    }

    case Service::Discogs: {
        pServiceMenu->addAction(
                composeActionText(prefixAction, artist),
                this,
                [this, service, trackSearchProperty, artist]() {
                    emit triggerBrowser(service, trackSearchProperty, artist);
                });
        break;
    }
    default:
        break;
    }
}

void WFindOnWebMenu::addActionsAlbum(Service service,
        const QString& albumName,
        QMenu* pServiceMenu,
        TrackSearchProperties trackSearchProperty) {
    const QString prefixAction = composePrefixAction(trackSearchProperty);
    switch (service) {
    case Service::SoundCloud: {
        pServiceMenu->addAction(
                composeActionText(prefixAction, albumName),
                this,
                [this, service, trackSearchProperty, albumName]() {
                    emit triggerBrowser(service, trackSearchProperty, albumName);
                });
        break;
    }

    case Service::LastFm: {
        pServiceMenu->addAction(
                composeActionText(prefixAction, albumName),
                this,
                [this, service, trackSearchProperty, albumName]() {
                    emit triggerBrowser(service, trackSearchProperty, albumName);
                });
        break;
    }

    case Service::Discogs: {
        pServiceMenu->addAction(
                composeActionText(prefixAction, albumName),
                this,
                [this, service, trackSearchProperty, albumName]() {
                    emit triggerBrowser(service, trackSearchProperty, albumName);
                });
        break;
    }
    default:
        break;
    }
}

void WFindOnWebMenu::addActionsTrackTitle(Service service,
        const QString& trackTitle,
        QMenu* pServiceMenu,
        TrackSearchProperties trackSearchProperty) {
    const QString prefixAction = composePrefixAction(trackSearchProperty);
    switch (service) {
    case Service::SoundCloud: {
        pServiceMenu->addAction(
                composeActionText(prefixAction, trackTitle),
                this,
                [this, service, trackSearchProperty, trackTitle]() {
                    emit triggerBrowser(service, trackSearchProperty, trackTitle);
                });
        break;
    }

    case Service::LastFm: {
        pServiceMenu->addAction(
                composeActionText(prefixAction, trackTitle),
                this,
                [this, service, trackSearchProperty, trackTitle]() {
                    emit triggerBrowser(service, trackSearchProperty, trackTitle);
                });
        break;
    }

    case Service::Discogs: {
        pServiceMenu->addAction(
                composeActionText(prefixAction, trackTitle),
                this,
                [this, service, trackSearchProperty, trackTitle]() {
                    emit triggerBrowser(service, trackSearchProperty, trackTitle);
                });
        break;
    }
    default:
        break;
    }
}

void WFindOnWebMenu::openInBrowser(Service service,
        TrackSearchProperties trackSearchProperty,
        const QString& query) {
    const QString serviceUrl = getServiceUrl(service, trackSearchProperty);
    QUrlQuery urlQuery;
    urlQuery.addQueryItem("q", query);
    QUrl url(serviceUrl);
    url.setQuery(urlQuery);
    QDesktopServices::openUrl(url);
}

void WFindOnWebMenu::populateFromTrackProperties(QMenu* pServiceMenu,
        const Track& track,
        const QString& serviceTitle,
        Service service) {
    const auto artist = track.getArtist();
    pServiceMenu = new QMenu(this);
    pServiceMenu->setTitle(serviceTitle);
    addMenu(pServiceMenu);
    addSeparator();
    {
        const auto trackTitle = track.getTitle();
        const auto artistWithTrackTitle = composeSearchQuery(trackTitle, artist);
        if (!trackTitle.isEmpty()) {
            addActionsTrackTitle(service,
                    trackTitle,
                    pServiceMenu,
                    WFindOnWebMenu::TrackSearchProperties::Title);
            addActionsTrackTitle(service,
                    artistWithTrackTitle,
                    pServiceMenu,
                    WFindOnWebMenu::TrackSearchProperties::ArtistAndTitle);
        }
    }
    {
        const auto album = track.getAlbum();
        const auto artistWithAlbum = composeSearchQuery(album, artist);
        if (!album.isEmpty()) {
            addActionsAlbum(service,
                    artistWithAlbum,
                    pServiceMenu,
                    WFindOnWebMenu::TrackSearchProperties::ArtistAndAlbum);
        }
    }
    {
        if (!artist.isEmpty()) {
            addActionsArtist(service,
                    artist,
                    pServiceMenu,
                    WFindOnWebMenu::TrackSearchProperties::Artist);
        }
    }
}
