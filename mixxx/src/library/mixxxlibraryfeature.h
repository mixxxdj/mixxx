// mixxxlibraryfeature.h
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)

#ifndef MIXXXLIBRARYFEATURE_H
#define MIXXXLIBRARYFEATURE_H

#include <QStringListModel>

#include "library/libraryfeature.h"

class LibraryTableModel;
class ProxyTrackModel;
class TrackCollection;

class MixxxLibraryFeature : public LibraryFeature {
    Q_OBJECT
    public:
    MixxxLibraryFeature(QObject* parent,
                        TrackCollection* pTrackCollection);
    virtual ~MixxxLibraryFeature();

    QVariant title();
    QIcon getIcon();

    bool dropAccept(QUrl url);
    bool dropAcceptChild(const QModelIndex& index, QUrl url);
    bool dragMoveAccept(QUrl url);
    bool dragMoveAcceptChild(const QModelIndex& index, QUrl url);

    QAbstractItemModel* getChildModel();

public slots:
    void activate();
    void activateChild(const QModelIndex& index);
    void onRightClick(const QPoint& globalPos);
    void onRightClickChild(const QPoint& globalPos, QModelIndex index);

private:
    LibraryTableModel* m_pLibraryTableModel;
    ProxyTrackModel* m_pLibraryTableModelProxy;
    QStringListModel m_childModel;
};


#endif /* MIXXXLIBRARYFEATURE_H */
