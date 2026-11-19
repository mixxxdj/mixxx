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
    void activate() override;
    void pasteChild(const QModelIndex& index) override;

  signals:
    void analyzeTracks(const QList<AnalyzerScheduledTrack>&);

  protected:
    const QString m_rootViewName;

    parented_ptr<TreeItemModel> m_pSidebarModel;
};
