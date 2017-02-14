// playlistfeature.h
// Created 8/17/09 by RJ Ryan (rryan@mit.edu)

#ifndef PLAYLISTFEATURE_H
#define PLAYLISTFEATURE_H

#include "library/features/baseplaylist/baseplaylistfeature.h"

class TrackCollection;
class TreeItem;

class PlaylistFeature : public BasePlaylistFeature {
    Q_OBJECT
  public:
    PlaylistFeature(UserSettingsPointer pConfig, 
                    Library* pLibrary,
                    QObject* parent, 
                    TrackCollection* pTrackCollection);
    virtual ~PlaylistFeature();

    QVariant title() override;
    QString getIconPath() override;
    QString getSettingsName() const override;
    bool isSinglePane() const override;
    TreeItemModel* getChildModel() override;

    bool dragMoveAccept(QUrl url);
    bool dropAcceptChild(const QModelIndex& index, QList<QUrl> urls, QObject* pSource);
    bool dragMoveAcceptChild(const QModelIndex& index, QUrl url);

  public slots:
    void onRightClick(const QPoint& globalPos);
    void onRightClickChild(const QPoint& globalPos, QModelIndex index);

  private slots:
    void slotPlaylistTableChanged(int playlistId);
    void slotPlaylistContentChanged(int playlistId);
    void slotPlaylistTableRenamed(int playlistId, QString a_strName);

 protected:
    const TreeItemModel* getConstChildModel() const override;
    void buildPlaylistList();
    void decorateChild(TreeItem *pChild, int playlist_id);
    parented_ptr<PlaylistTableModel> constructTableModel();

  private:
    QString getRootViewHtml() const;
    std::unique_ptr<TreeItemModel> m_pChildModel;
};

#endif /* PLAYLISTFEATURE_H */
