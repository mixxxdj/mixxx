// mixxxlibraryfeature.h
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)

#ifndef MIXXXLIBRARYFEATURE_H
#define MIXXXLIBRARYFEATURE_H

#include <QStringListModel>

#include "library/libraryfeature.h"

class LibraryTableModel;
class MissingTableModel;
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
    void setLibraryPrefix(QString sPrefix);

    QAbstractItemModel* getChildModel();

public slots:
    void activate();
    void activateChild(const QModelIndex& index);
    void onRightClick(const QPoint& globalPos);
    void onRightClickChild(const QPoint& globalPos, QModelIndex index);
    void refreshLibraryModels();
private:
    LibraryTableModel* m_pLibraryTableModel;
    MissingTableModel* m_pMissingTableModel;
    QStringListModel m_childModel;
};


#endif /* MIXXXLIBRARYFEATURE_H */
