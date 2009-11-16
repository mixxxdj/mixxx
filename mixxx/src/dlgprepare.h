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
    public slots:
    void tableSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void selectAll(); 
    void analyze();
    void trackAnalysisFinished(TrackInfoObject* tio);
    void trackAnalysisProgress(TrackInfoObject* tio, int progress);
    void showRecentSongs();
    void showAllSongs();
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
    QSqlTableModel* m_pCratesTableModel;
    QModelIndexList m_indexesBeingAnalyzed;
};

#endif //DLGTRIAGE_H
