#include <QtXml>
#include <QDebug>
#include <QDesktopServices>
#include "promotrackswebview.h"

#define LOAD_TIMEOUT 5000

PromoTracksWebView::PromoTracksWebView(QWidget* parent, QString mixxxPath, QString localURL, QString remoteURL) : QWebView(parent), LibraryView()
{
    m_sMixxxPath = mixxxPath;
    m_sLocalURL = localURL;
    m_sRemoteURL = remoteURL;
    m_bOfflineMode = false;

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
    if (!ok)
    {
        /* This doesn't work inside this signal handler for some reason:
        QWebView::stop();
        QWebView::load(QUrl(m_sLocalURL));
        */
        m_bOfflineMode = true;
        qDebug() << "PROMO: Loading local copy at" << m_sLocalURL;
    }
}

void PromoTracksWebView::checkWebpageLoadingProgress()
{
    if (QWebView::page()->bytesReceived() == 0) {
        qDebug() << "PROMO: Load timed out, loading local page";
        QWebView::stop();
        QWebView::load(QUrl(m_sLocalURL));
        m_bOfflineMode = true;
    }
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
