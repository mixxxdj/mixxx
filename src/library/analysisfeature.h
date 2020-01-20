#pragma once

#include <QStringListModel>
#include <QUrl>
#include <QObject>
#include <QVariant>
#include <QIcon>
#include <QList>

#include "library/libraryfeature.h"
#include "library/dlganalysis.h"
#include "library/treeitemmodel.h"
#include "analyzer/trackanalysisscheduler.h"
#include "preferences/usersettings.h"

class TrackCollection;

class AnalysisFeature : public LibraryFeature {
    Q_OBJECT
  public:
    AnalysisFeature(Library* pLibrary,
                    UserSettingsPointer pConfig);
    ~AnalysisFeature() override = default;

    QVariant title() override {
        return m_title;
    }

    QIcon getIcon() override {
        return m_icon;
    }

    bool dropAccept(const QList<QUrl>& urls, QObject* pSource) override;
    bool dragMoveAccept(const QUrl& url) override;
    void bindLibraryWidget(WLibrary* libraryWidget,
                    KeyboardEventFilter* keyboard) override;

    TreeItemModel* getChildModel() override;
    void refreshLibraryModels();

  signals:
    void analysisActive(bool bActive);

  public slots:
    void activate() override;
    void analyzeTracks(const QList<TrackId>& trackIds);

    void suspendAnalysis();
    void resumeAnalysis();
    void stopAnalysis();

  private slots:
    void onTrackAnalysisSchedulerProgress(AnalyzerProgress currentTrackProgress, int currentTrackNumber, int totalTracksCount);
    void onTrackAnalysisSchedulerFinished();

  private:
    // Sets the title of this feature to the default name, given by
    // m_sAnalysisTitleName
    void resetTitle();

    // Sets the title of this feature to the default name followed by (x / y)
    // where x is the current track being analyzed and y is the total number of
    // tracks in the job
    void setTitleProgress(int currentTrackNumber, int totalTracksCount);

    const QString m_baseTitle;
    const QIcon m_icon;

    TrackAnalysisScheduler::Pointer m_pTrackAnalysisScheduler;

    TreeItemModel m_childModel;
    DlgAnalysis* m_pAnalysisView;

    // The title is dynamic and reflects the current progress
    QString m_title;
};
