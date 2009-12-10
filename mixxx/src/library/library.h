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
class MixxxLibraryFeature;
class LibraryMIDIControl;

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
    void slotSwitchToView(const QString& view);
    void slotLoadTrack(TrackInfoObject* pTrack);
    void slotLoadTrackToPlayer(TrackInfoObject* pTrack, int player);
    void slotRestoreSearch(const QString& text);
    void slotRefreshLibraryModels();
signals:
    void showTrackModel(QAbstractItemModel* model);
    void switchToView(const QString& view);
    void loadTrack(TrackInfoObject* tio);
    void loadTrackToPlayer(TrackInfoObject* tio, int n);
    void restoreSearch(const QString&);

private:
    ConfigObject<ConfigValue>* m_pConfig;
    SidebarModel* m_pSidebarModel;
    TrackCollection* m_pTrackCollection;
    QList<LibraryFeature*> m_features;
    const static QString m_sTrackViewName;
    const static QString m_sPrepareViewName;
    const static QString m_sAutoDJViewName;
    MixxxLibraryFeature* m_pMixxxLibraryFeature;
    LibraryMIDIControl* m_pLibraryMIDIControl;
};

#endif /* LIBRARY_H */
