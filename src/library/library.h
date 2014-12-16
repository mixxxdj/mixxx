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
#include "analysisfeature.h"
#include "library/coverartcache.h"

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
class PlayerManagerInterface;

class Library : public QObject {
    Q_OBJECT
public:
    Library(QObject* parent,
            ConfigObject<ConfigValue>* pConfig,
            PlayerManagerInterface* pPlayerManager,
            RecordingManager* pRecordingManager);
    virtual ~Library();

    void bindWidget(WLibrary* libraryWidget,
                    MixxxKeyboard* pKeyboard);
    void bindSidebarWidget(WLibrarySidebar* sidebarWidget);

    void addFeature(LibraryFeature* feature);
    QStringList getDirs();

    // TODO(rryan) Transitionary only -- the only reason this is here is so the
    // waveform widgets can signal to a player to load a track. This can be
    // fixed by moving the waveform renderers inside player and connecting the
    // signals directly.
    TrackCollection* getTrackCollection() {
        return m_pTrackCollection;
    }

    //static Library* buildDefaultLibrary();

    enum RemovalType {
        LeaveTracksUnchanged = 0,
        HideTracks,
        PurgeTracks
    };

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
    void slotRequestAddDir(QString directory);
    void slotRequestRemoveDir(QString directory, Library::RemovalType removalType);
    void slotRequestRelocateDir(QString previousDirectory, QString newDirectory);
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
    // emit this signal to enable/disable the cover art widget
    void enableCoverArtDisplay(bool);
    void trackSelected(TrackPointer pTrack);

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
    AnalysisFeature* m_pAnalysisFeature;
    LibraryControl* m_pLibraryControl;
    RecordingManager* m_pRecordingManager;
};

#endif /* LIBRARY_H */
