#pragma once

#include "analyzer/analyzerscheduledtrack.h"
#include "library/libraryfeature.h"
#include "util/parented_ptr.h"

class BaseTrackSetFeature : public LibraryFeature {
    Q_OBJECT

  public:
    BaseTrackSetFeature(Library* pLibrary,
            UserSettingsPointer pConfig,
            const QString& rootViewName,
            const QString& iconName);

  signals:
    void analyzeTracks(const QList<AnalyzerScheduledTrack>&);

  public slots:
    void activate() override;

  protected:
    const QString m_rootViewName;

    parented_ptr<TreeItemModel> m_pSidebarModel;
};
