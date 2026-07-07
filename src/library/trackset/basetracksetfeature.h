#pragma once

#include <QFileDialog>

#include "library/libraryfeature.h"
#include "util/parented_ptr.h"

class AnalyzerScheduledTrack;

class BaseTrackSetFeature : public LibraryFeature {
    Q_OBJECT

  public:
    BaseTrackSetFeature(Library* pLibrary,
            UserSettingsPointer pConfig,
            const QString& rootViewName,
            const QString& iconName);

    void pasteChild(const QModelIndex& index) override;
  signals:
    void analyzeTracks(const QList<AnalyzerScheduledTrack>&);

  public slots:
    void activate() override;

  protected:
    const QString m_rootViewName;

    parented_ptr<TreeItemModel> m_pSidebarModel;

    QStringList getPlaylistFiles(QFileDialog::FileMode mode) const;
    QStringList getPlaylistFiles() const;
    QString getPlaylistFile() const;

    static bool exportPlaylistItemsIntoFile(
            QString playlistFilePath,
            const QList<QString>& playlistItemLocations,
            bool useRelativePath);
};
