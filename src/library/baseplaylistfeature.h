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

#include "library/libraryfeature.h"
#include "library/dao/playlistdao.h"
#include "library/dao/trackdao.h"
#include "track/track.h"

class WLibrary;
class KeyboardEventFilter;
class PlaylistTableModel;
class TrackCollection;
class TreeItem;

class BasePlaylistFeature : public LibraryFeature {
    Q_OBJECT
  public:
    BasePlaylistFeature(QObject* parent,
                        UserSettingsPointer pConfig,
                        TrackCollection* pTrackCollection,
                        QString rootViewName);
    virtual ~BasePlaylistFeature();

    TreeItemModel* getChildModel();

    void bindWidget(WLibrary* libraryWidget,
                    KeyboardEventFilter* keyboard);

  signals:
    void showPage(const QUrl& page);
    void analyzeTracks(QList<TrackId>);

  public slots:
    virtual void activate();
    virtual void activateChild(const QModelIndex& index);
    virtual void activatePlaylist(int playlistId);
    virtual void htmlLinkClicked(const QUrl& link);

    virtual void slotPlaylistTableChanged(int playlistId) = 0;
    virtual void slotPlaylistContentChanged(int playlistId) = 0;
    virtual void slotPlaylistTableRenamed(int playlistId, QString a_strName) = 0;
    void slotCreatePlaylist();

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

    struct IdAndLabel {
        int id;
        QString label;
    };


    virtual QModelIndex constructChildModel(int selected_id);
    virtual void updateChildModel(int selected_id);
    virtual void clearChildModel();
    virtual QList<IdAndLabel> createPlaylistLabels() = 0;
    virtual QString fetchPlaylistLabel(int playlistId) = 0;
    virtual void decorateChild(TreeItem *pChild, int playlistId) = 0;
    virtual void addToAutoDJ(bool bTop);

    int playlistIdFromIndex(QModelIndex index);
    // Get the QModelIndex of a playlist based on its id.  Returns QModelIndex()
    // on failure.
    QModelIndex indexFromPlaylistId(int playlistId);

    TrackCollection* m_pTrackCollection;
    PlaylistDAO &m_playlistDao;
    TrackDAO &m_trackDao;
    PlaylistTableModel* m_pPlaylistTableModel;
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
    QModelIndex m_lastRightClickedIndex;
    TreeItemModel m_childModel;
    TrackPointer m_pSelectedTrack;

  private slots:
    void slotTrackSelected(TrackPointer pTrack);
    void slotResetSelectedTrack();

  private:
    virtual QString getRootViewHtml() const = 0;

    QSet<int> m_playlistsSelectedTrackIsIn;
    QString m_rootViewName;
};

#endif /* BASEPLAYLISTFEATURE_H */
