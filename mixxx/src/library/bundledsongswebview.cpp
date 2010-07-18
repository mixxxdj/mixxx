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
#include "bundledsongswebview.h"

#define CONFIG_KEY "[Promo]"

BundledSongsWebView::BundledSongsWebView(QWidget* parent, QString promoBundlePath, 
                                         QString localURL, bool firstRun,
                                         ConfigObject<ConfigValue>* config) : 
                             
    QWebView(parent), 
    LibraryView(), 
    m_bFirstRun(firstRun),
    m_pConfig(config)
{
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

}

BundledSongsWebView::~BundledSongsWebView()
{

}

void BundledSongsWebView::attachObjects()
{
    qDebug() << "attachObjects()";
    page()->mainFrame()->addToJavaScriptWindowObject("mixxx", this);
}

void BundledSongsWebView::setup(QDomNode node)
{

}

void BundledSongsWebView::onShow()
{
    qDebug() << ">>>>>>BundledSongsWebView::onShow()";
    //Trigger the splash() function that's defined in our HTML page's javascript
    //Qt rocks!
    if (firstRun())
        page()->mainFrame()->evaluateJavaScript("splash();");
    //else
    //    page()->mainFrame()->evaluateJavaScript("showMainStuff(0, 0);");
}

void BundledSongsWebView::handleClickedLink(const QUrl& url)
{
    //qDebug() << "link clicked!" << url; 

    if (url.scheme() == "deck1")
    {
        TrackInfoObject* track = new TrackInfoObject(m_sPromoBundlePath + "/" + url.path());
        emit(loadTrackToPlayer(track, 1));
    }
    else if (url.scheme() == "deck2")
    {
        TrackInfoObject* track = new TrackInfoObject(m_sPromoBundlePath + "/" + url.path());
        emit(loadTrackToPlayer(track, 2));
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
void BundledSongsWebView::keyPressEvent(QKeyEvent* event)
{
    //Look at WTrackTableView::keyPressEvent(...) for some
    //code to start with...
}

bool BundledSongsWebView::statTracking() const 
{ 
    return m_statTracking; 
};

void BundledSongsWebView::setStatTracking(bool statTracking) 
{ 
    qDebug() << "setStatTracking" << statTracking;
    m_statTracking = statTracking;
    m_pConfig->set(ConfigKey(CONFIG_KEY,"StatTracking"), ConfigValue(m_statTracking));
};


bool BundledSongsWebView::firstRun() const 
{ 
    return m_bFirstRun; 
};

void BundledSongsWebView::setFirstRun(bool firstRun) 
{ 
    m_bFirstRun = firstRun;
};
