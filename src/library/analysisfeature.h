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
#include "preferences/usersettings.h"
#include "util/db/dbconnectionpool.h"

class Library;
class TrackCollection;
class AnalyzerQueue;

class AnalysisFeature : public LibraryFeature {
    Q_OBJECT
  public:
    AnalysisFeature(Library* parent,
                    UserSettingsPointer pConfig,
                    TrackCollection* pTrackCollection);
    virtual ~AnalysisFeature();

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
    void trackAnalysisStarted(int size);

  public slots:
    void activate();
    void analyzeTracks(QList<TrackId> trackIds);

  private slots:
    void slotProgressUpdate(int num_left);
    void stopAnalysis();
    void cleanupAnalyzer();

  private:
    // Sets the title of this feature to the default name, given by
    // m_sAnalysisTitleName
    void setTitleDefault();

    // Sets the title of this feature to the default name followed by (x / y)
    // where x is the current track being analyzed and y is the total number of
    // tracks in the job
    void setTitleProgress(int trackNum, int totalNum);

    UserSettingsPointer m_pConfig;
    Library* m_pLibrary;
    mixxx::DbConnectionPoolPtr m_pDbConnectionPool;
    TrackCollection* m_pTrackCollection;
    AnalyzerQueue* m_pAnalyzerQueue;
    // Used to temporarily enable BPM detection in the prefs before we analyse
    int m_iOldBpmEnabled;
    // The title returned by title()
    QVariant m_Title;
    TreeItemModel m_childModel;
    const static QString m_sAnalysisViewName;
    QString m_analysisTitleName;
    DlgAnalysis* m_pAnalysisView;
    QIcon m_icon;
};


#endif /* ANALYSISFEATURE_H */
