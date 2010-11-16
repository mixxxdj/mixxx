/***************************************************************************
                          promotrackswebview.h
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

#ifndef __FEATUREDARTISTSWEBVIEW_H_
#define __FEATUREDARTISTSWEBVIEW_H_

#include <QWebView>
#include <QUrl>
#include "trackinfoobject.h"
#include "library/libraryview.h"

class SongDownloader;

class FeaturedArtistsWebView : public QWebView, public LibraryView
{
    Q_OBJECT
    public:
        FeaturedArtistsWebView(QWidget* parent, QString libraryPath,
                               QString remoteURL, SongDownloader* downloader);
        ~FeaturedArtistsWebView();
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
        void loadTrackToPlayer(TrackInfoObject* pTrack, QString group);
    protected:
        //virtual QString userAgentForUrl (const QUrl & url) const;

    private:
        QString m_sLibraryPath; /** Top-level path to directory that contains the promo directory. */
        QString m_sRemoteURL; /** URL to remotely hosted (promo.mixxx.org) copy of promo tracks web page.*/
        QString m_sLocalErrorURL; /** URL for local error message / offline mode page */
        bool m_bOfflineMode; /** Load error page locally if we're offline */
        SongDownloader* m_pSongDownloader;
};


#endif //__FEATUREDARTISTSWEBVIEW_H_
