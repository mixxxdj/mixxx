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
//      https://github.com/Holzhaus/serato-tags/blob/master/scripts/database_v2.py

#include <QFuture>
#include <QFutureWatcher>
#include <QStringListModel>
#include <QtConcurrentRun>
#include <fstream>

#include "library/baseexternallibraryfeature.h"
#include "library/baseexternaltrackmodel.h"
#include "library/serato/seratoplaylistmodel.h"
#include "library/treeitemmodel.h"

class SeratoFeature : public BaseExternalLibraryFeature {
    Q_OBJECT
  public:
    SeratoFeature(Library* pLibrary, UserSettingsPointer pConfig);
    ~SeratoFeature() override;

    QVariant title() override;
    QIcon getIcon() override;
    static bool isSupported();
    void bindLibraryWidget(WLibrary* libraryWidget,
            KeyboardEventFilter* keyboard) override;

    TreeItemModel* getChildModel() override;

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
    BaseSqlTableModel* getPlaylistModelForPlaylist(const QString& playlist) override;

    TreeItemModel m_childModel;
    SeratoPlaylistModel* m_pSeratoPlaylistModel;

    QFutureWatcher<QList<TreeItem*>> m_databasesFutureWatcher;
    QFuture<QList<TreeItem*>> m_databasesFuture;
    QFutureWatcher<QString> m_tracksFutureWatcher;
    QFuture<QString> m_tracksFuture;
    QString m_title;

    QSharedPointer<BaseTrackCache> m_trackSource;
    QIcon m_icon;
};
