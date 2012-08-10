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

    bool dropAccept(QList<QUrl> urls);
    bool dropAcceptChild(const QModelIndex& index, QList<QUrl> urls);
    bool dragMoveAccept(QUrl url);
    bool dragMoveAcceptChild(const QModelIndex& index, QUrl url);

    void bindWidget(WLibrarySidebar* sidebarWidget,
                    WLibrary* libraryWidget,
                    MixxxKeyboard* keyboard);

    TreeItemModel* getChildModel();
    void refreshLibraryModels();

  signals:
    void trackAnalysisProgress(TrackPointer pTrack, int progress);
    void trackAnalysisFinished(TrackPointer pTrack);
    void analysisActive(bool bActive);

  public slots:
    void activate();
    void activateChild(const QModelIndex& index);
    void onRightClick(const QPoint& globalPos);
    void onRightClickChild(const QPoint& globalPos, QModelIndex index);
    void onLazyChildExpandation(const QModelIndex& index);

  private slots:
    void analyzeTracks(QList<int> trackIds);
    void stopAnalysis();
    void slotTrackAnalysisProgress(TrackPointer pTrack, int progress);
    void slotTrackAnalysisFinished(TrackPointer pTrack);

  private:
    void cleanupAnalyser();
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
