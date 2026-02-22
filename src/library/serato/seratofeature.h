#pragma once
// seratofeature.h
// Created 2020-01-31 by Jan Holthuis
//
// This feature reads tracks and crates from removable Serato Libraries,
// either in the Music directory or on removable devices (USB drives, etc),
// by parsing the contents of the _Serato_ directory on each device.
//
// Most of the groundwork for this has been done here:
//
//      https://github.com/Holzhaus/serato-tags
//      https://github.com/Holzhaus/serato-tags/blob/main/scripts/database_v2.py

#include <QFuture>
#include <QFutureWatcher>

#include "library/baseexternallibraryfeature.h"
#include "util/parented_ptr.h"

class SeratoPlaylistModel;
class BaseTrackCache;

class SeratoFeature : public BaseExternalLibraryFeature {
    Q_OBJECT
  public:
    SeratoFeature(Library* pLibrary, UserSettingsPointer pConfig);
    ~SeratoFeature() override;

    QVariant title() override;
    static bool isSupported();
    void bindLibraryWidget(WLibrary* libraryWidget,
            KeyboardEventFilter* keyboard) override;

    TreeItemModel* sidebarModel() const override;

  public slots:
    void activate() override;
    void activateChild(const QModelIndex& index) override;
    void refreshLibraryModels();
    void onSeratoDatabasesFound();
    void onTracksFound();

  private slots:
    void htmlLinkClicked(const QUrl& link);

  private:
    QString formatRootViewHtml() const;
    std::unique_ptr<BaseSqlTableModel> createPlaylistModelForPlaylist(
            const QVariant& data) override;

    parented_ptr<TreeItemModel> m_pSidebarModel;
    SeratoPlaylistModel* m_pSeratoPlaylistModel;

    QFutureWatcher<QList<TreeItem*>> m_databasesFutureWatcher;
    QFuture<QList<TreeItem*>> m_databasesFuture;
    QFutureWatcher<QString> m_tracksFutureWatcher;
    QFuture<QString> m_tracksFuture;
    QString m_title;

    QSharedPointer<BaseTrackCache> m_trackSource;
};
