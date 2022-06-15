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
            Services::SoundCloud);
    WFindOnMenu::createService(m_pFindOnLastFm, track, QString(tr("LastFm")), Services::LastFm);
    WFindOnMenu::createService(m_pFindOnDiscogs, track, QString(tr("Discogs")), Services::Discogs);
}

void WFindOnMenu::addActionsArtist(
        Services serviceTitle, const QString& artist, QMenu* m_pService) {
    const QString prefixActionArtist = tr("Artist");
    switch (serviceTitle) {
    case Services::SoundCloud: {
        const auto soundCloudUrl = searchUrlSoundCloudArtist;
        m_pService->addAction(
                actionPrefixSuffixSeparator(artist, prefixActionArtist),
                this,
                [this, soundCloudUrl, artist]() {
                    emit triggerBrowser(soundCloudUrl, artist);
                });
        break;
    }

    case Services::LastFm: {
        const auto lastFmUrl = searchUrlLastFmArtist;
        m_pService->addAction(
                actionPrefixSuffixSeparator(artist, prefixActionArtist),
                this,
                [this, lastFmUrl, artist]() {
                    emit triggerBrowser(lastFmUrl, artist);
                });
        break;
    }

    case Services::Discogs: {
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
        Services serviceTitle, const QString& albumName, QMenu* m_pService) {
    const QString prefixActionAlbum = tr("Album");
    switch (serviceTitle) {
    case Services::SoundCloud: {
        const auto soundCloudUrl = searchUrlSoundCloudAlbum;
        m_pService->addAction(
                actionPrefixSuffixSeparator(albumName, prefixActionAlbum),
                this,
                [this, soundCloudUrl, albumName]() {
                    emit triggerBrowser(soundCloudUrl, albumName);
                });
        break;
    }

    case Services::LastFm: {
        const auto lastFmUrl = searchUrlLastFmAlbum;
        m_pService->addAction(
                actionPrefixSuffixSeparator(albumName, prefixActionAlbum),
                this,
                [this, lastFmUrl, albumName]() {
                    emit triggerBrowser(lastFmUrl, albumName);
                });
        break;
    }

    case Services::Discogs: {
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
        Services serviceTitle, const QString& trackTitle, QMenu* m_pService) {
    const QString prefixActionTrackTitle = tr("Title");
    switch (serviceTitle) {
    case Services::SoundCloud: {
        const auto soundCloudUrl = searchUrlSoundCloudTitle;
        m_pService->addAction(
                actionPrefixSuffixSeparator(trackTitle, prefixActionTrackTitle),
                this,
                [this, soundCloudUrl, trackTitle]() {
                    emit triggerBrowser(soundCloudUrl, trackTitle);
                });
        break;
    }

    case Services::LastFm: {
        const auto lastFmUrl = searchUrlLastFmTitle;
        m_pService->addAction(
                actionPrefixSuffixSeparator(trackTitle, prefixActionTrackTitle),
                this,
                [this, lastFmUrl, trackTitle]() {
                    emit triggerBrowser(lastFmUrl, trackTitle);
                });
        break;
    }

    case Services::Discogs: {
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
        const QString& serviceTitleDisplay,
        Services serviceTitle) {
    serviceMenu = new QMenu(this);
    serviceMenu->setTitle(serviceTitleDisplay);
    addMenu(serviceMenu);
    addSeparator();
    {
        const auto trackTitle = track.getTitle();
        if (!trackTitle.isEmpty()) {
            addActionsTrackTitle(serviceTitle, trackTitle, serviceMenu);
        }
    }
    {
        const auto albumTitle = track.getAlbum();
        if (!albumTitle.isEmpty()) {
            addActionsAlbum(serviceTitle, albumTitle, serviceMenu);
        }
    }
    {
        const auto artistTitle = track.getArtist();
        if (!artistTitle.isEmpty()) {
            addActionsArtist(serviceTitle, artistTitle, serviceMenu);
        }
    }
}
