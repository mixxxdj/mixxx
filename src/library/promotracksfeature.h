/***************************************************************************
                          promotracksfeature.cpp
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

#ifndef PROMOTRACKSFEATURE_H
#define PROMOTRACKSFEATURE_H

#include <QStringListModel>

#include "treeitemmodel.h"
#include "trackinfoobject.h"
#include "library/libraryfeature.h"
#include "library/dao/playlistdao.h"
#include "library/cratetablemodel.h"
#include "configobject.h"


class PlaylistTableModel;
class ProxyTrackModel;
class TrackCollection;
class TrackInfoObject;
class BundledSongsWebView;
class FeaturedArtistsWebView;
class SongDownloader;

class PromoTracksFeature : public LibraryFeature {
    Q_OBJECT
  public:
    PromoTracksFeature(QObject* parent,
                  ConfigObject<ConfigValue>* pConfig,
                  TrackCollection* pTrackCollection,
                  bool firstRun);
    virtual ~PromoTracksFeature();
    static bool isSupported(ConfigObject<ConfigValue>* config);
    QList<TrackPointer> getTracksToAutoLoad();

    QVariant title();
    QIcon getIcon();

    void bindWidget(WLibrary* libraryWidget,
                    MixxxKeyboard* keyboard);

    TreeItemModel* getChildModel();

public slots:
    void activate();
    void activateChild(const QModelIndex& index);

private:
    ConfigObject<ConfigValue>* m_pConfig;
    TrackCollection* m_pTrackCollection;
    FeaturedArtistsWebView* m_pFeaturedArtistsView;
    BundledSongsWebView* m_pBundledSongsView;
    static QString m_sPromoLocalHTMLLocation;
    static QString m_sPromoRemoteHTMLLocation;
    QString m_sPromoAutoloadLocation;
    const static QString m_sFeaturedArtistsViewName;
    const static QString m_sBundledSongsViewName;
    const static QString m_sMyDownloadsViewName;
    TreeItemModel m_childModel;
    CrateTableModel m_downloadsTableModel;
    SongDownloader* m_pSongDownloader;
    bool m_bFirstRun;
    QList<TrackPointer> m_tracksToAutoLoad;
};


#endif /* PROMOTRACKSFEATURE_H */
