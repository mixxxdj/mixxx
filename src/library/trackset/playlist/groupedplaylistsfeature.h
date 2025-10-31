#pragma once

#include <QModelIndex>
#include <QObject>
#include <QUrl>
#include <QVariant>

#include "library/trackset/playlist/basegroupedplaylistsfeature.h"
#include "preferences/usersettings.h"

class TreeItem;
class QPoint;

class GroupedPlaylistsFeature : public BaseGroupedPlaylistsFeature {
    Q_OBJECT

  public:
    GroupedPlaylistsFeature(
            Library* pLibrary,
            UserSettingsPointer pConfig);
    ~GroupedPlaylistsFeature() override = default;

    QVariant title() override;

    bool dropAcceptChild(const QModelIndex& index,
            const QList<QUrl>& urls,
            QObject* pSource) override;
    bool dragMoveAcceptChild(const QModelIndex& index, const QUrl& url) override;

  public slots:
    void onRightClick(const QPoint& globalPos) override;
    void onRightClickChild(const QPoint& globalPos, const QModelIndex& index) override;

  private slots:
    void slotPlaylistTableChanged(int playlistId) override;
    void slotPlaylistContentOrLockChanged(const QSet<int>& playlistIds) override;
    void slotPlaylistTableRenamed(int playlistId, const QString& newName) override;
    void slotShufflePlaylist();

  protected:
    void decorateChild(TreeItem* pChild, int playlistId) override;
    QList<IdAndLabel> createPlaylistLabels();
    QModelIndex constructChildModel(int selectedId);

  private:
    QString getRootViewHtml() const override;

    QAction* m_pShufflePlaylistAction;
};
