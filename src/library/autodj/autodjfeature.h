// AutoDJfeature.h
// FORK FORK FORK on 11/1/2009 by Albert Santoni (alberts@mixxx.org)
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)

#ifndef AUTODJFEATURE_H
#define AUTODJFEATURE_H

#include <QObject>
#include <QStringListModel>
#include <QVariant>
#include <QIcon>
#include <QUrl>
#include <QList>
#include <QModelIndex>
#include <QPoint>
#include <QAction>
#include <QPointer>

#include "library/libraryfeature.h"
#include "preferences/usersettings.h"
#include "library/treeitemmodel.h"
#include "library/crate/crate.h"
#include "library/dao/autodjcratesdao.h"

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
    QIcon getIcon() override;

    bool dropAccept(const QList<QUrl>& urls, QObject* pSource) override;
    bool dragMoveAccept(const QUrl& url) override;

    void bindLibraryWidget(WLibrary* libraryWidget,
                    KeyboardEventFilter* keyboard) override;
    void bindSidebarWidget(WLibrarySidebar* pSidebarWidget) override;

    TreeItemModel* getChildModel() override;

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
    TreeItemModel m_childModel;
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

    QIcon m_icon;
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


#endif /* AUTODJFEATURE_H */
