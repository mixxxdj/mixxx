// mixxxlibraryfeature.h
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)

#ifndef MIXXXLIBRARYFEATURE_H
#define MIXXXLIBRARYFEATURE_H

#include "library/libraryfeature.h"

class LibraryTableModel;
class TrackCollection;

class MixxxLibraryFeature : public LibraryFeature {
    Q_OBJECT
    public:
    MixxxLibraryFeature(QObject* parent,
                        TrackCollection* pTrackCollection);
    virtual ~MixxxLibraryFeature();

    QVariant title();
    QIcon getIcon();
    int numChildren();
    QVariant child(int n);
    bool dropAccept(const QModelIndex& index, QUrl url);    
    bool dragMoveAccept(const QModelIndex& index, QUrl url);
public slots:
    void activate();
    void activateChild(int n);
    void onRightClick(const QPoint& globalPos, QModelIndex index);
    void onClick(QModelIndex index);

private:
    LibraryTableModel* m_pLibraryTableModel;
};


#endif /* MIXXXLIBRARYFEATURE_H */
