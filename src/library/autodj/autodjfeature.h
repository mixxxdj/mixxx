#pragma once

#include <QJsonArray>
#include <QList>
#include <QObject>
#include <QPointer>
#include <QUrl>
#include <QVariant>

#include "library/autodj/harmonicmixing.h"
#include "library/dao/autodjcratesdao.h"
#include "library/libraryfeature.h"
#include "library/trackset/crate/crate.h"
#include "preferences/usersettings.h"
#include "util/parented_ptr.h"

class DlgAutoDJ;
class Library;
class PlayerManagerInterface;
class TrackCollection;
class AutoDJProcessor;
class WLibrarySidebar;
class QAction;
class QModelIndex;
class QPoint;

namespace mixxx {
namespace ai {
class AiMixPlanner;
} // namespace ai
} // namespace mixxx

class AutoDJFeature : public LibraryFeature {
    Q_OBJECT
  public:
    AutoDJFeature(Library* pLibrary,
                  UserSettingsPointer pConfig,
                  PlayerManagerInterface* pPlayerManager);
    virtual ~AutoDJFeature();

    QVariant title() override;

    void clear() override;
    void paste() override;
    void deleteItem(const QModelIndex& index) override;

    bool dropAccept(const QList<QUrl>& urls, QObject* pSource) override;
    bool dragMoveAccept(const QList<QUrl>& urls) override;

    void bindLibraryWidget(WLibrary* libraryWidget,
                    KeyboardEventFilter* keyboard) override;
    void bindSidebarWidget(WLibrarySidebar* pSidebarWidget) override;

    TreeItemModel* sidebarModel() const override;

    bool hasTrackTable() override {
        return true;
    }

  public slots:
    void activate() override;

    void onRightClick(const QPoint& globalPos) override;
    // Temporary, until WCrateTableView can be written.
    void onRightClickChild(const QPoint& globalPos, const QModelIndex& index) override;

  private:
    TrackCollection* const m_pTrackCollection;

    PlaylistDAO& m_playlistDao;
    // The id of the AutoDJ playlist.
    int m_iAutoDJPlaylistId;
    AutoDJProcessor* m_pAutoDJProcessor;
    parented_ptr<TreeItemModel> m_pSidebarModel;
    DlgAutoDJ* m_pAutoDJView;
    const QString m_viewName;

    // Initialize the list of crates loaded into the auto-DJ queue.
    void constructCrateChildModel();
    void removeCrateFromAutoDj(CrateId crateId = CrateId());

    // Determine the keys and the tempo windows that a newly added track has to
    // match in order to mix harmonically with the track at the end of the Auto
    // DJ queue. The windows are ordered from the configured tolerance to
    // progressively wider ones, so the caller can fall back to mixing by key
    // only instead of giving up on harmonic mixing. Returns false if harmonic
    // mixing is disabled or if the reference track carries no usable tempo or
    // key information.
    bool getHarmonicSelectionConstraints(
            QList<int>* pCompatibleKeyIds,
            QList<mixxx::HarmonicMixing::BpmRange>* pBpmRanges) const;

    // Collects up to AiMixSettings::maxCandidateTracks tracks from the library
    // and describes them for the AI service. The matching track ids are kept in
    // m_aiCandidateTrackIds, indexed exactly like the candidates sent out.
    QJsonArray buildAiCandidateTracks();

    // The "Crates" tree-item under the "Auto DJ" tree-item.
    TreeItem* m_pCratesTreeItem;

    // The crate ID and name of all loaded crates.
    // Its indices correspond one-to-one with tree-items contained by the
    // "Crates" tree-item.
    QList<Crate> m_crateList;

    // How we access the auto-DJ-crates database.
    AutoDJCratesDAO m_autoDjCratesDao;

    // Optional AI set planner. Only used when it is enabled in the preferences
    // and when the build provides secure API key storage.
    mixxx::ai::AiMixPlanner* m_pAiMixPlanner = nullptr;
    // Candidate tracks that were sent to the AI service, indexed like the
    // candidates themselves so that the returned indices can be resolved.
    QList<TrackId> m_aiCandidateTrackIds;

    parented_ptr<QAction> m_pEnableAutoDJAction;
    parented_ptr<QAction> m_pDisableAutoDJAction;
    parented_ptr<QAction> m_pClearQueueAction;

    // A context-menu item that allows crates to be removed from the
    // auto-DJ list.
    parented_ptr<QAction> m_pRemoveCrateFromAutoDjAction;

    QPointer<WLibrarySidebar> m_pSidebarWidget;

  private slots:
    void slotEnableAutoDJ();
    void slotDisableAutoDJ();
    void slotClearQueue();

    // Add a crate to the auto-DJ queue.
    void slotAddCrateToAutoDj(CrateId crateId);
    // Implements the context-menu item.
    void slotRemoveCrateFromAutoDj();
    void slotCrateChanged(CrateId crateId);
    // Adds a random track from all loaded crates to the auto-DJ queue.
    void slotAddRandomTrack();
    // Adds a random track from the queue upon hitting minimum number
    // of tracks in the playlist
    void slotRandomQueue(int numTracksToAdd);

    // Asks the AI assistant for a set list and loads it into the queue.
    void slotAiMixRequested(const QString& instruction);
    void slotAiMixSucceeded(const QList<int>& trackOrder);
    void slotAiMixFailed(const QString& errorMessage);
};
