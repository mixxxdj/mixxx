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
#include <QSignalMapper>

#include "library/libraryfeature.h"
#include "preferences/usersettings.h"
#include "library/treeitemmodel.h"
#include "library/crate/crate.h"
#include "library/dao/autodjcratesdao.h"

class DlgAutoDJ;
class Library;
class PlayerManagerInterface;
class TrackCollection;
class AutoDJProcessor;

class AutoDJFeature : public LibraryFeature {
    Q_OBJECT
  public:
    AutoDJFeature(Library* pLibrary,
                  UserSettingsPointer pConfig,
                  PlayerManagerInterface* pPlayerManager,
                  TrackCollection* pTrackCollection);
    virtual ~AutoDJFeature();

    QVariant title();
    QIcon getIcon();

    bool dropAccept(QList<QUrl> urls, QObject* pSource);
    bool dragMoveAccept(QUrl url);

    void bindWidget(WLibrary* libraryWidget,
                    KeyboardEventFilter* keyboard);

    TreeItemModel* getChildModel();

  public slots:
    void activate();

    // Temporary, until WCrateTableView can be written.
    void onRightClickChild(const QPoint& globalPos, QModelIndex index);

  private:
    UserSettingsPointer m_pConfig;
    Library* m_pLibrary;
    TrackCollection* m_pTrackCollection;
    PlaylistDAO& m_playlistDao;
    // The id of the AutoDJ playlist.
    int m_iAutoDJPlaylistId;
    AutoDJProcessor* m_pAutoDJProcessor;
    const static QString m_sAutoDJViewName;
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

    // Used to map menu-item signals.
    QSignalMapper m_crateMapper;

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
