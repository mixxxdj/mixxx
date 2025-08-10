#pragma once

#include <QList>
#include <QModelIndex>
#include <QPointer>
#include <QUrl>
#include <QVariant>

#include "library/trackset/basetracksetfeature.h"
#include "library/trackset/genre/genre.h"
#include "library/trackset/genre/genretablemodel.h"
#include "preferences/usersettings.h"
#include "track/trackid.h"
#include "util/parented_ptr.h"

// forward declaration(s)
class Library;
class WLibrarySidebar;
class QAction;
class QPoint;
class GenreSummary;

class GenreFeature : public BaseTrackSetFeature {
    Q_OBJECT

  public:
    GenreFeature(Library* pLibrary,
            UserSettingsPointer pConfig);
    ~GenreFeature() override = default;

    QVariant title() override;

    bool dropAcceptChild(const QModelIndex& index,
            const QList<QUrl>& urls,
            QObject* pSource) override;
    bool dragMoveAcceptChild(const QModelIndex& index, const QUrl& url) override;

    void bindLibraryWidget(WLibrary* libraryWidget,
            KeyboardEventFilter* keyboard) override;
    void bindSidebarWidget(WLibrarySidebar* pSidebarWidget) override;

    TreeItemModel* sidebarModel() const override;

  public slots:
    void activate() override;
    void activateChild(const QModelIndex& index) override;
    void onRightClick(const QPoint& globalPos) override;
    void onRightClickChild(const QPoint& globalPos, const QModelIndex& index) override;
    void slotCreateGenre();
    void deleteItem(const QModelIndex& index) override;
    void renameItem(const QModelIndex& index) override;

#ifdef __ENGINEPRIME__
  signals:
    void exportAllGenres();
    void exportGenre(GenreId genreId);
#endif

  private slots:
    void slotDeleteGenre();
    void slotRenameGenre();
    void slotDuplicateGenre();
    void slotAutoDjTrackSourceChanged();
    void slotToggleGenreLock();
    void slotImportPlaylist();
    void slotImportPlaylistFile(const QString& playlistFile, GenreId genreId);
    void slotCreateImportGenre();
    void slotExportPlaylist();
    // Copy all of the tracks in a genre to a new directory (like a thumbdrive).
    void slotExportTrackFiles();
    void slotAnalyzeGenre();
    void slotGenreTableChanged(GenreId genreId);
    void slotGenreContentChanged(GenreId genreId);
    void htmlLinkClicked(const QUrl& link);
    void slotTrackSelected(TrackId trackId);
    void slotResetSelectedTrack();
    void slotUpdateGenreLabels(const QSet<GenreId>& updatedGenreIds);

    void slotImportGenreModelFromCsv();
    void slotEditGenre();
    void slotEditGenreMulti();
    void slotMakeGenreInVisible();
    void slotSetAllGenresVisible();
    void slotEditOrphanTrackGenres();
    void slotExportGenresToCsv();
    void slotImportGenresFromCsv();

  private:
    void initActions();
    void connectLibrary(Library* pLibrary);
    void connectTrackCollection();

    bool activateGenre(GenreId genreId);

    std::unique_ptr<TreeItem> newTreeItemForGenreSummary(
            const GenreSummary& genreSummary);
    void updateTreeItemForGenreSummary(
            TreeItem* pTreeItem,
            const GenreSummary& genreSummary) const;

    QModelIndex rebuildChildModel(GenreId selectedGenreId = GenreId());
    void updateChildModel(const QSet<GenreId>& updatedGenreIds);

    GenreId genreIdFromIndex(const QModelIndex& index) const;
    QModelIndex indexFromGenreId(GenreId genreId) const;

    bool isChildIndexSelectedInSidebar(const QModelIndex& index);
    bool readLastRightClickedGenre(Genre* pGenre) const;

    QString formatRootViewHtml() const;

    const QIcon m_lockedGenreIcon;

    TrackCollection* const m_pTrackCollection;

    GenreTableModel m_genreTableModel;

    // Stores the id of a genre in the sidebar that is adjacent to the genre(genreId).
    void storePrevSiblingGenreId(GenreId genreId);
    // Can be used to restore a similar selection after the sidebar model was rebuilt.
    GenreId m_prevSiblingGenre;

    QModelIndex m_lastClickedIndex;
    QModelIndex m_lastRightClickedIndex;
    TrackId m_selectedTrackId;

    parented_ptr<QAction> m_pImportGenreModelFromCsvAction;
    parented_ptr<QAction> m_pEditGenreAction;
    parented_ptr<QAction> m_pEditGenreMultiAction;
    parented_ptr<QAction> m_pSetAllGenresVisibleAction;
    parented_ptr<QAction> m_pMakeGenreInVisible;
    parented_ptr<QAction> m_pEditOrphanGenresAction;
    parented_ptr<QAction> m_pExportGenresToCsv;
    parented_ptr<QAction> m_pImportGenresFromCsv;
    parented_ptr<QAction> m_pCreateGenreAction;
    parented_ptr<QAction> m_pDeleteGenreAction;
    parented_ptr<QAction> m_pRenameGenreAction;
    parented_ptr<QAction> m_pLockGenreAction;
    parented_ptr<QAction> m_pDuplicateGenreAction;
    parented_ptr<QAction> m_pAutoDjTrackSourceAction;
    parented_ptr<QAction> m_pImportPlaylistAction;
    parented_ptr<QAction> m_pCreateImportPlaylistAction;
    parented_ptr<QAction> m_pExportPlaylistAction;
    parented_ptr<QAction> m_pExportTrackFilesAction;
#ifdef __ENGINEPRIME__
    parented_ptr<QAction> m_pExportAllGenresAction;
    parented_ptr<QAction> m_pExportGenreAction;
#endif
    parented_ptr<QAction> m_pAnalyzeGenreAction;

    QPointer<WLibrarySidebar> m_pSidebarWidget;
};
