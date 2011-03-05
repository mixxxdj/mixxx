#ifndef DLGTRIAGE_H
#define DLGTRIAGE_H

#include <QItemSelection>
#include "ui_dlgprepare.h"
#include "configobject.h"
#include "library/libraryview.h"
#include "library/trackcollection.h"

class PrepareLibraryTableModel;
class WPrepareCratesTableView;
class WPrepareLibraryTableView;
class AnalyserQueue;
class QSqlTableModel;
class CrateView;

class DlgPrepare : public QWidget, public Ui::DlgPrepare, public virtual LibraryView {
    Q_OBJECT
public:
    DlgPrepare(QWidget *parent, ConfigObject<ConfigValue>* pConfig, TrackCollection* pTrackCollection);
    virtual ~DlgPrepare();
    virtual void setup(QDomNode node);
    virtual void onSearchStarting();
    virtual void onSearchCleared();
    virtual void onSearch(const QString& text);
    virtual void onShow();
    virtual QWidget* getWidgetForMIDIControl();
    public slots:
    void tableSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void selectAll();
    void analyze();
    void trackAnalysisFinished(TrackPointer tio);
    void trackAnalysisProgress(TrackPointer tio, int progress);
    void showRecentSongs();
    void showAllSongs();
    void installEventFilter(QObject* pFilter);
  signals:
    void loadTrack(TrackPointer pTrack);
    void loadTrackToPlayer(TrackPointer pTrack, QString player);

  private:
    void stopAnalysis();

    //Note m_pTrackTablePlaceholder is defined in the .ui file
    ConfigObject<ConfigValue>* m_pConfig;
    TrackCollection* m_pTrackCollection;
    AnalyserQueue* m_pAnalyserQueue;
    QButtonGroup m_songsButtonGroup;
    WPrepareLibraryTableView* m_pPrepareLibraryTableView;
    PrepareLibraryTableModel* m_pPrepareLibraryTableModel;
    WPrepareCratesTableView* m_pPrepareCratesTableView;
    CrateView* m_pCrateView;
    QSqlTableModel* m_pCratesTableModel;
    QModelIndexList m_indexesBeingAnalyzed;
    int m_iOldBpmEnabled; /** Used to temporarily enable BPM detection in the prefs before we analyse */
};

#endif //DLGTRIAGE_H
