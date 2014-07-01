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
    QString getIconName();
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
    void slotProgressUpdate(int num_left);
    void stopAnalysis();
    void cleanupAnalyser();

  private:
    void setTitleDefault();		// Set title to tr("Analyze")
    void setTitleProgress(int trackNum, int totalNum);	// Set title to "Analyze (x out of y)"

    ConfigObject<ConfigValue>* m_pConfig;
    TrackCollection* m_pTrackCollection;
    AnalyserQueue* m_pAnalyserQueue;
    // Used to temporarily enable BPM detection in the prefs before we analyse
    int m_iOldBpmEnabled;
    // The title returned by title()
    QVariant m_Title;
    TreeItemModel m_childModel;
    const static QString m_sAnalysisViewName;
    QString m_analysisTitleName;
    DlgAnalysis* m_pAnalysisView;
};


#endif /* ANALYSISFEATURE_H */
