#ifndef MIXXX_CRATEFEATURE_H
#define MIXXX_CRATEFEATURE_H


#include <QModelIndex>
#include <QList>
#include <QAction>
#include <QVariant>
#include <QUrl>
#include <QIcon>
#include <QPoint>

#include "library/crate/cratetablemodel.h"

#include "library/libraryfeature.h"
#include "library/treeitemmodel.h"

#include "track/track.h"

#include "preferences/usersettings.h"

#include "util/parented_ptr.h"

// forward declaration(s)
class Library;
class TrackCollection;


class CrateFeature : public LibraryFeature {
    Q_OBJECT
  public:
    CrateFeature(Library* pLibrary,
                 TrackCollection* pTrackCollection,
                 UserSettingsPointer pConfig);
    ~CrateFeature() override;

    QVariant title() override;
    QIcon getIcon() override;

    bool dropAcceptChild(const QModelIndex& index, QList<QUrl> urls,
                         QObject* pSource) override;
    bool dragMoveAcceptChild(const QModelIndex& index, QUrl url) override;

    void bindWidget(WLibrary* libraryWidget,
                    KeyboardEventFilter* keyboard) override;

    TreeItemModel* getChildModel() override;

  signals:
    void analyzeTracks(QList<TrackId>);

  public slots:
    void activate() override;
    void activateChild(const QModelIndex& index) override;
    void onRightClick(const QPoint& globalPos) override;
    void onRightClickChild(const QPoint& globalPos, QModelIndex index) override;
    void slotCreateCrate();

  private slots:
    void slotDeleteCrate();
    void slotRenameCrate();
    void slotDuplicateCrate();
    void slotAutoDjTrackSourceChanged();
    void slotToggleCrateLock();
    void slotImportPlaylist();
    void slotImportPlaylistFile(const QString &playlist_file);
    void slotCreateImportCrate();
    void slotExportPlaylist();
    // Copy all of the tracks in a crate to a new directory (like a thumbdrive).
    void slotExportTrackFiles();
    void slotAnalyzeCrate();
    void slotCrateTableChanged(CrateId crateId);
    void slotCrateContentChanged(CrateId crateId);
    void htmlLinkClicked(const QUrl& link);
    void slotTrackSelected(TrackPointer pTrack);
    void slotResetSelectedTrack();
    void slotUpdateCrateLabels(const QSet<CrateId>& updatedCrateIds);

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

    bool readLastRightClickedCrate(Crate* pCrate) const;

    QString formatRootViewHtml() const;

    const QIcon m_cratesIcon;
    const QIcon m_lockedCrateIcon;

    TrackCollection* m_pTrackCollection;

    CrateTableModel m_crateTableModel;
    TreeItemModel m_childModel;

    QModelIndex m_lastRightClickedIndex;
    TrackPointer m_pSelectedTrack;

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
    parented_ptr<QAction> m_pAnalyzeCrateAction;
};


#endif // MIXXX_CRATEFEATURE_H
