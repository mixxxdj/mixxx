// analysisfeature.h
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)
// Forked 11/11/2009 by Albert Santoni (alberts@mixxx.org)

#ifndef ANALYSISFEATURE_H
#define ANALYSISFEATURE_H

#include <QUrl>
#include <QList>

#include "library/libraryfeature.h"
#include "library/features/analysis/dlganalysis.h"
#include "library/treeitemmodel.h"
#include "util/db/dbconnectionpool.h"

class AnalyzerQueue;
class DlgAnalysis;
class TrackCollection;
class AnalyzerQueue;

class AnalysisFeature : public LibraryFeature {
    Q_OBJECT
  public:
    AnalysisFeature(UserSettingsPointer pConfig,
                    Library* pLibrary,
                    TrackCollection* pTrackCollection,
                    QObject* parent);
    virtual ~AnalysisFeature();

    QVariant title() override;
    QString getIconPath() override;
    QString getSettingsName() const override;

    bool dropAccept(QList<QUrl> urls, QObject* pSource);
    bool dragMoveAccept(QUrl url);

    parented_ptr<QWidget> createPaneWidget(KeyboardEventFilter*, int paneId,
                                           QWidget* parent) override;
    parented_ptr<QWidget> createInnerSidebarWidget(KeyboardEventFilter* pKeyboard,
                                                   QWidget* parent) override;

    QPointer<TreeItemModel> getChildModel();
    void refreshLibraryModels();
    void stopAnalysis();

  signals:
    void analysisActive(bool bActive);
    void trackAnalysisStarted(int size);

  public slots:
    void selectAll();
    void activate();
    void analyzeTracks(QList<TrackId> trackIds);

  private slots:
    void slotProgressUpdate(int num_left);
    void cleanupAnalyzer();
    void tableSelectionChanged(const QItemSelection&,
                               const QItemSelection&);

  private:
    // Sets the title of this feature to the default name, given by
    // m_sAnalysisTitleName
    void setTitleDefault();

    // Sets the title of this feature to the default name followed by (x / y)
    // where x is the current track being analyzed and y is the total number of
    // tracks in the job
    void setTitleProgress(int trackNum, int totalNum);

    AnalysisLibraryTableModel* getAnalysisTableModel();

    mixxx::DbConnectionPoolPtr m_pDbConnectionPool;
    AnalyzerQueue* m_pAnalyzerQueue;
    // Used to temporarily enable BPM detection in the prefs before we analyse
    int m_iOldBpmEnabled;
    // The title returned by title()
    QVariant m_Title;
    TreeItemModel m_childModel;
    QString m_analysisTitleName;
    QPointer<DlgAnalysis> m_pAnalysisView;
    AnalysisLibraryTableModel m_analysisLibraryTableModel;
};


#endif /* ANALYSISFEATURE_H */
