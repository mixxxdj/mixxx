#pragma once

#include <QList>
#include <QModelIndex>
#include <QPointer>
#include <QUrl>
#include <QVariant>

#include "library/trackset/basetracksetfeature.h"
#include "library/trackset/crate/crate.h"
#include "library/trackset/crate/cratetablemodel.h"
#include "preferences/usersettings.h"
#include "track/trackid.h"
#include "util/parented_ptr.h"

// forward declaration(s)
class Library;
class WLibrarySidebar;
class QAction;
class QPoint;
class CrateSummary;

class CrateFeature : public BaseTrackSetFeature {
    Q_OBJECT

  public:
    CrateFeature(Library* pLibrary,
            UserSettingsPointer pConfig);
    ~CrateFeature() override = default;

    QVariant title() override;

    bool dropAcceptChild(const QModelIndex& index,
            const QList<QUrl>& urls,
            QObject* pSource) override;
    bool dragMoveAcceptChild(const QModelIndex& index, const QUrl& url) override;

    void bindLibraryWidget(WLibrary* libraryWidget,
            KeyboardEventFilter* keyboard) override;
    void bindLibraryPreparationWindowWidget(
            WLibraryPreparationWindow* libraryPreparationWindowWidget,
            KeyboardEventFilter* keyboard) override;
    void bindSidebarWidget(WLibrarySidebar* pSidebarWidget) override;

    TreeItemModel* sidebarModel() const override;

  public slots:
    void activate() override;
    void activateChild(const QModelIndex& index) override;
    void onRightClick(const QPoint& globalPos) override;
    void onRightClickChild(const QPoint& globalPos, const QModelIndex& index) override;
    void slotCreateCrate();
    void deleteItem(const QModelIndex& index) override;
    void renameItem(const QModelIndex& index) override;

#ifdef __ENGINEPRIME__
  signals:
    void exportAllCrates();
    void exportCrate(CrateId crateId);
#endif

  private slots:
    void slotDeleteCrate();
    void slotRenameCrate();
    void slotDuplicateCrate();
    void slotAutoDjTrackSourceChanged();
    void slotToggleCrateLock();
    void slotImportPlaylist();
    void slotImportPlaylistFile(const QString& playlistFile, CrateId crateId);
    void slotCreateImportCrate();
    void slotExportPlaylist();
    // Copy all of the tracks in a crate to a new directory (like a thumbdrive).
    void slotExportTrackFiles();
    void slotAnalyzeCrate();
    void slotCrateTableChanged(CrateId crateId);
    void slotCrateContentChanged(CrateId crateId);
    void htmlLinkClicked(const QUrl& link);
    void slotTrackSelected(TrackId trackId);
    void slotResetSelectedTrack();
    void slotUpdateCrateLabels(const QSet<CrateId>& updatedCrateIds);
    void slotShowInPreparationWindow();

  private:
    void initActions();
    void connectLibrary(Library* pLibrary);
    void connectTrackCollection();

    bool activateCrate(CrateId crateId);

    std::unique_ptr<TreeItem> newTreeItemForCrateSummary(
            const CrateSummary& crateSummary);
    void updateTreeItemForCrateSummary(
            TreeItem* pTreeItem,
            const CrateSummary& crateSummary) const;

    QModelIndex rebuildChildModel(CrateId selectedCrateId = CrateId());
    void updateChildModel(const QSet<CrateId>& updatedCrateIds);

    CrateId crateIdFromIndex(const QModelIndex& index) const;
    QModelIndex indexFromCrateId(CrateId crateId) const;

    bool isChildIndexSelectedInSidebar(const QModelIndex& index);
    bool readLastRightClickedCrate(Crate* pCrate) const;

    QString formatRootViewHtml() const;

    const QIcon m_lockedCrateIcon;

    TrackCollection* const m_pTrackCollection;

    CrateTableModel m_crateTableModel;

    // Stores the id of a crate in the sidebar that is adjacent to the crate(crateId).
    void storePrevSiblingCrateId(CrateId crateId);
    // Can be used to restore a similar selection after the sidebar model was rebuilt.
    CrateId m_prevSiblingCrate;

    QModelIndex m_lastClickedIndex;
    QModelIndex m_lastRightClickedIndex;
    TrackId m_selectedTrackId;

    parented_ptr<QAction> m_pCreateCrateAction;
    parented_ptr<QAction> m_pDeleteCrateAction;
    parented_ptr<QAction> m_pRenameCrateAction;
    parented_ptr<QAction> m_pLockCrateAction;
    parented_ptr<QAction> m_pDuplicateCrateAction;
    parented_ptr<QAction> m_pAutoDjTrackSourceAction;
    parented_ptr<QAction> m_pImportPlaylistAction;
    parented_ptr<QAction> m_pCreateImportPlaylistAction;
    parented_ptr<QAction> m_pExportPlaylistAction;
    parented_ptr<QAction> m_pExportTrackFilesAction;
#ifdef __ENGINEPRIME__
    parented_ptr<QAction> m_pExportAllCratesAction;
    parented_ptr<QAction> m_pExportCrateAction;
#endif
    parented_ptr<QAction> m_pAnalyzeCrateAction;
    parented_ptr<QAction> m_pShowTrackModelInPreparationWindowAction;

    QPointer<WLibrarySidebar> m_pSidebarWidget;
};
