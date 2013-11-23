// preparefeature.h
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)
// Forked 11/11/2009 by Albert Santoni (alberts@mixxx.org)

#ifndef TRIAGEFEATURE_H
#define TRIAGEFEATURE_H

#include <QStringListModel>
#include "library/libraryfeature.h"
#include "configobject.h"
#include "treeitemmodel.h"
#include "dlgprepare.h"

class AnalyserQueue;
class LibraryTableModel;
class TrackCollection;

class PrepareFeature : public LibraryFeature {
    Q_OBJECT
  public:
    PrepareFeature(QObject* parent,
                   ConfigObject<ConfigValue>* pConfig,
                   TrackCollection* pTrackCollection);
    virtual ~PrepareFeature();

    QVariant title();
    QIcon getIcon();

    void bindWidget(WLibrary* libraryWidget,
                    MixxxKeyboard* keyboard);

    TreeItemModel* getChildModel();
    void refreshLibraryModels();

  signals:
    void analysisActive(bool bActive);

  public slots:
    void activate();

  private slots:
    void analyzeTracks(QList<int> trackIds);
    void stopAnalysis();
    void cleanupAnalyser();

  private:
    ConfigObject<ConfigValue>* m_pConfig;
    TrackCollection* m_pTrackCollection;
    AnalyserQueue* m_pAnalyserQueue;
    // Used to temporarily enable BPM detection in the prefs before we analyse
    int m_iOldBpmEnabled;
    TreeItemModel m_childModel;
    const static QString m_sPrepareViewName;
    DlgPrepare* m_pPrepareView;
};


#endif /* PREPAREFEATURE_H */
