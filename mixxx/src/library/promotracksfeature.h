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

#include "library/libraryfeature.h"
#include "library/dao/playlistdao.h"
#include "configobject.h"

class PlaylistTableModel;
class ProxyTrackModel;
class TrackCollection;
class TrackInfoObject;
class PromoTracksWebView;

class PromoTracksFeature : public LibraryFeature {
    Q_OBJECT
    public:
    PromoTracksFeature(QObject* parent,
                  ConfigObject<ConfigValue>* pConfig,
                  TrackCollection* pTrackCollection);
    virtual ~PromoTracksFeature();
    static bool isSupported(ConfigObject<ConfigValue>* config);
    QList<TrackInfoObject*> getTracksToAutoLoad();

    QVariant title();
    QIcon getIcon();

    bool dropAccept(QUrl url);
    bool dropAcceptChild(const QModelIndex& index, QUrl url);
    bool dragMoveAccept(QUrl url);
    bool dragMoveAcceptChild(const QModelIndex& index, QUrl url);

    void bindWidget(WLibrarySidebar* sidebarWidget,
                    WLibrary* libraryWidget);

    QAbstractItemModel* getChildModel();

public slots:
    void activate();
    void activateChild(const QModelIndex& index);
    void onRightClick(const QPoint& globalPos);
    void onRightClickChild(const QPoint& globalPos, QModelIndex index);

private:
    ConfigObject<ConfigValue>* m_pConfig;
    TrackCollection* m_pTrackCollection;
    PromoTracksWebView* m_pPromoTracksView;
    const static QString m_sPromoTracksViewName;
    static QString m_sPromoLocalHTMLLocation;
    static QString m_sPromoRemoteHTMLLocation;
    QString m_sPromoAutoloadLocation;
    QStringListModel m_childModel;
    QList<TrackInfoObject*> m_tracksToAutoLoad;
};


#endif /* PROMOTRACKSFEATURE_H */
