#pragma once

#include <QList>
#include <QModelIndex>
#include <QPointer>
#include <QSqlDatabase>
#include <QUrl>
#include <QVariant>

#include "library/trackset/basetracksetfeature.h"
#include "library/trackset/searchcrate/dlgsearchcrateinfo.h"
#include "library/trackset/searchcrate/searchcrate.h"
#include "library/trackset/searchcrate/searchcratestorage.h"
#include "library/trackset/searchcrate/searchcratetablemodel.h"
#include "preferences/usersettings.h"
#include "track/trackid.h"
#include "util/parented_ptr.h"

// forward declaration(s)
class Library;
class WLibrarySidebar;
class QAction;
class QPoint;
class SearchCrateSummary;
class dlgSearchCrateInfo;

class SearchCrateFeature : public BaseTrackSetFeature {
    Q_OBJECT

  public:
    SearchCrateFeature(
            Library* pLibrary,
            UserSettingsPointer pConfig);
    ~SearchCrateFeature() override = default;

    QVariant title() override;

    void bindLibraryWidget(WLibrary* libraryWidget,
            KeyboardEventFilter* keyboard) override;
    void bindSidebarWidget(WLibrarySidebar* pSidebarWidget) override;

    TreeItemModel* sidebarModel() const override;

  signals:
    //    void updateSearchCrateData(const QVariantList& searchCrateData);
    void setBlockerOff(QString signal);

  public slots:
    void activate() override;
    void activateChild(const QModelIndex& index) override;
    void onRightClick(const QPoint& globalPos) override;
    void onRightClickChild(const QPoint& globalPos, const QModelIndex& index) override;
    void slotCreateSearchCrate();
    void slotCreateSearchCrateFromUI();
    void slotFindPreviousSearchCrate();
    void slotFindNextSearchCrate();
    void slotCreateSearchCrateFromSearch(const QString& text);
    void deleteItem(const QModelIndex& index) override;
    void renameItem(const QModelIndex& index) override;
    void SetActiveSearchCrateToLastRightClicked(const QModelIndex& index) override;

  private slots:
    void slotDeleteSearchCrate();
    void slotEditSearchCrate();
    void slotRenameSearchCrate();
    void slotDuplicateSearchCrate();
    void slotToggleSearchCrateLock();
    void slotAnalyzeSearchCrate();
    void slotSearchCrateTableChanged(SearchCrateId searchCrateId);
    void slotSearchCrateContentChanged(SearchCrateId searchCrateId);
    void htmlLinkClicked(const QUrl& link);
    void slotTrackSelected(TrackId trackId);
    void slotResetSelectedTrack();
    void slotUpdateSearchCrateLabels(const QSet<SearchCrateId>& updatedSearchCrateIds);

  private:
    void initActions();
    void connectLibrary(Library* pLibrary);
    void connectTrackCollection();

    bool activateSearchCrate(SearchCrateId searchCrateId);

    std::unique_ptr<TreeItem> newTreeItemForSearchCrateSummary(
            const SearchCrateSummary& searchCrateSummary);
    void updateTreeItemForSearchCrateSummary(
            TreeItem* pTreeItem,
            const SearchCrateSummary& searchCrateSummary) const;

    void selectSearchCrateForEdit(SearchCrateId selectedSearchCrateId = SearchCrateId());

    QModelIndex rebuildChildModel(SearchCrateId selectedSearchCrateId = SearchCrateId());
    void updateChildModel(const QSet<SearchCrateId>& updatedSearchCrateIds);

    SearchCrateId searchCrateIdFromIndex(const QModelIndex& index) const;
    QModelIndex indexFromSearchCrateId(SearchCrateId searchCrateId) const;

    bool isChildIndexSelectedInSidebar(const QModelIndex& index);
    bool readLastRightClickedSearchCrate(SearchCrate* pSearchCrate) const;

    QString formatRootViewHtml() const;

    const QIcon m_lockedSearchCrateIcon;

    TrackCollection* const m_pTrackCollection;

    SearchCrateTableModel m_searchCrateTableModel;

    // Stores the id of a searchCrate in the sidebar that is adjacent to the
    // searchCrate(searchCrateId).
    void storePrevSiblingSearchCrateId(SearchCrateId searchCrateId);
    void storeNextSiblingSearchCrateId(SearchCrateId searchCrateId);
    // Can be used to restore a similar selection after the sidebar model was rebuilt.
    SearchCrateId m_prevSiblingSearchCrate;
    SearchCrateId m_nextSiblingSearchCrate;

    QModelIndex m_lastClickedIndex;
    QModelIndex m_lastRightClickedIndex;
    TrackId m_selectedTrackId;

    parented_ptr<QAction> m_pCreateSearchCrateAction;
    parented_ptr<QAction> m_pEditSearchCrateAction;
    parented_ptr<QAction> m_pDeleteSearchCrateAction;
    parented_ptr<QAction> m_pRenameSearchCrateAction;
    parented_ptr<QAction> m_pLockSearchCrateAction;
    parented_ptr<QAction> m_pDuplicateSearchCrateAction;
    parented_ptr<QAction> m_pAnalyzeSearchCrateAction;

    QPointer<WLibrarySidebar> m_pSidebarWidget;

    QVariantList searchCrateData;
};
