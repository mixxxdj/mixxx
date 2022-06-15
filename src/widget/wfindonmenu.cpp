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

QString actionPrefixSuffixSeparator(const QString& propertyType, const QString& text) {
    return propertyType + QStringLiteral(" | ") + text;
}

QString searchUrlSoundCloudArtist = QStringLiteral("https://soundcloud.com/search/people?");

QString searchUrlSoundCloudTitle = QStringLiteral("https://soundcloud.com/search/sounds?");

QString searchUrlSoundCloudAlbum = QStringLiteral("https://soundcloud.com/search/albums?");

QString searchUrlDiscogsGen = QStringLiteral("https://www.discogs.com/search/?");

QString searchUrlLastFmArtist = QStringLiteral("https://www.last.fm/search/artists?");

QString searchUrlLastFmTitle = QStringLiteral("https://www.last.fm/search/tracks?");

QString searchUrlLastFmAlbum = QStringLiteral("https://www.last.fm/search/albums?");
} // namespace

WFindOnMenu::WFindOnMenu(QWidget* parent)
        : QMenu(tr("Find On"), parent) {
}

void WFindOnMenu::createAllServices(const Track& track) {
    WFindOnMenu::createService(m_pFindOnSoundCloud,
            track,
            QString(tr("SoundCloud")),
            Service::SoundCloud);
    WFindOnMenu::createService(m_pFindOnLastFm, track, QString(tr("LastFm")), Service::LastFm);
    WFindOnMenu::createService(m_pFindOnDiscogs, track, QString(tr("Discogs")), Service::Discogs);
}

void WFindOnMenu::addActionsArtist(
        Service service, const QString& artist, QMenu* m_pService) {
    const QString prefixActionArtist = tr("Artist");
    switch (service) {
    case Service::SoundCloud: {
        const auto soundCloudUrl = searchUrlSoundCloudArtist;
        m_pService->addAction(
                actionPrefixSuffixSeparator(artist, prefixActionArtist),
                this,
                [this, soundCloudUrl, artist]() {
                    emit triggerBrowser(soundCloudUrl, artist);
                });
        break;
    }

    case Service::LastFm: {
        const auto lastFmUrl = searchUrlLastFmArtist;
        m_pService->addAction(
                actionPrefixSuffixSeparator(artist, prefixActionArtist),
                this,
                [this, lastFmUrl, artist]() {
                    emit triggerBrowser(lastFmUrl, artist);
                });
        break;
    }

    case Service::Discogs: {
        const auto discogsUrl = searchUrlDiscogsGen;
        m_pService->addAction(
                actionPrefixSuffixSeparator(artist, prefixActionArtist),
                this,
                [this, discogsUrl, artist]() {
                    emit triggerBrowser(discogsUrl, artist);
                });
        break;
    }
    default:
        break;
    }
}

void WFindOnMenu::addActionsAlbum(
        Service service, const QString& albumName, QMenu* m_pService) {
    const QString prefixActionAlbum = tr("Album");
    switch (service) {
    case Service::SoundCloud: {
        const auto soundCloudUrl = searchUrlSoundCloudAlbum;
        m_pService->addAction(
                actionPrefixSuffixSeparator(albumName, prefixActionAlbum),
                this,
                [this, soundCloudUrl, albumName]() {
                    emit triggerBrowser(soundCloudUrl, albumName);
                });
        break;
    }

    case Service::LastFm: {
        const auto lastFmUrl = searchUrlLastFmAlbum;
        m_pService->addAction(
                actionPrefixSuffixSeparator(albumName, prefixActionAlbum),
                this,
                [this, lastFmUrl, albumName]() {
                    emit triggerBrowser(lastFmUrl, albumName);
                });
        break;
    }

    case Service::Discogs: {
        const auto discogsUrl = searchUrlDiscogsGen;
        m_pService->addAction(
                actionPrefixSuffixSeparator(albumName, prefixActionAlbum),
                this,
                [this, discogsUrl, albumName]() {
                    emit triggerBrowser(discogsUrl, albumName);
                });
        break;
    }
    default:
        break;
    }
}

void WFindOnMenu::addActionsTrackTitle(
        Service service, const QString& trackTitle, QMenu* m_pService) {
    const QString prefixActionTrackTitle = tr("Title");
    switch (service) {
    case Service::SoundCloud: {
        const auto soundCloudUrl = searchUrlSoundCloudTitle;
        m_pService->addAction(
                actionPrefixSuffixSeparator(trackTitle, prefixActionTrackTitle),
                this,
                [this, soundCloudUrl, trackTitle]() {
                    emit triggerBrowser(soundCloudUrl, trackTitle);
                });
        break;
    }

    case Service::LastFm: {
        const auto lastFmUrl = searchUrlLastFmTitle;
        m_pService->addAction(
                actionPrefixSuffixSeparator(trackTitle, prefixActionTrackTitle),
                this,
                [this, lastFmUrl, trackTitle]() {
                    emit triggerBrowser(lastFmUrl, trackTitle);
                });
        break;
    }

    case Service::Discogs: {
        const auto discogsUrl = searchUrlDiscogsGen;
        m_pService->addAction(
                actionPrefixSuffixSeparator(trackTitle, prefixActionTrackTitle),
                this,
                [this, discogsUrl, trackTitle]() {
                    emit triggerBrowser(discogsUrl, trackTitle);
                });
        break;
    }
    default:
        break;
    }
}

void WFindOnMenu::openTheBrowser(const QString& serviceUrl,
        const QString& query) {
    QUrlQuery qurlquery;
    qurlquery.addQueryItem("q", query);
    QUrl url(serviceUrl);
    url.setQuery(qurlquery);
    QDesktopServices::openUrl(url);
}

void WFindOnMenu::createService(QMenu* serviceMenu,
        const Track& track,
        const QString& serviceTitle,
        Service service) {
    serviceMenu = new QMenu(this);
    serviceMenu->setTitle(serviceTitle);
    addMenu(serviceMenu);
    addSeparator();
    {
        const auto trackTitle = track.getTitle();
        if (!trackTitle.isEmpty()) {
            addActionsTrackTitle(service, trackTitle, serviceMenu);
        }
    }
    {
        const auto albumTitle = track.getAlbum();
        if (!albumTitle.isEmpty()) {
            addActionsAlbum(service, albumTitle, serviceMenu);
        }
    }
    {
        const auto artistTitle = track.getArtist();
        if (!artistTitle.isEmpty()) {
            addActionsArtist(service, artistTitle, serviceMenu);
        }
    }
}
