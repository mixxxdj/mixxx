/***************************************************************************
                          bundledsongswebview.cpp
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
#include <QWebFrame>
#include "library/trackcollection.h"
#include "library/dao/trackdao.h"
#include "bundledsongswebview.h"

#define CONFIG_KEY "[Promo]"

BundledSongsWebView::BundledSongsWebView(QWidget* parent,
                                         TrackCollection* trackCollection,
                                         QString promoBundlePath,
                                         QString localURL, bool firstRun,
                                         ConfigObject<ConfigValue>* config) :
    QWebView(parent),
    LibraryView(),
    m_bFirstRun(firstRun),
    m_pConfig(config),
    m_pTrackCollection(trackCollection) {
    m_sPromoBundlePath = promoBundlePath;
    m_sLocalURL = localURL;
    m_statTracking = (int)m_pConfig->getValueString(ConfigKey(CONFIG_KEY,"StatTracking")).toInt();

    //Disable right-click
    QWidget::setContextMenuPolicy(Qt::PreventContextMenu);

    //Hook up a bunch of signals to make this class exposed to the javascript
    //inside our HTML page.
    connect(page()->mainFrame(), SIGNAL(loadStarted()), this, SLOT(attachObjects()));
    attachObjects();
    connect(page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(attachObjects()) );

    //Load the promo tracks webpage
    QWebView::load(QUrl(m_sLocalURL));

    //Let us manually handle links that are clicked via the linkClicked()
    //signal...
    QWebPage* page = QWebView::page();
    page->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);

    connect(this, SIGNAL(linkClicked(const QUrl&)),
            this, SLOT(handleClickedLink(const QUrl&)));
    connect(this, SIGNAL(loadFinished(bool)),
            this, SLOT(loadFinished(bool)));
}

BundledSongsWebView::~BundledSongsWebView()
{

}

void BundledSongsWebView::attachObjects()
{
    //qDebug() << "attachObjects()";
    page()->mainFrame()->addToJavaScriptWindowObject("mixxx", this);
}

void BundledSongsWebView::loadFinished(bool ok) {
    Q_UNUSED(ok);
    if (m_bFirstRun)
        page()->mainFrame()->evaluateJavaScript("splash();");
}

void BundledSongsWebView::onShow() {
    //qDebug() << ">>>>>>BundledSongsWebView::onShow()";
    //Trigger the splash() function that's defined in our HTML page's javascript
    //Qt rocks!
    //if (firstRun())
    //    page()->mainFrame()->evaluateJavaScript("splash();");
    //else
    //    page()->mainFrame()->evaluateJavaScript("showMainStuff(0, 0);");
}

/* Google Analytics doesn't like our crappy malformed "Mixxx 1.8" string
   as a user agent. Let Qt construct it for us instead by leaving this commented out.
QString PromoTracksWebView::userAgentForUrl (const QUrl & url) const
{
    return QCoreApplication::applicationName() + " " + QCoreApplication::applicationVersion();
} */

void BundledSongsWebView::handleClickedLink(const QUrl& url) {
    //qDebug() << "link clicked!" << url;

    if (url.scheme().startsWith("deck"))
    {
        QString location = m_sPromoBundlePath + "/" + url.path();
        QFileInfo fileInfo(location);
        location = fileInfo.absoluteFilePath();

        // Try to get TrackInfoObject* from library, identified by location.
        TrackDAO& trackDao = m_pTrackCollection->getTrackDAO();
        TrackPointer pTrack = trackDao.getTrack(trackDao.getTrackId(location));
        // If not, create a new TrackInfoObject*
        if (pTrack == NULL)
        {
            qDebug () << "Didn't find promo track in the library";
            pTrack = TrackPointer(new TrackInfoObject(location), &QObject::deleteLater);
            //Let's immediately save the track so that the FIXME
            trackDao.saveTrack(pTrack);
        }

        if (url.scheme() == "deck1")
        {
            emit(loadTrackToPlayer(pTrack, "[Channel1]"));
        }
        else if (url.scheme() == "deck2")
        {
            emit(loadTrackToPlayer(pTrack, "[Channel2]"));
        }
    }
    else
    {
        QDesktopServices::openUrl(url);
    }
    //emit(loadTrack(track));
    //int player = 1;
    //emit(loadTrackToPlayer(track, player));
}

//TODO: Implement this for MIDI control
void BundledSongsWebView::keyPressEvent(QKeyEvent* event) {
    Q_UNUSED(event);
    //Look at WTrackTableView::keyPressEvent(...) for some
    //code to start with...
}

bool BundledSongsWebView::statTracking() const {
    return m_statTracking;
};

void BundledSongsWebView::setStatTracking(bool statTracking) {
    //qDebug() << "setStatTracking" << statTracking;
    m_statTracking = statTracking;
    m_pConfig->set(ConfigKey(CONFIG_KEY,"StatTracking"), ConfigValue(m_statTracking));
};


bool BundledSongsWebView::firstRun() const {
    return m_bFirstRun;
};

void BundledSongsWebView::setFirstRun(bool firstRun) {
    m_bFirstRun = firstRun;
};

void BundledSongsWebView::loadSelectedTrack() {
    // Do nothing for now. The web view doesn't have the concept of a selection right now.
}

void BundledSongsWebView::moveSelection(int delta) {
    Q_UNUSED(delta);
    // Do nothing for now. The web view doesn't have the concept of a selection right now.
}
