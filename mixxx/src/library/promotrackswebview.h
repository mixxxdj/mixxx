#ifndef __PROMOTRACKSWEBVIEW_H_
#define __PROMOTRACKSWEBVIEW_H_

#include <QWebView>
#include <QUrl>
#include "trackinfoobject.h"
#include "library/libraryview.h"

#define MIXXX_PROMO_HTML_LOCATION "promo/promotracks.html"

class PromoTracksWebView : public QWebView, public LibraryView
{
    Q_OBJECT 
    public:
        PromoTracksWebView(QWidget* parent);
        ~PromoTracksWebView();
        virtual void setup(QDomNode node);
        virtual void onSearchStarting() {};
        virtual void onSearchCleared()  {};
        virtual void onSearch(const QString&) {};
        virtual void onShow() {};
        virtual QWidget* getWidgetForMIDIControl() { return this; };
        virtual void keyPressEvent(QKeyEvent* event);

    public slots: 
        void handleClickedLink(const QUrl& url);

    signals:
        void loadTrack(TrackInfoObject* pTrack);
        void loadTrackToPlayer(TrackInfoObject* pTrack, int player);
};


#endif //__PROMOTRACKSWEBVIEW_H_
