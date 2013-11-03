// analysisfeature.h
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)
// Forked 11/11/2009 by Albert Santoni (alberts@mixxx.org)

#ifndef TRIAGEFEATURE_H
#define TRIAGEFEATURE_H

#include <QStringListModel>
#include "library/libraryfeature.h"
#include "configobject.h"
#include "treeitemmodel.h"
#include "dlganalysis.h"

class AnalyserQueue;
class TrackCollection;

class AnalysisFeature : public LibraryFeature {
    Q_OBJECT
  public:
    AnalysisFeature(QObject* parent,
                   ConfigObject<ConfigValue>* pConfig,
                   TrackCollection* pTrackCollection);
    virtual ~AnalysisFeature();

    QVariant title();
    QIcon getIcon();

    bool dropAccept(QList<QUrl> urls, QObject* pSource);
    bool dragMoveAccept(QUrl url);
    void bindWidget(WLibrary* libraryWidget,
                    MixxxKeyboard* keyboard);

    TreeItemModel* getChildModel();
    void refreshLibraryModels();

  signals:
    void analysisActive(bool bActive);
    void trackAnalysisStarted(int size);

  public slots:
    void activate();
    void analyzeTracks(QList<int> trackIds);

  private slots:
    void stopAnalysis();
    void cleanupAnalyser();

  private:
    ConfigObject<ConfigValue>* m_pConfig;
    TrackCollection* m_pTrackCollection;
    AnalyserQueue* m_pAnalyserQueue;
    // Used to temporarily enable BPM detection in the prefs before we analyse
    int m_iOldBpmEnabled;
    TreeItemModel m_childModel;
    const static QString m_sAnalysisViewName;
    DlgAnalysis* m_pAnalysisView;
};


#endif /* ANALYSISFEATURE_H */
