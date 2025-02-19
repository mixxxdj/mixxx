#pragma once

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

    // Used in searchCrate to send signal to dlg
    // New signal for updating searchCrate data
    void updateSearchCrateData(const QVariantList& data);

  public slots:
    void activate() override;

  protected:
    const QString m_rootViewName;

    parented_ptr<TreeItemModel> m_pSidebarModel;
};
