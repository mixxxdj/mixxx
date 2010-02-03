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
        PromoTracksWebView(QWidget* parent, QString mixxxPath, 
                           QString localURL, QString remoteURL);
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
        void handleLoadFinished(bool ok);
        void checkWebpageLoadingProgress();

    signals:
        void loadTrack(TrackInfoObject* pTrack);
        void loadTrackToPlayer(TrackInfoObject* pTrack, int player);

    private:
        QString m_sMixxxPath; /** Top-level path to directory that contains the promo directory. */
        QString m_sLocalURL; /** URL to local copy of the promo tracks web page. */
        QString m_sRemoteURL; /** URL to remotely hosted (promo.mixxx.org) copy of promo tracks web page.*/
        bool m_bOfflineMode; /** Load promo tracks page locally if we're offline */
};


#endif //__PROMOTRACKSWEBVIEW_H_
