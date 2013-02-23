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

#ifndef __BUNDLEDSONGSWEBVIEW_H_
#define __BUNDLEDSONGSWEBVIEW_H_

#include <QWebView>
#include <QUrl>
#include "configobject.h"
#include "trackinfoobject.h"
#include "library/libraryview.h"

class TrackCollection;

#define MIXXX_PROMO_HTML_LOCATION "promo/promotracks.html"

class BundledSongsWebView : public QWebView, public LibraryView
{
  Q_OBJECT
  Q_PROPERTY(bool m_statTracking READ statTracking WRITE setStatTracking)
  Q_PROPERTY(bool m_bFirstRun READ firstRun WRITE setFirstRun)
  public:
    BundledSongsWebView(QWidget* parent, TrackCollection* trackCollection,
                        QString promoBundlePath,
                        QString localURL, bool firstRun,
                        ConfigObject<ConfigValue>* config);
    virtual ~BundledSongsWebView();

    void onShow();
    void keyPressEvent(QKeyEvent* event);
    void loadSelectedTrack();
    void moveSelection(int delta);
    bool firstRun() const;

  public slots:
    void handleClickedLink(const QUrl& url);
    void attachObjects();
    void setStatTracking(bool statTracking);
    bool statTracking() const; //has to be a slot to get it into javascript land
    void setFirstRun(bool firstRun);
    void loadFinished(bool ok);

  signals:
    void loadTrack(TrackPointer pTrack);
    void loadTrackToPlayer(TrackPointer pTrack, QString group, bool play = false);

  protected:
    //virtual QString userAgentForUrl (const QUrl & url) const;

  private:
    QString m_sPromoBundlePath; /** Directory that contains the promo bundle, which contains
                                    the local HTML page and music/ folder. */
    QString m_sLocalURL; /** URL to local copy of the promo tracks web page. */
    bool m_statTracking;
    bool m_bFirstRun;
    ConfigObject<ConfigValue>* m_pConfig;
    TrackCollection* m_pTrackCollection;
};


#endif //__BUNDLEDSONGSWEBVIEW_H_
