#pragma once

#include <QList>
#include <QObject>
#include <QPointer>
#include <QUrl>
#include <QVariant>

#include "library/dao/autodjcratesdao.h"
#include "library/libraryfeature.h"
#include "library/trackset/crate/crate.h"
#include "preferences/usersettings.h"
#include "util/parented_ptr.h"

class DlgAutoDJ;
class DlgAutoDJPreparationWindow;
class Library;
class PlayerManagerInterface;
class TrackCollection;
class AutoDJProcessor;
class WLibrarySidebar;
class QAction;
class QModelIndex;
class QPoint;

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
    bool dragMoveAccept(const QUrl& url) override;

    void bindLibraryWidget(WLibrary* libraryWidget,
            KeyboardEventFilter* keyboard) override;
    void bindLibraryPreparationWindowWidget(
            WLibraryPreparationWindow* libraryPreparationWindowWidget,
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
    DlgAutoDJPreparationWindow* m_pAutoDJPreparationWindowView;

    // Initialize the list of crates loaded into the auto-DJ queue.
    void constructCrateChildModel();
    void removeCrateFromAutoDj(CrateId crateId = CrateId());

    // The "Crates" tree-item under the "Auto DJ" tree-item.
    TreeItem* m_pCratesTreeItem;

    // The crate ID and name of all loaded crates.
    // Its indices correspond one-to-one with tree-items contained by the
    // "Crates" tree-item.
    QList<Crate> m_crateList;

    // How we access the auto-DJ-crates database.
    AutoDJCratesDAO m_autoDjCratesDao;

    parented_ptr<QAction> m_pEnableAutoDJAction;
    parented_ptr<QAction> m_pDisableAutoDJAction;
    parented_ptr<QAction> m_pClearQueueAction;
    parented_ptr<QAction> m_pShowTrackModelInPreparationWindowAction;

    // A context-menu item that allows crates to be removed from the
    // auto-DJ list.
    parented_ptr<QAction> m_pRemoveCrateFromAutoDjAction;

    QPointer<WLibrarySidebar> m_pSidebarWidget;

  private slots:
    void slotEnableAutoDJ();
    void slotDisableAutoDJ();
    void slotClearQueue();

    void slotShowInPreparationWindow();

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
};
