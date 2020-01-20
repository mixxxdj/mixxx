#pragma once

#include <QIcon>
#include <QModelIndex>
#include <QObject>
#include <QPoint>
#include <QPointer>
#include <QUrl>
#include <QVariant>

#include "library/baseplaylistfeature.h"
#include "preferences/usersettings.h"

class TrackCollection;
class TreeItem;
class WLibrarySidebar;

class PlaylistFeature : public BasePlaylistFeature {
    Q_OBJECT
  public:
    PlaylistFeature(
            Library* pLibrary,
            UserSettingsPointer pConfig);
    virtual ~PlaylistFeature();

    QVariant title() override;
    QIcon getIcon() override;

    void bindSidebarWidget(WLibrarySidebar* pSidebarWidget) override;

    bool dropAcceptChild(const QModelIndex& index,
            const QList<QUrl>& urls,
            QObject* pSource) override;
    bool dragMoveAcceptChild(const QModelIndex& index, const QUrl& url) override;

  public slots:
    void onRightClick(const QPoint& globalPos) override;
    void onRightClickChild(const QPoint& globalPos, const QModelIndex& index) override;

  private slots:
    void slotPlaylistTableChanged(int playlistId) override;
    void slotPlaylistContentChanged(QSet<int> playlistIds) override;
    void slotPlaylistTableRenamed(int playlistId, const QString& newName) override;

  protected:
    QList<BasePlaylistFeature::IdAndLabel> createPlaylistLabels() override;
    QString fetchPlaylistLabel(int playlistId) override;
    void decorateChild(TreeItem* pChild, int playlist_id) override;

  private:
    QString getRootViewHtml() const override;
    const QIcon m_icon;
    QPointer<WLibrarySidebar> m_pSidebarWidget;
};
