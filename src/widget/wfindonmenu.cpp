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
        Service service, const QString& artist, QMenu* pServiceMenu) {
    const QString prefixActionArtist = tr("Artist");
    switch (service) {
    case Service::SoundCloud: {
        const auto soundCloudUrl = searchUrlSoundCloudArtist;
        pServiceMenu->addAction(
                composeActionText(prefixActionArtist, artist),
                this,
                [this, soundCloudUrl, artist]() {
                    emit triggerBrowser(soundCloudUrl, artist);
                });
        break;
    }

    case Service::LastFm: {
        const auto lastFmUrl = searchUrlLastFmArtist;
        pServiceMenu->addAction(
                composeActionText(prefixActionArtist, artist),
                this,
                [this, lastFmUrl, artist]() {
                    emit triggerBrowser(lastFmUrl, artist);
                });
        break;
    }

    case Service::Discogs: {
        const auto discogsUrl = searchUrlDiscogsGen;
        pServiceMenu->addAction(
                composeActionText(prefixActionArtist, artist),
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
        Service service, const QString& albumName, QMenu* pServiceMenu) {
    const QString prefixActionAlbum = tr("Album");
    switch (service) {
    case Service::SoundCloud: {
        const auto soundCloudUrl = searchUrlSoundCloudAlbum;
        pServiceMenu->addAction(
                composeActionText(prefixActionAlbum, albumName),
                this,
                [this, soundCloudUrl, albumName]() {
                    emit triggerBrowser(soundCloudUrl, albumName);
                });
        break;
    }

    case Service::LastFm: {
        const auto lastFmUrl = searchUrlLastFmAlbum;
        pServiceMenu->addAction(
                composeActionText(prefixActionAlbum, albumName),
                this,
                [this, lastFmUrl, albumName]() {
                    emit triggerBrowser(lastFmUrl, albumName);
                });
        break;
    }

    case Service::Discogs: {
        const auto discogsUrl = searchUrlDiscogsGen;
        pServiceMenu->addAction(
                composeActionText(prefixActionAlbum, albumName),
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
        Service service, const QString& trackTitle, QMenu* pServiceMenu) {
    const QString prefixActionTrackTitle = tr("Title");
    switch (service) {
    case Service::SoundCloud: {
        const auto soundCloudUrl = searchUrlSoundCloudTitle;
        pServiceMenu->addAction(
                composeActionText(prefixActionTrackTitle, trackTitle),
                this,
                [this, soundCloudUrl, trackTitle]() {
                    emit triggerBrowser(soundCloudUrl, trackTitle);
                });
        break;
    }

    case Service::LastFm: {
        const auto lastFmUrl = searchUrlLastFmTitle;
        pServiceMenu->addAction(
                composeActionText(prefixActionTrackTitle, trackTitle),
                this,
                [this, lastFmUrl, trackTitle]() {
                    emit triggerBrowser(lastFmUrl, trackTitle);
                });
        break;
    }

    case Service::Discogs: {
        const auto discogsUrl = searchUrlDiscogsGen;
        pServiceMenu->addAction(
                composeActionText(prefixActionTrackTitle, trackTitle),
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

void WFindOnMenu::openInBrowser(const QString& serviceUrl,
        const QString& query) {
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
    pServiceMenu = new QMenu(this);
    pServiceMenu->setTitle(serviceTitle);
    addMenu(pServiceMenu);
    addSeparator();
    {
        const auto trackTitle = track.getTitle();
        if (!trackTitle.isEmpty()) {
            addActionsTrackTitle(service, trackTitle, pServiceMenu);
        }
    }
    {
        const auto albumTitle = track.getAlbum();
        if (!albumTitle.isEmpty()) {
            addActionsAlbum(service, albumTitle, pServiceMenu);
        }
    }
    {
        const auto artistTitle = track.getArtist();
        if (!artistTitle.isEmpty()) {
            addActionsArtist(service, artistTitle, pServiceMenu);
        }
    }
}
