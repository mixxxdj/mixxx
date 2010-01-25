#include <QDebug>
#include <QDesktopServices>
#include "promotrackswebview.h"

PromoTracksWebView::PromoTracksWebView(QWidget* parent) : QWebView(parent), LibraryView()
{
    //Load our promo tracks webpage off the disk
    QWebView::load(QUrl(MIXXX_PROMO_HTML_LOCATION));

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

void PromoTracksWebView::handleClickedLink(const QUrl& url)
{
    qDebug() << "link clicked!" << url; 

    if (url.scheme() == "deck1")
    {
        TrackInfoObject* track = new TrackInfoObject(url.path());
        emit(loadTrackToPlayer(track, 1));
    }
    else if (url.scheme() == "deck2")
    {
        TrackInfoObject* track = new TrackInfoObject(url.path());
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
