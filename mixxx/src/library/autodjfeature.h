// AutoDJfeature.h
// FORK FORK FORK on 11/1/2009 by Albert Santoni (alberts@mixxx.org)
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)

#ifndef AUTODJFEATURE_H
#define AUTODJFEATURE_H

#include <QStringListModel>

#include "library/libraryfeature.h"
#include "configobject.h"
#include "treeitemmodel.h"
#include "dlgautodj.h"

#ifdef __AUTODJCRATES__
#include "library/dao/autodjcratesdao.h"
#endif // __AUTODJCRATES__

class PlaylistTableModel;
class TrackCollection;

class AutoDJFeature : public LibraryFeature {
    Q_OBJECT
  public:
    AutoDJFeature(QObject* parent,
                  ConfigObject<ConfigValue>* pConfig,
                  TrackCollection* pTrackCollection);
    virtual ~AutoDJFeature();

    QVariant title();
    QIcon getIcon();

    bool dropAccept(QList<QUrl> urls,QWidget *pSource);
    bool dragMoveAccept(QUrl url);

    void bindWidget(WLibrary* libraryWidget,
                    MixxxKeyboard* keyboard);

    TreeItemModel* getChildModel();

  public slots:
    void activate();

    // Temporary, until WCrateTableView can be written.
    #ifdef __AUTODJCRATES__
    void onRightClickChild(const QPoint& globalPos, QModelIndex index);
    #endif // __AUTODJCRATES__

  private:
    ConfigObject<ConfigValue>* m_pConfig;
    TrackCollection* m_pTrackCollection;
    CrateDAO& m_crateDao;
    PlaylistDAO& m_playlistDao;
    const static QString m_sAutoDJViewName;
    TreeItemModel m_childModel;
    DlgAutoDJ* m_pAutoDJView;

#ifdef __AUTODJCRATES__

    TreeItem* m_pCratesTreeItem;
        // The "Crates" tree-item under the "Auto DJ" tree-item.
    QList<QPair<int, QString> > m_crateList;
        // The crate ID and name of all loaded crates.
        // Its indices correspond one-to-one with tree-items contained by the
        // "Crates" tree-item.
    AutoDJCratesDAO m_autoDjCratesDao;
        // How we access the auto-DJ-crates database.
    QModelIndex m_lastRightClickedIndex;
        // The model-index of the last tree-item that was right-clicked on.
        // Only stored for tree-items contained by the "Crates" tree-item.
    QAction *m_pRemoveCrateFromAutoDj;
        // A context-menu item that allows crates to be removed from the
        // auto-DJ list.

    void constructCrateChildModel();
        // Initialize the list of crates loaded into the auto-DJ queue.

#endif // __AUTODJCRATES__

  private slots:
    void slotRemoveCrateFromAutoDj();
        // Implements the context-menu item.
    void slotCrateAdded (int a_iCrateId);
        // Signaled by the crate DAO when a crate is added.
    void slotCrateRenamed (int a_iCrateId, QString a_strName);
        // Signaled by the crate DAO when a crate is renamed.
    void slotCrateDeleted (int a_iCrateId);
        // Signaled by the crate DAO when a crate is deleted.
    void slotCrateAutoDjChanged (int a_iCrateId, bool a_bIn);
        // Signaled by the crate DAO when a crate's auto-DJ status changes.
    void slotAddRandomTrack(bool);
        // Adds a random track from all loaded crates to the auto-DJ queue.

  signals:
    void enableAddRandom (bool a_bEnabled);
};


#endif /* AUTODJFEATURE_H */
