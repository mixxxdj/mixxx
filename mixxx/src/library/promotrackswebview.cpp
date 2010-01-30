#include <QtXml>
#include <QDebug>
#include <QDesktopServices>
#include "promotrackswebview.h"

PromoTracksWebView::PromoTracksWebView(QWidget* parent, QString mixxxPath, QString localURL, QString remoteURL) : QWebView(parent), LibraryView()
{
    m_sMixxxPath = mixxxPath;
    m_sLocalURL = localURL;
    m_sRemoteURL = remoteURL;

    //Allow us to catch if opening the HTML file on promo.mixxx.org
    //fails, and display a local copy instead.
    connect(this, SIGNAL(loadFinished(bool)),
            this, SLOT(handleLoadFinished(bool)));
    
    //Load our promo tracks webpage off the disk
    QWebView::load(QUrl(m_sRemoteURL));

    //Let us manually handle links that are clicked via the linkClicked()
    //signal...
    QWebPage* page = QWebView::page();
    page->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);

    connect(this, SIGNAL(linkClicked(const QUrl&)), 
            this, SLOT(handleClickedLink(const QUrl&)));
}

PromoTracksWebView::~PromoTracksWebView()
{

}

void PromoTracksWebView::setup(QDomNode node)
{

}

void PromoTracksWebView::handleLoadFinished(bool ok)
{
    //If the remote webpage failed to load, show the
    //local copy of it.
    /*
    if (!ok)
    {
        QWebView::stop();
        QWebView::load(QUrl(m_sLocalURL));
        qDebug() << "PROMO: Loading local copy at" << m_sLocalURL;
    }
    */
}

void PromoTracksWebView::handleClickedLink(const QUrl& url)
{
    qDebug() << "link clicked!" << url; 

    if (url.scheme() == "deck1")
    {
        TrackInfoObject* track = new TrackInfoObject(m_sMixxxPath + "/" + url.path());
        emit(loadTrackToPlayer(track, 1));
    }
    else if (url.scheme() == "deck2")
    {
        TrackInfoObject* track = new TrackInfoObject(m_sMixxxPath + "/" + url.path());
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
void PromoTracksWebView::keyPressEvent(QKeyEvent* event)
{
    //Look at WTrackTableView::keyPressEvent(...) for some
    //code to start with...
}
