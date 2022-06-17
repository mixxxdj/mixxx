#include "widget/wfindonmenu.h"

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

QString composeSearchQuery(const QString& trackAlbumOrTitle, const QString& artistTitle) {
    return trackAlbumOrTitle + QStringLiteral(" ") + artistTitle;
}

QString composePrefixAction(WFindOnMenu::TrackProperty trackProperty) {
    if (trackProperty == WFindOnMenu::TrackProperty::Title) {
        return QStringLiteral("Title");
    } else if (trackProperty == WFindOnMenu::TrackProperty::Artist) {
        return QStringLiteral("Artist");
    } else if (trackProperty == WFindOnMenu::TrackProperty::ArtistTitle) {
        return QStringLiteral("Title && Artist");
    } else if (trackProperty == WFindOnMenu::TrackProperty::ArtistAlbum) {
        return QStringLiteral("Album && Artist");
    } else {
        return QStringLiteral("Album");
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

QString getServiceUrl(WFindOnMenu::Service service, WFindOnMenu::TrackProperty trackProperty) {
    switch (service) {
        {
        case WFindOnMenu::Service::Discogs:
            return searchUrlDiscogsGen;
            break;
        }
        {
        case WFindOnMenu::Service::LastFm:
            if (trackProperty == WFindOnMenu::TrackProperty::Title ||
                    trackProperty == WFindOnMenu::TrackProperty::ArtistTitle) {
                return searchUrlLastFmTitle;
            } else if (trackProperty == WFindOnMenu::TrackProperty::Artist) {
                return searchUrlLastFmArtist;
            } else {
                return searchUrlLastFmAlbum;
            }
            break;
        }
        {
        case WFindOnMenu::Service::SoundCloud:
            if (trackProperty == WFindOnMenu::TrackProperty::Title ||
                    trackProperty == WFindOnMenu::TrackProperty::ArtistTitle) {
                return searchUrlSoundCloudTitle;
            } else if (trackProperty == WFindOnMenu::TrackProperty::Artist) {
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

WFindOnMenu::WFindOnMenu(QWidget* parent)
        : QMenu(tr("Find On"), parent) {
}

void WFindOnMenu::createAllSubmenusForWebLookups(const Track& track) {
    WFindOnMenu::populateWebLookUpQueries(m_pFindOnSoundCloud,
            track,
            QString(tr("SoundCloud")),
            Service::SoundCloud);
    WFindOnMenu::populateWebLookUpQueries(
            m_pFindOnLastFm, track, QString(tr("LastFm")), Service::LastFm);
    WFindOnMenu::populateWebLookUpQueries(
            m_pFindOnDiscogs, track, QString(tr("Discogs")), Service::Discogs);
}

void WFindOnMenu::addActionsArtist(
        Service service, const QString& artist, QMenu* pServiceMenu, TrackProperty trackProperty) {
    const QString prefixAction = composePrefixAction(trackProperty);
    switch (service) {
    case Service::SoundCloud: {
        pServiceMenu->addAction(
                composeActionText(prefixAction, artist),
                this,
                [this, service, trackProperty, artist]() {
                    emit triggerBrowser(service, trackProperty, artist);
                });
        break;
    }

    case Service::LastFm: {
        pServiceMenu->addAction(
                composeActionText(prefixAction, artist),
                this,
                [this, service, trackProperty, artist]() {
                    emit triggerBrowser(service, trackProperty, artist);
                });
        break;
    }

    case Service::Discogs: {
        pServiceMenu->addAction(
                composeActionText(prefixAction, artist),
                this,
                [this, service, trackProperty, artist]() {
                    emit triggerBrowser(service, trackProperty, artist);
                });
        break;
    }
    default:
        break;
    }
}

void WFindOnMenu::addActionsAlbum(Service service,
        const QString& albumName,
        QMenu* pServiceMenu,
        TrackProperty trackProperty) {
    const QString prefixAction = composePrefixAction(trackProperty);
    switch (service) {
    case Service::SoundCloud: {
        pServiceMenu->addAction(
                composeActionText(prefixAction, albumName),
                this,
                [this, service, trackProperty, albumName]() {
                    emit triggerBrowser(service, trackProperty, albumName);
                });
        break;
    }

    case Service::LastFm: {
        pServiceMenu->addAction(
                composeActionText(prefixAction, albumName),
                this,
                [this, service, trackProperty, albumName]() {
                    emit triggerBrowser(service, trackProperty, albumName);
                });
        break;
    }

    case Service::Discogs: {
        pServiceMenu->addAction(
                composeActionText(prefixAction, albumName),
                this,
                [this, service, trackProperty, albumName]() {
                    emit triggerBrowser(service, trackProperty, albumName);
                });
        break;
    }
    default:
        break;
    }
}

void WFindOnMenu::addActionsTrackTitle(Service service,
        const QString& trackTitle,
        QMenu* pServiceMenu,
        TrackProperty trackProperty) {
    const QString prefixAction = composePrefixAction(trackProperty);
    switch (service) {
    case Service::SoundCloud: {
        pServiceMenu->addAction(
                composeActionText(prefixAction, trackTitle),
                this,
                [this, service, trackProperty, trackTitle]() {
                    emit triggerBrowser(service, trackProperty, trackTitle);
                });
        break;
    }

    case Service::LastFm: {
        pServiceMenu->addAction(
                composeActionText(prefixAction, trackTitle),
                this,
                [this, service, trackProperty, trackTitle]() {
                    emit triggerBrowser(service, trackProperty, trackTitle);
                });
        break;
    }

    case Service::Discogs: {
        pServiceMenu->addAction(
                composeActionText(prefixAction, trackTitle),
                this,
                [this, service, trackProperty, trackTitle]() {
                    emit triggerBrowser(service, trackProperty, trackTitle);
                });
        break;
    }
    default:
        break;
    }
}

void WFindOnMenu::openInBrowser(Service service,
        TrackProperty trackProperty,
        const QString& query) {
    const QString serviceUrl = getServiceUrl(service, trackProperty);
    QUrlQuery qurlquery;
    qurlquery.addQueryItem("q", query);
    QUrl url(serviceUrl);
    url.setQuery(qurlquery);
    QDesktopServices::openUrl(url);
}

void WFindOnMenu::populateWebLookUpQueries(QMenu* pServiceMenu,
        const Track& track,
        const QString& serviceTitle,
        Service service) {
    const auto artistTitle = track.getArtist();
    pServiceMenu = new QMenu(this);
    pServiceMenu->setTitle(serviceTitle);
    addMenu(pServiceMenu);
    addSeparator();
    {
        const auto trackTitle = track.getTitle();
        const auto artistTitleWithTitle = composeSearchQuery(trackTitle, artistTitle);
        if (!trackTitle.isEmpty()) {
            addActionsTrackTitle(service,
                    trackTitle,
                    pServiceMenu,
                    WFindOnMenu::TrackProperty::Title);
            addActionsTrackTitle(service,
                    artistTitleWithTitle,
                    pServiceMenu,
                    WFindOnMenu::TrackProperty::ArtistTitle);
        }
    }
    {
        const auto albumTitle = track.getAlbum();
        const auto artistTitleWithAlbum = composeSearchQuery(albumTitle, artistTitle);
        if (!albumTitle.isEmpty()) {
            addActionsAlbum(service,
                    artistTitleWithAlbum,
                    pServiceMenu,
                    WFindOnMenu::TrackProperty::ArtistAlbum);
        }
    }
    {
        if (!artistTitle.isEmpty()) {
            addActionsArtist(service,
                    artistTitle,
                    pServiceMenu,
                    WFindOnMenu::TrackProperty::Artist);
        }
    }
}
