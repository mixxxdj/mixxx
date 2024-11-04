#pragma once

#include <QList>
#include <QModelIndex>
#include <QPointer>
#include <QSqlDatabase>
#include <QUrl>
#include <QVariant>

#include "library/trackset/basetracksetfeature.h"
#include "library/trackset/smarties/dlgsmartiesinfo.h"
#include "library/trackset/smarties/smarties.h"
#include "library/trackset/smarties/smartiesstorage.h"
#include "library/trackset/smarties/smartiestablemodel.h"
#include "preferences/usersettings.h"
#include "track/trackid.h"
#include "util/parented_ptr.h"

// forward declaration(s)
class Library;
class WLibrarySidebar;
class QAction;
class QPoint;
class SmartiesSummary;
class dlgSmartiesInfo;

class SmartiesFeature : public BaseTrackSetFeature {
    Q_OBJECT

  public:
    SmartiesFeature(
            Library* pLibrary,
            UserSettingsPointer pConfig);
    ~SmartiesFeature() override = default;

    QVariant title() override;

    //    bool dropAcceptChild(const QModelIndex& index,
    //            const QList<QUrl>& urls,
    //            QObject* pSource) override;
    //    bool dragMoveAcceptChild(const QModelIndex& index, const QUrl& url) override;

    void bindLibraryWidget(WLibrary* libraryWidget,
            KeyboardEventFilter* keyboard) override;
    void bindSidebarWidget(WLibrarySidebar* pSidebarWidget) override;

    TreeItemModel* sidebarModel() const override;

  signals:
    void updateSmartiesData(const QVariantList& smartiesData);

  public slots:
    void activate() override;
    void activateChild(const QModelIndex& index) override;
    void onRightClick(const QPoint& globalPos) override;
    void onRightClickChild(const QPoint& globalPos, const QModelIndex& index) override;
    void slotCreateSmarties();
    void slotCreateSmartiesFromUI();
    void slotFindPreviousSmarties();
    void slotFindNextSmarties();
    void slotCreateSmartiesFromSearch(const QString& text);
    void deleteItem(const QModelIndex& index) override;
    void renameItem(const QModelIndex& index) override;
    void SetActiveSmartiesToLastRightClicked(const QModelIndex& index) override;

    // #ifdef __ENGINEPRIME__
    //   signals:
    //     void exportAllSmarties();
    //     void exportSmarties(SmartiesId smartiesId);
    // #endif

  private slots:
    void slotDeleteSmarties();
    void slotEditSmarties();

    void slotRenameSmarties();
    void slotDuplicateSmarties();
    //    void slotAutoDjTrackSourceChanged();
    void slotToggleSmartiesLock();
    //    void slotImportPlaylist();
    //    void slotImportPlaylistFile(const QString& playlistFile, SmartiesId smartiesId);
    //    void slotCreateImportSmarties();
    //    void slotExportPlaylist();
    // Copy all of the tracks in a smarties to a new directory (like a thumbdrive).
    //    void slotExportTrackFiles();
    void slotAnalyzeSmarties();
    void slotSmartiesTableChanged(SmartiesId smartiesId);
    void slotSmartiesContentChanged(SmartiesId smartiesId);
    void htmlLinkClicked(const QUrl& link);
    void slotTrackSelected(TrackId trackId);
    void slotResetSelectedTrack();
    void slotUpdateSmartiesLabels(const QSet<SmartiesId>& updatedSmartiesIds);

  private:
    void initActions();
    void connectLibrary(Library* pLibrary);
    void connectTrackCollection();

    bool activateSmarties(SmartiesId smartiesId);

    std::unique_ptr<TreeItem> newTreeItemForSmartiesSummary(
            const SmartiesSummary& smartiesSummary);
    void updateTreeItemForSmartiesSummary(
            TreeItem* pTreeItem,
            const SmartiesSummary& smartiesSummary) const;

    void selectSmartiesForEdit(SmartiesId selectedSmartiesId = SmartiesId());

    QModelIndex rebuildChildModel(SmartiesId selectedSmartiesId = SmartiesId());
    void updateChildModel(const QSet<SmartiesId>& updatedSmartiesIds);

    SmartiesId smartiesIdFromIndex(const QModelIndex& index) const;
    QModelIndex indexFromSmartiesId(SmartiesId smartiesId) const;

    bool isChildIndexSelectedInSidebar(const QModelIndex& index);
    bool readLastRightClickedSmarties(Smarties* pSmarties) const;

    QString formatRootViewHtml() const;

    const QIcon m_lockedSmartiesIcon;

    TrackCollection* const m_pTrackCollection;

    //    dlgSmartiesInfoHelper m_dlgSmartiesInfoHelper;
    SmartiesTableModel m_smartiesTableModel;
    //    dlgSmartiesInfoHelper m_dlgSmartiesInfoHelper;
    //    dlgSmartiesInfo m_dlgSmartiesInfo;

    // Stores the id of a smarties in the sidebar that is adjacent to the smarties(smartiesId).
    void storePrevSiblingSmartiesId(SmartiesId smartiesId);
    void storeNextSiblingSmartiesId(SmartiesId smartiesId);
    // Can be used to restore a similar selection after the sidebar model was rebuilt.
    SmartiesId m_prevSiblingSmarties;
    SmartiesId m_nextSiblingSmarties;

    QModelIndex m_lastClickedIndex;
    QModelIndex m_lastRightClickedIndex;
    TrackId m_selectedTrackId;

    parented_ptr<QAction> m_pCreateSmartiesAction;
    parented_ptr<QAction> m_pEditSmartiesAction;
    parented_ptr<QAction> m_pDeleteSmartiesAction;
    parented_ptr<QAction> m_pRenameSmartiesAction;
    parented_ptr<QAction> m_pLockSmartiesAction;
    parented_ptr<QAction> m_pDuplicateSmartiesAction;
    //    parented_ptr<QAction> m_pAutoDjTrackSourceAction;
    //    parented_ptr<QAction> m_pImportPlaylistAction;
    //    parented_ptr<QAction> m_pCreateImportPlaylistAction;
    //    parented_ptr<QAction> m_pExportPlaylistAction;
    //    parented_ptr<QAction> m_pExportTrackFilesAction;
    // #ifdef __ENGINEPRIME__
    //    parented_ptr<QAction> m_pExportAllSmartiesAction;
    //    parented_ptr<QAction> m_pExportSmartiesAction;
    // #endif
    parented_ptr<QAction> m_pAnalyzeSmartiesAction;

    QPointer<WLibrarySidebar> m_pSidebarWidget;

    //    QList<QVariantList> m_smartiesList;
    QVariantList smartiesData;
};
