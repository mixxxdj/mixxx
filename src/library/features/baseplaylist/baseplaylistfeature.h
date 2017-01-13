#ifndef BASEPLAYLISTFEATURE_H
#define BASEPLAYLISTFEATURE_H

#include <QAction>
#include <QUrl>
#include <QObject>
#include <QModelIndex>
#include <QAction>
#include <QList>
#include <QPair>
#include <QSet>
#include <QString>

#include "library/dao/playlistdao.h"
#include "library/dao/trackdao.h"
#include "library/libraryfeature.h"
#include "track/track.h"

class KeyboardEventFilter;
class PlaylistTableModel;
class TrackCollection;
class TreeItem;
class WLibraryPane;
class WLibraryStack;

class BasePlaylistFeature : public LibraryFeature {
    Q_OBJECT
  public:
    BasePlaylistFeature(UserSettingsPointer pConfig,
                        Library* pLibrary,
                        QObject* parent,
                        TrackCollection* pTrackCollection);
    virtual ~BasePlaylistFeature();

    TreeItemModel* getChildModel();

    QWidget* createPaneWidget(KeyboardEventFilter*pKeyboard, int paneId) override;

  signals:
    void showPage(const QUrl& page);
    void analyzeTracks(QList<TrackId>);

  public slots:
    void activate() override;
    void activateChild(const QModelIndex& index) override;
    void invalidateChild() override;

    virtual void activatePlaylist(int playlistId);
    virtual void htmlLinkClicked(const QUrl& link);

    virtual void slotPlaylistTableChanged(int playlistId) = 0;
    virtual void slotPlaylistContentChanged(int playlistId) = 0;
    virtual void slotPlaylistTableRenamed(int playlistId, QString a_strName) = 0;
    void slotCreatePlaylist();
    void setFeaturePaneId(int focus);

  protected slots:
    void slotDeletePlaylist();
    void slotDuplicatePlaylist();
    void slotAddToAutoDJ();
    void slotAddToAutoDJTop();
    void slotRenamePlaylist();
    void slotTogglePlaylistLock();
    void slotImportPlaylist();
    void slotImportPlaylistFile(const QString &playlist_file);
    void slotCreateImportPlaylist();
    void slotExportPlaylist();
    // Copy all of the tracks in a playlist to a new directory.
    void slotExportTrackFiles();
    void slotAnalyzePlaylist();

  protected:
    
    struct PlaylistItem {
        PlaylistItem() : id(-1) {}
        PlaylistItem(int id) : id(id) {}
        PlaylistItem(int id, QString name) : id(id), name(name) {}
        
        int id;
        QString name;
        
        bool operator==(const PlaylistItem& other) {
            return this->id == other.id;
        }
    };
    
    virtual QModelIndex constructChildModel(int selected_id);
    virtual void updateChildModel(int selectedId);
    virtual void buildPlaylistList() = 0;
    virtual void decorateChild(TreeItem *pChild, int playlist_id) = 0;
    virtual void addToAutoDJ(bool bTop);
    QString getValidPlaylistName() const;
    
    QPointer<PlaylistTableModel> getPlaylistTableModel(int paneId = -1);
    virtual PlaylistTableModel* constructTableModel() = 0;
    
    virtual QSet<int> playlistIdsFromIndex(const QModelIndex& index) const;
    int playlistIdFromIndex(const QModelIndex& index) const;
    void showTable(int paneId);
    void showBrowse(int paneId);
    
    // Get the QModelIndex of a playlist based on its id.  Returns QModelIndex()
    // on failure.
    virtual QModelIndex indexFromPlaylistId(int playlistId) const;

    PlaylistDAO &m_playlistDao;
    TrackDAO &m_trackDao;
    QPointer<PlaylistTableModel> m_pPlaylistTableModel;
    QHash<int, QPointer<PlaylistTableModel> > m_playlistTableModel;
    QAction *m_pCreatePlaylistAction;
    QAction *m_pDeletePlaylistAction;
    QAction *m_pAddToAutoDJAction;
    QAction *m_pAddToAutoDJTopAction;
    QAction *m_pRenamePlaylistAction;
    QAction *m_pLockPlaylistAction;
    QAction *m_pImportPlaylistAction;
    QAction *m_pCreateImportPlaylistAction;
    QAction *m_pExportPlaylistAction;
    QAction *m_pExportTrackFilesAction;
    QAction *m_pDuplicatePlaylistAction;
    QAction *m_pAnalyzePlaylistAction;
    QList<PlaylistItem> m_playlistList;
    QModelIndex m_lastRightClickedIndex;
    TreeItemModel* m_childModel;
    TrackPointer m_pSelectedTrack;
    
    QHash<int, QModelIndex> m_lastChildClicked;

  private slots:
    void slotTrackSelected(TrackPointer pTrack);

  private:
    virtual QString getRootViewHtml() const = 0;

    QSet<int> m_playlistsSelectedTrackIsIn;
    
    QHash<int, QPointer<WLibraryStack> > m_panes;
    QHash<int, int> m_browseIndexByPaneId;
    QHash<int, int> m_tableIndexByPaneId;
};

#endif /* BASEPLAYLISTFEATURE_H */
