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

    // Used in smarties to send signal to dlg
    // New signal for updating smarties data
    void updateSmartiesData(const QVariantList& data);

  public slots:
    void activate() override;

  protected:
    const QString m_rootViewName;

    parented_ptr<TreeItemModel> m_pSidebarModel;
};
