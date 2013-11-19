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
#include "trackinfoobject.h"
#include "recording/recordingmanager.h"
#include "preparefeature.h"

class TrackModel;
class TrackCollection;
class SidebarModel;
class LibraryFeature;
class LibraryTableModel;
class WLibrarySidebar;
class WLibrary;
class WSearchLineEdit;
class MixxxLibraryFeature;
class PlaylistFeature;
class CrateFeature;
class LibraryControl;
class MixxxKeyboard;

class Library : public QObject {
    Q_OBJECT
public:
    Library(QObject* parent,
            ConfigObject<ConfigValue>* pConfig,
            bool firstRun, RecordingManager* pRecordingManager);
    virtual ~Library();

    void bindWidget(WLibrary* libraryWidget,
                    MixxxKeyboard* pKeyboard);
    void bindSidebarWidget(WLibrarySidebar* sidebarWidget);

    void addFeature(LibraryFeature* feature);
    QList<TrackPointer> getTracksToAutoLoad();

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
    void slotLoadTrack(TrackPointer pTrack);
    void slotLoadTrackToPlayer(TrackPointer pTrack, QString group, bool play);
    void slotLoadLocationToPlayer(QString location, QString group);
    void slotRestoreSearch(const QString& text);
    void slotRefreshLibraryModels();
    void slotCreatePlaylist();
    void slotCreateCrate();
    void onSkinLoadFinished();

  signals:
    void showTrackModel(QAbstractItemModel* model);
    void switchToView(const QString& view);
    void loadTrack(TrackPointer pTrack);
    void loadTrackToPlayer(TrackPointer pTrack, QString group, bool play = false);
    void restoreSearch(const QString&);
    void search(const QString& text);
    void searchCleared();
    void searchStarting();

  private:
    ConfigObject<ConfigValue>* m_pConfig;
    SidebarModel* m_pSidebarModel;
    TrackCollection* m_pTrackCollection;
    QList<LibraryFeature*> m_features;
    const static QString m_sTrackViewName;
    const static QString m_sAutoDJViewName;
    MixxxLibraryFeature* m_pMixxxLibraryFeature;
    PlaylistFeature* m_pPlaylistFeature;
    CrateFeature* m_pCrateFeature;
#ifdef __PROMO__
    class PromoTracksFeature;
    PromoTracksFeature* m_pPromoTracksFeature;
#endif
    PrepareFeature* m_pPrepareFeature;
    LibraryControl* m_pLibraryControl;
    RecordingManager* m_pRecordingManager;
};

#endif /* LIBRARY_H */
