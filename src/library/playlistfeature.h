// playlistfeature.h
// Created 8/17/09 by RJ Ryan (rryan@mit.edu)

#ifndef PLAYLISTFEATURE_H
#define PLAYLISTFEATURE_H

#include <QVariant>
#include <QIcon>
#include <QModelIndex>
#include <QUrl>
#include <QObject>
#include <QPoint>

#include "library/baseplaylistfeature.h"
#include "preferences/usersettings.h"

class TrackCollection;
class TreeItem;

class PlaylistFeature : public BasePlaylistFeature {
    Q_OBJECT
  public:
    PlaylistFeature(QObject* parent, TrackCollection* pTrackCollection,
                    UserSettingsPointer pConfig);
    virtual ~PlaylistFeature();

    QVariant title();
    QIcon getIcon();

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
    QList<BasePlaylistFeature::IdAndLabel> createPlaylistLabels() override;
    QString fetchPlaylistLabel(int playlistId) override;
    void decorateChild(TreeItem *pChild, int playlist_id) override;

  private:
    QString getRootViewHtml() const;
    QIcon m_icon;
};

#endif /* PLAYLISTFEATURE_H */
