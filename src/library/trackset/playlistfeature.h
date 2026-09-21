#pragma once

#include <QModelIndex>
#include <QObject>
#include <QUrl>
#include <QVariant>
#include <QList>

#include "library/trackset/baseplaylistfeature.h"
#include "preferences/usersettings.h"
#include "util/parented_ptr.h"

class TreeItem;
class QPoint;

class PlaylistFeature : public BasePlaylistFeature {
    Q_OBJECT

  public:
    struct ExtendedPlaylistLabel {
        int id;
        QString label;
        int parentId;
        bool isFolder;
    };

    PlaylistFeature(
            Library* pLibrary,
            UserSettingsPointer pConfig);
    ~PlaylistFeature() override = default;

    QVariant title() override;

    PlaylistDAO& playlistDao() { return m_playlistDao; }

    bool dropAcceptChild(const QModelIndex& index,
            const QList<QUrl>& urls,
            QObject* pSource) override;
    bool dragMoveAcceptChild(const QModelIndex& index, const QList<QUrl>& urls) override;

  public slots:
    void onRightClick(const QPoint& globalPos) override;
    void onRightClickChild(const QPoint& globalPos, const QModelIndex& index) override;
    void slotCreatePlaylist() override;

  private slots:
    void slotPlaylistTableChanged(int playlistId) override;
    void slotPlaylistContentOrLockChanged(const QSet<int>& playlistIds) override;
    void slotPlaylistTableRenamed(int playlistId, const QString& newName) override;
    void slotShufflePlaylist();
    void slotOrderTracksByCurrentPosition();
    void slotUnlockAllPlaylists();
    void slotDeleteAllUnlockedPlaylists();
    void slotCreateFolder();

    void slotMovePlaylist();
    int getParentIdForNewItem() const;

  protected:

    void decorateChild(TreeItem* pChild, int playlistId) override;
    QList<IdAndLabel> createPlaylistLabels(); // Need for QML support
    QList<ExtendedPlaylistLabel> createExtendedPlaylistLabels();
    QModelIndex constructChildModel(int selectedId);

  private:
    QString getRootViewHtml() const override;

    parented_ptr<QAction> m_pShufflePlaylistAction;
    parented_ptr<QAction> m_pOrderByCurrentPosAction;
    parented_ptr<QAction> m_pUnlockPlaylistsAction;
    parented_ptr<QAction> m_pDeleteAllUnlockedPlaylistsAction;
    parented_ptr<QAction> m_pMovePlaylistAction;
};
