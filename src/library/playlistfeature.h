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
#include <QPointer>

#include "library/baseplaylistfeature.h"
#include "preferences/usersettings.h"

class TrackCollection;
class TreeItem;
class WLibrarySidebar;

class PlaylistFeature : public BasePlaylistFeature {
    Q_OBJECT
  public:
    PlaylistFeature(QObject* parent, TrackCollection* pTrackCollection,
                    UserSettingsPointer pConfig);
    virtual ~PlaylistFeature();

    QVariant title() override;
    QIcon getIcon() override;

    void bindSidebarWidget(WLibrarySidebar* pSidebarWidget) override;

    bool dropAcceptChild(const QModelIndex& index, QList<QUrl> urls, QObject* pSource) override;
    bool dragMoveAcceptChild(const QModelIndex& index, QUrl url) override;

  public slots:
    void onRightClick(const QPoint& globalPos) override;
    void onRightClickChild(const QPoint& globalPos, QModelIndex index) override;

  private slots:
    void slotPlaylistTableChanged(int playlistId) override;
    void slotPlaylistContentChanged(int playlistId) override;
    void slotPlaylistTableRenamed(int playlistId, QString a_strName) override;

 protected:
    QList<BasePlaylistFeature::IdAndLabel> createPlaylistLabels() override;
    QString fetchPlaylistLabel(int playlistId) override;
    void decorateChild(TreeItem *pChild, int playlist_id) override;

  private:
    QString getRootViewHtml() const override;
    QIcon m_icon;
    QPointer<WLibrarySidebar> m_pSidebarWidget;
};

#endif /* PLAYLISTFEATURE_H */
