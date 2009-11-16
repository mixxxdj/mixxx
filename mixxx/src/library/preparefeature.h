// preparefeature.h
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)
// Forked 11/11/2009 by Albert Santoni (alberts@mixxx.org) 

#ifndef TRIAGEFEATURE_H 
#define TRIAGEFEATURE_H 

#include <QStringListModel>
#include "library/libraryfeature.h"

class LibraryTableModel;
class ProxyTrackModel;
class TrackCollection;

class PrepareFeature : public LibraryFeature {
    Q_OBJECT
    public:
    PrepareFeature(QObject* parent,
                        TrackCollection* pTrackCollection);
    virtual ~PrepareFeature();

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
    QStringListModel m_childModel;
//    LibraryTableModel* m_pLibraryTableModel;
//    ProxyTrackModel* m_pLibraryTableModelProxy;
};


#endif /* PREPAREFEATURE_H */
