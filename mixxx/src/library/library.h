// library.h
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)

// A Library class is a container for all the model-side aspects of the library.
// A library widget can be attached to the Library object by calling bindWidget.

#ifndef LIBRARY_H
#define LIBRARY_H

#include <QList>
#include <QObject>
#include <QAbstractItemModel>

#include "configobject.h"

class TrackModel;
class TrackCollection;
class TrackInfoObject;
class SidebarModel;
class LibraryFeature;
class LibraryTableModel;
class WLibrarySidebar;
class WLibrary;
class WSearchLineEdit;

class Library : public QObject {
    Q_OBJECT
public:
    Library(QObject* parent,
            ConfigObject<ConfigValue>* pConfig);
    virtual ~Library();
    
    void bindWidget(WLibrarySidebar* sidebarWidget, 
                    WLibrary* libraryWidget);
    void addFeature(LibraryFeature* feature);
    
    // TODO(rryan) Transitionary only -- the only reason this is here is so the
    // waveform widgets can signal to a player to load a track. This can be
    // fixed by moving the waveform renderers inside player and connecting the
    // signals directly.
    TrackCollection* getTrackCollection() {
        return m_pTrackCollection;
    }

    //static Library* buildDefaultLibrary();

public slots:
    void slotShowTrackModel(QAbstractItemModel* model);
    void slotSearch(const QString&);
    void slotSearchCleared();
    void slotSearchStarting();
    void slotLoadTrack(TrackInfoObject* pTrack);
    void slotLoadTrackToPlayer(TrackInfoObject* pTrack, int player);
signals:
    void showTrackModel(QAbstractItemModel* model);
    void search(const QString&);
    void searchCleared();
    void searchStarting();
    void loadTrack(TrackInfoObject* tio);
    void loadTrackToPlayer(TrackInfoObject* tio, int n);
    
private:
    ConfigObject<ConfigValue>* m_pConfig;
    SidebarModel* m_pSidebarModel;
    TrackCollection* m_pTrackCollection;
    QList<LibraryFeature*> m_features;
};

#endif /* LIBRARY_H */
