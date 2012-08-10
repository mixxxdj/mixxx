/***************************************************************************
                          promotrackswebview.cpp
                             -------------------
    begin                : Jan 2010
    copyright            : (C) 2010 Albert Santoni
    email                : alberts@mixxx.org
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QtXml>
#include <QDebug>
#include <QDesktopServices>
#include "featuredartistswebview.h"

#define LOAD_TIMEOUT 10000

FeaturedArtistsWebView::FeaturedArtistsWebView(QWidget* parent, QString libraryPath, QString remoteURL, SongDownloader* downloader) : QWebView(parent), LibraryView()
{
    m_sLibraryPath = libraryPath;
    m_sRemoteURL = remoteURL;
    m_sLocalErrorURL = "about:qt";
    m_bOfflineMode = false;
    m_pSongDownloader = downloader;

    QWidget::setContextMenuPolicy(Qt::PreventContextMenu);

    //Allow us to catch if opening the HTML file on promo.mixxx.org
    //fails, and display a local copy instead.
    connect(this, SIGNAL(loadFinished(bool)),
            this, SLOT(handleLoadFinished(bool)));

    //Load the promo tracks webpage
    QWebView::load(QUrl(m_sRemoteURL));

    //Let us manually handle links that are clicked via the linkClicked()
    //signal...
    QWebPage* page = QWebView::page();
    page->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);

    connect(this, SIGNAL(linkClicked(const QUrl&)),
            this, SLOT(handleClickedLink(const QUrl&)));

    QTimer* loadingTimer = new QTimer(this);
    connect(loadingTimer, SIGNAL(timeout()),
            this, SLOT(checkWebpageLoadingProgress()));
    loadingTimer->start(LOAD_TIMEOUT);
}


/* Google Analytics doesn't like our crappy malformed "Mixxx 1.8" string
   as a user agent. Let Qt construct it for us instead by leaving this commented out.
QString FeaturedArtistsWebView::userAgentForUrl (const QUrl & url) const
{
    return QCoreApplication::applicationName() + " " + QCoreApplication::applicationVersion();
} */

void FeaturedArtistsWebView::handleLoadFinished(bool ok)
{
    //If the remote webpage failed to load, show the
    //local copy of it.
    if (!ok)
    {
        /* This doesn't work inside this signal handler for some reason:
        QWebView::stop();
        QWebView::load(QUrl(m_sLocalErrorURL));
        */
        m_bOfflineMode = true;
        qDebug() << "PROMO: handleLoadFinished, error loading page!";
    }
}

void FeaturedArtistsWebView::checkWebpageLoadingProgress()
{
    if (QWebView::page()->bytesReceived() == 0) {
        qDebug() << "PROMO: Load timed out, loading local page";
        QWebView::stop();
        QWebView::load(QUrl(m_sLocalErrorURL));
        m_bOfflineMode = true;
    }
}

FeaturedArtistsWebView::~FeaturedArtistsWebView()
{

}

void FeaturedArtistsWebView::setup(QDomNode node)
{
	Q_UNUSED(node);
}

void FeaturedArtistsWebView::handleClickedLink(const QUrl& url)
{
    qDebug() << "link clicked!" << url;

    /*
    if (url.scheme() == "deck1")
    {
        TrackInfoObject* track = new TrackInfoObject(m_sMixxxPath + "/" + url.path());
        emit(loadTrackToPlayer(track, "[Channel1]"));
    }
    else if (url.scheme() == "deck2")
    {
        TrackInfoObject* track = new TrackInfoObject(m_sMixxxPath + "/" + url.path());
        emit(loadTrackToPlayer(track, "[Channel2]"));
    }
    */
    if (url.host().contains("mixxx.org")) {
        //Allow navigation through any Mixxx site in the browser
        this->load(url);
    }
    else
    {
        QDesktopServices::openUrl(url);
    }
}

//TODO: Implement this for MIDI control
void FeaturedArtistsWebView::keyPressEvent(QKeyEvent* event)
{
    Q_UNUSED(event);
    //Look at WTrackTableView::keyPressEvent(...) for some
    //code to start with...
}
