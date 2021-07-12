#pragma once

#include <QAction>
#include <QList>
#include <QModelIndex>
#include <QObject>
#include <QPoint>
#include <QPointer>
#include <QStringListModel>
#include <QUrl>
#include <QVariant>

#include "library/dao/autodjcratesdao.h"
#include "library/libraryfeature.h"
#include "library/trackset/crate/crate.h"
#include "library/treeitemmodel.h"
#include "preferences/usersettings.h"
#include "util/parented_ptr.h"

class DlgAutoDJ;
class Library;
class PlayerManagerInterface;
class TrackCollection;
class TrackCollectionManager;
class AutoDJProcessor;
class WLibrarySidebar;

class AutoDJFeature : public LibraryFeature {
    Q_OBJECT
  public:
    AutoDJFeature(Library* pLibrary,
                  UserSettingsPointer pConfig,
                  PlayerManagerInterface* pPlayerManager);
    virtual ~AutoDJFeature();

    QVariant title() override;

    bool dropAccept(const QList<QUrl>& urls, QObject* pSource) override;
    bool dragMoveAccept(const QUrl& url) override;

    void bindLibraryWidget(WLibrary* libraryWidget,
                    KeyboardEventFilter* keyboard) override;
    void bindSidebarWidget(WLibrarySidebar* pSidebarWidget) override;

    TreeItemModel* sidebarModel() const override;

    bool hasTrackTable() override {
        return true;
    }

  public slots:
    void activate() override;

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

    // Initialize the list of crates loaded into the auto-DJ queue.
    void constructCrateChildModel();

    // The "Crates" tree-item under the "Auto DJ" tree-item.
    TreeItem* m_pCratesTreeItem;

    // The crate ID and name of all loaded crates.
    // Its indices correspond one-to-one with tree-items contained by the
    // "Crates" tree-item.
    QList<Crate> m_crateList;

    // How we access the auto-DJ-crates database.
    AutoDJCratesDAO m_autoDjCratesDao;

    // A context-menu item that allows crates to be removed from the
    // auto-DJ list.
    QAction *m_pRemoveCrateFromAutoDj;

    QPointer<WLibrarySidebar> m_pSidebarWidget;

  private slots:
    // Add a crate to the auto-DJ queue.
    void slotAddCrateToAutoDj(int iCrateId);

    // Implements the context-menu item.
    void slotRemoveCrateFromAutoDj();

    void slotCrateChanged(CrateId crateId);

    // Adds a random track from all loaded crates to the auto-DJ queue.
    void slotAddRandomTrack();

    // Adds a random track from the queue upon hitting minimum number
    // of tracks in the playlist
    void slotRandomQueue(int numTracksToAdd);
};
