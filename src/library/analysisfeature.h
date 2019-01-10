// analysisfeature.h
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)
// Forked 11/11/2009 by Albert Santoni (alberts@mixxx.org)

#ifndef ANALYSISFEATURE_H
#define ANALYSISFEATURE_H

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

class Library;
class TrackCollection;

class AnalysisFeature : public LibraryFeature {
    Q_OBJECT
  public:
    AnalysisFeature(Library* parent,
                    UserSettingsPointer pConfig);
    ~AnalysisFeature() override = default;

    void stop();

    QVariant title();
    QIcon getIcon();

    bool dropAccept(QList<QUrl> urls, QObject* pSource);
    bool dragMoveAccept(QUrl url);
    void bindWidget(WLibrary* libraryWidget,
                    KeyboardEventFilter* keyboard);

    TreeItemModel* getChildModel();
    void refreshLibraryModels();

  signals:
    void analysisActive(bool bActive);

  public slots:
    void activate();
    void analyzeTracks(QList<TrackId> trackIds);

    void suspendAnalysis();
    void resumeAnalysis();

  private slots:
    void onTrackAnalysisSchedulerProgress(AnalyzerProgress currentTrackProgress, int currentTrackNumber, int totalTracksCount);
    void stopAnalysis();

  private:
    // Sets the title of this feature to the default name, given by
    // m_sAnalysisTitleName
    void setTitleDefault();

    // Sets the title of this feature to the default name followed by (x / y)
    // where x is the current track being analyzed and y is the total number of
    // tracks in the job
    void setTitleProgress(int currentTrackNumber, int totalTracksCount);

    Library* m_library;

    UserSettingsPointer m_pConfig;
    TrackAnalysisScheduler::Pointer m_pTrackAnalysisScheduler;

    // The title returned by title()
    QVariant m_Title;
    TreeItemModel m_childModel;
    const static QString m_sAnalysisViewName;
    QString m_analysisTitleName;
    DlgAnalysis* m_pAnalysisView;
    QIcon m_icon;
};


#endif /* ANALYSISFEATURE_H */
